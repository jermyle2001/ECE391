#ifndef _EXCEPTION_WRAP_H
#define _EXCEPTION_WRAP_H


/* Header file for exceptions functon wrapping. */
#include "exceptions.h"

/* Declare external function wrapper for usage in wrapper and handler*/
extern void exception_wrapper( uint32_t exception_id );

#endif
