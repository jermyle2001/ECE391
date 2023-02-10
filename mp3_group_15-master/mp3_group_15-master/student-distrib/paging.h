#ifndef _PAGING_H
#define _PAGING_H

#define NUM_PAGES               1024
#define STRUCT_SIZE             4
#define SHIFT_12_VIRTUAL_ADDR   12

#define VIDEO_MEM_START_ADDR    0xB8000
#define VIDEO_MEM_BG_START_ADDR 0xB9000
#define KERNEL_START_ADDR       0x400000
#define USER_START_ADDR         0x8000000

/* Defining the page directory entry struct */
typedef struct __attribute__((packed)) page_directory_entry_t {
    unsigned int present         : 1;    /* Bit 0: Present (P), If bit set --> Page in physical memory at the moment        */
    unsigned int read_write      : 1;    /* Bit 1: Read/Write (R/W), If bit set --> Read/Write, Else --> Read only          */
    unsigned int user_supervisor : 1;    /* Bit 2: User/Supervisor (U/S), Controls access to page based on privilege level  */
    unsigned int write_through   : 1;    /* Bit 3: Write-through (PWT), If bit set --> Write-through caching enabled        */
    unsigned int cache_disable   : 1;    /* Bit 4: Cache Disable (PCD), If bit set --> Page not cached                      */
    unsigned int accessed        : 1;    /* Bit 5: Accessed (A), Determines if a PDE or PTE was read during VA translation  */
    unsigned int available_1     : 1;    /* Bit 6: Available (AVL), Unused                                                  */
    unsigned int page_size       : 1;    /* Bit 7: Page Size (PS), Stores page size for this specific entry                 */ 
    unsigned int global          : 1;    /* Bit 8: Global (G), Tells the processor not to invalid corresponding TLB entry   */
    unsigned int available_3     : 3;    /* Bits 11-9: Available (AVL), Unused                                              */
    unsigned int virtual_address : 20;   /* Bits 31-12: 20 bit virtual address to translate (4kB aligned)                   */
} page_directory_entry_t;

/* Defining the page directory entry struct */
typedef struct __attribute__((packed)) page_table_entry_t {
    unsigned int present              : 1;    /* Bit 0: Present (P), If bit set --> Page in physical memory at the moment        */
    unsigned int read_write           : 1;    /* Bit 1: Read/Write (R/W), If bit set --> Read/Write, Else --> Read only          */
    unsigned int user_supervisor      : 1;    /* Bit 2: User/Supervisor (U/S), Controls access to page based on privilege level  */
    unsigned int write_through        : 1;    /* Bit 3: Write-through (PWT), If bit set --> Write-through caching enabled        */
    unsigned int cache_disable        : 1;    /* Bit 4: Cache Disable (PCD), If bit set --> Page not cached                      */
    unsigned int accessed             : 1;    /* Bit 5: Accessed (A), Determines if a PDE or PTE was read during VA translation  */
    unsigned int dirty                : 1;    /* Bit 6: Dirty (D), Determine whether a page has been written to                  */ 
    unsigned int page_attribute_table : 1;    /* Bit 7: Page Attribute Table (PAT), Reserved --> Set to 0                        */
    unsigned int global               : 1;    /* Bit 8: Global (G), Tells processor whether to invalidate TLB entry upon MOV/CL3 */
    unsigned int available_3          : 3;    /* Bits 11-9: Available (AVL), Unused                                              */
    unsigned int virtual_address      : 20;   /* Bits 31-12: 20 bit virtual address to translate (4kB aligned)                   */
} page_table_entry_t;

page_directory_entry_t page_directory[NUM_PAGES] __attribute__((aligned(4096)));
page_table_entry_t page_table[NUM_PAGES] __attribute__((aligned(4096))); 
page_table_entry_t vid_page_table[NUM_PAGES] __attribute__((aligned(4096))); 

/* Called by kernel.c initializes page tables and directory */
extern void page_init( void );

/* Loads a given page directory */
extern void loadPageDirectory( unsigned int *arg );

/* Enable paging and in specific enables 4MB paging*/
extern void enablePaging( void );

/* Clears the tlb by reloading Directory Base Address into register CR3 */
extern void flush_tlb( void );

#endif /* PAGING_H */
