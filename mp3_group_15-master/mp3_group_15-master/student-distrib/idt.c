#include "idt.h"

/* void init_idt                                                */
/* Description:         Initiates the IDT's values, setting its */
/*                      contents as appropriate.                */
/* Inputs:              None.                                   */
/* Outputs:             None.                                   */
/* Side Effects:        Initializes the IDT's values in the     */
/*                      struct provided in x86_desc.h. Sets the */
/*                      values accordingly, and also            */
/*                      initializes entries for RTC and         */
/*                      keyboard.                               */
void init_idt( )
{   
    /* Set the common values for each of the IDT Descriptors in the IDT. */
    int i;
    for( i = 0; i < NUM_VEC; i++ )
    {
        /* Set present bit to 1 to make descriptor valid */
        idt[ i ].present = 1;
        /* The DPL (Descriptor Privilege Level) is determined by if the */
        /* call made is a System Call or not. If it is a system call,   */
        /* then set the DPL to 3. Otherwise, set to 0 to prevent any    */
        /* user-level programs from calling into these routines using   */
        /* an int instruction. For now, we'll set all of the DPLs to 0, */
        /* and set the dpl for 0x80 (the System Call Offset) afterwards */
        idt[ i ].dpl = 0;
        /* Value of reserved0 is 0 - Refer to IA-32 Manual 5.11 for     */
        /* details                                                      */
        idt[ i ].reserved0 = 0;
        /* Set the size of our handlers. All of our handlers are 32-bit */
        idt[ i ].size = 1;
        /* Value of reserved1 is 1 - Refer to IA-32 Manual Section 5.11 */
        idt[ i ].reserved1 = 1;
        /* Value of reserved2 is 1 - Refer to IA-32 Manual Section 5.11 */
        idt[ i ].reserved2 = 1;
        /* Value of reserved3 varies depending on if it's a TRAP gate   */
        /* or an Interrupt gate. The bit is 0 if it is an Interrupt,    */
        /* 1 if it is a TRAP gate. The first 32 sections in the IDT are */
        /* TRAP gates. Set accordingly.                                 */
        /* Linux is weird so we actually want to use 0 regardless.      */
        if( ( i >= IDT_USER_INTERRUPT_START ) && ( i <= IDT_USER_INTERRUPT_END ) )
        {
            idt[ i ].reserved3 = 0; 
        }
        else
        {
            idt[ i ].reserved3 = 0;
        }
        /* Set reserved4 to 0, is designated to be set to 0.            */
        idt[ i ].reserved4 = 0;
        /* Set the Segment Selector. All of our interrupts */
        /* are handled in the kernel code of our OS.       */
        idt[ i ].seg_selector = KERNEL_CS;

    }

    /* Common values for IDT set. Set the offsets for each entry and    */
    /* any other things needed.                                         */
    /* Set the System Call priority level */
    idt[ SYSTEM_CALL_VECTOR ].dpl = 3;
    /* Also set System Call's handler */
    SET_IDT_ENTRY( idt[ SYSTEM_CALL_VECTOR ], syscall_wrapper );
    /* Vector 2 is actually an interrupt. Set reserved3 appropriately. */
    idt[ 0x02 ].reserved3 = 0;
    /* Set the offsets for each entry */
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_DE  ],  exception_handler_DE  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_DB  ],  exception_handler_DB  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_NMI ],  exception_handler_NMI );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_BP  ],  exception_handler_BP  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_OF  ],  exception_handler_OF  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_BR  ],  exception_handler_BR  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_UD  ],  exception_handler_UD  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_NM  ],  exception_handler_NM  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_DF  ],  exception_handler_DF  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_CSO ],  exception_handler_CSO );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_TS  ],  exception_handler_TS  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_NP  ],  exception_handler_NP  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_SS  ],  exception_handler_SS  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_GP  ],  exception_handler_GP  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_PF  ],  exception_handler_PF  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_15  ],  exception_handler_15  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_MF  ],  exception_handler_MF  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_AC  ],  exception_handler_AC  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_MC  ],  exception_handler_MC  );
    SET_IDT_ENTRY( idt[ EXCEPTION_VECTOR_XF  ],  exception_handler_XF  );

    /* Set keyboard interrupt handler */
    SET_IDT_ENTRY( idt[KEYBOARD_VECTOR], keyboard_handler_linkage);

    /* Set RTC interrupt handler */
    SET_IDT_ENTRY( idt[RTC_VECTOR], rtc_handler_linkage);

    /* Set PIT interrupt handler */
    SET_IDT_ENTRY( idt[PIT_VECTOR], pit_handler_linkage);
}


/*           FUNCTION HEADER FOR ALL EXCEPTION HANDLERS                 */
/* Description:     handles the corresponding exception by calling the  */
/*                  assembly linkage and general exception handler. For */
/*                  Checkpoint 3.1, the handler will be configured to   */
/*                  loop infinitely.                                    */
/* Inputs:          None.                                               */
/* Outputs:         None.                                               */
/* Side Effects:    Handles the corresponding exception. Prints the     */
/*                  exception handled. Loops the program infinitely if  */
/*                  it is Checkpoint 3.1                                */
void exception_handler_DE( )
{
    exception_wrapper( EXCEPTION_VECTOR_DE );
    printf("Exception 0 (#DE) (Division by Zero) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_DB( )
{
    exception_wrapper( EXCEPTION_VECTOR_DB );
    printf("Exception 1 (#DB) (Debug) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_NMI( )
{
    exception_wrapper( EXCEPTION_VECTOR_NMI );
    printf("Exception 2 (NMI) (Non-Maskable Interrupt) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_BP( )
{
    exception_wrapper( EXCEPTION_VECTOR_BP );
    printf("Exception 3 (#BP) (Breakpoint) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_OF( )
{
    exception_wrapper( EXCEPTION_VECTOR_OF );
    printf("Exception 4 (#OF) (Overflow) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_BR( )
{
    exception_wrapper( EXCEPTION_VECTOR_BR );
    printf("Exception 5 (#BR) (Bound Range Exceeded) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_UD( )
{
    exception_wrapper( EXCEPTION_VECTOR_UD );
    printf("Exception 6 (#UD) (Invalid Opcode) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_NM( )
{
    exception_wrapper( EXCEPTION_VECTOR_NM );
    printf("Exception 7 (#NM) (Coprocessor Not Available) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_DF( )
{
    exception_wrapper( EXCEPTION_VECTOR_DF );
    printf("Exception 8 (#DF) (Double Fault) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_CSO( )
{
    exception_wrapper( EXCEPTION_VECTOR_CSO );
    printf("Exception 9 (CSO) (Coprocessor Segment Overrun) invoked. Looping...\n");
    
    while(1){ }
}

void exception_handler_TS( )
{
    exception_wrapper( EXCEPTION_VECTOR_TS );
    printf("Exception 10 (#TS) (Invalid Task State Segment) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_NP( )
{
    exception_wrapper( EXCEPTION_VECTOR_NP );
    printf("Exception 11 (#NP) (Segment Not Present) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_SS( )
{
    exception_wrapper( EXCEPTION_VECTOR_SS );
    printf("Exception 12 (#SS) (Stack Segment Fault) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_GP( )
{
    exception_wrapper( EXCEPTION_VECTOR_GP );
    printf("Exception 13 (#GP) (General Protection Fault) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_PF( )
{
    exception_wrapper( EXCEPTION_VECTOR_PF );
    printf("Exception 14 (#PF) (Page Fault) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_15( )
{
    exception_wrapper( EXCEPTION_VECTOR_15 );
    printf("Exception 15 (-) (Reserved) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_MF( )
{
    exception_wrapper( EXCEPTION_VECTOR_MF );
    printf("Exception 16 (#MF) (x87 Floating Point Exception) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_AC( )
{
    exception_wrapper( EXCEPTION_VECTOR_AC );
    printf("Exception 17 (#AC) (Alignment Check) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_MC( )
{
    exception_wrapper( EXCEPTION_VECTOR_MC );
    printf("Exception 18 (#MC) (Machine Check) invoked. Looping...\n");

    while(1){ }
}

void exception_handler_XF( void )
{
    exception_wrapper( EXCEPTION_VECTOR_XF );
    printf("Exception 19 (#XF) (SIMD Floating-Point Exception) invoked. Looping...\n");
    
    while(1){ }
}

void exception_handler_sys_call( )
{
    syscall_wrapper( );
    printf("System Call invoked! Looping in initial handler...\n");

    while(1){ }
}

/* Vectors 20-31 reserved by Intel. Do not use. */
