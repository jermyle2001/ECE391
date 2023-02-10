/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/* Defined a flag for MTCP_ACK */
int ack_flag;
/* Keep track of previous state for reset, as outlined by documentation */
char prev_state[ LED_COMMAND_SIZE ];
/* Also keep track of handled packets, since the ioctl can't pass in the args for the packets */
char handled_packets[ NUM_PACKETS ];

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];
	

	/* Since we use MTCP_BIOC, we are required to handle MTCP_BIOC_EVENT, */
	/* as well as MTCP_BIOC_EVENT and MTCP_ACK. The first byte of the     */
	/* packet is our opcode, so handle accordingly... 					  */
	switch( a )
	{
		/* Reset the device, but must re-initialize the controller to the */
		/* same state it was in before it was reset. Thus we must 		  */
		/* internally keep track of the state of the device. 			  */
		case MTCP_RESET:
			// printk( "MTCP_RESET called! Resetting...\n" );
			/* Re-initialize variables and values */
			tuxctl_init( tty );

			if( ack_flag == ACK_OFF )
			{
				/* Restore LEDs to previous state */
				tuxctl_ldisc_put( tty, prev_state, LED_COMMAND_SIZE );
			}
			break;

		/* MTCP_BIOC_EVENT sends a signal when a new set of buttons has   */
		/* been pressed. Set the packets and let the ioctl handle calling */
		/* the tuxctl_buttons subroutine.								  */
		case MTCP_BIOC_EVENT:
			// printk( "MTCP_BIOC_EVENT called! Setting packets...\n" );

			handled_packets[ 0 ] = packet [ 0 ];
			handled_packets[ 1 ] = packet [ 1 ];
			handled_packets[ 2 ] = packet [ 2 ];
			break;

		/* If ACK is low, then we should exit the function, as the flag */
		/* indicates if the device is ready to handle another request.  */
		/* If low, exit from the function which is returning, returning */
		/* 0 if it was a valid request, and -1 if invalid. 			    */
		case MTCP_ACK:
			// printk( "MTCP_ACK! Setting flag to OFF...\n " );
			ack_flag = ACK_OFF;
			break;

		default:
			// printk( "OpCode from TUX unknown. Ignoring...\n" );
			break;

	}

}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
	switch (cmd) {
		case TUX_INIT:
			/* ioctl calls init, call subroutine and return 0 */
			tuxctl_init( tty );
			return 0;

		case TUX_BUTTONS:
			/* ioctl wants to set buttons, call subroutine and return -EINVAL error if pointer not valid. */
			return tuxctl_buttons( tty, arg );
		
		case TUX_SET_LED:
			/* ioctl wants to set LEDs, call subroutine and return 0 */
			return tuxctl_set_led( tty, arg );
		
		case TUX_LED_ACK:
			/* Not implemented, return -EINVAL */
			return -EINVAL;
		case TUX_LED_REQUEST:
			/* Not implemented, return -EINVAL */
			return -EINVAL;
		case TUX_READ_LED:
			/* Not implemented, return -EINVAL */
			return -EINVAL;
		default:
			/* Return -EINVAL by default */
			return -EINVAL;
    }
}

/*
 *	tuxcl_init 
 * 		DESCRIPTION : 	Initializes any variables associated with the driver and returns 0.
 * 						Assume that any user-level code that interacts with your device will
 * 						call this ioctl before any others.
 * 			 INPUTS :	None
 * 			OUTPUTS :	None
 * 	   RETURN VALUE :	0 ( always )
 * 	   SIDE EFFECTS :   Initializes any vars associated with driver to 0.
 * 
 */
int
tuxctl_init( struct tty_struct* tty )
{
	/* We are going to use the function tuxctl_ldisc_put to write bytes to the device. */
	/* The function takes in a struct tty, which is passed in to the function, a       */
	/* buffer with the commands we are passing in, and the number of commands we are   */
	/* passing in. Pass in the corresponding mtcp commands and arguments to initialize */
	/* the controller.																   */

	/* Specifically, we are looking to enable the Tux Controller to allow the LEDs to  */
	/* be set, as well as */

	/* We are passing in two commands, set the buffer size appropriately */
	unsigned char command[ INIT_NUM_COMMANDS ];

	/* Set ack_flag to off in order to wait for MTCP_ACK to be raised again */
	ack_flag = ACK_ON;
	// printk("Init: Setting flag to on\n");

	/* Enable Button Interrupt-on-Change with command MTCP_BIOC_ON */
	/* MTCP_BIOC_ON - Enable Button interrupt-on-change. MTCP_ACK is returned. */
	command[ 0 ] = MTCP_BIOC_ON;

	/* We also want to enable the LED display to be set by MTCP_LED_SET, with command */ 
	/* MTCP_LED_USR. In this mode, the value specified by the MTCP_LED_SET command is */
	/* displayed.  		   															  */
	command[ 1 ] = MTCP_LED_USR;

	/* Call put() command to write to device with the appropriate arguments  */
	tuxctl_ldisc_put( tty, command, INIT_NUM_COMMANDS );


	/* Also set the LEDs to be off initially! */
	prev_state[ 0 ] = MTCP_LED_SET;
	prev_state[ 1 ] = EMPTY;
	prev_state[ 2 ] = EMPTY;
	prev_state[ 3 ] = EMPTY;
	prev_state[ 4 ] = EMPTY;
	prev_state[ 5 ] = EMPTY;

	/* Set LEDs to empty on start, so that we have something to display */
	tuxctl_ldisc_put( tty, prev_state, LED_COMMAND_SIZE );

	// printk( "Tux initialized!\n" );

	/* Return 0 when finished */
	return 0;

}


/*
 *	tuxctl_set_led 
 * 		DESCRIPTION : 	Sets the LEDs ( Seven-segment Displays ) as dictated by the
 * 						pointer passed in. The low 16-bits specify a number whose 
 * 						hexadecimal value is to be displayed on the 7-segment displays.
 * 						The low 4 bits of the third byte species which LEDs should be 
 * 						turned on.
 * 						The low 4 bytes of the highest byte ( bits 27:24 ) specify
 * 						whether the corresponding decimal points should be turned on.
 * 						Should always return 0.
 * 			 INPUTS :	32-bit integer
 * 			OUTPUTS :	None
 * 	   RETURN VALUE :	0 ( always )
 * 	   SIDE EFFECTS :   Sets the LEDs as dictated by 32-bit integer
 * 
 */
int
tuxctl_set_led( struct tty_struct* tty, unsigned long arg )
{
	/* Declare relevant elements */
	uint16_t leds_hex_value;
	unsigned char leds_on, decimals_on;
	/* Declare command buffer to be passed into put() */
	unsigned char command_buffer[ LED_COMMAND_SIZE ];

	/* Declare array for preset HEX values */
	unsigned char hex_values[ NUM_CHARS ];

	/* Declare elements relevant to half byte selection */
	unsigned char cur_val;
	int i;	

	/* If ACK is ON, then device is busy and cannot take another request. Return... */	
	if( ack_flag == ACK_ON )
	{
		return 0;
	}
	// printk("LED: Setting ACK to on...\n");
	ack_flag = ACK_ON;

	/* To avoid overcomplicating things, we will always write 6 bytes */
	/* to the command buffer, with the following format: 			  */
	/* Byte 0: OpCode MTCP_LED_SET, send OpCode to device driver.     */
	/* Byte 1: LED Set Bitmask, determines which LEDs will be set.    */
	/* Byte 2: LED Data for LED0                                      */
	/* Byte 3: LED Data for LED1                                      */
	/* Byte 4: LED Data for LED2                                      */
	/* Byte 5: LED Data for LED3                                      */
	/* Further detail will be included later in the code.  			  */

	/* Set the OpCode of the command buffer to MTCP_LED_SET */
	command_buffer[ 0 ] = MTCP_LED_SET;

	/* Set the LEDs to be set to be the mask for ALL LEDs.  */
	command_buffer[ 1 ] = MASK_ALL_LEDS;

	/* Also set all LED bytes to zero for now, will update as appropriate */
	command_buffer[ 2 ] = EMPTY;
	command_buffer[ 3 ] = EMPTY;
	command_buffer[ 4 ] = EMPTY;
	command_buffer[ 5 ] = EMPTY;


	/* Initialize values for Seven-Segment Display mapping */
	hex_values[ 0 ] 	= 0xE7; /* '0' */
	hex_values[ 1 ] 	= 0x06; /* '1' */
	hex_values[ 2 ] 	= 0xCB; /* '2' */
	hex_values[ 3 ] 	= 0x8F; /* '3' */
	hex_values[ 4 ] 	= 0x2E; /* '4' */
	hex_values[ 5 ] 	= 0xAD; /* '5' */
	hex_values[ 6 ] 	= 0xED; /* '6' */
	hex_values[ 7 ] 	= 0x86; /* '7' */
	hex_values[ 8 ] 	= 0xEF; /* '8' */
	hex_values[ 9 ] 	= 0xAE; /* '9' */
	hex_values[ 10 ] 	= 0xEE; /* 'A' */
	hex_values[ 11 ] 	= 0x6D; /* 'B' */
	hex_values[ 12 ] 	= 0xE1; /* 'C' */
	hex_values[ 13 ] 	= 0x4F; /* 'D' */
	hex_values[ 14 ] 	= 0xE9; /* 'E' */
	hex_values[ 15 ] 	= 0xE8; /* 'F' */

	/* Get number whose hexadecimal value is to be displayed, */
	/* stored in lower 16 bits of arg.                        */
	leds_hex_value = ( arg & LED_LOW_MASK );

	/* Get byte which species which LEDs should be turned on, */
	/* stored in low 4 bits of third byte. */
	leds_on = ( arg & LEDS_ON_MASK ) >> LEDS_ON_SHIFT;

	/* Get byte which species which decimal points should be  */
	/* turned on. Stored in last 4 bits of the highest byte.  */
	decimals_on = ( arg & DECIMAL_POINTS_MASK ) >> DECIMAL_POINTS_SHIFT;

	/* Each half byte in the leds_hex_value corresponds to a  */
	/* number on the display.  */

	/* Set the LED bytes by selecting which half byte corresponds */
	/* to which LED. Each half byte in the leds_hex_value 		  */
	/* corresponds to a number on the display. Iterate through    */
	/* and set the byte accordingly.							  */
	for ( i = 0; i < NUM_DISPLAYS; i++ )
	{
		/* Check if the LED is masked, or designated to be on. If  */
		/* so, then set its corresponding byte. Otherwise, ignore. */
		if( (leds_on >> i ) & LED_ON_MASK )
		{
			/* Shift default mask by i half bytes to mask corresponding */
			/* half byte, shift back to index into hex_values properly  */
			// cur_val = ( leds_hex_value & ( DEFAULT_MASK << ( i * HALF_BYTE ) ) ) >> ( i * HALF_BYTE );
			cur_val = ( leds_hex_value >> ( i * HALF_BYTE ) ) & BOTTOM_BYTE_MASK;
			/* Set the corresponding byte to the corresponding mapped value */
			command_buffer[ LED_BUFFER_OFFSET + i ] = hex_values[ cur_val ];

			/* Also set the decimal point accordingly */
			command_buffer[ LED_BUFFER_OFFSET + i ] |= ( ( ( decimals_on >> i ) & DECIMAL_MASK ) << DECIMAL_SHIFT ); // Shift to right by i times, mask bit			
		}

	}

	/* Save current state to a previous state in case of reset */ 
	for( i = 0; i < LED_COMMAND_SIZE; i++ )
	{
		prev_state[ i ] = command_buffer[ i ];
	}

	/* Use put() to send command and data to device */
	tuxctl_ldisc_put( tty, command_buffer, LED_COMMAND_SIZE );


	/* Finished setting LEDs, return 0 */
	return 0;

}


/*
 *	tuxctl_buttons 
 * 		DESCRIPTION : 	Sets the button input in user space. 
 * 			 INPUTS :	Pointer to 32-bit Integer.
 * 			OUTPUTS :	None
 * 	   RETURN VALUE :	-EINVAL if the pointer is not valid. Otherwise, 0.
 * 	   SIDE EFFECTS :   Sets the byte at the location pointed to by the input
 * 						to button input.
 * 
 */
int 
tuxctl_buttons( struct tty_struct* tty, unsigned long arg )
{
	/* Declare all relevant variables */
	unsigned char result_byte, byte_1, byte_2;
	unsigned char c, b, a, s, right, left, down,up;
	int flag;

	/* The packet format is as follows: */
 	/* Packet format:										*/
	/*	Byte 0 - MTCP_BIOC_EVENT							*/
	/*	byte 1  +-7-----4-+-3-+-2-+-1-+---0---+				*/
	/*		| 1 X X X | C | B | A | START |					*/
	/*		+---------+---+---+---+-------+					*/
	/*	byte 2  +-7-----4-+---3---+--2---+--1---+-0--+		*/
	/*		| 1 X X X | right | down | left | up |			*/
	/*		+---------+-------+------+------+----+			*/
	/* We want to transform the packet into the 	    	*/
	/* following byte:								    	*/
	/*	+---7---+--6---+--5---+-4--+-3-+-2-+-1-+---0---+	*/
	/*	| right | left | down | up | C | B | A | START |	*/

	/* Set result_byte to 0x00 for now */
	result_byte = EMPTY;
	
	/* Set handled bytes appropriately */
	byte_1 = handled_packets[ PACKETS_BYTE_1 ];
	byte_2 = handled_packets[ PACKETS_BYTE_2 ];
	
	/* Mask each input bit and shift as needed */
	c = byte_1 & C_MASK;
	b = byte_1 & B_MASK;
	a = byte_1 & A_MASK;
	s = byte_1 & S_MASK;
	right = byte_2 & R_MASK;
	down = byte_2 & D_MASK;
	left = byte_2 & L_MASK;
	up = byte_2 & U_MASK;

	/* Down and left are switched, shift them to switch to appropriate format */
	down = down >> DOWN_DATA_SHIFT;
	left = left << DOWN_DATA_SHIFT;

	/* Shift RLDU to the right by half a byte to align appropriately */
	right = right << BYTE_2_SHIFT;
	left = left << BYTE_2_SHIFT;
	down = down << BYTE_2_SHIFT;
	up = up << BYTE_2_SHIFT;

	/* Combine bits by ORing with 0x00 */
	result_byte |= right | left | down | up | c | b | a | s;	

	/* Set arg so that we can send  */
	/* Send data to user and check if valid. Return accordingly. */
	// printk("Buttons: Setting flag...");
	flag = copy_to_user( ( unsigned long* ) arg, &result_byte, sizeof( result_byte ) );
	// printk("flag set!\n");
	if( flag != 0 )
	{
		// printk( "Set buttons failed: returning -EINVAL\n" );
		return -EINVAL;
	}
	else
	{		
		return 0;
	}

	return 0;
}
