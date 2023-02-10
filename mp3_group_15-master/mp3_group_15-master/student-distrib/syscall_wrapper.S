#define ASM 1

/* For system calls, the arguments and pertinent information is passed  */
/* in the following format.                                             */
/* Call Number      -> EAX                                              */
/* First Argument   -> EBX                                              */
/* Second Argument  -> ECX                                              */
/* Third Argument   -> EDX                                              */
/* No call uses more than three arguments. The return value is placed   */
/*   in EAX if the call returns (not all do). A value of -1 indicates   */
/*   an error, while others indicate some form of success. Unless       */
/*   specified otherwise, successful calls should return 0, and failed  */
/*   calls should return -1.                                            */

.globl syscall_wrapper
    syscall_wrapper:
        # Save callee saved registers, including flags
        pushl   %ebp
        pushl   %esi
        pushl   %edi  
        pushfl 
        # Check whether the given Call Number is valid. Already stored in 
        # EAX, we must support ten system calls (numbered one through ten).
        # Check if EAX less than one
        cmpl    $1, %eax 
        jl      invalid_code
        cmpl    $10, %eax    
        jg      invalid_code
        # Otherwise, a valid code was pushed. Jump to the standard procedure.
        jmp     valid_code
    valid_code:
        # Though the argument of our codes are 1-10, the contents of
        # the table are still zero-indexed. Decrement value of EAX to
        # properly align our argument value and table.
        decl    %eax 
        
        # Valid code called. Push the arguments onto the stack.
        pushl   %edx 
        pushl   %ecx 
        pushl   %ebx 

        # Re-enable interrupts after system call handled!
        sti     

        # Call functions outlined in jump table, similar to ioctls in MP1
        call    *syscall_table( , %eax, 4 );

        # Pop args off stack
        popl    %ebx 
        popl    %ecx 
        popl    %edx

        # Pop saved registers off the stack
        popfl   
        popl    %edi
        popl    %esi
        popl    %ebp   

        # iret at end
        iret 

    # Invalid code called. Pop registers off stack and return -1. 
    invalid_code:
        # Pop all saved registers, including flags
        popfl   
        popl    %edi
        popl    %esi
        popl    %ebp   
        # Set return value to -1
        movl    $-1, %eax 
        # iret at end 
        iret 

# Define jump table, similar to mp1. Formatted in the order of 
#   call numbers. 
syscall_table:
    .long   syscall_halt, syscall_execute, syscall_read, syscall_write, syscall_open, syscall_close, syscall_getargs, syscall_vidmap, syscall_set_handler, syscall_sigreturn

