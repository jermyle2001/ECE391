#ifndef _FOPS_H
#define _FOPS_H

#include "types.h"

/* Struct for generic file operations table */
typedef struct fops_table_t { 
    int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} fops_table_t;

/* Functions to get specific file operations tables */
extern fops_table_t* get_RTC_table(void);
extern fops_table_t* get_file_table(void);
extern fops_table_t* get_dir_table(void);
extern fops_table_t* get_terminal_table(void);
extern fops_table_t* get_stdout_table(void);
extern fops_table_t* get_stdin_table(void);

#endif
