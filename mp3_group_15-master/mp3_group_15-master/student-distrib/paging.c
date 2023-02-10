#include "lib.h"
#include "paging.h"
#include "types.h"

/* Define as "1" for CP5. Define as "0" for else.               */
#define CP5 1
#if CP5
    #define ALT_VID_PAGE_START  0xB8
    #define NUM_ALT_VID_PAGES   5
#endif


/* Define constants for page mapping. We need to define some of */
/* the bits for our virtual address in paging.                  */
#define PRESENT_EN      0x00000001
#define RW_EN           0x00000002
#define RW_SHIFT        1
#define FOUR_MB_PAGE_EN 0x00000080
#define FOUR_MB         0x00400000
#define USER_BIT_EN     0x00000004
#define EIGHT_MB_OFFSET 0x00800000
#define USER_PAGE       32

/* Declare control registers CR0, CR3, and CR4 to be used below */
static unsigned int cr0, cr3, cr4;

/* void page_init( void );
 *   Inputs: none
 *   Return Value: none
 *   Function: Initializes page tables and directory 
 *             (Currently just kernel and video memory) */
void page_init( void ) {    
    unsigned int i;       
    
    /* Loops through and initializes all page directory entries, enables both read and write and sets all */
    /* page tables to be a single 4MB page, marks the page tables that are not being used as not present  */
    for(i = 0; i < NUM_PAGES; i++)
    {
        page_directory[i].present         = 0;
        page_directory[i].read_write      = 1;
        page_directory[i].user_supervisor = 0;
        page_directory[i].write_through   = 0;
        page_directory[i].cache_disable   = 0;
        page_directory[i].accessed        = 0;
        page_directory[i].available_1     = 0;
        page_directory[i].page_size       = 1;
        page_directory[i].global          = 1;
        page_directory[i].available_3     = 0;
        
        /* Initializes the first entry of the page directory to be present and broken into 4KB pages */
        /* Also stores the virtual address of the first page table that contains the video memory    */
        if (i == 0) {
            page_directory[i].present         = 1;
            page_directory[i].page_size       = 0;
            page_directory[i].virtual_address = ( (uint32_t) page_table ) >> SHIFT_12_VIRTUAL_ADDR;
        } 
        /* Initializes the second entry of the page directory to be present and stores */
        /* the starting address of the kernel (4MB Page)                               */
        else if (i == 1) {
            page_directory[i].present         = 1;
            page_directory[i].virtual_address = ( (uint32_t) KERNEL_START_ADDR ) >> SHIFT_12_VIRTUAL_ADDR;
        } 
    }
    
    /* Loops through and initializes all pages in the page table, enables both read and write */
    for(i = 0; i < NUM_PAGES; i++)
    {
        page_table[i].present              = 0;
        page_table[i].read_write           = 1;
        page_table[i].user_supervisor      = 0;
        page_table[i].write_through        = 0;
        page_table[i].cache_disable        = 0;
        page_table[i].accessed             = 0;
        page_table[i].dirty                = 0;
        page_table[i].page_attribute_table = 0;
        page_table[i].global               = 0;
        page_table[i].available_3          = 0;
        page_table[i].virtual_address      = i;

        /* Sets the video memory section to be present */
        if (i << SHIFT_12_VIRTUAL_ADDR == VIDEO_MEM_START_ADDR) {
            page_table[i].present = 1;
        }        
    }  

    #if CP5
        /* CP3.5: Sets the pages for Video Memory. Sets pages for   */
        /* B8, B9, BA, BB, BC for alternative video memory storage. */
        for( i = ALT_VID_PAGE_START; i < ALT_VID_PAGE_START + NUM_ALT_VID_PAGES; i++ )
        {
            page_table[i].present              = 1;
            page_table[i].read_write           = 1;
            page_table[i].user_supervisor      = 1;
            page_table[i].write_through        = 0;
            page_table[i].cache_disable        = 0;
            page_table[i].accessed             = 0;
            page_table[i].dirty                = 0;
            page_table[i].page_attribute_table = 0;
            page_table[i].global               = 0;
            page_table[i].available_3          = 0;
            page_table[i].virtual_address      = i;
        }
    #endif

    
    loadPageDirectory((unsigned int*) page_directory);
    enablePaging();
}

/* void loadPageDirectory(unsigned int *arg);
 *   Inputs: unsigned int *arg --> A pointer to a given page directory
 *   Return Value: none
 *   Function: Loads a given page directory */
void loadPageDirectory(unsigned int *arg) {
    asm volatile 
    (
        "mov 8(%%esp), %%eax    ;"
        "mov %%eax, %%cr3       ;"
        : "=r"(cr3)
        : "r"(cr3)
        : "%eax"
    );
    return;
}

/* void enablePaging( void );
 *   Inputs: none
 *   Return Value: none
 *   Function: Sets Bit 31 of CR0 and Bit 4 of CR4 to enable
 *             paging and specifically 4MB paging */
void enablePaging( void ) {
    asm volatile 
    (
        "mov %%cr4, %%eax           ;"  /* eax <-- cr4, Stores cr4 in eax */
        "or $0x00000010, %%eax      ;"  /* Sets CR4 Bit 4: If bit set --> Enable 4MB Paging */
        "mov %%eax, %%cr4           ;"  /* cr4 <-- eax, Saves eax back into cr4 */                  
        : "=r"(cr4)      
    );
    asm volatile
    (
        "mov %%cr0, %%eax           ;"  /* eax <-- cr0, Stores cr0 in eax */     
        "or $0x80000001, %%eax      ;"  /* Sets CR0 Bit 31: If bit set --> Enable Paging */
        "mov %%eax, %%cr0           ;"  /* cr0 <-- eax, Saves eax back into cr0 */ 
        : "=r"(cr0)
    );    
    return;
}

void flush_tlb( void )
{
    /* Flush the TLB by reloading the Page Directory Base Addr  */
    /* into register CR3. Code can be found referenced here:    */
    /* https://forum.osdev.org/viewtopic.php?f=1&t=25543        */
    asm volatile( 
                  "movl %%cr3, %%eax;" 
                  "movl %%eax, %%cr3;"
                  : /* Output Operands, names of C variables    */
                    /* modified by the ASM code.                */
                  : /* Input Operands, names of C variables     */
                    /* made available to the ASM coce.          */
                  : /* Clobbers, registers of special clobbers  */
                    /* made aware to the assembler.             */
                  "memory", /* Tells the compiler that code     */
                    /* performs memory reads or writes other    */
                    /* than those listed in the input and       */
                    /* output operands.                         */
                  "cc" /* Tells the compiler that code modifies */
                    /* the flags register.                      */
                );


}
