
/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = INITIALIZE_MASK; /* IRQs 0-7  */
uint8_t slave_mask = INITIALIZE_MASK;  /* IRQs 8-15 */

/* void i8259_init( void );
 *   Inputs: none
 *   Return Value: none
 *   Function: Initializes the PIC (both Primary and Secondary) using the four control words 
 *             The secondary PIC is connected to IR2 */
void i8259_init( void ) {
    /* Mask all interrupts on both PICs */
    outb(MASK_ALL, MASTER_8259_PORT_D);
    outb(MASK_ALL, SLAVE_8259_PORT_D);

    /* Sets the four initialization control words for Primary PIC */
    outb(ICW1, MASTER_8259_PORT_C);
    outb(ICW2_MASTER, MASTER_8259_PORT_D);
    outb(ICW3_MASTER, MASTER_8259_PORT_D);
    outb(ICW4, MASTER_8259_PORT_D);

    /* Sets the four initialization control words for Secondary PIC */
    outb(ICW1, SLAVE_8259_PORT_C);
    outb(ICW2_SLAVE, SLAVE_8259_PORT_D);
    outb(ICW3_SLAVE, SLAVE_8259_PORT_D);
    outb(ICW4, SLAVE_8259_PORT_D);

    /* Restores previous mass on both PICs */
    outb(master_mask, MASTER_8259_PORT_D);
    outb(slave_mask, SLAVE_8259_PORT_D);

    /* Enables interrupts for the slave connection IRQ */
    enable_irq(SLAVE_CONNECTION);
}

/* void enable_irq(uint32_t irq_num);
 *   Inputs: uint32_t irq_num --> the IRQ number to enable 
 *   Return Value: none
 *   Function: Enables (unmasks) the specified IRQ
 *             Masks are zero-set:
 *                  0 --> unmasked 
 *                  1 --> masked */
void enable_irq(uint32_t irq_num) {
    unsigned int value_to_unmask;
    unsigned int new_mask;
    unsigned int base_mask = 0x1;

    /* Checks whether the given irq_num is valid */
    if (irq_num < 0 || irq_num > 15) {
        return;
    }

    /* Master PIC (IRQs 0-7) */
    if (irq_num < 8) {
        /* Shifts the base mask by the IRQ number to unmask */
        value_to_unmask = base_mask << irq_num;

        /* Negates value_to_unmask since the masks are zero-set */
        value_to_unmask = ~value_to_unmask;

        /* Combines the previous mask with the new IRQ to unmask */
        new_mask = master_mask & value_to_unmask;

        /* Writes the new mask to the Primary PIC Data port*/
        outb(new_mask, MASTER_8259_PORT_D);

        /* Updates the master_mask value with the new mask */
        master_mask = new_mask;
    }
    /* Slave PIC (IRQs 8-15) */ 
    else {
        /* Subtracts 8 from irq_num to get it in the 0-7 range for computation */
        irq_num = irq_num - 8;

        /* Shifts the base mask by the updated IRQ number to unmask */
        value_to_unmask = base_mask << irq_num;

        /* Negates value_to_unmask since the masks are zero-set */
        value_to_unmask = ~value_to_unmask;

        /* Combines the previous mask with the new IRQ to unmask */
        new_mask = slave_mask & value_to_unmask;                

        /* Writes the new mask to the Primary PIC Data port*/
        outb(new_mask, SLAVE_8259_PORT_D);

        /* Updates the slave_mask value with the new mask */
        slave_mask = new_mask;        
    }
}

/* void disable_irq(uint32_t irq_num);
 *   Inputs: uint32_t irq_num --> the IRQ number to disable 
 *   Return Value: none
 *   Function: Disables (masks) the specified IRQ
 *             Masks are zero-set:
 *                  0 --> unmasked 
 *                  1 --> masked */
void disable_irq(uint32_t irq_num) {
    unsigned int value_to_unmask;
    unsigned int new_mask;
    unsigned int base_mask = 0x1;

    /* Checks whether the given irq_num is valid */
    if (irq_num < 0 || irq_num > 15) {
        return;
    }    

    /* Master PIC (IRQs 0-7) */
    if (irq_num < 8) {
        /* Shifts the base mask by the IRQ number to unmask */
        /* No need to negate since masks are zero-set */
        value_to_unmask = base_mask << irq_num;

        /* Combines the previous mask with the new IRQ to unmask */
        new_mask = master_mask | value_to_unmask;

        /* Writes the new mask to the Primary PIC Data port*/
        outb(new_mask, MASTER_8259_PORT_D);

        /* Updates the master_mask value with the new mask */
        master_mask = new_mask;
    }
    /* Slave PIC (IRQs 8-15) */ 
    else {
        /* Subtracts 8 from irq_num to get it in the 0-7 range for computation */
        irq_num = irq_num - 8;

        /* Shifts the base mask by the updated IRQ number to unmask */
        /* No need to negate since masks are zero-set */
        value_to_unmask = base_mask << irq_num;

        /* Combines the previous mask with the new IRQ to unmask */
        new_mask = slave_mask | value_to_unmask;                

        /* Writes the new mask to the Primary PIC Data port */
        outb(new_mask, SLAVE_8259_PORT_D);

        /* Updates the slave_mask value with the new mask */
        slave_mask = new_mask;        
    }
}

/* void send_eoi(uint32_t irq_num);
 *   Inputs: uint32_t irq_num --> the IRQ number to send the end-of-interrutp signal for
 *   Return Value: none
 *   Function: Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    unsigned int end_of_interrupt;

    /* Checks whether the given irq_num is valid */
    if (irq_num < 0 || irq_num > 15) {
        return;
    }    

    /* Master PIC */
    if (irq_num < 8) {
        /* ORs the end-of-interrupt signal with the interrupt number as per i8259.h */
        end_of_interrupt = EOI | irq_num;

        /* Writes the end-of-interrupt signal to the Primary PIC Data port */
        outb(end_of_interrupt, MASTER_8259_PORT_C);
    }
    /* Slave PIC */
    else {
        /* Subtracts 8 from irq_num to get it in the 0-7 range for computation */
        irq_num = irq_num - 8;

        /* ORs the End-of-interrupt signal with the interrupt number as per i8259.h */
        end_of_interrupt = EOI | irq_num;

        /* Writes the end-of-interrupt signal to the Secondary PIC Data port */
        outb(end_of_interrupt, SLAVE_8259_PORT_C);

        /* Also has to write the end-of-interrupt signal to the Primary PIC Data port at the IR where the Secondary PIC is connected */
        outb(EOI + SLAVE_CONNECTION, MASTER_8259_PORT_C);
    }
}
