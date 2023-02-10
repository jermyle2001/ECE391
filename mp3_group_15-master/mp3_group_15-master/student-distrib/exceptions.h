#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "lib.h"
#include "syscall.h"

/* General handler for exceptions prints our the exception that */
/* is called and then will loop the program once the exception  */
/* is raised                                                    */
void exception_handler_general( uint32_t id );
#define EXCEPTION_MAX_ID 19

/* Defining the vector number of the different exceptions       */
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
#define VECTOR_SYS_CALL         0x80

#endif
