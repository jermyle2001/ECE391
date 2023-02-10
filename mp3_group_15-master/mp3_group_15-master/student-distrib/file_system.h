#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

/* Include Statements */
#include "types.h"
#include "fops.h"

/* Definitions */
#define RESERVED_D_SIZE      24
#define RESERVED_B_SIZE      52
#define DIR_ENTRIES_SIZE     63
#define NUM_DATA_BLOCKS      1024
#define SIZE_DATA_BLOCK      4096
#define MAX_FILE_NAME_LENGTH 32
#define FILE_ARRAY_SIZE      8
#define RTC_TYPE             0
#define DIRECTORY_TYPE       1
#define REG_FILE_TYPE        2
#define TERMINAL_FILE_TYPE   3
#define INIT_FILE_POSITION   0
#define FD_FREE              0
#define FD_IN_USE            1
#define FILE_SYS_OFFSET     157

/* Struct Definitions */
typedef struct dentry_t {
    char file_name[MAX_FILE_NAME_LENGTH];
    unsigned int file_type;
    unsigned int index_node_num;
    char reserved[RESERVED_D_SIZE];
} dentry_t;

typedef struct boot_block_t {
    unsigned int num_dir_entries;
    unsigned int num_inodes;
    unsigned int num_data_blocks;
    char reserved[RESERVED_B_SIZE];
    dentry_t dir_entries[DIR_ENTRIES_SIZE];
} boot_block_t;

typedef struct inode_t {
    unsigned int file_size;
    unsigned int data_blocks[NUM_DATA_BLOCKS - 1];
} inode_t;

typedef struct data_block_t {   
    char data[SIZE_DATA_BLOCK];
} data_block_t;

typedef struct open_file_t {
    fops_table_t* fops_ptr;
    unsigned int index_node_num;
    unsigned int file_position;
    unsigned int flags;
} open_file_t;

/* Declares global pointers to boot_block, inode, and data_block*/
boot_block_t* p_boot_block_addr;
inode_t* p_inode_addr;
data_block_t* p_data_block_addr;

/* Also make the file array global so we can synchronize it */
/* with each individual process's file array.               */
extern open_file_t file_array[FILE_ARRAY_SIZE];

/* Function Declarations */
/* Initializes the file system and the corresponding global pointers */
extern void fileSystem_init(uint32_t* fs_start);

/* Searches for a directory based on a passed in file name */
extern int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

/* Copies over the data of a directory entry specified by the passed in index */
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/* Copies over the data of a given inode */
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* File read, write, open, and close system calls */
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t file_open(const uint8_t* filename);
extern int32_t file_close(int32_t fd);

/* Directory read, write, open, and close system calls */
extern int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t dir_open(const uint8_t* filename);
extern int32_t dir_close(int32_t fd);

extern int load_file( dentry_t file_entry, uint8_t* eip_buf );

/* Helper function to get the size of a file */
extern uint32_t get_file_size( uint32_t inode );

/* Helper funtion to print a given string */
extern void print_string( uint8_t* buf );

#endif /* _FILE_SYSTEM_H */
