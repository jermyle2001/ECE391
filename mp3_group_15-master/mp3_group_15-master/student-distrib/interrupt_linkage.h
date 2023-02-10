#ifndef ASM
/* Links the keyboard interrupt handler function through assembly linkage */
extern void keyboard_handler_linkage();

/* Links the RTC interrupt handler function through assembly linkage */
extern void rtc_handler_linkage();

/* Links the PIT interrupt handler funciton through assembly linkage */
extern void pit_handler_linkage();

#endif
