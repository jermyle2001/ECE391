/*									tab:8
 *
 * photo.c - photo display functions
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
 * Creation Date:   Fri Sep  9 21:44:10 2011
 * Filename:	    photo.c
 * History:
 *	SL	1	Fri Sep  9 21:44:10 2011
 *		First written (based on mazegame code).
 *	SL	2	Sun Sep 11 14:57:59 2011
 *		Completed initial implementation of functions.
 *	SL	3	Wed Sep 14 21:49:44 2011
 *		Cleaned up code for distribution.
 */


#include <string.h>

#include "assert.h"
#include "modex.h"
#include "photo.h"
#include "photo_headers.h"
#include "world.h"

/* GLOBAL OCTREE ELEMENTS */
struct octree_node octree_level4[ NUM_NODES_LEVEL4 ];
struct octree_node octree_level2[ NUM_NODES_LEVEL2 ];


/* types local to this file (declared in types.h) */

/* 
 * A room photo.  Note that you must write the code that selects the
 * optimized palette colors and fills in the pixel data using them as 
 * well as the code that sets up the VGA to make use of these colors.
 * Pixel data are stored as one-byte values starting from the upper
 * left and traversing the top row before returning to the left of
 * the second row, and so forth.  No padding should be used.
 */
struct photo_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t        palette[192][3];     /* optimized palette colors */
    uint8_t*       img;                 /* pixel data               */
};

/* 
 * An object image.  The code for managing these images has been given
 * to you.  The data are simply loaded from a file, where they have 
 * been stored as 2:2:2-bit RGB values (one byte each), including 
 * transparent pixels (value OBJ_CLR_TRANSP).  As with the room photos, 
 * pixel data are stored as one-byte values starting from the upper 
 * left and traversing the top row before returning to the left of the 
 * second row, and so forth.  No padding is used.
 */
struct image_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t*       img;                 /* pixel data               */
};


/* file-scope variables */

/* 
 * The room currently shown on the screen.  This value is not known to 
 * the mode X code, but is needed when filling buffers in callbacks from 
 * that code (fill_horiz_buffer/fill_vert_buffer).  The value is set 
 * by calling prep_room.
 */
static const room_t* cur_room = NULL; 


/* 
 * fill_horiz_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the leftmost 
 *                pixel of a line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- leftmost pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_horiz_buffer (int x, int y, unsigned char buf[SCROLL_X_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgx;  /* loop index over pixels in object image      */ 
    int            yoff;  /* y offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_X_DIM; idx++) {
        buf[idx] = (0 <= x + idx && view->hdr.width > x + idx ?
		    view->img[view->hdr.width * y + x + idx] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (y < obj_y || y >= obj_y + img->hdr.height ||
	    x + SCROLL_X_DIM <= obj_x || x >= obj_x + img->hdr.width) {
	    continue;
	}

	/* The y offset of drawing is fixed. */
	yoff = (y - obj_y) * img->hdr.width;

	/* 
	 * The x offsets depend on whether the object starts to the left
	 * or to the right of the starting point for the line being drawn.
	 */
	if (x <= obj_x) {
	    idx = obj_x - x;
	    imgx = 0;
	} else {
	    idx = 0;
	    imgx = x - obj_x;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_X_DIM > idx && img->hdr.width > imgx; idx++, imgx++) {
	    pixel = img->img[yoff + imgx];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/* 
 * fill_vert_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the top pixel of 
 *                a vertical line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- top pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_vert_buffer (int x, int y, unsigned char buf[SCROLL_Y_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgy;  /* loop index over pixels in object image      */ 
    int            xoff;  /* x offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_Y_DIM; idx++) {
        buf[idx] = (0 <= y + idx && view->hdr.height > y + idx ?
		    view->img[view->hdr.width * (y + idx) + x] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (x < obj_x || x >= obj_x + img->hdr.width ||
	    y + SCROLL_Y_DIM <= obj_y || y >= obj_y + img->hdr.height) {
	    continue;
	}

	/* The x offset of drawing is fixed. */
	xoff = x - obj_x;

	/* 
	 * The y offsets depend on whether the object starts below or 
	 * above the starting point for the line being drawn.
	 */
	if (y <= obj_y) {
	    idx = obj_y - y;
	    imgy = 0;
	} else {
	    idx = 0;
	    imgy = y - obj_y;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_Y_DIM > idx && img->hdr.height > imgy; idx++, imgy++) {
	    pixel = img->img[xoff + img->hdr.width * imgy];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/* 
 * image_height
 *   DESCRIPTION: Get height of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_height (const image_t* im)
{
    return im->hdr.height;
}


/* 
 * image_width
 *   DESCRIPTION: Get width of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_width (const image_t* im)
{
    return im->hdr.width;
}

/* 
 * photo_height
 *   DESCRIPTION: Get height of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_height (const photo_t* p)
{
    return p->hdr.height;
}


/* 
 * photo_width
 *   DESCRIPTION: Get width of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_width (const photo_t* p)
{
    return p->hdr.width;
}


/* 
 * prep_room
 *   DESCRIPTION: Prepare a new room for display.  You might want to set
 *                up the VGA palette registers according to the color
 *                palette that you chose for this room.
 *   INPUTS: r -- pointer to the new room
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes recorded cur_room for this file
 */
void
prep_room (const room_t* r)
{
    /* Record the current room. */
    cur_room = r;

	/* We need the current photo of the room. Grab with the given struct. */
	photo_t* p = room_photo( r );

	/* Set palette based on room photo... */
	set_palette( p->palette );
}


/* 
 * read_obj_image
 *   DESCRIPTION: Read size and pixel data in 2:2:2 RGB format from a
 *                photo file and create an image structure from it.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the image
 */
image_t*
read_obj_image (const char* fname)
{
    FILE*    in;		/* input file               */
    image_t* img = NULL;	/* image structure          */
    uint16_t x;			/* index over image columns */
    uint16_t y;			/* index over image rows    */
    uint8_t  pixel;		/* one pixel from the file  */

    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the image pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (img = malloc (sizeof (*img))) ||
	NULL != (img->img = NULL) || /* false clause for initialization */
	1 != fread (&img->hdr, sizeof (img->hdr), 1, in) ||
	MAX_OBJECT_WIDTH < img->hdr.width ||
	MAX_OBJECT_HEIGHT < img->hdr.height ||
	NULL == (img->img = malloc 
		 (img->hdr.width * img->hdr.height * sizeof (img->img[0])))) {
	if (NULL != img) {
	    if (NULL != img->img) {
	        free (img->img);
	    }
	    free (img);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }

    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = img->hdr.height; y-- > 0; ) {

	/* Loop over columns from left to right. */
	for (x = 0; img->hdr.width > x; x++) {

	    /* 
	     * Try to read one 8-bit pixel.  On failure, clean up and 
	     * return NULL.
	     */
	    if (1 != fread (&pixel, sizeof (pixel), 1, in)) 
		{
			free (img->img);
			free (img);
			(void)fclose (in);
			return NULL;
	    }

	    /* Store the pixel in the image data. */
	    img->img[img->hdr.width * y + x] = pixel;
	}
    }

    /* All done.  Return success. */
    (void)fclose (in);
    return img;
}


/* 
 * read_photo
 *   DESCRIPTION: Read size and pixel data in 5:6:5 RGB format from a
 *                photo file and create a photo structure from it.
 *                Code provided simply maps to 2:2:2 RGB.  You must
 *                replace this code with palette color selection, and
 *                must map the image pixels into the palette colors that
 *                you have defined.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the photo
 */
photo_t*
read_photo (const char* fname)
{
    FILE*    in;	/* input file               */
    photo_t* p = NULL;	/* photo structure          */
    uint16_t x;		/* index over image columns */
    uint16_t y;		/* index over image rows    */
    uint16_t pixel;	/* one pixel from the file  */
	unsigned int red;
	unsigned int green;
	unsigned int blue;
	unsigned int level4_node_index;
	unsigned int level2_node_index;
	int i;
	unsigned int new_pixel_palette_index;
	unsigned int palette_red;
	unsigned int palette_green;
	unsigned int palette_blue;
	unsigned int closest_index;
    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the photo pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (p = malloc (sizeof (*p))) ||
	NULL != (p->img = NULL) || /* false clause for initialization */
	1 != fread (&p->hdr, sizeof (p->hdr), 1, in) ||
	MAX_PHOTO_WIDTH < p->hdr.width ||
	MAX_PHOTO_HEIGHT < p->hdr.height ||
	NULL == (p->img = malloc 
		 (p->hdr.width * p->hdr.height * sizeof (p->img[0])))) {
	if (NULL != p) {
	    if (NULL != p->img) {
	        free (p->img);
	    }
	    free (p);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }

	/* Initialize values of octree before we attempt to map pixels */
	octree_init_level4_and_level2( );

    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = p->hdr.height; y-- > 0; ) 
	{

		/* Loop over columns from left to right. */
		for (x = 0; p->hdr.width > x; x++)
		{

			/* 
			* Try to read one 16-bit pixel.  On failure, clean up and 
			* return NULL.
			*/
			if (1 != fread (&pixel, sizeof (pixel), 1, in))
			{
			free (p->img);
			free (p);
				(void)fclose (in);
			return NULL;

			}

			/* Iterate through image and gather the number of pixels based on the */
			/* 12-bit mapping of each pixel's data. Pixel data is in form		  */
			/* RRRRR GGGGGG BBBBB. Mask 4 MSBs of each color to map.			  */
			
			/* Mask red with 0b11110 000000 00000 */
			/* Shift red from position 16 -> position 12 */
			red = ( pixel & RED_MASK_LVL4 ) >> RED_CONCATENATE_SHIFT_LVL4;
			
			/* Mask green with 0b00000 111100 00000 */
			/* Shift green from position 11 -> position 8 */
			green = ( pixel & GREEN_MASK_LVL4 ) >> GREEN_CONCATENATE_SHIFT_LVL4;
			
			/* Mask blue with 0b00000 000000 11110 */
			/* Shift blue from position 5 -> position 4 */
			blue = ( pixel & BLUE_MASK_LVL4 ) >> BLUE_CONCATENATE_SHIFT_LVL4;

			/* Combine MSBs together into one value */
			level4_node_index = 0;
			level4_node_index = red | green | blue;


			/* Now index into octree_level4 via the index just calculated, and    		*/
			/* increase its values accordingly.								      		*/
			/* Mask red to get total value of red, not just MSBs, and shift to    		*/
			/* get the actual value, from position 16 -> position 5				  		*/
			red = ( pixel & RED_MASK_TOTAL_LVL4 ) >> RED_TOTAL_SHIFT_LVL4;
			
			/* Mask green to get total value of green, not just MSBs, and shift to  	*/
			/* get the actual value, from position 11 -> position 6				    	*/
			green = ( pixel & GREEN_MASK_TOTAL_LVL4 ) >> GREEN_TOTAL_SHIFT_LVL4;

			/* Mask blue to get total value of blue, not just MSBs, and shift to	   	*/
			/* get the actual value, from position 5 -> position 5 ( no shift ) 	  	*/
			blue = ( pixel & BLUE_MASK_TOTAL_LVL4 );

			/* Add the values to the totals of each node. Also keep track of the node's */
			/* index for later reference, and increment the number of pixels. 			*/
			octree_level4[ level4_node_index ].red_total += red;
			octree_level4[ level4_node_index ].green_total += green;
			octree_level4[ level4_node_index ].blue_total += blue;
			octree_level4[ level4_node_index ].node_index = level4_node_index;
			octree_level4[ level4_node_index ].num_pixels += 1;
		}

    }

	/* Finished putting all pixels into level 4 of Octree, now sort the octree...  */
	/* Call qsort and sort fourth level */
	qsort( octree_level4, ( size_t ) NUM_NODES_LEVEL4, ( size_t ) sizeof( octree_node ), compare_num_pixels );

	/* Now that the octree is sorted from most frequent to least frequent, 	*/
	/* We can assign the first 128 indices from the octree to our palette.  */
	for( i = 0; i < LEVEL4_INIT_PALETTE; i++ )
	{
		/* Average the values of the colors inividually and set the palette.*/	
		/* To avoid DIVIDE BY ZERO error, only average if we have pixels. 	*/
		if( octree_level4[ i ].num_pixels != 0 )
		{
			palette_red 	= ( ( octree_level4[ i ].red_total << PALETTE_SHIFT )/ octree_level4[ i ].num_pixels ) ;
			palette_green 	= ( octree_level4[ i ].green_total / octree_level4[ i ].num_pixels );
			palette_blue 	= ( ( octree_level4[ i ].blue_total << PALETTE_SHIFT ) / octree_level4[ i ].num_pixels );
		}
		else
		{
			palette_red 	= NO_COLOR;
			palette_green 	= NO_COLOR;
			palette_blue 	= NO_COLOR;
		}

		/* Assign the palette values starting from 0. Will be offset in  	*/
		/* set_palette to account for 64 essential VGA registers. 			*/
		p->palette[ i ][ PALETTE_RED_INDEX ] 	= palette_red;
		p->palette[ i ][ PALETTE_GREEN_INDEX ] 	= palette_green;
		p->palette[ i ][ PALETTE_BLUE_INDEX ] 	= palette_blue;
		
		/* Also assign the node's sorted index. We will reference this  	*/
		/* later when we try to re-map each pixel's color.					*/
		octree_level4[ i ].node_index_sorted = i;
	}

	/* Now we take all the remainig colors not used, and re-map them into our 	*/
	/* Level2 octree, using the 2 MSBs of each color instead. Repeat the same   */
	/* procedure. 																*/
	for( i = LEVEL4_INIT_PALETTE; i < NUM_NODES_LEVEL4; i++ )
	{
		/* We already have the first four MSBs of each pixel stored, now we 	*/
		/* just have to mask and shift from the index to get the level2 index.	*/
		

		/* Mask red and shift from position 12 -> position 6 					*/
		red = ( octree_level4[ i ].node_index & RED_MASK_LVL2 ) >> RED_CONCATENATE_SHIFT_LVL2;

		/* Mask green and shift from position 8 -> position 4 					*/
		green = ( octree_level4[ i ].node_index & GREEN_MASK_LVL2 ) >> GREEN_CONCATENATE_SHIFT_LVL2;

		/* Mask blue and shift from position 4 -> position 2 					*/
		blue = ( octree_level4[ i ].node_index & BLUE_MASK_LVL2 ) >> BLUE_CONCATENATE_SHIFT_LVL2;

		/* Combine MSBs into one 6-bit value					 				*/
		level2_node_index = 0;
		level2_node_index = red | green | blue;

		/* Add totals in level2 octree 											*/
		octree_level2[ level2_node_index ].red_total += octree_level4[ i ].red_total;
		octree_level2[ level2_node_index ].green_total += octree_level4[ i ].green_total;
		octree_level2[ level2_node_index ].blue_total += octree_level4[ i ].blue_total;
		octree_level2[ level2_node_index ].num_pixels += octree_level4[ i ].num_pixels;
	}

	/* Insert the remaining nodes into our palette using a similar procedures 	*/
	/* as before. Remember to offset the palette index by 128 to account for 	*/
	/* 128 level4 colors we already added. 										*/
	for( i = 0; i < LVL2_SIZE; i++ )
	{
		if( octree_level2[ i ].num_pixels != 0 )
		{
			palette_red 	= ( ( octree_level2[ i ].red_total  << PALETTE_SHIFT ) / octree_level2[ i ].num_pixels );
			palette_green 	= ( octree_level2[ i ].green_total / octree_level2[ i ].num_pixels );
			palette_blue 	= ( ( octree_level2[ i ].blue_total  << PALETTE_SHIFT ) / octree_level2[ i ].num_pixels );
		}
		else
		{
			palette_red 	= NO_COLOR;
			palette_green 	= NO_COLOR;
			palette_blue 	= NO_COLOR;
		}
		p->palette[ i + LVL2_PALETTE_OFFSET ][ PALETTE_RED_INDEX ] 		= palette_red;
		p->palette[ i + LVL2_PALETTE_OFFSET ][ PALETTE_GREEN_INDEX ] 	= palette_green;
		p->palette[ i + LVL2_PALETTE_OFFSET ][ PALETTE_BLUE_INDEX ] 	= palette_blue;
	
	}

	/* Now that we have optimized the palette for our image, we need to go iterate 	*/
	/* through the image again. However, we need to reset the file pointer to the 	*/
	/* beginning of the file again using fseek. 									*/
	/* fseek( FILE *stream, long offset, int origin ) 								*/
	/* fseek sets the file position indicator to the position pointed by offset in	*/
	/* file *stream, from origin. SEEK_SET indicates seeking from the beginning of  */
	/* the file. Thus with the input file ( in ), skip over the header at p->hdr  	*/
	/* and reset the file position indicator.								 		*/
	fseek( in, sizeof( p->hdr ), SEEK_SET );

    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = p->hdr.height; y-- > 0; ) 
	{

		/* Loop over columns from left to right. */
		for (x = 0; p->hdr.width > x; x++)
		{
			/* 
			* Try to read one 16-bit pixel.  On failure, clean up and 
			* return NULL.
			*/
			if ( 1 != fread( &pixel, sizeof( pixel ), 1, in ) )
			{
				free (p->img);
				free (p);
				(void)fclose (in);
				return NULL;
			}			
			/* Rewrites pixels' palette register value. First check if the index 	*/
			/* fits in the first 128 palette register values, or in otherwords  	*/
			/* exists on level4 of our octree. If not, re-map the index to the 		*/
			/* level2 octree and update the pixel accordingly. 						*/
			
			closest_index = find_closest_palette_color( pixel, p );
			new_pixel_palette_index = closest_index;

			/* Remember to offset the value of our indexed VGA register by 64 	*/
			/* to avoid indexing into reserved VGA registers for our items. 	*/
			new_pixel_palette_index += VGA_ESSENTIAL_REGISTER_OFFSET;
			p->img[ p->hdr.width * y + x ] = new_pixel_palette_index;

		}

	}	

    /* All done.  Return success. */
    (void)fclose (in);
    return p;
}


/* Octtree implementation included below! */
/* First step in algorithm: */
/* Count the number of pixels in each node at level four of an octree. */
/* In other words, count the number of pixels of the 128 most common   */
/* colors in the tree *besides* the first 64 colors.  */
void octree_init_level4_and_level2( )
{
	int i;
	/* Initialize all values in nodes to besides sorted index to zero for now. */
	/* Initialize sorted indexes to -1 for sorting.							   */
	
	/* Initialize Level2 */
	for ( i = 0; i < NUM_NODES_LEVEL2; i++ )
	{
		octree_level2[ i ].node_index = i;
		octree_level2[ i ].node_index_sorted = 129;
		octree_level2[ i ].num_pixels = 0;
		octree_level2[ i ].red_total = 0;
		octree_level2[ i ].green_total = 0;
		octree_level2[ i ].blue_total = 0;
	}

	/* Initialize Level4 */
	for( i = 0; i < NUM_NODES_LEVEL4; i++ )
	{
		octree_level4[ i ].node_index = i;
		octree_level4[ i ].node_index_sorted = 129;
		octree_level4[ i ].num_pixels = 0;
		octree_level4[ i ].red_total = 0;
		octree_level4[ i ].green_total = 0;
		octree_level4[ i ].blue_total = 0;
	}

}


/* Technically sorts in descending order  */
/* a > b -> 1; a < b -> -1; a == b -> 0  */
int compare_num_pixels( const void* a, const void* b )
{
	int num_pixels_a;
	int num_pixels_b;
	num_pixels_a = ( (struct octree_node*) a)->num_pixels;
	num_pixels_b = ( (struct octree_node*) b)->num_pixels;

	/* Compare and return appropriately */
	if( num_pixels_a > num_pixels_b )
	{
		return -1;
	}
	else if( num_pixels_a < num_pixels_b )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned int find_closest_palette_color( unsigned int pixel, const photo_t* p )
{
	/* Declare iteration elements */
	unsigned int i;
	unsigned int lowest_index;
	int prev_distance;
	int distance_red;
	int distance_green;
	int distance_blue;
	int distance_total;

	/* Set the lowest index to zero and previous distance to its maximum value */
	lowest_index = 0;
	prev_distance = 0x7FFFFFFF;

	/* Declare elements relevant to pixel */
	int pixel_red;
	int pixel_green;
	int pixel_blue;

	pixel_red = ( pixel & PIXEL_RED_DISTANCE_MASK ) >> PIXEL_RED_TOTAL_SHIFT;
	pixel_green = ( pixel & PIXEL_GREEN_DISTANCE_MASK ) >> PIXEL_GREEN_TOTAL_SHIFT;
	pixel_blue = ( pixel & PIXEL_BLUE_DISTANCE_MASK ) << PIXEL_BLUE_TOTAL_SHIFT;

	/* Declare elements relevant to palette */
	int palette_red;
	int palette_green;
	int palette_blue;

	/* Loop through palette and find nearest pixel color based on calculated distance */
	for( i = 0; i < PALETTE_SIZE; i++ )
	{
		palette_red = ( p->palette[ i ][ 0 ] );
		palette_green = ( p->palette[ i ][ 1 ] );
		palette_blue = ( p->palette[ i ][ 2 ] );

		distance_red = ( palette_red - pixel_red ) * ( palette_red - pixel_red );
		distance_green = ( palette_green - pixel_green ) * ( palette_green - pixel_green );
		distance_blue = ( palette_blue - pixel_blue ) * ( palette_blue - pixel_blue );

		distance_total = distance_red + distance_green + distance_blue;

		if( distance_total < prev_distance )
		{
			lowest_index = i;
			prev_distance = distance_total;
		}

	}

	return lowest_index;

}



