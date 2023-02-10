#include "lib.h"
#include "keyboard.h"
#include "terminal.h"
#include "paging.h"
#include "scheduling.h"

uint8_t     terminal_buffer[ BUFFER_SIZE ];
uint32_t    read_ready;
uint32_t    terminal_vid_mem[ NUM_TERMINALS ][ TERMINAL_MEMORY_SIZE ];

/* Implemented as a part of the scheduler, initializes  */
/* the 3 terminal instances with intial bootup method   */
/* and sets the virtual memory locations of each        */
/* Inputs: None.                                        */
/* Outputs: None.                                       */

void terminals_init( void ){
    int i;
    for (i = 0; i < NUM_TERMINALS; i++ ) {
        /* Since there are no current processes running */
        terminals[i].num_processes = 0;
        terminals[i].initialized = 0;
        terminals[i].pid = -1;
        terminals[i].saved_esp = 0;
        terminals[i].saved_ebp = 0;
        /* Set the buffers to null just to be safe      */
        memset(terminals[i].terminal_buffer, '\0', BUFFER_SIZE);
    }

    sched_terminal = 2;
    display_terminal = 2;    
}



/*                 terminal_open                        */
/* Opens the specified file descriptor and provides     */
/* access to the filesystem. Since this is the terminal */
/* this will technically do nothing, and return -1,     */
/* since technically unsuccessful.                      */
/* Inputs: None.                                        */
/* Outputs: -1 if file pointer filename is NULL,        */
/* othrwise will return 0.                              */
/* Side Effects: None.                                  */
int32_t terminal_open( const uint8_t* filename )
{
    if( filename == NULL )
    {
        /* Nothing to initialize since all relevant devices */
        /* have already been initialized in keyboard_init   */
        return -1;
    }
    
    /* Filename is valid. Return 0. */
    return 0;

}



/*                  terminal_close                      */
/* Closes the specified file descriptor and makes       */
/* available for return from later calls to open.       */
/* Successful closes return 0, while trying to          */
/* close an invalid descriptor returns -1.              */
/* Inputs: fd -> File Descriptor. Technically, is not   */
/* valid since this is the terminal.                    */
/* Outputs: -1 if file descriptor not valid. 0 if close */
/* is successful.                                       */
/* Side Effects: Technically closes the file descriptor */
/* but in reality, does nothing.                        */
int32_t terminal_close( int32_t fd )
{
    /* Return -1 since this is the terminal (you        */
    /* shouldn't be able to close the terminal)         */
    return -1;
}



/*                     terminal_read                    */
/* Reads data from the keyboard. Read should return the */
/* data from one line by pressing Enter, or as much as  */
/* fits in the buffer from one such line. The line read */
/* SHOULD INCLUDE the line feed ('\n') character.       */
/* Inputs: fd -> File Descriptor. Unused in terminal    */
/*               driver.                                */
/*         buf -> buffer to be filled.                  */
/*         nbytes -> num of bytes to be read. Unused in */
/*                   terminal driver.                   */
/* Outputs: Num of bytes read from the keyboard.        */
/* Side Effects: Fills the Terminal Buffer with the     */
/* data read from the keyboard.                         */
int32_t terminal_read( int32_t fd, void* buf, int32_t nbytes )
{
    /* word_count and keyboard_buffer provided in       */
    /* keyboard.c. Both are kept track of, and can be   */
    /* used here to read to the terminal.               */


    /* Wait for the signal to read. Will use a global   */
    /* flag to do so, which will be updated by the      */
    /* keyboards driver. Set to 0 on start, but wait    */
    /* for other program to set.                        */
    read_ready = 0;

    /* Loop while ready flag not raised, or the "Enter" */
    /* key has not been pressed yet...                  */
    while( !read_ready ){ }

    /* Reset the read_ready signal in case we try to    */
    /* run terminal_read again.                         */
    read_ready = 0;

    /* Check if the buffer is NULL. If so, then return. */
    if( buf == NULL )
    {
        reset_keyboard_buffer( );
        return 0;
    }

    /* We also need to cast buf as a uint8_t type in    */
    /* order to function properly, as we cannot do      */
    /* anything with a void pointer.                    */
    uint8_t* read_buf = buf;

    /* Reset the terminal buffer so that we can make    */
    /* sure that commands don't get repeated/input      */
    /* doesn't get super weird or anything...           */
    int i;
    for( i = 0; i < BUFFER_SIZE; i++ )
    {
        read_buf[ i ] = 0;
    }

    /* Now copy the contents of the keyboard buffer     */
    /* into the terminal buffer. The buffer should      */
    /* include the line feed character at the end of    */
    /* the sequence of characters.                      */
    int count;
    count = 0;
    for( i = 0; i < BUFFER_SIZE; i++ )
    {
        /* We have reached the end of the keyboard buffer   */
        /* if we hit the character '\n'. Thus, we can       */
        /* keep looping until we get to '\n', in which case */
        /* we can break the loop and return the number of   */
        /* characters written.                              */
        read_buf[ i ] = keyboard_buffer[ display_terminal ][ i ];
        count++;
        if( read_buf[ i ] == '\n' )
        {
            break;
        }
    }

    /* Clear the keyboard buffer and word_count now that we */
    /* have taken the data passed through                   */
    reset_keyboard_buffer( );

    /* Since each character is one byte, we can just return the */
    /* number of characters written to the buffer!              */
    return count;
}



/*                   terminal_write                     */
/* Writes the terminal buffer to the terminal. All data */
/* should be displayed on the screen immediately.       */
/* Returns the number of bytes written, or -1 on        */
/* failure.                                             */
/* Inputs: None.                                        */
/* Outputs: Number of bytes written, or -1 on failure.  */
/* Side Effects: Prints the contents of the terminal    */
/* buffer to the screen.                                */
int32_t terminal_write( int32_t fd, const void* buf, int32_t nbytes )
{
    /* Check if the terminal buffer has contents in it. If not, */
    /* then return failure.                                     */
    if( buf == NULL )
    {
        return -1;
    }
    if( nbytes == NULL )
    {
        return -1;
    }
    if( nbytes == 0 )
    {
        return 0;
    }

    /* We also need to cast buf as a uint8_t type in    */
    /* order to function properly, as we cannot do      */
    /* anything with a void pointer.                    */
    const uint8_t* write_buf = buf;

    /* We want to print the number of bytes specified in the    */
    /* arguments passed in. If the number of bytes EXCEEDS the  */
    /* number of characters in the buffer, then the terminal    */
    /* will only display up to the buffer's last charcter. If   */
    /* the number of bytes is less than the number of           */
    /* characters in the buffer, then the function will only    */
    /* print out as many characters as specified by nbytes.     */
    int i;
    uint8_t c;
    uint32_t num_bytes = 0;
    for( i = 0; i < nbytes; i++ )
    {
        c = write_buf[ i ];
        /* If character is not valid, then break.   */
        if( c == '\0' || c == 0 )
        {
            break;
        }
        keyboard_putc( c );
        reset_keyboard_buffer( );
        num_bytes++;
    }

    /* Because we used the put_string function, our keyboard    */
    /* buffer gets filled, thus we need to reset it in order    */
    /* for our functions to work properly again.                */
    reset_keyboard_buffer( );

    /* Return the number of bytes read.                         */
    return num_bytes;
}

void switch_terminal( uint32_t terminal_target_index )
{

    /* Check if the argument passed in is valid                 */
    if( terminal_target_index < 0 || terminal_target_index >= NUM_TERMINALS )
    {
        return;
    }

    if (display_terminal == terminal_target_index) {
        return;
    }

    
    /* Copy current physical video memory into an alternate     */
    /* location in virtual memory                               */
    memcpy((void*) ((VIDEO_ALT_START + display_terminal) * SCHED_FOUR_KB), (void*) VIDEO_MEM_LOC, SCHED_FOUR_KB);  

    /* Update the new display terminal */
    display_terminal = terminal_target_index;     

    /* Copy video memory from alternate virtual address to      */
    /* physical address to be displayed on screen               */
    memcpy((void*) VIDEO_MEM_LOC, (void*) ((VIDEO_ALT_START + display_terminal) * SCHED_FOUR_KB), SCHED_FOUR_KB);

    /* Print the cursor at the corresponding location.*/
    terminal_print_cursor( terminal_y[ display_terminal ], terminal_x[ display_terminal ] );    
}

