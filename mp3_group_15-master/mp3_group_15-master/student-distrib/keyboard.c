#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "syscall.h"

#define TESTMODE 1

/* Keep track of where in the screen we are currently printing  */
/* to. We will create a custom putc and clear function to       */
/* address this.                                                */
int     terminal_x[ NUM_TERMINALS ]; 
int     terminal_y[ NUM_TERMINALS ]; 
char*   terminal_video_mem;

/* Also keep track of the wordcount, and characters typed to    */
/* the keyboard. Since the size of the terminal buffer is 128,  */
/* then the maximum number of allowed character is 127, since   */
/* the last character must be the newline/line feed character.  */
int     word_count[ NUM_TERMINALS ];

/* Also keep track of keyboard buffer. That is, keep track of   */
/* the characters passed into the keyboard so that we can pass  */
/* the values to the terminal later on. The maximum number of   */
/* characters in the buffer is 128. Initialize to '0' on start  */
uint8_t  keyboard_buffer[ NUM_TERMINALS ][ BUFFER_SIZE ];

/* Keep track of the last character in the line printed for     */
/* backspace support, initialized to zero at start.             */
static int  end_of_line[ NUM_ROWS ] = { 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0
};



/* Keep track of whether certain characters were pressed.   */
/* Special characters should be intepreted and processsed   */
/* accordingly. Scancodes for their press and release are   */
/* different, and thus we need to keep track of whether the */
/* special characters were pressed or released.             */
uint8_t shift       = 0;
uint8_t ctrl        = 0;
uint8_t alt         = 0;
uint8_t capslock    = 0;
uint8_t switch_status = 0;


/* Scancode Set 1 from https://wiki.osdev.org/PS/2_Keyboard and https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html */
/* Defines the mapping from scancodes to keycodes, including the SHIFT keys (not implemented in Checkpoint 1) */
unsigned char scancode_to_char[NUM_SCANCODES][SHIFT_PAIR] = {
    {0x0, 0x0},  /* Error Code */
    {ESCAPE, ESCAPE},
    {'1', '!'},  /* Nonshift - 1, Shift - ! */
    {'2', '@'},  /* Nonshift - 2, Shift - @ */
    {'3', '#'},  /* Nonshift - 3, Shift - # */
    {'4', '$'},  /* Continues all the way down */
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {BACKSPACE, BACKSPACE}, 
    {TAB, TAB}, 
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {ENTER, ENTER},
    {LCTRL, LCTRL},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'}, 
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '"'},
    {'`', '~'},
    {LSHIFT, LSHIFT},
    {'\\', '|'},
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {RSHIFT, RSHIFT},
    {KEYPAD_STAR, KEYPAD_STAR},
    {LEFT_ALT, LEFT_ALT},
    {' ', ' '},
    {CAPS, CAPS},
};

/* void keyboard_init( void );
 *   Inputs: none
 *   Return Value: none
 *   Function: Initializes the keyboard IRQ on the Primary PIC */
void keyboard_init( void ) {
    enable_irq(KEYBOARD_IRQ_NUM);

    int i;
    /* Also initialize values for terminal printing */
    for( i = 0; i < NUM_TERMINALS; i++ )
    {
        terminal_x[ i ] = 0;
        terminal_y[ i ] = 0;
    }
    terminal_video_mem = (char *)VIDEO_MEM_LOC;

    /* Also initialize the keyboard buffer and word_count */
    reset_keyboard_buffer( );

    memset( terminal_vid_mem, 0, TERMINAL_MEMORY_SIZE );


}

/* void keyboard_handler( void );
 *   Inputs: none
 *   Return Value: none
 *   Function: Handles keyboard interrupts when a keystroke is pressed to print the corresponding character to the screen */
void keyboard_handler( void ) {
    /* Reads in a physical keystroke */
    unsigned int scancode = inb(KEYBOARD_PORT_IO);
    int scancode_flag = 0;

    /* Update the special characters. Function also tells us if */
    /* the scancode falls outside the acceptable bounds.        */
    scancode_flag = process_type_of_character( scancode );

    /* If scancode_flag is -1, then that indicates our value is */
    /* out of acceptable bounds, and we should end the          */
    /* the interrupt. If scancode_flag is 1, then that          */
    /* indicates we updated special characters, and thus we     */
    /* don't actually have any character to print.              */
    if( ( scancode_flag == -1 ) || ( scancode_flag == 1 ) )
    {
        send_eoi(KEYBOARD_IRQ_NUM);
        return;
    }

    /* Use shift_enable to determine which of the SHIFT PAIR we */
    /* want to display.                                         */
    uint8_t shift_enable;

    /* We only want to create an inversion effect with capslock */
    /* if we're using lettered characters. If not, shift_enable */
    /* should just go with shift. Mask the scancode to ignore   */
    /* key releases.                                            */
    unsigned int scancode_masked = scancode && SCANCODE_MASK;
    if( ( scancode_masked >= KBD_LETTER_ROW1_LEFT && scancode_masked <= KBD_LETTER_ROW1_RIGHT ) ||
        ( scancode_masked >= KBD_LETTER_ROW2_LEFT && scancode_masked <= KBD_LETTER_ROW2_RIGHT ) ||
        ( scancode_masked >= KBD_LETTER_ROW3_LEFT && scancode_masked <= KBD_LETTER_ROW3_RIGHT ) )
    {
        /* If is a letter, then take capslock into account. */
        shift_enable = shift ^ capslock;

    }
    else
    {
        /* If not a letter, don't take capslock into account.   */
        shift_enable = shift;
    }

    /* By XOR'ing CapsLock and Shift, we can create an inversion */
    /* sort of effect, that makes it so either inverts the shift */
    /* of the other.                                             */
    /* Translates the scancode to a keycode for a US QWERTY keyboard (Shifts automatically) */
    unsigned char keycode = scancode_to_char[scancode][shift_enable];

    /* Check if ctrl + l or L was pressed. If so, then clear the screen. */
    if( ctrl )
    {
        /* If scancode is found to be == 'l' then clear screen. */
        if( scancode == 0x26 )
        {
            /* Clear and reset the terminal */
            clear_and_reset_screen( );
            /* Make sure to end the interrupt after cleared */
            send_eoi( KEYBOARD_IRQ_NUM );
            /* Return to avoid printing anything else */
            return;
        }
    }

    if( alt )
    {
        switch( scancode )
        {

            case F1:
                switch_terminal( 0 );
                send_eoi( KEYBOARD_IRQ_NUM );
                return;
            case F2:
                switch_terminal( 1 );
                send_eoi( KEYBOARD_IRQ_NUM );
                return;
            case F3:
                switch_terminal( 2 );
                send_eoi( KEYBOARD_IRQ_NUM );
                return;
            
            #if TESTMODE
            case 0x02:
                switch_terminal( 0 );
                send_eoi( KEYBOARD_IRQ_NUM );
                return;
            case 0x03:
                switch_terminal( 1 );
                send_eoi( KEYBOARD_IRQ_NUM );
                return;
            case 0x04:
                switch_terminal( 2 );
                send_eoi( KEYBOARD_IRQ_NUM );
                return;
            #endif
            
            default:
                break;

        }
    }

    /* Prints the translated scancode character to the screen. */
    if( scancode <= MAX_ACCEPTED_SCANCODE )
    {
        /* ONLY print on keypress, NOT release.                    */
        keyboard_putc( keycode );
    }


    /* Sends end-of-interrupt signal to PIC to notify that we are done handling keyboard interrupt */
    send_eoi( KEYBOARD_IRQ_NUM );
}

/*            process_type_of_character             */
/* Based on Scancode, update the global variables   */
/* For special character tracking at the top of the */
/* file. Returns 0 if no special character updated  */
/* or corresponding to keycode, 1 if special        */
/* character was keycode and subsequently updated,  */
/* and -1 if the keycode fell outside the bounds of */
/* of our mapped characters.                        */
int process_type_of_character( unsigned int scancode )
{
    /* Since the last highest bit of the scancode is just   */
    /* flipped on release, we can check to see if our       */
    /* scancode falls within the range of keypresses we are */
    /* looking for. Mask with 0xFF to ignore flip bit.      */
    unsigned int scancode_check = 0;
    scancode_check = scancode & SCANCODE_MASK;

    /* Check if scancode is NOT one of the special      */
    /* characters we are looking for. Check if the      */
    /* masked scancodes fall out of bounds first.       */
    if( scancode_check > MAX_ACCEPTED_SCANCODE )
    {
        /* Return -1 to indicate that scancode is       */
        /* outside accepted bounds                      */
        return -1;
    }

    /* Ignore TAB and ESCAPE for now. Also ignore keypad pressed.   */
    if( scancode_check == ESCAPE_PRESSED || scancode_check == TAB_PRESSED || scancode_check == KEYPAD_STAR_PRESSED )
    {
        /* Return -1 to indicate that the scnadoe should    */
        /* be ignored.                                      */
        return -1;
    }

    /* Check if keycode pertains any special characters. If */
    /* the keycode exceeds the bounds of normal characters, */
    /* then it is either a special charcacter, or an        */
    /* unmapped character. */
    if( scancode != LEFT_CTRL_PRESSED     && 
        scancode != LEFT_CTRL_RELEASED    &&
        scancode != LEFT_SHIFT_PRESSED    &&
        scancode != LEFT_SHIFT_RELEASED   &&
        scancode != RIGHT_SHIFT_PRESSED   &&
        scancode != RIGHT_SHIFT_RELEASED  &&
        scancode != LEFT_ALT_PRESSED      &&
        scancode != LEFT_ALT_RELEASED     &&
        scancode != CAPS_LOCK_PRESSED     &&
        scancode != CAPS_LOCK_RELEASED        )
    {
        /* Else, return 0 to indicate that scancode     */
        /* should be processed normally.                */
        return 0;
    }

    /* Process the scancode as appropriate. If the scancode */
    /* tells us that a key is being pressed that is alraedy */
    /* being pressed, then update anyways, since we want to */
    /* keep indicating the key is being pressed.            */
    /* A value of 1 indicates the key is being pressed, and */
    /* a value of 0 indiactes the key is NOT being pressed. */
    switch( scancode )
    {
        case LEFT_CTRL_PRESSED:
            ctrl = 1;
            break;
        case LEFT_CTRL_RELEASED:
            ctrl = 0;
            break;
        case LEFT_SHIFT_PRESSED:
            shift = 1;
            break;
        case LEFT_SHIFT_RELEASED:
            shift = 0;
            break;
        case RIGHT_SHIFT_PRESSED:
            shift = 1;
            break;
        case RIGHT_SHIFT_RELEASED:
            shift = 0;
            break;
        case LEFT_ALT_PRESSED:
            alt = 1;
            break;
        case LEFT_ALT_RELEASED:
            alt = 0;
            break;
        /* Invert CapsLock ONLY when it gets pressed.   */
        /* Since CapsLock is toggled, we only want to   */
        /* invert on keypress.                          */
        case CAPS_LOCK_PRESSED:
            /* Invert bits and mask to leave only last  */
            /* bit remaining.                           */
            capslock = ( ~capslock ) & CAPSLOCK_MASK;
            break;
        default:
            break;
    }

    return 1;

}

/*             void clear_and_reset( void )             */
/* Description: clears the video memory, and resets the */
/* printing location / mouse location to the top left   */
/* corner of the screen after call. Modified version of */
/* void clear( void ) from lib.c                        */
/* Inputs: none                                         */
/* Outputs: none                                        */
/* Side Effects: Clears the video memory and resets     */
/* terminal_y and terminal_x. Does NOT reset the        */
/* keyboard or terminal buffer                          */
void clear_and_reset_screen( void )
{
    int32_t i;
    /* Clear video memory by setting all to ' ' and ATTRIB.     */
    for( i = 0; i < NUM_ROWS * NUM_COLS; i++ )
    {
        *(uint8_t *)(VIDEO_MEM_LOC + (i << 1)) = ' ';
        *(uint8_t *)(VIDEO_MEM_LOC + (i << 1) + 1) = ATTRIB;
    }

    /* Reset x and y values so that we can print to the         */
    /* right place.                                             */
    terminal_x[ display_terminal ] = 0;
    terminal_y[ display_terminal ] = 0;

    /* Also set end_of_line tracker */
    for( i = 0; i < NUM_ROWS; i++ )
    {
        end_of_line[ i ] = 0;
    }

    /* Finally, reset the cursor. */
    terminal_print_cursor( terminal_y[ display_terminal ], terminal_x[ display_terminal ] );

    return;
}

/*           void terminal_putc( uint8_t c )            */
/* Description: prints the character to the console.    */
/* Customized to handle newlines, backspace, line       */
/* overflow.                                            */
/* Will support terminal buffer in the future...        */
/* Inputs: c -> character to be printed                 */
/* Outputs: None.                                       */
/* Side Effects: prints given character to screen, or   */
/* deletes a character from the screen, or scrolls the  */
/* screen, depending on what is passsed in, and the     */
/* current x and y location.                            */
void keyboard_putc( uint8_t c )
{    
    /* First, check if the buffer is full. If so, then  */
    /* do NOT allow more printing to occur. However, we */
    /* want to allow '\n' and BACKSPACE, since we want  */
    /* to be able to remove characters from the buffer, */
    /* and use '\n' to "enter" the command to the       */
    /* terminal.                                        */
    if( ( word_count[ display_terminal ] >= BUFFER_SIZE - 1 ) && ( c != '\n' && c != BACKSPACE ) )
    {
        /* Do not nothing if buffer full. Since the last character  */
        /* in the buffer must be '\n', we want to reserve the very  */
        /* last index of the buffer for such.                       */
        
        /* Update the cursor */
        terminal_print_cursor( terminal_y[ display_terminal ], terminal_x[ display_terminal ] );

        return;
    }

    /* Also don't allow printing if the buffer is empty and we      */
    /* attempt to delete a character.                               */
    if( ( word_count[ display_terminal ] == 0 ) && ( c == BACKSPACE ) )
    {
        terminal_print_cursor( terminal_y[ display_terminal ], terminal_x[ display_terminal ] );

        return;
    }
    

    /* First, check if newline passed through. If so,   */
    /* move characters to new line and reset x value.   */
    /* Additionally, if printing causes the line to run */
    /* out, then go to the next line if available.      */
    /* NOTE: Currently configured NOT to support new    */
    /* rows, i.e. if at bottom of screen, remain at     */
    /* bottom.                                          */
    if( c == '\n' || c == '\r' )
    {
        /* If NOT at bottom of screen, go to new line.  */
        if( terminal_y[ display_terminal ] != NUM_ROWS - 1 )
        {
            /* Set end of line to terminal_x - 1, since */
            /* terminal_x and terminal_y represent the  */
            /* next printable space.                    */
            if( terminal_x[ display_terminal ] != 0 )
            {
                end_of_line[ terminal_y[ display_terminal ] ] = terminal_x[ display_terminal ] - 1;
            }
            /* Set y row to next row and x to start of  */
            /* row.                                     */
            terminal_y[ display_terminal ] = ( terminal_y[ display_terminal ] + 1 ) % NUM_ROWS;
            terminal_x[ display_terminal ] = 0;
        }
        else
        {
            /* Else, we are at the bottom of the screen.    */
            /* Add a newline by scrolling the screen down   */
            /* and resetting the terminal_x value.          */
            /* Also, update end of line tracker.            */
            if( terminal_x[ display_terminal ] != 0 )
            {
                end_of_line[ terminal_y[ display_terminal ] ] = terminal_x[ display_terminal ] - 1;
            }
            else
            {
                end_of_line[ terminal_y[ display_terminal ] ] = terminal_x[ display_terminal ];
            }
            /* Scroll screen.                               */
            scroll_screen( );
        }

        /* Update the keyboard buffer by passing in the character   */
        /* into the buffer. Also update the word_count. These two   */
        /* operations will go towards the terminal support.         */
        keyboard_buffer[ display_terminal ][ word_count[ display_terminal ] ] = c;
        word_count[ display_terminal ]++;

        /* Set the read_ready flag to indicate that the enter key   */
        /* has been pressed.                                        */
        read_ready = 1;

    }
    /* Check if BACKSPACE was passed through.       */
    /* If so, then replace last char with ' '. We   */
    /* should support backspace into the previous   */
    /* line as well.                                */
    else if( c == BACKSPACE )
    {
        /* Check if backspace will move into previous   */
        /* line. If so, change x and y accordingly.     */

        /* If terminal_x is at zero, then the last char */
        /* printed was on the previous line. Check if   */
        /* at top of screen. If so, do nothing. Else,   */
        /* delete from end of last line.                */
        if( terminal_x[ display_terminal ] == 0 )
        {
            /* Do nothing if at top-left corner of screen. */
            if( terminal_y[ display_terminal ] == 0 )
            {
                /* Update end of line tracker to be beginning of line */
                end_of_line[ terminal_y[ display_terminal ] ] = 0;
                return;
            }
            /* Else, get location of last printed character. */
            else
            {
                /* Update end of line for current line */
                end_of_line[ terminal_y[ display_terminal ] ] = 0;

                /* Update terminal_x to print to the right space. */
                terminal_x[ display_terminal ] = end_of_line[ terminal_y[ display_terminal ] - 1 ];
                /* Also update terminal_y to prev line. */
                terminal_y[ display_terminal ]--;
            }
        }
        else
        {
            terminal_x[ display_terminal ]--;
            if( terminal_x[ display_terminal ] != 0 )
            {
                end_of_line[ terminal_y[ display_terminal ] ] = terminal_x[ display_terminal ];
            }
            else
            {
                end_of_line[ terminal_y[ display_terminal ] ] = 0;
            }
        }
        /* Set c to ' ' to figuratively "delete" the last character.     */
        c = ' ';
        /* Print over character pointed to by terminal_y and terminal_x. */
        *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * terminal_y[ display_terminal ] + terminal_x[ display_terminal ]) << 1)) = c;
        *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * terminal_y[ display_terminal ] + terminal_x[ display_terminal ]) << 1) + 1) = ATTRIB;

        /* Decrease wordcount. Since this section is already configured */
        /* to return if at top-left corner, we can safely decrement the */
        /* word_count, since this part is designed to make sure that a  */
        /* character exists that can be deleted.                        */
        word_count[ display_terminal ]--;

        /* Remove the character from the keyboard buffer. */
        keyboard_buffer[ display_terminal ][ word_count[ display_terminal ] ] = 0;
    }
    /* Check if printing a character at the current terminal_x value    */
    /* prints outside of the allowed bounds. If so, scroll to the next  */
    /* line.                                                            */
    else if( terminal_x[ display_terminal ] >= NUM_COLS )
    {
        /* Reset the value of terminal_x to zero and move to the next   */
        /* line. Scroll screen if necessary.                            */
        if( terminal_y[ display_terminal ] != NUM_ROWS - 1 )
        {
            /* Not at bottom of screen, no need to scroll. Find the     */
            /* next y value and reset x to zero.                        */
            terminal_y[ display_terminal ] = ( terminal_y[ display_terminal ] + 1 ) % NUM_ROWS;
        }
        else
        {
            /* Scroll the screen and reset x to the beginning of line.  */   
            scroll_screen( );
        }
        terminal_x[ display_terminal ] = 0;

        /* Print at the current location, then update the values of x   */
        /* and y accordingly.                                           */
        *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * terminal_y[ display_terminal ] + terminal_x[ display_terminal ]) << 1)) = c;
        *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * terminal_y[ display_terminal ] + terminal_x[ display_terminal ]) << 1) + 1) = ATTRIB;
        terminal_x[ display_terminal ]++;

        /* Also update the end of line tracker, add the character to    */
        /* the keyboard buffer, and increase the word count.            */
        end_of_line[ terminal_y[ display_terminal ] ] = terminal_x[ display_terminal ];
        keyboard_buffer[ display_terminal ][ word_count[ display_terminal ] ] = c;
        word_count[ display_terminal ]++;
    }
    /* Else, print the charcater normally and increment the values of   */
    /* terminal_x and terminal_y accordingly.                           */
    else
    {
        /* Assumes that the value of x is valid for printing. Don't     */
        /* worry about line overflow, as that should be supported by    */
        /* the previous if statement.                                   */

        /* Update the end of line tracker before printing.              */
        end_of_line[ terminal_y[ display_terminal ] ] = terminal_x[ display_terminal ];
        /* Print the character to the screen at the current location    */
        /* determined by terminal_x. terminal_x should not be able to   */
        /* overflow, and thus we can print without worry.               */
        *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * terminal_y[ display_terminal ] + terminal_x[ display_terminal ]) << 1)) = c;
        *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * terminal_y[ display_terminal ] + terminal_x[ display_terminal ]) << 1) + 1) = ATTRIB;
        terminal_x[ display_terminal ]++;

        /* Also add the character to the keyboard buffer and increment  */
        /* the word_count for tracking.                                 */
        keyboard_buffer[ display_terminal ][ word_count[ display_terminal ] ] = c;
        word_count[ display_terminal ]++;
    }

    /* Also, update cursor */
    terminal_print_cursor( terminal_y[ display_terminal ], terminal_x[ display_terminal ] );
}

/*             terminal_print_cursor                */
/* Prints the cursor to the terminal, moving it to the position */
/* pointed to by terminal_y and terminal_x. Code referenced     */
/* from osdev.org. Link:                                        */
/* https://wiki.osdev.org/Text_Mode_Cursor#Moving_the_Cursor    */
/* Inputs: target_row, target_col. Would use terminal_y and     */
/* terminal_x instead and give the function no inputs, but this */
/* allows the function to actually be called outside of the     */
/* context of the keyboard.                                     */
/* Outputs: none.                                               */
/* Side Effects: Updates the cursor to the location pointed to  */
/* by terminal_y and terminal_x                                 */
void terminal_print_cursor( int cur_row, int cur_col )
{
    /* Cursor works by enabling the scanlines / rows where the  */    
    /* cursor starts and ends.                                  */
    /* Find the position on the VGA that corresponds to where   */
    /* the cursor should be.                                    */
    uint16_t cursor_position = NUM_COLS * cur_row + cur_col;

    /* Set the VGA registers accordingly using the outb         */
    /* function provided by lib.h. Make sure to connect to the  */
    /* appropriate VGA registers.                               */
    /* Honestly, some kind of VGA magic provided by OSDEV is    */
    /* present here.                                            */
    /* Note: the syntax for outb is OPPOSITE of OSDEV.org. Here */
    /* the format is outb( data, port ).                        */
    /* Set lower part of cursor                                 */
    outb( CURSOR_LOW, VGA_BASE1 );
    
    /* Mask position and send to VGA_BASE2                      */
    uint16_t cursor_position_masked = cursor_position & BYTE_MASK;
    outb( cursor_position_masked, VGA_BASE2 );

    /* Set higher part of cursor                                */
    outb( CURSOR_HIGH, VGA_BASE1 );

    /* Shift and mask position and send to VGA_BASE2            */
    uint16_t cursor_position_shifted_masked = ( cursor_position >> 8 ) & BYTE_MASK;
    outb( cursor_position_shifted_masked, VGA_BASE2 );
}

/*              void scroll_screen( void )                  */
/* Scrolls the screen, adding another line to the bottom of */
/* the screen while erasing the top line of the screen.     */
/* Inputs: none.                                            */
/* Outputs: none.                                           */
/* Side Effects: Scrolls the screen. May erase lines from   */
/* the top to make room for the bottom. Used in             */
/* keyboard_putc to implement newline scrolling.            */
void scroll_screen( void )
{
    /* Accomplish scrolling by shifting video memory up the */
    /* screen. We do not have to account for history, nor   */
    /* support scrolling the screen up. We only need to     */
    /* support scrolling the screen down.                   */

    /* Scroll the video memory up */
    int cur_row;
    int cur_col;
    for( cur_row = 0; cur_row < NUM_ROWS - 1; cur_row++ )
    {
        for( cur_col = 0; cur_col < NUM_COLS; cur_col++ )
        {
            /* Take on the memory of the row below the current row. */
            *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * cur_row + cur_col) << 1)) = *(uint8_t *)(terminal_video_mem + ((NUM_COLS * ( cur_row + 1 ) + cur_col) << 1));
            *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * cur_row + cur_col) << 1) + 1) = *(uint8_t *)(terminal_video_mem + ((NUM_COLS * ( cur_row + 1 ) + cur_col) << 1) + 1);
        }
    }

    cur_row = NUM_ROWS - 1;
    uint8_t c = ' ';
    /* On the last row, set the values to blank. */
    for( cur_col = 0; cur_col < NUM_COLS; cur_col++ )
    {
        *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * cur_row + cur_col) << 1)) = c;
        *(uint8_t *)(VIDEO_MEM_LOC + ((NUM_COLS * cur_row + cur_col) << 1) + 1) = ATTRIB;
        
    }

    int i;
    /* Shift the values in the end of line buffer to        */
    /* account for the scrolling                            */
    for( i = 0; i < NUM_ROWS - 1; i++ )
    {
        end_of_line[ i ] = end_of_line[ i + 1 ];
    }
    end_of_line[ NUM_ROWS - 1 ] = 0;

    /* Also reset terminal x and y values just in case... */
    terminal_x[ display_terminal ] = 0;
    terminal_y[ display_terminal ] = NUM_ROWS - 1;

    /* And don't forget to update the cursor after scrolling. */
    terminal_print_cursor( terminal_y[ display_terminal ], terminal_x[ display_terminal ] );

}


/* void put_string( uint8_t* string )   */
/* Description: prints a string to the  */
/* console using the putc command       */
/* repeatedly.                          */
/* Inputs: uint8_t* string -> string to */
/* print                                */
/* Outputs: none                        */
/* Side effects: prints the string to   */
/* the screen, and updates the terminal */
/* x and y coords accordingly. May      */
/* scroll the screen if needed.         */
void put_string( const uint8_t* string )
{
    int32_t index = 0;
    while( string[ index ] != '\0' )
    {
        keyboard_putc( string[ index ] );
        index++;
    }
}


/*                 reset_keyboard_buffer                    */
/* Resets the keyboard buffer, initializing all of its      */
/* contents to 0, which we will use in determining whether  */
/* we have reached the end of the buffer or not.            */
/* Inputs: None.                                            */
/* Outputs: None.                                           */
/* Side Effects: Clears keyboard_buffer and word_count.     */
void reset_keyboard_buffer( void )
{
    /* Reset keyboard_buffer to 0 on request */
    int i;
    for( i = 0; i < BUFFER_SIZE; i++ )
    {
        keyboard_buffer[ display_terminal ][ i ] = 0;
    }
    /* Also reset the word_count */
    word_count[ display_terminal ] = 0;

}


