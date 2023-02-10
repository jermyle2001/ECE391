/*									tab:8
 *
 * text.h - font data and text to mode X conversion utility header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
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
 * Version:	    2
 * Creation Date:   Thu Sep  9 22:08:16 2004
 * Filename:	    text.h
 * History:
 *	SL	1	Thu Sep  9 22:08:16 2004
 *		First written.
 *	SL	2	Sat Sep 12 13:40:11 2009
 *		Integrated original release back into main code base.
 */

#ifndef TEXT_H
#define TEXT_H

/* The default VGA text mode font is 8x16 pixels. */
#define FONT_WIDTH   8
#define FONT_HEIGHT 16


/* These constants were added by the writer for the text_to_buffer function */
#define SCREEN_WIDTH     320     /* Width of Overall Screen is 320 */
#define STATUS_BAR_HEIGHT 18    /* Height of Status Bar is 180*/
#define NUM_PLANES         4    /* We have four image planes to draw to */
#define STATUS_BAR_SIZE 1440    /* Width (320) * Height (18) / # of Planes (4) */
/* In our ASCII Font Data, the length of our row is 16 bits. */
#define CHAR_WIDTH      8
#define CHAR_HEIGHT     16

#define MAX_TYPED_LEN 20

/* Standard VGA text font. */
extern unsigned char font_data[ 256 ][ 16 ];

/* Add Status Bar Buffer for External Usage */
extern unsigned char status_buffer[ 5760 ];

/* Add Text to Buffer Function so we can write characters to buffer */
extern void text_to_buffer( const char* room_status, const char* typed_command, int alignment_mode );

void write_text_to_bar( const char* string, int start );

/* Add an enum so we can understand our alignment better */
enum alignment_mode{ 
    clear, 
    status_msg_mode, 
    room_command 
};

extern enum alignment_mode cur_alignment_mode;
#endif /* TEXT_H */
