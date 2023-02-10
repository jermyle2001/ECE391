#include "exceptions.h"
#include "lib.h"

/*          exception_handler_general                   */
/* General handler for exceptions. For Checkpoint 3.1,  */
/* this will entail just printing out the current       */
/* exception called, its mnenomic, and its id number.   */
/* Is also configured for Checkpoint 3.1 to handle      */
/* system calls. Will loop the program infinitely once  */
/* an exception is raised.                              */
/* Inputs: Vector ID number                             */
/* Outputs: None, technically.                          */
/* Side Effects: Prints the exception to the screen.    */
void exception_handler_general( uint32_t id )
{
    /* Test to see if the handler was invoked at all. */
    printf("Exception handler called!\n");
    /* Check if id passed in is an actual exception. If not, return from handler.*/
    /* For Checkpoint 3.1, we are including System Calls.                        */
    if( ( id > EXCEPTION_MAX_ID ) && ( id != VECTOR_SYS_CALL ) )
    {
        printf("Exception handler called but ID passed is not an actual exception. Returning...\n"); 
        return;
    }

    /* Else, check the ID, print the exception, and quash the user program. */
    switch( id )
    {
        case EXCEPTION_VECTOR_DE:
            printf("Exception 0 (#DE) (Division by Zero) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_DB:
            printf("Exception 1 (#DB) (Debug) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_NMI:
            printf("Exception 2 (NMI) (Non-Maskable Interrupt) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_BP:
            printf("Exception 3 (#BP) (Breakpoint) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_OF:
            printf("Exception 4 (#OF) (Overflow) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_BR:
            printf("Exception 5 (#BR) (Bound Range Exceeded) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_UD:
            printf("Exception 6 (#UD) (Invalid Opcode) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_NM:
            printf("Exception 7 (#NM) (Coprocessor Not Available) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_DF:
            printf("Exception 8 (#DF) (Double Fault) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_CSO:
            printf("Exception 9 (CSO) (Coprocessor Segment Overrun) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_TS:
            printf("Exception 10 (#TS) (Invalid Task State Segment) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_NP:
            printf("Exception 11 (#NP) (Segment Not Present) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_SS:
            printf("Exception 12 (#SS) (Stack Segment Fault) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_GP:
            printf("Exception 13 (#GP) (General Protection Fault) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_PF:
            printf("Exception 14 (#PF) (Page Fault) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_15:
            printf("Exception 15 (-) (Reserved) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_MF:
            printf("Exception 16 (#MF) (x87 Floating Point Exception) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_AC:
            printf("Exception 17 (#AC) (Alignment Check) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_MC:
            printf("Exception 18 (#MC) (Machine Check) invoked. Looping...\n");
            break;
        case EXCEPTION_VECTOR_XF:
            printf("Exception 19 (#XF) (SIMD Floating-Point Exception) invoked. Looping...\n");
            break;
        case VECTOR_SYS_CALL:
            printf("System Call invoked! Looping...\n");
            break;
        default:
            printf("Exception %d invoked.\n", id );
            break;
    }


    /* Exception identified and handled. Quash the user program.*/
    /* For Checkpoint 3.1, that entails just looping.           */
    /* while(1){ } */

    /* For Checkpoint 3.3, that means we halt the program, with */
    /* status 256.                                              */
    syscall_halt( (uint8_t)256 );

}

