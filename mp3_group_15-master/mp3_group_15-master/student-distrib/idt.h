#ifndef _IDT_H
#define _IDT_H
#include "x86_desc.h"
#include "lib.h"

/* Include files for keyboard and interrupt linkage */
#include "interrupt_linkage.h"
#include "keyboard.h"
#include "exception_wrapper.h"
#include "syscall_wrapper.h"

#define SYSTEM_CALL_VECTOR       0x80
#define IDT_USER_INTERRUPT_START 32
#define IDT_USER_INTERRUPT_END   36

/* Defining interrupt vectors for devices */
#define PIT_VECTOR               0x20
#define KEYBOARD_VECTOR          0x21
#define RTC_VECTOR               0x28

/* Vectors nums for exceptions */
#define EXCEPTION_VECTOR_DE     0
#define EXCEPTION_VECTOR_DB     1
#define EXCEPTION_VECTOR_NMI    2
#define EXCEPTION_VECTOR_BP     3
#define EXCEPTION_VECTOR_OF     4
#define EXCEPTION_VECTOR_BR     5
#define EXCEPTION_VECTOR_UD     6
#define EXCEPTION_VECTOR_NM     7
#define EXCEPTION_VECTOR_DF     8
#define EXCEPTION_VECTOR_CSO    9
#define EXCEPTION_VECTOR_TS     10
#define EXCEPTION_VECTOR_NP     11
#define EXCEPTION_VECTOR_SS     12
#define EXCEPTION_VECTOR_GP     13
#define EXCEPTION_VECTOR_PF     14
#define EXCEPTION_VECTOR_15     15
#define EXCEPTION_VECTOR_MF     16
#define EXCEPTION_VECTOR_AC     17
#define EXCEPTION_VECTOR_MC     18
#define EXCEPTION_VECTOR_XF     19

/* Functions defined  */
void exception_handler_DE( );
void exception_handler_DB( );
void exception_handler_NMI( );
void exception_handler_BP( );
void exception_handler_OF( );
void exception_handler_BR( );
void exception_handler_UD( );
void exception_handler_NM( );
void exception_handler_DF( );
void exception_handler_CSO( );
void exception_handler_TS( );
void exception_handler_NP( );
void exception_handler_SS( );
void exception_handler_GP( );
void exception_handler_PF( );
void exception_handler_15( );
void exception_handler_MF( );
void exception_handler_AC( );
void exception_handler_MC( );
void exception_handler_XF( );
void exception_handler_sys_call( );

#endif







