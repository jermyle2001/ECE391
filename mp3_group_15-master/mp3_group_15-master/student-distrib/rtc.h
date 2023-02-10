/* rtc.h - Defines used in interactions with the rtc device
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

// Files to include
#include "types.h"
#include "lib.h"

/*Four registers in the RTC avaliable
* Below is a description of each and the functionality of each bit in the register 
* Register A
*   Bit 7: UIP
*       (Update in progress)
*           When 1, update transfer occurs soon 
*   Bits 6,5,4: DV(2-0)
*       ()
*           Turn oscillator on and off 010 turns on the oscillator
*   Bits 3-0: RS(3-0)
*       ()
*           Rate slector chooses the rate of the oscillator 
* ----------------------------------------------------------------------------------------------------------------------
* Register B
*   Bit 7: Set
*       (Set)
*           When 0, update transfer functions act normal advancing count 1 per/sec
*           When 1, update transfer inhibited, can initilize time and calendat bytes without interruptions
*   Bit 6: PIE
*       (Periodic Interrupt Enable)
*           When 1, periodic interrupts occur at IRQ (low) at rate from RS(3-0) of reg A
*           When 0, blocks the IRQ output being driven by a periodic interrupt
*           Set to 0 when reset
*   Bit 5: AIE
*       (Alarm Interrupt Enable)
*           When 1, allows alarm flag in Reg C to assert IRQ. Occurs when three time bytes = 3 alarm bytes
*           When 0, does not allow alarm flag to assert IRQ
*           Set to 0 when reset
*   Bit 4: UIE
*       (Update-Ended Interrupt Enable)
*           When 1, allows update flag in REG C to assert IRQ
*           When 0, does not allow update flag to assert IRQ
*           Set to 0 on reset
*   Bit 3: SQWE
*       (Square-Wave Enable)
*           When 1, square wave signal set by RS(0-3) of REG A is driven out of SQW pin
*           When 0, SQW pin is held at low
*           Set to 0 on reset
*   Bit 2: DM
*       (Data Mode)
*           When 1, calendar information read in binary data format
*           When 0, calendar information read in BCD format
*           Not modified by reset
*   Bit 1: 24/12
*       (Hours byte format)
*           When 1: 24 hour mode
*           When 0: 12 hour mode
*           Not Modified by reset
*   Bit 0: DSE
*       (Daylight Savings Enable)
*           When 1: Applies daylight savings time operations
*           When 0: Does not apply daylight savings operations
*           Not modified by reset 
* ----------------------------------------------------------------------------------------------------------------------
* Register C
*   Bit 7: IRQF
*       (Interrupt Request Flag)
*           When 1: One of the following is true, PF = PIE = 1, AF = AIE = 1, UP = UIE = 1. When 1 the IRQ pin is driven low
*           When 0: None of the above are 1
*           Cleared by reset of readhing register C
*   Bit 6: PF
*       (Period Interrupt Flag) Read only
*           When 1, a edge was detected on the divider chain defined by RS(3-0) in Reg A
*           When 0, no edge was detected
*           When both PF and PIE are 1 IRQ signal is active and sends IRQF bit
*           Cleared by reset or reading register C
*   Bit 5: AF
*       (Alarm Interrupt Flag)
*           When 1, Current time has matched the alarm time
*           When 0, Current time does nto match the alarm time
*           When both AF and AIR are 1 IRQ signal is active and sends IRQF bit
*           Cleared by reset or reading register C
*   Bit 4: UF
*       (Update_Ended Interrupt Flag)
*           When 1, after each update cycle
*           when 0, not after a update cycle
*           When both UF and UIR are 1 IRQF but is to be 1, which asserts IRQ pin
*           Cleared by reset or reading register C
*   Bit (3-0): 
*       (Unused)
*           Always set to 0  
*           Cannot be written
* ---------------------------------------------------------------------------------------------------------------------
* Register D
*   Bit 7: VRT
*       (Valid RAM and Time)
*           When 1: The energy source and state of chip is good
*           When 0: The internal lithium energy source is exhausted
*           Should always be 1 and uaffected by reset
*   Bit (6-0):
*       (Unused)
*           Always set to 0
*           Cannot be written
* ---------------------------------------------------------------------------------------------------------------------
*/
/* Defining constants for rtc.c             */
#define REGISTER_SELECT                 0x70
#define REGISTER_A                      0x0A
#define REGISTER_B                      0x0B
#define REGISTER_C                      0x0C
#define RTC_PORT                        0x70
#define CMOS_PORT                       0x71
#define RATE_MASK                       0x0F
#define REGISTER_A_RATE_MASK            0x0F
#define REMOVE_C_CONTENTS               0x71
#define DISABLE_NMI                     0x80
#define PERIODIC_INTERRUPT_ENABLE       0x40
#define CLOCK_FREQ                      32768
#define RTC_IRQ_NUM                     8
#define FREQ_MASK                       0xF0
#define INIT_FREQ                       0x06
#define HZ_RATE_2                       0x0F
#define HZ_RATE_1024                    0x06   
#define POWER_2_MASK                    0x0001  

/* Initilize the rtc device, map to PIC, and enable interrupts */
int rtc_init();

/* Handler for the RTC calls the interrupt_interrupts function */
void rtc_handler();

/* Function to set the frequency of the RTC */
int rtc_set_freq(uint32_t rate);

/* Initalizes the rtc when a file is open to 2 Hz */
int32_t rtc_open(const uint8_t* filename);

/* Reads the state of the RTC when an interrupt occurs */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* Writes a new periodic interrupt value to the rtc from a buffer */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* Resets the value of the periodic intterrupt when a file is closed */
int32_t rtc_close(int32_t fd);

#endif
