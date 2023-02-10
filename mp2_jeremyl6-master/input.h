/*									tab:8
 *
 * input.h - header file for input control to maze game
 *
 * "Copyright (c) 2004-2011 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:	    Steve Lumetta
 * Version:	    3
 * Creation Date:   Thu Sep  9 22:22:00 2004
 * Filename:	    input.h
 * History:
 *	SL	1	Thu Sep  9 22:22:00 2004
 *		First written.
 *	SL	2	Sun Sep 13 04:11:44 2009
 *		Changed display interface for Tux controller.
 *	SL	3	Wed Sep 14 02:06:59 2011
 *		Updated command names and numbers for adventure game.
 */

#ifndef INPUT_H
#define INPUT_H

/* possible commands from input device, whether keyboard or game controller */
typedef enum {
    CMD_NONE, CMD_RIGHT, CMD_LEFT, CMD_UP, CMD_DOWN,
    CMD_MOVE_LEFT, CMD_ENTER, CMD_MOVE_RIGHT,
    CMD_TYPED,
    CMD_QUIT,
    NUM_COMMANDS
} cmd_t;

#define MAX_TYPED_LEN 20

#define MIN_TENS_SHIFT 12
#define MIN_ONES_SHIFT 8
#define SEC_TENS_SHIFT 4
#define ZERO_LEDS_ON 0x00000000
#define ONE_LED_ON 0x00010000
#define TWO_LEDS_ON 0x00030000
#define THREE_LEDS_ON 0x00070000
#define FOUR_LEDS_ON 0x000F0000
#define INIT_ZERO 0x00

#define TUX_SET_LED _IOR('E', 0x10, unsigned long)
#define TUX_BUTTONS _IOW('E', 0x12, unsigned long*)
#define TUX_INIT _IO('E', 0x13)

/* Initialize the input device. */
extern int init_input ();

/* Read a command from the input device. */
extern cmd_t get_command ();

/* Get currently typed command string. */
extern const char* get_typed_command ();

/* Reset typed command. */
extern void reset_typed_command ();

/* Shut down the input device. */
extern void shutdown_input ();

/*
 * Show the elapsed seconds on the Tux controller (no effect when
 * compiled for a keyboard).
 */
extern void display_time_on_tux (int num_seconds);

extern int 
ioctl_button_helper( );

#endif /* INPUT_H */
