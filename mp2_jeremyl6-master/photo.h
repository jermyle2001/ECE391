/*									tab:8
 *
 * photo.h - photo display header file
 *
 * "Copyright (c) 2011 by Steven S. Lumetta."
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
 * Creation Date:   Fri Sep  9 21:45:34 2011
 * Filename:	    photo.h
 * History:
 *	SL	1	Fri Sep  9 21:45:34 2011
 *		First written.
 *	SL	2	Sun Sep 11 09:59:23 2011
 *		Completed initial implementation.
 *	SL	3	Wed Sep 14 21:56:08 2011
 *		Cleaned up code for distribution.
 */
#ifndef PHOTO_H
#define PHOTO_H


#include <stdint.h>

#include "types.h"
#include "modex.h"
#include "photo_headers.h"
#include "world.h"

/* limits on allowed size of room photos and object images */
#define MAX_PHOTO_WIDTH   1024
#define MAX_PHOTO_HEIGHT  1024
#define MAX_OBJECT_WIDTH  160
#define MAX_OBJECT_HEIGHT 100


/* Fill a buffer with the pixels for a horizontal line of current room. */
extern void fill_horiz_buffer (int x, int y, unsigned char buf[SCROLL_X_DIM]);

/* Fill a buffer with the pixels for a vertical line of current room. */
extern void fill_vert_buffer (int x, int y, unsigned char buf[SCROLL_Y_DIM]);

/* Get height of object image in pixels. */
extern uint32_t image_height (const image_t* im);

/* Get width of object image in pixels. */
extern uint32_t image_width (const image_t* im);

/* Get height of room photo in pixels. */
extern uint32_t photo_height (const photo_t* p);

/* Get width of room photo in pixels. */
extern uint32_t photo_width (const photo_t* p);

/* 
 * Prepare room for display (record pointer for use by callbacks, set up
 * VGA palette, etc.). 
 */
extern void prep_room (const room_t* r);

/* Read object image from a file into a dynamically allocated structure. */
extern image_t* read_obj_image (const char* fname);

/* Read room photo from a file into a dynamically allocated structure. */
extern photo_t* read_photo (const char* fname);

/* 
 * N.B.  I'm aware that Valgrind and similar tools will report the fact that
 * I chose not to bother freeing image data before terminating the program.
 * It's probably a bad habit, but ... maybe in a future release (FIXME).
 * (The data are needed until the program terminates, and all data are freed
 * when a program terminates.)
 */

/* Octtree implementation included here! */
/* Declare struct for each level of the Octtree */
// struct octtree_array
// {

// };


struct octree_node
{
    /* The color space is broken up into "level 4" octcubes, meaning that each color axis */
    /* is divided into 16 parts, for a total of 16 x 16 x 16 = 4096 Level 4 octcubes.     */
    /* Set asde 64 colors for the 64 nodes of the second level of an Octree, then use the */
    /* remaining colors (in this case, 128 of them) to represent those nodes in the       */
    /* fourth level of an octree that contains the most pixels from the image.            */
    /* In other words, you choose the 128 colors that comprise as much of the image as    */
    /* possible and assign color values for them. */
    unsigned int node_index;
    unsigned int node_index_sorted;
    unsigned int num_pixels;
    unsigned int red_total;
    unsigned int green_total;
    unsigned int blue_total;
} octree_node;


/* Octree Constants! */
#define NUM_NODES_LEVEL4 4096
#define NUM_NODES_LEVEL2 64
#define LEVEL4_INIT_PALETTE 128 

#define RED_MASK_LVL4 0xF000
#define GREEN_MASK_LVL4 0x0780
#define BLUE_MASK_LVL4 0x001E

#define RED_CONCATENATE_SHIFT_LVL4 4
#define GREEN_CONCATENATE_SHIFT_LVL4 3
#define BLUE_CONCATENATE_SHIFT_LVL4 1

#define RED_MASK_TOTAL_LVL4 0xF800
#define GREEN_MASK_TOTAL_LVL4 0x07E0
#define BLUE_MASK_TOTAL_LVL4 0x001F

#define RED_TOTAL_SHIFT_LVL4 11
#define GREEN_TOTAL_SHIFT_LVL4 5

#define RED_MASK_LVL2 0x0C00
#define GREEN_MASK_LVL2 0x00C0
#define BLUE_MASK_LVL2 0x000C

#define RED_CONCATENATE_SHIFT_LVL2 6
#define GREEN_CONCATENATE_SHIFT_LVL2 4
#define BLUE_CONCATENATE_SHIFT_LVL2 2

#define RED_TOTAL_SHIFT_LVL2 4
#define GREEN_TOTAL_SHIFT_LVL2 2

#define PALETTE_RED_INDEX 0
#define PALETTE_GREEN_INDEX 1
#define PALETTE_BLUE_INDEX 2

#define LVL2_PALETTE_OFFSET 128
#define LVL2_SIZE 64

#define PALETTE_SHIFT 1

#define PIXEL_RED_TOTAL_SHIFT 10
#define PIXEL_GREEN_TOTAL_SHIFT 5
#define PIXEL_BLUE_TOTAL_SHIFT 1
#define PIXEL_RED_DISTANCE_MASK 0xF800
#define PIXEL_GREEN_DISTANCE_MASK 0x07E0
#define PIXEL_BLUE_DISTANCE_MASK 0x001F
#define PALETTE_DISTANCE_SHIFT 1
#define PALETTE_SIZE 192
#define VGA_ESSENTIAL_REGISTER_OFFSET 64

#define NO_COLOR 0

// 0bRRRRRGGGGGGBBBBB
//     0bRRRRGGGGBBBB

/* Functions relevant to octrees */
/* Function initializes values of level 4 and level 2 octrees */
void octree_init_level4_and_level2( );
/* Function provided to qsort compare two pixels */
int compare_num_pixels( const void* a, const void* b );

unsigned int find_closest_palette_color( unsigned int pixel, const photo_t* p );

#endif /* PHOTO_H */
