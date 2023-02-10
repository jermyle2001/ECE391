#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib.h"
#include "file_system.h"
#include "types.h"
#include "fops.h"
#include "paging.h"
#include "x86_desc.h"
#include "syscall_wrapper.h"
#include "keyboard.h"
#include "tests.h"


/* Constants relevant to System Calls */
#define FD_MAX_VAL      7               /* Maximum 8 files open in File Array 0-indexed */
                                        /* (0-indexed).                                 */
#define FD_MIN_VAL      2               /* First two FDs in File Array are reserved for */
                                        /* stdin and stdout. Thus we can only assign    */
                                        /* fd values ( 0-indexed ) 2-7.                 */
#define MAX_NUM_FILES   8               /* Maximum 8 files open in File Array.          */
#define MAX_NUM_PROGS   5               /* Maximum 6 programs open (0-indexed).         */
#define FAILURE         -1              /* Used to return -1 when a function has failed */
#define EIGHT_MB        0x00800000      /* First PCB starts at 8MB. Since that doesn't  */
                                        /* translate very nicely in hex, we'll set it   */
                                        /* to the minimum number of bits needed to      */
                                        /* represent.                                   */
#define VIRT_VID_MEM    0x08800000      /* Virtual Video Memory should start at 136 MB. */
#define FOUR_KB         0x1000          /* Used to adjust video memory addresses        */
#define EIGHT_KB        0x2000          /* Each PCB starts at 8MB - 8KB * Process Num   */
                                        /* Process Number is the same as Process ID     */
                                        /* aka PID. Keep track of the PID for the       */
                                        /* current process so that we can determine its */
                                        /* PCB and identify it as needed (for example,  */
                                        /* in scheduling).                              */
#define FOUR_MB         0x00400000      /* Used to make sure the screen start address   */
                                        /* is outside the kernel address space          */                                       
#define MAX_NUM_ARGS    5               /* NOT SURE ABOUT VALUE but used for parsing    */
#define MAGIC_NUM_0     0x7F            /* Magic numbers that identify executables      */
#define MAGIC_NUM_1     0x45            
#define MAGIC_NUM_2     0x4C
#define MAGIC_NUM_3     0x46
#define PROG_IMG_START  0x8048000       /* Program image starts at 0x08048000 as        */
                                        /* outlined by documentation.                   */
#define BUFFER_SIZE     128             /* Buffer max size is 128.                      */
#define MAX_ARG_LEN     35              /* Max combined length of arguments is 35       */
#define NUM_EIP_BYTES   4               /* Bytes 27-24 of the inode store the EIP       */
#define BYTE_SIZE       8               /* Number of bits in a byte                     */
#define IF_ENABLE       0x00000200      /* IF enable for iret context.                  */
#define BOTTOM          0x083FFFFC      /* The bottom of the memory                     */
#define EIP_BYTE_OFFSET 24              /* Byte offset for EIP                          */
#define PID_FREE        0               /* PID Free definition for PID Array searching  */
#define PID_IN_USE      1               /* PID In Use definition for PID Array search   */
#define HALT_ERROR      37              /* Error #37 is the halt indicator for error    */
#define HALT_ERROR_CODE 256             /* due to exception. Return 256 at end of halt  */
                                        /* to indicate such.                            */
#define USER_PAGE       32              /* Page directory index of the user page        */
                                        /* Takes the top 10 bits of user virtual start  */
                                        /* address 0x8000000 */

/* Struct for Process Control Block (PCB) */
typedef struct pcb_t {
        int32_t         pid;                             /* Process ID # for the current process */
        int32_t         parent_id;                       /* Parent Process ID # to return to     */
        open_file_t     fd_array[ MAX_NUM_FILES ];       /* Array of files currently open        */
        int32_t         filetype_array[ MAX_NUM_FILES ]; /* Array of filetypes                   */
        uint32_t        saved_esp;                       /* ESP of parent to return to           */
        uint32_t        saved_ebp;                       /* EBP of parent to return to           */
        uint32_t        active;                          /* Active indicator for scheduling      */
        uint32_t        parent_return_addr;              /* Return address of parent process     */
        uint32_t        saved_eip;                       /* Program counter of parent process    */
        /* SS/ESP fields are saved values for user privilege level.                              */
        uint32_t        esp0;                            /* ESP0 of process, passed down by TSS  */
        uint32_t        ss0;                             /* SS0 of process, passed down by TSS   */
        /* Also store args and size of for later use (like syscall_getargs)                      */
        uint8_t         saved_command[ BUFFER_SIZE ];    /* Saved command for get_args           */  

} pcb_t;

/* Global variable PID - current process the system      */
/* is processing for.                                    */
extern int32_t curr_pid;
extern int32_t active_pid;

/* Can only have 6 processes open outside of STDIN/OUT   */
extern int32_t pid_array[MAX_NUM_FILES - 2];

/* Define System Call Functions. Prototypes provided by  */
/* Appendix B of MP3 Documentation                       */
int32_t syscall_halt( uint8_t status );
int32_t syscall_execute( const uint8_t* command );
int32_t syscall_read( int32_t fd, void* buf, int32_t nbytes );
int32_t syscall_write( int32_t fd, const void* buf, int32_t nbytes );
int32_t syscall_open( const uint8_t* filename );
int32_t syscall_close( int32_t fd );
int32_t syscall_getargs( uint8_t* buf, int32_t nbytes );
int32_t syscall_vidmap( uint8_t** screen_start );
int32_t syscall_set_handler( int32_t signum, void* handler_address );
int32_t syscall_sigreturn( void );

/* Helper functions for our system calls. PCB and map    */
/* are the most prevalent to all system calls.           */
int get_fname( const uint8_t* command );
int get_args( const uint8_t* command );
pcb_t* get_pcb(uint32_t pid);
void switch_context(uint32_t pid);
void map_prog_to_page( int32_t pid );
void close_all_files( void );

/* Arrays for the syscall_execute filename and args.     */
/* Helper functions will update these arrays as needed.  */
char file_name[MAX_FILE_NAME_LENGTH]; 
char cmd_args[MAX_NUM_ARGS][MAX_FILE_NAME_LENGTH]; 
uint32_t cmd_args_length;
#endif
