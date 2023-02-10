/* Include statements */
#include "i8259.h"
#include "terminal.h"

/* Define Constants */
#define KEYBOARD_IRQ_NUM        1
#define KEYBOARD_PORT_IO        0x60
#define KEYBOARD_PORT_CS        0x64
#define NUM_SCANCODES           59
#define SHIFT_PAIR              2
#define NO_SHIFT                0

/* Defined key constants. Not the scancode, */
/* but rather the ASCII value.              */
#define ESCAPE      0x1B
#define BACKSPACE   0x08
#define TAB         0x09
#define ENTER       0x0A
#define LCTRL       0x1D 
#define LSHIFT      0x2A  
#define RSHIFT      0x0  
#define KEYPAD_STAR 0x0     /* May need to handle later */
#define LEFT_ALT    0x38 
#define RIGHT_ALT   0xE0
#define CAPS        0x3A  
#define F1          0x3B
#define F2          0x3C
#define F3          0x3D

/* Constants used to access the VGA registers   */
/* in order to update the cursor.               */
#define BYTE_MASK   0xFF		
#define CURSOR_LOW  0x0F
#define CURSOR_HIGH 0x0E
#define VGA_BASE1   0x3D4		
#define VGA_BASE2   0x3D5
/* Define video memory constants*/
#define VIDEO_MEM_LOC           0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7

/* Define Special Character Key Constants*/
#define ESCAPE_PRESSED          0x01
#define ESCAPE_RELEASED         0x81
#define BACKSPACE_PRESSED       0x0E
#define BACKSPACE_RELEASED      0x8F
#define TAB_PRESSED             0x0F
#define TAB_RELEASED            0x8F
#define LEFT_CTRL_PRESSED       0x1D
#define LEFT_CTRL_RELEASED      0x9D
#define LEFT_SHIFT_PRESSED      0x2A
#define LEFT_SHIFT_RELEASED     0xAA
#define RIGHT_SHIFT_PRESSED     0x36
#define RIGHT_SHIFT_RELEASED    0xB6
#define CAPS_LOCK_PRESSED       0x3A
#define CAPS_LOCK_RELEASED      0xBA
#define LEFT_ALT_PRESSED        0x38
#define LEFT_ALT_RELEASED       0xB8

/* Special keys to ignore */
#define KEYPAD_STAR_PRESSED     0x37

/* Other keyboard and scancode constants    */
#define SCANCODE_MASK           0x3F
#define CAPSLOCK_MASK           0x01
#define MAX_ACCEPTED_SCANCODE   0x3D
#define KBD_LETTER_ROW1_LEFT    0x10
#define KBD_LETTER_ROW1_RIGHT   0x19
#define KBD_LETTER_ROW2_LEFT    0x1E
#define KBD_LETTER_ROW2_RIGHT   0x26
#define KBD_LETTER_ROW3_LEFT    0x2C
#define KBD_LETTER_ROW3_RIGHT   0x32


#define BUFFER_SIZE             128


/* Declare global variables. terminal_x and terminal_y  */
/* are extern so that we can update them in other       */
/* functions.                                           */
extern int      terminal_x[ NUM_TERMINALS ];
extern int      terminal_y[ NUM_TERMINALS ];
extern char*    terminal_video_mem;
extern uint8_t  keyboard_buffer[ NUM_TERMINALS ][ BUFFER_SIZE ];
extern int      word_count[ NUM_TERMINALS ];

/* Declare functions */

/* Initializes the keyboard */
extern void keyboard_init( void );

/* Handles keyboard interrupts when a keystroke is pressed to print the corresponding character to the screen */
extern void keyboard_handler( void );

/* Helper function for keyboard handler - updates the special scancodes we're looking for. */
extern int process_type_of_character( unsigned int scancode ); 

/* Helper function to clear the screen and reset the printing location */
extern void clear_and_reset_screen( void );

/* Helper function to print character to screen. Modified version of putc. */
extern void keyboard_putc( uint8_t c );

/* Function to print cursor to screen */
extern void terminal_print_cursor( int cur_row, int cur_col );

/* Function to scroll the screen */
extern void scroll_screen( void );

/* Function to print a string to the screen. Follows very closely to puts. */
extern void put_string( const uint8_t* string );

/* Function to reset the keyboard buffer. */
extern void reset_keyboard_buffer( void );



