// All necessary declarations for the Tux Controller driver must be in this file

#ifndef TUXCTL_H
#define TUXCTL_H

#define TUX_SET_LED _IOR('E', 0x10, unsigned long)
#define TUX_READ_LED _IOW('E', 0x11, unsigned long*)
#define TUX_BUTTONS _IOW('E', 0x12, unsigned long*)
#define TUX_INIT _IO('E', 0x13)
#define TUX_LED_REQUEST _IO('E', 0x14)
#define TUX_LED_ACK _IO('E', 0x15)

#endif

/* Constants for overall function */
#define NUM_PACKETS 3
#define PACKETS_ARG 0
#define PACKETS_BYTE_1 1
#define PACKETS_BYTE_2 2
#define HALF_BYTE 4
#define BOTTOM_BYTE_MASK 0x0F

/* Constants for init */
#define INIT_NUM_COMMANDS 2


/* Constants for set_led */
/* The following are masks and shifts corresponding to the documentation */
#define LED_LOW_MASK 0x0000FFFF
#define LEDS_ON_MASK 0x000F0000
#define LEDS_ON_SHIFT 16
#define DECIMAL_POINTS_MASK 0x0F000000
#define DECIMAL_POINTS_SHIFT 24
#define NUM_DISPLAYS 4
#define LED_BUFFER_OFFSET 2
#define DECIMAL_SHIFT 4
#define DECIMAL_MASK 0x01

/* Packet size is 6 - The first byte is the opcode, the second byte */
/* determines which LEDs to set, and the following bytes contain    */
/* the data for each LED, in increasing LED number                  */
#define LED_COMMAND_SIZE 6

/* We will be setting ALL of our LEDs, set them all to EMPTY initially, */
/* as well as set the mask to be for ALL LEDs. */
#define EMPTY 0
#define MASK_ALL_LEDS 0xF
#define NUM_CHARS 16
#define LED_ON_MASK 0x01
/* Define default mask to be 0x000F */
/* and dot mask to be 0x0010 */
#define DEFAULT_MASK 0x000F
#define DOT_MASK 0x0010


/* Constans for buttons */
#define LEFT_DATA_SHIFT 1
#define DOWN_DATA_SHIFT 1
#define LEFT_DATA_MASK 0x04
#define DOWN_DATA_MASK 0x02
#define BYTE_2_INIT 0x09
#define BYTE_1_MASK 0x0F
#define BYTE_2_SHIFT 4
#define C_MASK 0x08
#define B_MASK 0x04
#define A_MASK 0x02
#define S_MASK 0x01
#define R_MASK 0x08
#define D_MASK 0x04
#define L_MASK 0x02
#define U_MASK 0x01


/* The ACK flag indicates whether the device is currently handling a request or not.    */  
/* A high flag indicates that the device is ready to handle another request (ACK_OFF)   */
/* and a low flag indicates that the device is currently busy handling another request. */
#define ACK_ON 0
#define ACK_OFF 1

/* Functions for the IOCTLs declared here! */
int tuxctl_init( struct tty_struct* tty );
int tuxctl_set_led( struct tty_struct* tty, unsigned long arg );
int tuxctl_buttons( struct tty_struct* tty, unsigned long arg );
