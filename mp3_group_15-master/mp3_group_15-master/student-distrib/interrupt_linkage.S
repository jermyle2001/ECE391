#define ASM 1

#include "interrupt_linkage.h"

# Obtained from the "MP3.1 Review" slides
# Linkage macro to link interrupt handlers to be used properly
# pushal --> Pushes all general registers
# pushfl --> Pushes flags registers
# popfl --> Restores flags registers
# popal --> Restores all general registers
# iret --> Used to return from interrupt
#define INTR_LINK( name, func ) \
    .globl name                ;\
    name:                      ;\
        pushal                 ;\
        pushfl                 ;\
        call func              ;\
        popfl                  ;\
        popal                  ;\
        iret 

INTR_LINK(keyboard_handler_linkage, keyboard_handler);  # Creates the keyboard handler linkage
INTR_LINK(rtc_handler_linkage, rtc_handler);            # Creates the RTC handler linkage
INTR_LINK(pit_handler_linkage, pit_handler);            # Creates the PIT handler linkage
