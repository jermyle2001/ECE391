/* rtc.c - Functions to interact with the rtc device
 * vim:ts=4 noexpandtab
 */

#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"
#include "tests.h"

/* Turn on Macro to test RTC */
#define TEST_RTC 0

/* void rtc_init();
*  Inputs: None  
*  Return Value: None
*  Function: Initializes the RTC and maps to IRQ on PIC
*   also ensures that periodic interrupts are allowed
*/
volatile int rtc_interrupt_occured;

int rtc_init(){
    /* Turning on periodic interrupts (from https://wiki.osdev.org/RTC)                             */    
    outb((DISABLE_NMI | REGISTER_B), RTC_PORT);             /* Select register B                    */
    char prev = inb(CMOS_PORT);                             /* Get current value of reg B           */
    outb((DISABLE_NMI | REGISTER_B), RTC_PORT);             /* Select register B again              */
    outb((prev | PERIODIC_INTERRUPT_ENABLE), CMOS_PORT);    /* Set the PIE bit to 1                 */

    /* Setting the rate of the periodc interrupts to 2 hz                                           */
    outb((DISABLE_NMI | REGISTER_A), RTC_PORT);             /* Select register A                    */
    prev = inb(CMOS_PORT);                                  /* Get the contents of reg A            */
    prev = prev & FREQ_MASK;                                /* Clear the bottom 4 bits              */
    prev = prev | HZ_RATE_2;                                /* Set the bottom 4 bits to 0x0F = 2Hz  */ 
    outb((DISABLE_NMI | REGISTER_A), RTC_PORT);             /* Select register A                    */
    outb(prev, CMOS_PORT);                                  /* Write the bits to the memory         */
    enable_irq(RTC_IRQ_NUM);                                /* Unmask the IRQ input                 */
    return 1;                     
}

/* void rtc_handler();
*  Inputs: None  
*  Return Value: None
*  Function: Handler for when an interrupt is invoked by the RTC
*            currently calls the test_interrupt function to make the
*            screen go crazy
*/
void rtc_handler(){
    /* first 2 lines from https://wiki.osdev.org/RTC                                                                */
    cli();
    outb(REGISTER_C, RTC_PORT);                         /* Select register C                                        */
    inb(REMOVE_C_CONTENTS);                             /* Clear the contents to allow for new interrupts           */
    
    #if TEST_RTC
        printf("!");
    #endif
    
    send_eoi(RTC_IRQ_NUM);                              /* Send eoi signal                                          */
    rtc_interrupt_occured = 1;                          /* Set the interrupt flag for the read command              */                                      
    sti();
}



/* void rtc_set_freq();
*  Inputs: 32 bit rate value: Hz value we want to set clock to  
*  Return Value: 1 is successfully set rate, 0 if invalid input
*  Function: Takes in a clock rate in hertz as a parameter, then
*            checks to see if it is valid, if it is, it updates
*            the RTC clock rate, otherwise it does nothing
*/
int rtc_set_freq(uint32_t rate){
    int i;                                              /* Counter variable for loops                               */                                              
    int power_2_check = 0;                              /* Used to check if number is a power of 2                  */
    int freq_array_index = 0;                           /* Used to index the freq_array                             */
    for (i = 0; i < 32; i++){                           /* Loop through all 32 bits of rate variable                */
        if ( ( (rate >> i) & POWER_2_MASK ) != 0){      /* Checks if each bit is non-zero                           */
            power_2_check++;                            /* If it is non-zero then increment var                     */
            freq_array_index = i;                       /* When we get a one set it to the freq_array_index         */
        }
    }
    freq_array_index -= 1;                              /* Make it one less to work with freq_ array correctly      */                      

    /* Check to see if it is within the allowed clock rates and if it is a power of 2                               */
    if ( (rate > 1024) || (rate < 2) || (power_2_check > 1)){
        return 0;                                       /* Return value is used for test cases                      */
    }
    
    /* Update the freq of the clock */
    else {
        /* Get the correct value to set RS bits in Register A                                                       */
        /* Corresponding Hz values  2, 4, 8, 16, 32, 64, 128, 256, 512, 1024                                        */
        char freq_Array[10] = {0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06};
        char new_freq = freq_Array[freq_array_index];

        char prev;
        outb((DISABLE_NMI | REGISTER_A), RTC_PORT);     /* Select register A                                        */
        prev = inb(CMOS_PORT);                          /* Get the contents of reg A                                */
        prev = prev & FREQ_MASK;                        /* Clear the bottom 4 bits                                  */
        prev = prev | new_freq;                         /* Set the bottom 4 bits to 0x0F = 2Hz                      */ 
        outb((DISABLE_NMI | REGISTER_A), RTC_PORT);     /* Select register A                                        */
        outb(prev, CMOS_PORT);                          /* Write the bits to the memory                             */
        return 1;                                       /* Return value is used for test cases                      */
    }
}

/* int32_t rtc_open(const uint8_t* filename);
*  Inputs: the filename that we are opening  
*  Return Value: always 0
*  Function: Opens the rtc and initializes the clock to 2Hz
*/
int32_t rtc_open(const uint8_t* filename){
    if (filename == NULL){                              /* If the file is NULL then just return -1                      */
        return -1;
    }
    rtc_init();                                         /* Just have to init the RTC, sets rate to 2Hz and enable irq   */
    return 0;                                           /* Return 0 on success                                          */
}

/* int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
*  Inputs: fd, buf, and nbytes  
*  Return Value: always 0
*  Function: Reads the state of the RTC and returns when an interrupt has occured
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    while (!rtc_interrupt_occured) {}                   /* While loop to wait for next interrupt                        */
    rtc_interrupt_occured = 0;                          /* Reset the flag back to 0                                     */
    return 0;                                           /* Should alwauys return zero as specified in documentation     */
}

/* int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
*  Inputs: fd, buf, and nbytes  
*  Return Value: 0 on success, -1 on failure
*  Function: Set the interrupt rte of the RTC to the number specified by the buffer
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    /* The buffer will contain the 4 bytes that we will use to set the clock rate */
    /* Intial check to make sure argument for set_freq is correct                 */
    /* All other checks for a correct clock value is in rtc_set_freq              */
    if (buf == NULL){
        return -1;
    }
    if (nbytes != 4){
        return -1;
    }
    cli();                                              /* Block interrupts when writing to RTC (from doc)              */
    rtc_set_freq( *(( uint32_t *)buf) );                /* Pass the uint32_t from the buffer as the arg for the set_freq*/
    sti();                                              /* Allow interrupts again (from doc)                            */
    return 0;                                           /* Return 0 on success                                          */
}

/* int32_t rtc_close(int32_t fd);
*  Inputs: fd  
*  Return Value: 0 always
*  Function: Does not do anything just returns 0
*/
int32_t rtc_close(int32_t fd){
    rtc_set_freq(HZ_RATE_2);                            /* Set the interrupt rate back to 0                             */
    return 0;                                           /* Return 0 on success                                          */
}
