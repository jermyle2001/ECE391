#define ASM 1

/* Description                                              */
/* Exception wrapper for exceptions that don't have an      */
/* error code. In this version of the exception wrapper,    */
/* one less argument is pushed to the stack, since there is */
/* no error code available for the specific interrupt. The  */
/* wrapper pushes all registers to conserve them in case C  */
/* gets a little wonky, and uses the function name as an    */
/* argument to the handler. A general handler is used for   */
/* the exceptions passed through, and just squashes any     */
/* user programs. For Checkpoint 1, the handler is          */
/* configured to just loop, since there are no user         */
/* programs to quash.                                       */
/* exception_wrapper_no_error(exception_name, exception_id) */
/* Declare label for function so that we can call other     */
/* functions to be wrapped at this label.                   */
/* Inputs - Vector ID number                                */
/* Outputs - None                                           */
/* Side Effects - Prints the corresponding vector and       */
/*                quashes the user program                  */
#define void exception_wrapper( uint32_t exception_id )
.globl exception_wrapper
    exception_wrapper: 
        # Push EBP and get new ESP 
        pushl   %ebp
        movl    %esp, %ebp
        /*
        # No error code to push, push all registers to stack and
        # the ID number. Call the general exception handler after
        # setting up the stack properly.
        # Save all registers, in case something gets funky
        */
        pushal
        # Get vector ID number
        movl    8(%ebp), %edx 
        # Push ID to stack
        pushl   %edx
        # Call general exception handler
        call    exception_handler_general
        # Pop args off stack
        addl    $4, %esp 
        # Restore all registers to their previous state
        popal   
        # Use iret to return from interrupt
        iret 
