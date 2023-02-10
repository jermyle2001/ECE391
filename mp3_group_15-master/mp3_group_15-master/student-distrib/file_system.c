#include "file_system.h"
#include "types.h"
#include "lib.h"
#include "keyboard.h"

/* User Memory (0x8000000) + Page_4MB(0x400000) - sizeof(uint32)    */
/* or might be user start addr + some offset (VM starts at 128 MB)  */
#define PROGRAM_IMG_ADDRS 0x08048000 
#define PROGRAM_IMG_OFF   0x00048000
#define FOUR_MB           0x0400000 

/* Initialize global array of file descriptors */
open_file_t file_array[FILE_ARRAY_SIZE];

/* void fileArray_init();
 *   Inputs: None
 *   Return Value: None
 *   Function: Open stdin and stdout, mark the remainder of the file array as available */
void fileArray_init(void) {
    int i;

    /*Automatically open stdin*/
    /* file_array[0].fops_ptr = get_stdin_table(); */
    file_array[0].index_node_num = 0;    /*inode is only valid for data files*/
    file_array[0].flags = FD_IN_USE;
    file_array[0].file_position = INIT_FILE_POSITION;

    /*Automatically open stdout*/
    /* file_array[1].fops_ptr = get_stdout_table(); */
    file_array[1].index_node_num = 0;    /*inode is only valid for data files*/
    file_array[1].flags = FD_IN_USE;
    file_array[1].file_position = INIT_FILE_POSITION;

    /*The remaining file descriptors are available*/
    for (i = 2; i < FILE_ARRAY_SIZE; i++) {
        file_array[i].flags = FD_FREE;
    }
    return;
}

/* void fileSystem_init(uint32_t* fs_start);
 *   Inputs: uit32_t* fs_start --> The starting address of the file system
 *   Return Value: None
 *   Function: Initializes the file system and global pointers used in the file system functions */
void fileSystem_init(uint32_t* fs_start) {
    /* Initializes all global pointers */
    unsigned int num_inodes;
    p_boot_block_addr = (boot_block_t*) fs_start;
    num_inodes = p_boot_block_addr->num_inodes;
    p_inode_addr = (inode_t*) p_boot_block_addr + 1;
    p_data_block_addr = (data_block_t*) p_inode_addr + num_inodes;
    fileArray_init();
    return;
}

/* int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
 *   Inputs: const uint8_t* fname --> A pointer to the file name to search for
 *           dentry_t* dentry --> A pointer to the directory entry to pass back
 *   Return Value: 0 --> Success
 *                -1 --> Failure
 *   Function: Searches all directory entries via a given file name and passes back the
 *             corresponding directory entry if found */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) 
{
    /* Check if passed in pointer to the file name is empty. */
    if( fname == NULL )
    {
        return -1;
    }

    /* Get the length of the passed in file name. Truncate to   */
    /* maximum file length if needed.                           */
    unsigned int file_name_length = strlen( (int8_t*)fname );
    if( file_name_length > MAX_FILE_NAME_LENGTH )
    {
        file_name_length = MAX_FILE_NAME_LENGTH;
    }

    /* Remove the '\n' from the file name if included in the    */
    /* string passed in, to prevent confusion in matching.      */
    if( fname[ file_name_length - 1 ] == '\n' )
    {
        file_name_length = file_name_length - 1;
    }

    /* Get the total number of directory entries (dentries) to  */
    /* search through, and the actual array of dentries to      */
    /* search through. We will loop through the array and check */
    /* if any matches arise.                                    */
    unsigned int num_dentries = p_boot_block_addr->num_dir_entries;
    dentry_t* directories = p_boot_block_addr->dir_entries;

    /* Loop through all of the directory entries and search for */
    /* the passed in file name. We will check if the file name  */
    /* sizes match up first - by doing this, we can prevent     */
    /* substrings from coming up with matches. Aftewards, we    */
    /* will use strcmp to compare the filenames and decide      */
    /* matching filenames.                                      */
    dentry_t curr_directory;
    char* curr_file_name;
    unsigned int i;
    unsigned int curr_file_name_length;
    int result;
    for( i = 0; i < num_dentries; i++ )
    {
        /* Get the current dentry we are looking at stored in the   */
        /* directories array. Also get the corresponding filename   */
        /* for comparison.                                          */
        curr_directory = directories[ i ];
        curr_file_name = curr_directory.file_name;

        /* Also limit the curr_file_name_length if needed. If not   */
        /* limited/truncated, then the string may become too long   */
        /* and bugs may occur.                                      */
        curr_file_name_length = strlen( (int8_t*)curr_file_name );
        if( curr_file_name_length > MAX_FILE_NAME_LENGTH )
        {
            curr_file_name_length  = MAX_FILE_NAME_LENGTH;
        }

        /* Compare the lengths of the filenames. If they are not of */
        /* the same length, then they cannot be the same string.    */
        /* Keep looping if they are not of the same length.         */
        if( file_name_length != curr_file_name_length )
        {
            continue;
        }

        /* We have possibly found a matching string, but need to    */
        /* actually compare the contents of the two strings to      */
        /* determine if they are the same.                          */
        if( strncmp( (int8_t*)fname, (int8_t*)curr_file_name, file_name_length ) == 0 )
        {
            /* We have determined the file name and the directory   */
            /* file name are the same. Read the dentry into our     */
            /* passed in argument dentry using read_dentry_by_index */
            /* and check if the read failed or not. Return the      */
            /* corresponding completion status.                     */
            result = read_dentry_by_index( i, dentry );
            if( result == -1 )
            {
                /* Return FAILURE. */
                return -1;
            }
            else
            {
                /* Return SUCCESS. */
                return 0;
            }
        }

    }

    /* After loooping through the entire directory, we did not find */
    /* a corresponding directory entry for the filename passed in.  */
    /* Return FAILURE.                                              */
    return -1;
}

/* int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
 *   Inputs: uint32_t index --> The index of the directory to copy over
 *           dentry_t* dentry --> A pointer to the directory entry to pass back
 *   Return Value: 0 --> Success
 *                -1 --> Failure
 *   Function: Copies the data of a given directory based on the corresponding passed in directory index */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {

    /* Gets the total number of inodes */
    unsigned int num_inodes = p_boot_block_addr->num_inodes;

    /* Gets the actual array of directories */
    dentry_t* directories = p_boot_block_addr->dir_entries;

    /* Declare other local variables */
    dentry_t curr_directory;
    unsigned int i;    

    /* Checks if the passed in index is out of boundsn*/
    if (index < 0 || index >= num_inodes) {
        return -1;
    }

    /* Gets the current directory based on the passed in index */
    curr_directory = directories[index];

    /* Copies over all fields of the dentry_t struct */
    strcpy((int8_t*) dentry->file_name, (int8_t*) curr_directory.file_name);
    dentry->file_type = curr_directory.file_type;
    dentry->index_node_num = curr_directory.index_node_num;
    for (i = 0; i < RESERVED_D_SIZE; i++) {
        dentry->reserved[i] = curr_directory.reserved[i];
    }
    
    /* Returns 0 if everything is successful */
    return 0;
}

/* int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
 *   Inputs: uint32_t inode --> The inode index number that we want to read from
 *           uint32_t offset --> The offset value from the beginning of the inode where we want to start reading from
 *           uint8_t* buf --> A pointer to the buffer we write the data to
 *           uint32_t length --> The number of bytes we want to read
 *   Return Value: int32_t --> The number of bytes read
 *   Function: Reads data from a specified point of a given inode and writes it to a passed in buffer pointer */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    /* Gets the address of the inode to read from */
    inode_t* curr_inode = p_inode_addr + inode;

    /* Gets the file size of the corresponding inode to read from */
    unsigned int file_size = curr_inode->file_size;

    /* Gets an array of the data blocks for the corresponding inode to read from */
    unsigned int* data_blocks = curr_inode->data_blocks;

    /* Gets the total number of inodes */
    unsigned int num_inodes = p_boot_block_addr->num_inodes;
    
    /* Declare other local variables */
    unsigned int num_bytes_read_total = 0;
    unsigned int curr_data_block_num = 0;
    unsigned int curr_byte_index = 0;
    unsigned int curr_data_block_index;
    data_block_t* curr_data_block;
    unsigned int i;

    /* Checks if the given inode index number is out of bounds */
    if (inode < 0 || inode >= num_inodes) {
        return 0;
    }

    /* Checks if the given offset value is out of bounds */
    if (offset < 0 || offset >= file_size ) {
        return 0;
    }

    /* Edge case: whenever the offset is really big it  */
    /* breaks for some weird reason (specifically in    */
    /* the very large text file) so we'll just treat    */
    /* that as an edge case and add the offset.         */
    if( offset > 4096 && ( offset + FILE_SYS_OFFSET ) >= file_size )
    {
        return 0;
    }

    /* If the number of bytes to read is greater than the file size, */
    /* set the number of bytes to read to just be equal to the file size */
    if (length > file_size) {
        length = file_size;
    }

    /* If the offset + number of bytes to read is greater than the file size, */
    /* set the number of bytes to read to be the difference between the file size and offset value */
    if (offset + length > file_size) {
        length = file_size - offset;
    }  

    /* Set the current byte to start reading from to be equal to the offset value */
    curr_byte_index = offset;

    /* If the current byte to read from is greater than the size of a data block, reset it back to 0 */
    /* Do this both before AND after reading the data in case our offset passed in is too large.     */
    if (curr_byte_index >= SIZE_DATA_BLOCK) {
        curr_byte_index = 0;

        /* Update the current data block we are reading from */
        curr_data_block_num++;
    }

    /* Loop through all bytes to read and copy them to the passed in buffer */
    for (i = 0; i < length; i++) {        
        /* Gets the corresponding data block to read from */
        curr_data_block_index = data_blocks[curr_data_block_num];
        curr_data_block = (data_block_t*) p_data_block_addr + curr_data_block_index;

        /* Copies over a byte of data from the corresponding data block to the buffer */
        buf[i] = curr_data_block->data[curr_byte_index];
        curr_byte_index++;

        /* If the current byte to read from is greater than the size of a data block, reset it back to 0 */
        if (curr_byte_index >= SIZE_DATA_BLOCK) {
            curr_byte_index = 0;

            /* Update the current data block we are reading from */
            curr_data_block_num++;
        }

        /* Update the total number of bytes read */
        num_bytes_read_total++;
    }

    return num_bytes_read_total;
}

/* int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
 *   Inputs: int32_t fd --> The index into the file descriptor array
 *           void* buf --> A pointer to the buffer we write the data to
 *           int32_t nbytes --> The number of bytes to read
 *   Return Value: The amount of bytes read
 *   Function: Reads the data of a given open file and writes it to a passed in buffer pointer */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    /* Checks if fd is out of bounds */
    if (fd < 0 || fd > 7) {
        return 0;
    }

    /* Declare local variables */
    unsigned int num_bytes_read;
    unsigned int curr_file_position;
    unsigned int curr_inode_index;

    /* Initialize the contents of our buffer up to nbytes to '\0' so that   */
    /* we don't have to worry about the buffer ending at the wrong place.   */
    memset( buf, '\0', nbytes );    

    /* Gets the open file corresponding to the passed in file descriptor to read from */
    open_file_t curr_file = file_array[fd];  

    /* Gets the inode of the corresponding current open file */
    curr_inode_index = curr_file.index_node_num;

    /* Gets the file position offset of current open file */
    curr_file_position = curr_file.file_position;

    /* Reads nbytes of data from the current open file and copies it into the passed in buffer */
    num_bytes_read = read_data(curr_inode_index, curr_file_position, buf, nbytes);

    /* Increments and updates the current file position for the open file */
    curr_file_position += num_bytes_read;
    file_array[fd].file_position = curr_file_position;

    return num_bytes_read;
}

/* int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
 *   Inputs: N/A
 *   Return Value: N/A
 *   Function: Does nothing, not used in our implementation */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* int32_t file_open(const uint8_t* filename);
 *   Inputs: const uint8_t* filename --> A pointer to the file name to open
 *   Return Value: 0 --> Success
 *                 1 --> Failure
 *   Function: Searches all directory entries via a given file name and opens the corresponding file if found */
int32_t file_open(const uint8_t* filename) {
    dentry_t curr_dentry;
    unsigned int file_type;
    int i, result, fd;

    result = read_dentry_by_name(filename, &curr_dentry);

    if (result == -1) return result; /*FaiL: Named file does not exist*/

    file_type = curr_dentry.file_type;

    /* Finds the next available file descriptor position in the global file_array */
    for (i = 0; i < FILE_ARRAY_SIZE; i++) {
        if (file_array[i].flags != 0) {
            fd = i + 1;
        }
    } if (fd > FILE_ARRAY_SIZE - 1) return -1; /*Fail: No descriptors are free*/

    if (file_type == REG_FILE_TYPE) {
        file_array[fd].index_node_num = curr_dentry.index_node_num;
    } else {
        /* Set inode to 0 for directories and RTC device file */
        file_array[fd].index_node_num = 0; 
    }
    file_array[fd].file_position = 0;
    file_array[fd].flags = 1;
    
    return fd;
}

/* int32_t file_close(int32_t fd);
 *   Inputs: int32_t fd --> File descriptor to close
 *   Return Value: 0 --> Success
 *   Function: Closes a given file */
int32_t file_close(int32_t fd) {
    if (fd < 2 || fd > FILE_ARRAY_SIZE - 1) return -1;  /*User cannot close the default descriptors 0 or 1*/

    file_array[fd].flags = 0;
    return 0;
}

/* int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
 *   Inputs: int32_t fd --> The index into the file descriptor array
 *           void* buf --> A pointer to the buffer we write the data to
 *           int32_t nbytes --> The number of bytes to read
 *   Return Value: The amount of bytes read
 *   Function: Reads the file name of a given directory and writes it to a passed in buffer pointer */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    /* Checks if fd is out of bounds */
    if (fd < 0 || fd > 7) {
        return 0;
    }

    /* Gets the open directory corresponding to the passed in file descriptor to read from */
    open_file_t curr_file = file_array[fd];

    /* Gets the file position to determine which directory entry to read from */
    unsigned int curr_position = curr_file.file_position;

    /* Declare other local variables */
    dentry_t curr_dentry;
    unsigned int file_length, copy_length;
    int result;
    char* file_name;
    char* copy_name;

    /* Gets the directory entry corresponding to the given file position */
    result = read_dentry_by_index(curr_position, &curr_dentry);
    
    /* If the directory was not found/invalid, return 0 (No bytes were read) */
    if (result == -1) {
        return 0;
    }

    /* Initialize the contents of our buffer up to nbytes to '\0' so that   */
    /* we don't have to worry about the buffer ending at the wrong place.   */
    memset( buf, '\0', nbytes );

    /* Increments and updates the file position */
    curr_position += 1;
    file_array[fd].file_position = curr_position;

    /* Gets the current directory's file name and length */
    file_name = curr_dentry.file_name;
    file_length = strlen((const int8_t*) file_name);

    /* If the amount of bytes to read is greater than the file name length, then set the amount of bytes to read to be just the entire file name */
    if (nbytes > file_length) {
        nbytes = file_length;
    }
    else if( file_length > MAX_FILE_NAME_LENGTH )
    {
        file_length = MAX_FILE_NAME_LENGTH;
    }

    /* Copies nbytes of the file name into the passed in buffer */
    copy_name = strncpy((int8_t*) buf, (int8_t*) curr_dentry.file_name, nbytes);

    /* Adds a NULL-terminated character to the end of the buffer */
    ((char*)buf)[file_length] = '\0';

    /* Gets the length of the copied string to determine how many bytes were read */
    copy_length = strlen((const int8_t*) copy_name);
    
    return copy_length;
}

/* int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
 *   Inputs: N/A
 *   Return Value: N/A
 *   Function: Does nothing, not used in our implementation */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* int32_t dir_open(const uint8_t* filename);
 *   Inputs: const uint8_t* filename --> A pointer to the directory name to open
 *   Return Value: 0 --> Success
 *                 1 --> Failure
 *   Function: Searches all directory entries via a given file name and opens the corresponding directory if found */
int32_t dir_open(const uint8_t* filename) {
    /* Checks if the passed in pointer to the file name is NULL */
    if (filename == 0) {
        return -1;
    }

    /* Declare local variables */
    dentry_t curr_dentry;
    unsigned int file_type;
    int result, i, fd;

    /* Gets the directory entry corresponding to the passed in file name */
    result = read_dentry_by_name((uint8_t*) filename, &curr_dentry);

    /* If the file name is not found/invalid, return -1 (Failure) */
    if (result == -1) {
        return -1;
    }

    /* Gets the file type of the directory entry */
    file_type = curr_dentry.file_type;
    
    /* Checks if the file type is actually a directory */
    if (file_type == 1) {
        /* Finds the next available file descriptor position in the global file_array */
        for (i = 0; i < FILE_ARRAY_SIZE; i++) {
            if (file_array[i].flags != 0) {
                fd = i + 1;
            }
        } 
        if (fd > FILE_ARRAY_SIZE - 1) return -1; /*Fail: No descriptors are free*/

        /* Re-initializes the corresponding open file descriptor */
        file_array[fd].index_node_num = 0;
        file_array[fd].file_position = 0;
        file_array[fd].flags = 1;

        return fd;
    }
    /* Otherwise, return -1 (Failure) */
    else {
        return -1;
    }
}  

/* int32_t dir_close(int32_t fd);
 *   Inputs: int32_t fd --> File descriptor to close
 *   Return Value: 0 --> Success
 *   Function: Closes a directory */
int32_t dir_close(int32_t fd) {
    if (fd < 2 || fd > FILE_ARRAY_SIZE - 1) return -1;  /*User cannot close the default descriptors 0 or 1*/

    file_array[fd].flags = 0;
    return 0;
}

/* int load_file( dentry_t file_entry, uint8_t* eip_buf );
 *   Inputs: dentry_t file_entry, uint8_t* eip_buf --> file to load, buffer to load eip to
 *   Return Value: 0 --> Success, 1 --> Fail
 *   Function: Copy the program from the disk block (virtual) into the physical memory using read_data. Return the EIP onto eip_buf. */
int load_file( dentry_t file_entry, uint8_t* eip_buf )
{
    int flag1 = read_data( file_entry.index_node_num, 0, (uint8_t*)PROGRAM_IMG_ADDRS, FOUR_MB - PROGRAM_IMG_OFF );
    int flag2 = read_data( file_entry.index_node_num, 24, eip_buf, 4 );

    if( flag1 || flag2 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* uint32_t get_file_size( uint32_t inode );
 *   Inputs: uint32_t inode --> inode to retrieve associated file size
 *   Return Value: 0 --> file size of inode
 *   Function: Gets file size associated with an inode */
uint32_t get_file_size( uint32_t inode )
{
    inode_t* curr_inode = (inode_t*)(p_inode_addr + inode);
    return curr_inode->file_size;
}



