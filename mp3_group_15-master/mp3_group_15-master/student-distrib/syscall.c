#include "syscall.h"

/* Define a function pointer type so that our code is   */
/* easier to read! Defines a pointer to a function with */
/* return value of type int32_t and arbitrary args.     */
typedef int32_t ( *function )( );
typedef int32_t ( *read_function )( int32_t fd, void* buf, int32_t nbytes );
typedef int32_t ( *write_function )( int32_t fd, void* buf, int32_t nbytes );

/* Set curr_pid to -1 initially. Will be set in         */
/* execute, when we execute a new program!              */
int32_t curr_pid = -1;
int32_t active_pid;
int32_t prev_pid;
int32_t pid_array[MAX_NUM_FILES - 2] = { 0, 0, 0, 0, 0, 0};


#define SYSCALL_HEADER      \
    printf( "[SYSCALL %s] called!\n", __FUNCTION__ )


/*-------------------syscall_halt-----------------------*/
/* The halt system call terminates a process, returning */
/* the specified value to its parent process. The       */
/* system call handler itself is responsible for        */
/* expanding the 8-bit argument from BL into the 32-bit */
/* return value to the parent program's execute system  */
/* call. Do not return all 32 bits from EBX. This call  */
/* should never return to the caller.                   */
/* - Restore parent data                                */
/* - Restore parent paging                              */
/* - Close any relevant FDs                             */
/* - Jump to execute return                             */
/* Inputs: status       -> status of the task           */
/* Outputs: 0           -> on success of halt           */
/* Side Effects: halts the task specified               */
int32_t syscall_halt( uint8_t status )
{
    /* Parent EBP and ESP will be stored later in the   */
    /* function, stored within the PCB of the           */
    /* corresponding PID.                               */
    uint32_t parent_ebp;
    uint32_t parent_esp;

    /* Store the status of the halt. If the halt status */
    /* is 37, then the call indicates that an error     */
    /* occurred, and that we should return 256 to       */
    /* indicate the program died due to an exception.   */
    int close_status;
    close_status = status;
    if( close_status == HALT_ERROR )
    {
        close_status = HALT_ERROR_CODE;
    }

    /* Get the PCB for the current process. Use the PID */
    /* to identify the corresponding PCB.               */
    pcb_t* program_pcb = get_pcb( curr_pid );

    /* Regardless, set the PID in the PID Array to be   */
    /* free, since the Process will be quashed either   */
    /* way.                                             */
    pid_array[ curr_pid ] = PID_FREE;

    /* Iterate through the file array of the process    */
    /* and set all the files to closed (flags = 0 )     */
    close_all_files( );

    /* Also, set the PID being serviced to the PID of   */
    /* the previous process, since we aim to halt this  */
    /* process and want to return to the previous one.  */
    prev_pid = curr_pid;
    curr_pid = program_pcb->parent_id;  
    
    /* Parent process is NOT "shell", reset the PCB's   */
    /* fields and get the needed parent information to  */
    /* return to the parent process. Update the current */
    /* PID to be the parent's PID, get the EBP and ESP  */
    /* information, set the PCB's PID and Parent PID to */
    /* -1, and mark the PCB's process as inactive.      */
    parent_ebp = program_pcb->saved_ebp;
    parent_esp = program_pcb->saved_esp;
    program_pcb->saved_ebp = 0;
    program_pcb->saved_esp = 0;
    program_pcb->parent_id = -1;
    program_pcb->pid = -1;
    program_pcb->active = 0;

    /* Reset printf coordinates to be consistent w terminal's. Since    */
    /* we may be returning from a halt we want to print onto the next   */
    /* line as a means of making the terminal look cleaner. Update      */
    /* screen_x/y and determine if we want to add a newline.            */
    if( terminal_x[ display_terminal ] != 0 )
    {
        keyboard_putc( '\n' );
        reset_keyboard_buffer( );
    }

    /* If the previous PID was -1, then run the program */
    /* "shell", since we always want to have at least   */
    /* one program running at all times.                */
    if( curr_pid == -1 )
    {
        /* Need to cast to uint8_t* type to properly    */
        /* use as input to syscall_execute (basically   */
        /* cast to string pointer). Set the PID to FREE */
        /* and execute "shell" again.                   */
        syscall_execute( (uint8_t*)"shell" );
    } 

    /* Remap the User Page to be updated with the       */
    /* parent's information and process.                */
    map_prog_to_page( curr_pid );
    
    /* Update the Task Switch Segment (TSS) with updated SS0 and ESP0.  */
    /* SS0 is the Segment Selector used to load the stack from a lower  */
    /* privilege level to a higher one.                                 */
    /* ESP0 is the Stack Pointer used to load the stack from a lower    */
    /* privilege level to a higher one.                                 */
    /* Refer to: https://wiki.osdev.org/Task_State_Segment for more     */
    /* details on the TSS.                                              */
    /* Update the TSS to load in the parent task.                       */
    tss.ss0 = KERNEL_DS;
    /* 8 MB - 8KB * curr_pid, -4 for safety.                            */
    tss.esp0 = EIGHT_MB - EIGHT_KB * curr_pid;

    /* Jump to the parent process, resetting the stack  */
    /* and base pointer registers as well as calling    */
    /* syscall_execute.                                 */
    asm volatile(   "movl     %2, %%eax;"
                    "movl     %0, %%esp;"
                    "movl     %1, %%ebp;"
                    "leave;"
                    "ret;"
                    : 
                    : "r" ( parent_esp ), "r" ( parent_ebp ), "r" ( close_status )
                ); 

    return 0;
}

/*-------------------syscall_execute--------------------*/
/* The execute system call attempts to load and execute */
/* a new program, handing off the processor to the new  */
/* program until it terminates.                         */
/* - Parse args                                         */
/* - Executable check                                   */
/* - Set up progam paging                               */
/* - User-level Program Loader                          */
/* - Create PCB                                         */
/* - Context Switch                                     */
/*      - Create its own context switch stack           */
/*      - IRET                                          */
/* Inputs: command      -> space separated sequence of  */
/*                      words.                          */
/*                      The first word is the file name */
/*                      of the program to be executed.  */
/*                      The rest of the command         */
/*                      (stripped of leading spaces)    */
/*                      should be provided to the new   */
/*                      the new program on request via  */
/*                      the getargs system call.        */
/* Outputs: -1          -> if the command cannot be     */
/*                      executed; for example if the    */
/*                      program does not exist or the   */
/*                      filename specified is not an    */
/*                      executable.                     */
/*          256         -> if the program dies by an    */
/*                      exception                       */
/*          0 to 255    -> if the program executes a    */
/*                      halt system call, in which the  */
/*                      value returned is that given by */
/*                      the program's call to halt.     */
/* Side Effects: Hands off the processor to another     */
/*               program until it finishes executing.   */
int32_t syscall_execute( const uint8_t* command )
{

    int i;
    /* Reset printf coordinates to be consistent w terminal's. Since    */
    /* we may be returning from a halt we want to print onto the next   */
    /* line as a means of making the terminal look cleaner. Update      */
    /* screen_x/y and determine if we want to add a newline.            */
    if( terminal_x[ display_terminal ] != 0 )
    {
        keyboard_putc( '\n' );
        reset_keyboard_buffer( );
    }
    screen_x = terminal_x[ display_terminal ];
    screen_y = terminal_y[ display_terminal ];

    /* Store the ESP and the EBP so that we can return to it later on   */
    /* program halt. We'll do this sooner than later in case the stack  */
    /* gets screwy for some weird reason.                               */
    uint32_t parent_esp;
    uint32_t parent_ebp;
    asm volatile( "movl     %%esp, %[parent_esp];"
                  "movl     %%ebp, %[parent_ebp];"
                  : /* Output operands. C variables that the asm code   */
                    /* will output into.                                */
                    [parent_esp] "=m" (parent_esp), 
                    [parent_ebp] "=m" (parent_ebp)
                  : /* Input Operands. C variables taht the asm code    */
                    /* will use as inputs. No input operands used.      */
                  : /* Clobbers and Special Registers. Indicators that  */
                    /* the compiler should use special register or that */
                    /* certain components get clobbered. "Memory" gets  */
                    /* clobbered here.                                  */
                    "memory"
                ); 


    /* ------------------ SETUP AND INPUT VALIDATION ------------------ */
    /* Check if we are even allowed to open any new programs. The max   */
    /* number of programs allowed is 5.                                 */
    if( curr_pid >= MAX_NUM_PROGS )
    {
        // printf( "\nToo many programs are being run! Aborting execute...\n" );
        return FAILURE;
    }
    /* Check if command is NULL. If so, return failure since the call   */
    /* was not set up properly.                                         */
    if ( command == NULL )
    {
        // printf( "\nNULL Command! Aborting execute...\n" );    
        return FAILURE;
    }
    /* Check if the only thing entered in the command is '\0', or NULL. */
    /* If so, return failure since call was not set up properly.        */
    if ( command == '\0' )
    { 
        // printf( "\nEmpty Command! Aborting execute...\n" );
        return FAILURE;
    }
    /* Check if the command is too large. If so, return failure since   */
    /* the command was not passed in properly.                          */
    if ( strlen( (int8_t*)command ) > BUFFER_SIZE )
    {
        // printf( "\nCommand too long! Aborting execute...\n" );
        return FAILURE;
    }
    /* Load the file name and arguments into the declared arrays.       */
    if( !get_fname( command ) )
    {
        // printf( "\nFilename exceeds allowed size! Aborting execute...\n" );
        return FAILURE;
    }

    /* Declare a directory entry so that we can find the file that we   */
    /* are attempting to execute. Find the dentry by name, and store    */
    /* its data so that we can read for magic numbers and EIP. Also     */
    /* declare a read flag so we can read the status of the read.       */
    dentry_t dentry;
    int read_flag;
    uint8_t buf[ SIZE_DATA_BLOCK ];

    /* read_dentry_by_name loads the directory entry's address pointer  */
    /* into dentry. We dereference it to get its corresponding          */
    /* information.                                                     */
    read_flag = read_dentry_by_name( (uint8_t*)file_name, &dentry );
    if( read_flag == FAILURE )
    {
        return FAILURE;
    }

    /* Next, read the actual data from our directory entry. Store the   */
    /* data into our buffer. We don't need to read the entire block,    */
    /* just enough to get the information we need, which is the magic   */
    /* numbers of our file for file validation and our EIP.             */
    read_flag = read_data( dentry.index_node_num, 0, buf, 40 ); 
    if( ( buf[ 0 ] != MAGIC_NUM_0 ) || 
        ( buf[ 1 ] != MAGIC_NUM_1 ) ||
        ( buf[ 2 ] != MAGIC_NUM_2 ) ||
        ( buf[ 3 ] != MAGIC_NUM_3 )   )
      {
        return FAILURE;
      }
    
    /* Get a new PID for the new process. Loop through the PID array    */
    /* since our programs won't necessarily be executed and halted in   */
    /* order, as they all have different runtimes.                      */
    prev_pid = curr_pid;
    for( i = 0; i < 6; i++ )
    {
        if( pid_array[ i ] == PID_FREE )
        {
            pid_array[ i ] = 1;
            curr_pid = i;
            pid_array[ i ] = PID_IN_USE;
            break;
        }
    }
    /* If no PIDs are free, return FAILURE. */
    if( i > MAX_NUM_PROGS )
    {
        return FAILURE;
    }

    terminals[sched_terminal].pid = curr_pid;

    /* Get the PCB (Process Control Block) of the current process,  */
    /* which will hold all the relevant information to our process. */
    pcb_t* new_pcb = get_pcb( curr_pid );

    /* First clear the saved_command buffer */
    memset(new_pcb->saved_command, '\0', sizeof(new_pcb->saved_command));

    /* Copy the command into our PCB so that we can recall it later */
    /* when we try to call syscall_getargs.                         */
    strcpy( (int8_t*)new_pcb->saved_command, (int8_t*)command );

    /* We also need the EIP of our new process, which is given in   */
    /* the inode that we read earlier. That data is currenly stored */
    /* in the buffer. Bytes 27-24 of the inode (stored in the       */
    /* buffer) are defined to be the EIP of the program. We need to */
    /* align and concatenate the buffer.                            */
    int eip;
    eip = 0;
    for( i = 0; i < NUM_EIP_BYTES; i++ )
    {
        /* Shift each entry of eip_buf by its byte # and OR w EIP to combine. */
        eip |= buf[ EIP_BYTE_OFFSET + i ] << ( BYTE_SIZE * i );
    }

    /* Set up new page. Set the entries as appropriate. Also, set   */
    /* the virtual address according to the PID.                    */
    map_prog_to_page( curr_pid );

    /* Load file into memory. Get the File Size, and use read_data  */
    /* to read the entire file into the program image address as    */
    /* outlined by the documentation of MP3 (discussion).           */
    uint32_t file_size = get_file_size( dentry.index_node_num );
    read_flag = read_data( (uint32_t)dentry.index_node_num, (uint32_t)0, (uint8_t*)PROG_IMG_START, (uint32_t)file_size );

    /* Fill the PCB entries so that we can save the data for our program.   */
    /* Keep track of the parent's PID so that we can return to the parent   */
    /* program, store the current program's PID, in addition to the state   */
    /* of the EBP and ESP so that we can restore the stack later on. Set    */
    /* active to 1 to indicate the process is in use, and set the first two */
    /* files of the pcb to be STDIN and STDOUT, which involve the terminal  */
    /* driver. Additionally, set the rest of the file flags in the file     */
    /* array of our PCB to 0 so that we can indiate they're not in use.     */

    if( curr_pid < 3 )
    {
        prev_pid = -1;
    }

    new_pcb->parent_id = prev_pid;
    new_pcb->pid = curr_pid;
    new_pcb->saved_ebp = parent_ebp;
    new_pcb->saved_esp = parent_esp;
    new_pcb->active = 1;

    /* First file is STDIN, whose table is just terminal's with WRITE set   */
    /* to NULL. Second file is STDOUT, whose table is just temrinal with    */
    /* READ set to NULL. Set the rest of the flags as not in use/available. */
    new_pcb->fd_array[ 0 ].fops_ptr = get_terminal_table( );
    new_pcb->fd_array[ 0 ].index_node_num = -1;
    new_pcb->fd_array[ 0 ].file_position = 0;
    new_pcb->fd_array[ 0 ].flags = 1;
    new_pcb->filetype_array[ 0 ] = 3;
    new_pcb->fd_array[ 1 ].fops_ptr = get_terminal_table( );
    new_pcb->fd_array[ 1 ].index_node_num = -1;
    new_pcb->fd_array[ 1 ].file_position = 0;
    new_pcb->fd_array[ 1 ].flags = 1;
    new_pcb->filetype_array[ 1 ] = 3;

    new_pcb->fd_array[ 2 ].flags = 0;
    new_pcb->fd_array[ 3 ].flags = 0;
    new_pcb->fd_array[ 4 ].flags = 0;
    new_pcb->fd_array[ 5 ].flags = 0;
    new_pcb->fd_array[ 6 ].flags = 0;
    new_pcb->fd_array[ 7 ].flags = 0;

    new_pcb->esp0 = tss.esp0; 
    new_pcb->ss0 = tss.ss0;   
         
    /* Update the Task Switch Segment (TSS) with updated SS0 and ESP0.  */
    /* SS0 is the Segment Selector used to load the stack from a lower  */
    /* privilege level to a higher one.                                 */
    /* ESP0 is the Stack Pointer used to load the stack from a lower    */
    /* privilege level to a higher one.                                 */
    /* Refer to: https://wiki.osdev.org/Task_State_Segment for more     */
    /* details on the TSS.                                              */
    tss.ss0 = KERNEL_DS;
    /* 8 MB - 8KB * curr_pid, -4 for safety.                            */
    tss.esp0 = EIGHT_MB - EIGHT_KB * curr_pid - 4; 

    /* Load the return address ( given as a label ) into our PCB, so    */
    /* that we can return to the appropriate place later.               */
    asm volatile (
				"leal HALT_RET, %%eax		\n\
				 movl %%eax, %[PCB_RETURN_ADDR]"
				:[PCB_RETURN_ADDR] "=m" (new_pcb->parent_return_addr)
				:
				: "eax", "memory"
				);    

    /* Set up the kernel stack by pushing the USER_DS ( User Data Seg ) */
    /* the BOTTOM of the memory, the EFLAGS ( with interrupts enabled ) */
    /* the USER_CS ( User Code Seg ), and EIP ( Insruction Pointer ).   */
    asm volatile(   "pushl  %[USR_DS];"
                    "pushl  %[BOT];"
                    "pushfl;"
                    "popl   %%ebx;"
                    "orl    %[IF_EN], %%ebx;"
                    "pushl  %%ebx;"
                    "pushl  %[USR_CS];"
                    "pushl  %[ip];"
                    : /* No Output Operands */
                    :   [USR_DS] "i" (USER_DS),
                        [BOT] "i" (BOTTOM),
                        [IF_EN] "i" (IF_ENABLE),
                        [USR_CS] "i" (USER_CS),
                        [ip] "r" (eip)
                    : "ebx"
                );

    /* Use "IRET" to effectively "execute" our new program. Will be */
    /* returned to on HALT.                                         */
    asm volatile( "iret" );

    /* HALT_RET label for return on HALT.                           */
    asm volatile( "HALT_RET:" );

    return 0;
}

/*-------------------syscall_read-----------------------*/
/* Reads data tfrom keyboard, a file, device (RTC), or  */
/* directory. This call returns the number of bytes     */
/* read. Use the file operations jump table to call the */
/* corresponding read or write function!                */
/* - Use file operations jump table to call the         */
/*   corresponding read or write function.              */
/* Inputs: fd           -> file descriptor              */
/*         buf          -> buffer of data               */
/*         nbytes       -> number of bytes to read      */
/* Outputs: This call should return the number of bytes */
/*          read. Otherwise, the return varies per file */
/*          type read.                                  */
/*          Terminal    -> returns data from one line   */
/*                      that has been terminated by     */
/*                      pressing enter. The data will   */
/*                      be returned in buf.             */
/*          File        -> Data should be read to the   */
/*                      end of the file or the end of   */
/*                      the buffer provided.            */
/*          Directory   -> Only the filename should be  */
/*                      provided, and subsequent reads  */
/*                      should read from successive     */
/*                      directory entries until the     */
/*                      last is reached, at which read  */
/*                      should repeatedly return 0.     */
/*          RTC         -> Should always return 0, but  */
/*                      only after an interrupt has     */
/*                      occurred.                       */
/* You should use a jump table referenced by the task's */
/* file array to call from a generic handler for this   */
/* call into a file-type-specific functio. This jump    */
/* table should be inserted into the file array on the  */
/* open system call.                                    */
/* Side Effects: Executes the corresponding read        */
/* function.                                            */
int32_t syscall_read( int32_t fd, void* buf, int32_t nbytes )
{
    /* Get the PCB for the current process. Use the PID */
    /* to identify the corresponding PCB.               */
    pcb_t* program_pcb = get_pcb( curr_pid );
    
    /* Check that the file descriptor is valid. If not, */
    /* then return -1.                                  */
    if( fd > FD_MAX_VAL || fd < 0 || fd == 1 )
    {
        return FAILURE;
    }

    /* Also check if the buffer (buf) is valid, since   */
    /* we will be filling it up with the data read.     */
    if( buf == NULL )
    {
        return FAILURE;
    }

    /* May also want to check flags of the file         */
    /* to make sure that we are actually able to read.  */
    if( program_pcb->fd_array[ fd ].flags == 0 )
    {
        return FAILURE;
    }

    /* Synchronize the file_system global file array    */
    /* with the PCB's local file array so that we don't */
    /* get issues with opening and reading files or     */
    /* writing files.                                   */
    int i;
    for( i = 0; i < MAX_NUM_FILES; i++ )
    {
        file_array[ i ].fops_ptr = program_pcb->fd_array[ i ].fops_ptr;
        file_array[ i ].index_node_num = program_pcb->fd_array[ i ].index_node_num;
        file_array[ i ].file_position = program_pcb->fd_array[ i ].file_position;
        file_array[ i ].flags = program_pcb->fd_array[ i ].flags;
    }

    /* Call the corresponding function based on the     */
    /* file type. Use its return value as the return    */
    /* value for this function.                         */
    int filetype;
    filetype = program_pcb->filetype_array[ fd ];
    switch( filetype )
    {
        /* Case 0: RTC Type */
        case RTC_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_RTC_table( );
            break;

        /* Case 1: Directory Type */
        case DIRECTORY_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_dir_table( );
            break;

        /* Case 2: Regular File Type */
        case REG_FILE_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_file_table( );
            break;

        /* Case 3: Terminal Type*/
        case TERMINAL_FILE_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_terminal_table( );
            break;

        /* If program type does not match any of these, */
        /* then an error occurred. Return FAILURE.      */
        default:
            return FAILURE;
    } 
    
    function func_read = (void*)program_pcb->fd_array[ fd ].fops_ptr->read;
    /* Read result returns the # of bytes read.         */
    int read_result = (func_read)( fd, buf, nbytes );
    /* However, since the pcb and filesys file entries  */
    /* are separate we need to update the pcb's file    */
    /* entries with the filesys's file entries.         */
    for( i = 0; i < MAX_NUM_FILES; i++ )
    {
        program_pcb->fd_array[ i ].fops_ptr = file_array[ i ].fops_ptr;
        program_pcb->fd_array[ i ].index_node_num = file_array[ i ].index_node_num;
        program_pcb->fd_array[ i ].file_position = file_array[ i ].file_position;
        program_pcb->fd_array[ i ].flags = file_array[ i ].flags; 
    }
    
    return read_result;
}

/*-------------------syscall_write----------------------*/
/* The write system call writes data to the terminal or */
/* to a device (RTC). Writes to the terminal should     */
/* display to the screen immediately. Writes to the RTC */
/* should always accept only a 4-byte integer           */
/* specifying the interrupt rate in Hz, and should set  */
/* the rate of periodic interrupts accordingly. Writes  */
/* to files should always return -1 to indicate failure */
/* since the file system is read-only. The call returns */
/* the number of bytes written, or -1 on failure.       */
/* - Use file operations jump table to call the         */
/*   corresponding read or write function.              */
/* Inputs: fd           -> file descriptor              */
/*         buf          -> buffer of data               */
/*         nbytes       -> number of bytes to write     */
/* Outputs: Terminal    -> number of bytes written      */
/*          RTC         -> 0 if success, -1 if failure  */
/*          File        -> -1, since filesystem is read */
/*                      only.                           */
/*          Overall, return 0 on success and -1 on      */
/*          failure, or number of bytes written for     */
/*          terminal.                                   */
/* Side Effects: Writes text to the terminal and        */
/*               displays onscreen, sets interrupt      */
/*               rate, or does nothing.                 */
int32_t syscall_write( int32_t fd, const void* buf, int32_t nbytes )
{
    /* Get the PCB for the current process. Use the PID */
    /* to identify the corresponding PCB.               */
    pcb_t* program_pcb = get_pcb( curr_pid );
    
    /* Check that the file descriptor is valid. If not, */
    /* then return -1.                                  */
    if( fd > FD_MAX_VAL || fd < 0 || fd == 0 )
    {
        return FAILURE;
    }

    /* Also check if the buffer (buf) is valid, since   */
    /* we will be filling it up with the data read.     */
    if( buf == NULL )
    {
        return FAILURE;
    }

    /* May also want to check flags of the file         */
    /* to make sure that we are actually able to read.  */
    if( program_pcb->fd_array[ fd ].flags == 0 )
    {
        return FAILURE;
    }

    /* Synchronize the file_system global file array    */
    /* with the PCB's local file array so that we don't */
    /* get issues with opening and reading files or     */
    /* writing files.                                   */
    int i;
    for( i = 0; i < MAX_NUM_FILES; i++ )
    {
        file_array[ i ].fops_ptr = program_pcb->fd_array[ i ].fops_ptr;
        file_array[ i ].index_node_num = program_pcb->fd_array[ i ].index_node_num;
        file_array[ i ].file_position = program_pcb->fd_array[ i ].file_position;
        file_array[ i ].flags = program_pcb->fd_array[ i ].flags;
    }

    /* Call the corresponding function based on the     */
    /* file type. Use its return value as the return    */
    /* value for this function. Due to the fops         */
    /* implementation, we need to reset the fops table  */
    /* every time we read and write, because those      */
    /* functions are unique to each file type.          */
    int filetype;
    filetype = program_pcb->filetype_array[ fd ];
    switch( filetype )
    {
        /* Case 0: RTC Type */
        case RTC_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_RTC_table( );
            break;
        /* Case 1: Directory Type */
        case DIRECTORY_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_dir_table( );
            break;
        /* Case 2: Regular File Type */
        case REG_FILE_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_file_table( );
            break;
        /* Case 3: Terminal File Type */
        case TERMINAL_FILE_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_terminal_table( );
            break;
    }   

    function func_write = (void*)program_pcb->fd_array[ fd ].fops_ptr->write;
    /* Read result returns the # of bytes read.         */
    int read_result = (func_write)( fd, buf, nbytes );
    /* However, since the pcb and filesys file entries  */
    /* are separate we need to update the pcb's file    */
    /* entries with the filesys's file entries.         */
    for( i = 0; i < MAX_NUM_FILES; i++ )
    {
        program_pcb->fd_array[ i ].fops_ptr = file_array[ i ].fops_ptr;
        program_pcb->fd_array[ i ].index_node_num = file_array[ i ].index_node_num;
        program_pcb->fd_array[ i ].file_position = file_array[ i ].file_position;
        program_pcb->fd_array[ i ].flags = file_array[ i ].flags; 
    }
    return read_result; 
}

/*-------------------syscall_open-----------------------*/
/* The open system call provides access to the file     */
/* system. The call should find the directory entry     */
/* corresponding to the named file, allocate an unused  */
/* file descriptor, and set up any data necessary to    */
/* handle the given type of file (directory, RTC device */
/* or regular file). If the named files does not exist  */
/* or no descriptors are free, the call returns -1.     */
/* - Find the file in the file system and assign an     */
/*   unused file descriptor                             */
/* - File descriptors need to be set up according to    */
/*   the file type                                      */
/* Inputs: filename     -> named file we want to find.  */
/* Outputs: 0 on success, -1 if failure (named file     */
/*          does not exist or not descriptors are free. */
/* Side Effects: Provides access to file system.        */
int32_t syscall_open( const uint8_t* filename )
{
    /* First, check if filename is valid. If not valid, */
    /* return -1 for failure.                           */
    if( filename == NULL )
    {
        return FAILURE;
    }

    /* See if we can find the directory entry. If so,   */
    /* then store it into an instance of dentry_t.      */
    /* read_dentry_by_name returns 1 if it fails, and 0 */
    /* if it passes, while passing the dentry instance  */
    /* to the second argument.                          */
    dentry_t dentry;
    int dentry_pass = read_dentry_by_name( filename, &dentry );
    if( dentry_pass == FAILURE )
    {
        return FAILURE;
    }

    /* Next, get the pCB corresponding to the current   */
    /* process and figure out if we can open up any     */
    /* more files by checking if the file array is full */
    pcb_t* program_pcb = get_pcb( curr_pid );

    /* Iterate through PCB's file array and check flags */
    /* of files. If we find a flag that is 0 before we  */
    /* reach the end of the array, then we can replace  */
    /* that file with the file we are attempting to     */
    /* open. If we reach the end of the array, then the */
    /* maximum number of files has been reached, and    */
    /* return failure.                                  */
    int fd;
    int file_flag;
    for( fd = 0; fd < FILE_ARRAY_SIZE; fd++ )
    {
        file_flag = program_pcb->fd_array[ fd ].flags;
        if( file_flag == FD_FREE )
        {
            break;
        }
    }

    /* If we managed to iterate through the array but   */
    /* found only open files, then the array is full,   */
    /* and return FAIL. Else, fd holds the current      */
    /* index of the next open spot in the file array.   */
    if( fd == FILE_ARRAY_SIZE )
    {
        return FAILURE;
    }

    /* Finally, based off of file type, we open the     */
    /* file using the corresponding open function. Set  */
    /* the entries of pcb file array entry based on     */
    /* such.                                            */
    switch( dentry.file_type )
    {
        /* File type is RTC. Initialize pcb file array  */
        /* entry as such.                               */
        case RTC_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_RTC_table( );
            break;

        /* File type is Directory. Initialize pcb file  */
        /* array entry as such.                         */
        case DIRECTORY_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_dir_table( );
            break;
            
        /* File type is regular file type. Initialize   */
        /* pcb file array entry as such.                */
        case REG_FILE_TYPE:
            program_pcb->fd_array[ fd ].fops_ptr = get_file_table( );
            break;

        /* File type not recognized, return failure.    */
        default:
            return FAILURE;
    }

    /* Set the common entries between the files as      */
    /* needed.                                          */
    program_pcb->filetype_array[ fd ] = dentry.file_type;
    program_pcb->fd_array[ fd ].index_node_num = dentry.index_node_num;
    program_pcb->fd_array[ fd ].file_position = 0;
    program_pcb->fd_array[ fd ].flags = 1;

    /* Also run the associated open function with the   */
    /* given file type and f_ops pointer.               */
    function func_sys_open = (void*)program_pcb->fd_array[ fd ].fops_ptr->open;
    uint32_t open_status = (func_sys_open)( filename );
    if( open_status < 0 )
    {
        return FAILURE;
    }
    return ( fd );
}

/*-------------------syscall_close----------------------*/
/* The close system call closes the specified file      */
/* descriptor and makes it available for return from    */
/* later calls to open. The user should not be able to  */
/* close the default descriptors (0 for input and 1 for */
/* output). Trying to close an invalid descriptor       */
/* should result in a return value of -1; successful    */
/* closes should return 0.                              */
/* - Close the file descriptor passed in (set it to be  */
/*   available                                          */
/* - Check for invalid descriptors                      */
/* Inputs: fd       -> File Descriptor                  */
/* Outputs: 0 if successful close, -1 if invalid.       */
/* Side Effects: Closes the descriptor and makes it     */
/* available for return from later calls to open.       */
int32_t syscall_close( int32_t fd )
{
    /* Get the PCB for the current process. Use the PID */
    /* to identify the corresponding PCB.               */
    pcb_t* program_pcb = get_pcb( curr_pid );

    /* Check if fd is valid. If not, then we failed to  */
    /* close specified file descriptor. We cannot close */
    /* the first two fds (stdin and stdout), thus we    */
    /* need to check for the minimum value also.        */
    if( fd < FD_MIN_VAL || fd > FD_MAX_VAL || fd == NULL )
    {
        return FAILURE;
    }

    /* Also check if flag tells us that the fd is       */
    /* invalid or closed.                               */
    if( program_pcb->fd_array[ fd ].flags == 0 )
    {
        return FAILURE;
    }

    /* Both checks passed, close the file by resetting  */
    /* the file descriptor's elements to zero.          */
    program_pcb->fd_array[ fd ].fops_ptr = NULL;
    program_pcb->fd_array[ fd ].index_node_num = 0;
    program_pcb->fd_array[ fd ].file_position = 0;
    program_pcb->fd_array[ fd ].flags = 0;
    program_pcb->filetype_array[ fd ] = 0;
    
    return 0;    
}

/*-------------------syscall_getargs----------------------*/
/* the syscall_getargs reads the programs command line    */
/* arguments and puts them into a user-level buffer       */
/* we can find these arguments since they are stored in   */
/* the task data when a program is loaded                 */
/* Inputs: buf -> buffer that we want to copy the args to */
/*         nbytes -> number pf bytes we want to copy      */
/* Outputs: -1 if invalid buf or argument else 0          */
/* Side effects: Copies the args to user space            */
int32_t syscall_getargs( uint8_t* buf, int32_t nbytes )
{
    /* Should initilize the shells task argument data to a empty string */

    /* If the buffer given is invalid return FAILURE */
    if (buf == NULL){
        return FAILURE;
    }

    /* If we try to copy 0 bytes of data just return FAILURE */
    if (nbytes == 0){
        return FAILURE;
    }

    /* Get the current pcb */
    pcb_t* curr_pcb = get_pcb(curr_pid);

    /* Make the a copy of the command in the pcb                */
    uint8_t curr_cmd[BUFFER_SIZE];
    strcpy((int8_t*)curr_cmd, (int8_t*)curr_pcb->saved_command);

    /* Now we need to pase the command. From piazza post #916   */
    /* we know that we have at most 1 argument in mp3 so we can */
    /* ignore any extra ones that we get                        */
    int i = 0;

    /* Ignore leading spaces */
    while (curr_cmd[i] == ' '){ 
        i++; 
    }
    /* Ignore the file name */
    while (curr_cmd[i] != ' ' && curr_cmd[i] != '\0'){ 
        i++; 
    }

    /* If there are no args */
    if (curr_cmd[i] == '\0'){
        return FAILURE;
    }

    /* Skip the leading spaces of arg */
    while(curr_cmd[i] == ' '){ 
        i++; 
    }

    /* Buffer for the output */
    uint8_t output_buf[BUFFER_SIZE];    
    /* Clear the output buffer in case there are random numbers floating    */
    /* around. Also clear the buffer we are going to return just in case.   */
    memset( output_buf, '\0', BUFFER_SIZE );

    /* Counter for copying to output_buf */
    int j = 0;                          
    /* Now we just have the arg to copy left */
    while (curr_cmd[i] != '\0'){
        output_buf[j] = curr_cmd[i];
        i++;
        j++;
    }
    /* If the command is larger than the number of bytes to return FAILURE*/ 
    if (strlen(((int8_t*)output_buf) + 1) > nbytes){
        return FAILURE;
    }

    /* Now we need to copy the string from the saved_command to the buffer          */
    /* First clear the buffer in case there are random numbers or previous commands */
    /* Potential bug, may want to clear the entire buffer instead of just nbytes    */
    memset(buf, '\0', nbytes);

    /* Now we want to copy from the task to the buf                                 */
    memcpy(buf, output_buf, nbytes);

    return 0; 
}   

/*-------------------- syscall_vidmap --------------------- */
/* Maps the text-mode video memory into user space at a     */
/* pre-set virtual address. Maps into a 4kB page.           */
int32_t syscall_vidmap( uint8_t** screen_start )
{
    /* Checks if the passed in screen_start is valid and is in the correct address range */
    if (screen_start == NULL) {
        return FAILURE;
    } else if ((uint32_t) screen_start >= FOUR_MB && (uint32_t) screen_start < EIGHT_MB) { // no sneaky kernel moves
        return FAILURE;
    }

    /* Sets the screen start virtual address */
    *screen_start = (uint8_t*)(VIRT_VID_MEM);

    /* Gets the corresponding page directory index by getting the top 10 bits of the screen start address */
    /* since there are 1024 total page directory entries */
    uint32_t PDE_index = VIRT_VID_MEM >> 22;

    /* Sets the necessary page directory parameters */
    page_directory[PDE_index].present = 1;
    page_directory[PDE_index].read_write = 1;
    page_directory[PDE_index].user_supervisor = 1;
    page_directory[PDE_index].page_size = 0;
    page_directory[PDE_index].virtual_address = ((int)(vid_page_table)) / FOUR_KB;

    /* Sets the necessary video page table parameter */
    vid_page_table[0].present = 1;
    vid_page_table[0].read_write = 1;
    vid_page_table[0].user_supervisor = 1;
    vid_page_table[0].virtual_address = VIDEO_MEM_START_ADDR / FOUR_KB; // 0xB8000

    /* Flushes the TLB */
    flush_tlb();

    return 0;
}

/*---------------- syscall_set_handler----------------------*/
int32_t syscall_set_handler( int32_t signum, void* handler_address )
{
    return FAILURE;    
}

/*-------------------syscall_sigreturn----------------------*/
int32_t syscall_sigreturn( void )
{
    return FAILURE;    
}


/* ----------------- HELPER FUNCTIONS --------------------- */
/* ----------------- map_prog_to_page --------------------- */
/* Maps the program to a page in the page table. Maps the   */
/* user to page 32, defined to be the user page.            */
void map_prog_to_page( int32_t pid )
{
    /* Set up new page. Set the entries as appropriate. Also, set   */
    /* the virtual address according to the PID.                    */
    page_directory[ USER_PAGE ].present         = 1;
    page_directory[ USER_PAGE ].read_write      = 1;
    page_directory[ USER_PAGE ].user_supervisor = 1;
    page_directory[ USER_PAGE ].write_through   = 0;
    page_directory[ USER_PAGE ].cache_disable   = 0;
    page_directory[ USER_PAGE ].accessed        = 0;
    page_directory[ USER_PAGE ].available_1     = 0;
    page_directory[ USER_PAGE ].page_size       = 1;
    page_directory[ USER_PAGE ].global          = 1;
    page_directory[ USER_PAGE ].available_3     = 0;
    page_directory[ USER_PAGE ].virtual_address = ( (uint32_t)( EIGHT_MB + ( pid * FOUR_MB ) ) ) >> 12;

    /* Flush the TLB since a new page has been set and old entries  */
    /* are not irrelevant.                                          */
    flush_tlb( );
}

/* ------------------ get_fname ----------------------- */
/* Helper function for syscall_execute to get the       */
/* filename of the associated command. Parses the       */
/* command for the filename and store into the array    */
/* file_name declared in the header.                    */
/* Inputs: const uint8_t* command -> Pointer to the     */
/*              command string.                         */
/* Outputs: 0 -> failure. The filename exceeds the      */
/*      maximum allowed length.                         */
/*          1 -> success. The filename has been read    */
/*      and placed into the buffer.                     */
/* Side Effects: resets the file_name buffer/array      */
/*      and gets the command's filename.                */
int get_fname( const uint8_t* command ) {
    int i = 0;
    int j = 0; 

    /* Reset the file name buffer. Technically a redundancy.        */
    for( j = 0; j < MAX_FILE_NAME_LENGTH; j++ )
    {
        /* Set the current entry to NULL or EMPTY. */
        file_name[ j ] = '\0';
    }

    /* Skip all of the leading spaces */
    while(command[i] == ' ')
    { 
        i++; 
    }

    /* Now we are at the start of the file name. Store all of the   */
    /* following characters until the next space as the file name.  */
    j = 0;                             
    while(command[i] != ' ' && command[i] != '\0') {              
        /* Check if the file name length has exceeded the maximum   */
        /* allowed length. If so, return FAIL.                      */
        if( j >= MAX_FILE_NAME_LENGTH )
        {
            return 0;
        }
        file_name[j] = command[i];          
        i++;                                
        j++;
    }
    /* Set the last entry in the buffer to NULL so that we can      */
    /* indicate the end of the filename.                            */
    file_name[j] = '\0';
    return 1;
}


/* ------------------ get_args ------------------------ */
/* Parses the command and gets the args associated with */
/* the command. Stores the args in the array cmd_args.  */
/* Inputs: const uint8_t* command -> Pointer to the     */
/*              command string.                         */
/* Outputs: 0 -> failure. The combined length of args   */
/*      exceeds the maximum allowed length.             */
/*          1 -> success. The args have been read and   */
/*      placed into the buffer.                         */
/* Side Effects: resets the command arguments buffer or */
/*      array, and gets the command's arguments.        */
int get_args( const uint8_t* command ) {
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    int arg_length = 0;
    int arg_num = 0;
    cmd_args_length = 0;

    /* Reset the argument buffer. Technically a redundancy. */
    for( k = 0; k < MAX_NUM_ARGS; k++ )
    {
        for( l = 0; l < MAX_FILE_NAME_LENGTH; l++ )
        {
            /* Set the current entry to NULL or EMPTY. */
            cmd_args[ k ][ l ] = '\0';
        }
    }

    /*Ignore leading spaces*/
    while(command[i] == ' ') { i++; }
    /*Ignore the file name*/
    while(command[i] != ' ') { i++; }

    /* Reads in arguments if there are any */
    if(command[i] == ' ') { 
        /* Start at the beginning of the first argument */

        /* Loop until end of command entry */
        while(command[i] != '\0') {
            /* Reads past any space characters to get to the actual argument */
            if (command[i] == ' ') {
                cmd_args[arg_num][j] = '\0'; //Null terminate each argument
                i++;
                arg_num++;
                j = 0;

                /* Once we see a space, disregard everything after that until we encounter a non-space character */
                while (command[i] != '\0' && command[i] == ' ') { i++; }
            } 
            /* Store argument into 2D array of arguments one character at a time */
            else {
                cmd_args[arg_num][j] = command[i];
                i++;
                j++;
                arg_length++;
                cmd_args_length++;
            }
        }
    }

    /* Check if the combined total length of the arguments exceeds  */
    /* the maximum allowed total length of arguments.               */
    if( arg_length > MAX_ARG_LEN )
    {
        return 0;
    }

    return 1;
}

/* ------------------ get_pcb ------------------------- */
/* Gets the PCB corresponding to the PID passed in.     */
/* Takes in the PID and maps it accordingly.            */
pcb_t* get_pcb(uint32_t pid) {
    return (pcb_t*)( EIGHT_MB - EIGHT_KB*( 1 + pid ) );
}

/* ------------------ close_all_files ----------------- */
/* Iterate through the file array of the process        */
/* and set all the files to closed (flags = 0 )         */
void close_all_files( void )
{
    int i;
    for( i = 0; i < MAX_NUM_FILES; i++ )
    {
       syscall_close( i );
    }
}


