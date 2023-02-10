#ifndef _TERMINAL_H
#define _TERMINAL_H

#define BUFFER_SIZE             128     /* Buffer size and number of terminals      */
#define NUM_TERMINALS           3       /* outlined by MP3 documentation.           */
#define SCREEN_SIZE             4096    /* Define the screen size as 4096 to avoid  */
                                        /* accidentally overlapping video memory.   */
#define VGA_HIGH_BYTE_OFF       0x0C
#define VGA_LOW_BYTE_OFF        0x0D
#define BYTE_0_MASK             0xFF    /* Mask used to mask Byte 0.                */
#define HIGH_BYTE_SHIFT         8       /* Used to shift high byte into lower byte. */

#define TERMINAL_MEMORY_START   0xB9000
#define TERMINAL_MEMORY_SIZE    0x1000

extern uint8_t  terminal_buffer[ BUFFER_SIZE ];
extern uint32_t terminal_vid_mem[ NUM_TERMINALS ][ TERMINAL_MEMORY_SIZE ];
extern uint32_t read_ready;

/* Struct of terminal and contains necessary info for scheduler  */
typedef struct terminal_t {
    uint32_t num_processes;                         /* Keep track of num current processes to make sure no more than 6  */
    uint8_t  terminal_buffer[ TERMINAL_MEMORY_SIZE ];    /* Instance of a terminal buffer for each of the terminals          */
    
    uint32_t initialized;                     /* Flag to determine if a terminal has already been initialized */
    int32_t  pid;                             /* Process ID # for the current process */
    uint32_t saved_esp;                       /* ESP of parent to return to           */
    uint32_t saved_ebp;                       /* EBP of parent to return to           */
} terminal_t;

terminal_t terminals[NUM_TERMINALS];
int32_t sched_terminal;
int32_t display_terminal;

/* Basic Terminal Functions. A majority of the support  */
/* actually arises from the keyboard driver.            */
extern int32_t terminal_open( const uint8_t* filename );
extern int32_t terminal_close( int32_t fd );
extern int32_t terminal_read( int32_t fd, void* buf, int32_t nbytes );
extern int32_t terminal_write( int32_t fd, const void* buf, int32_t nbytes );
extern  void   switch_terminal( uint32_t terminal_target_index );
extern  void   terminals_init( void );

#endif
