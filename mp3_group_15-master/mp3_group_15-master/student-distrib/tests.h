#ifndef _TESTS_H
#define _TESTS_H

#include "lib.h"

#define NUM_EXCEPTIONS 20
#define EXCEPTION_VECTOR_DE     0
#define EXCEPTION_VECTOR_DB     1
#define EXCEPTION_VECTOR_NMI    2
#define EXCEPTION_VECTOR_BP     3
#define EXCEPTION_VECTOR_OF     4
#define EXCEPTION_VECTOR_BR     5
#define EXCEPTION_VECTOR_UD     6
#define EXCEPTION_VECTOR_NM     7
#define EXCEPTION_VECTOR_DF     8
#define EXCEPTION_VECTOR_CSO    9
#define EXCEPTION_VECTOR_TS     10
#define EXCEPTION_VECTOR_NP     11
#define EXCEPTION_VECTOR_SS     12
#define EXCEPTION_VECTOR_GP     13
#define EXCEPTION_VECTOR_PF     14
#define EXCEPTION_VECTOR_15     15
#define EXCEPTION_VECTOR_MF     16
#define EXCEPTION_VECTOR_AC     17
#define EXCEPTION_VECTOR_MC     18
#define EXCEPTION_VECTOR_XF     19
#define VECTOR_SYS_CALL         0x80

/* Used for rtc_frequency_change_test */
#define SMALL_RATE              1
#define LARGE_RATE              1052
#define NON_2_RATE              42
#define ACCEPTED_RATE_1         4
#define ACCEPTED_RATE_2         16
#define ACCEPTED_RATE_3         1024
#define NEGATIVE_IRQ_NUM        -1

#define VERY_LARGE_IRQ_NUM      100
#define VERY_LARGE_NUM_SLEEP    1000000000
#define MAX_FILE_LENGTH         32

#define KERNEL_START_ADDR       0x400000
#define KERNEL_END_ADDR         0x7FFFFF
#define VIDEO_START_ADDR        0xB8000
#define VIDEO_END_ADDR          0xB8FFF
#define NULL_ADDR               0x0

/* Test Launcher. All tests should be launched from this function.  */
void launch_tests( void );

/* Define tests for various components of Operating System! */

/* //////////////////////////////////////////////////////// */
/* ------------------ CHECKPOINT 1 TESTS ------------------ */
/* //////////////////////////////////////////////////////// */

/* Test checks that the IDT has been initialized correctly. */
int idt_test( void );

/* Test checks that the exceptions work properly. Change    */
/* the number of the arg pass in to test if different       */
/* vectors work. Because the handlers are designed to loop  */
/* when called for Checkpoint 1, exception tests should be  */
/* called at the last test, as they will not allow any      */
/* other program to be run.                                 */
int exception_vector_test( int arg );

/* Same as exception_vector_test, except specifically tests */
/* exception 0 (Divide Error). Refer to previous test for   */
/* more details.                                            */
int exception_DE_test( void );

/* Tests whether passing in a negative IRQ number and an IRQ    */
/* number greater than 15 when trying to enable that specific   */
/* IRQ number will cause the PIC to break   		            */
int enable_irq_inval_test( void );

/* Tests whether passing in a negative IRQ number and an IRQ   */
/* number greater than 15 when trying to disable that specific */
/* IRQ number will cause the PIC to break   		           */
int disable_irq_inval_test( void );

/* Tests whether passing in a negative IRQ number and an IRQ   */
/* number greater than 15 when trying to send an 			   */
/* end-of-interrupt signal for that specific IRQ number will   */
/* cause the PIC to break   		          				   */ 
int send_eoi_inval_test( void );

/* Tests whether RTC is being properly initialized to 2Hz      */ 
int rtc_init_test( void );

/* Tests whether kernel and video memory pages are being set   */
/*  up correctly      										   */ 
int paging_init_test( void );

/* Tests the bounds of the  kernel and video memory pages, */
/* Should page fault for each case since we are testing    */
/* upper and lower out-of-bounds cases                     */
int paging_bounds_test( unsigned int test_num );



/* //////////////////////////////////////////////////////////// */
/* -------------------- CHECKPOINT 2 TESTS -------------------- */
/* //////////////////////////////////////////////////////////// */

/* Helper funtion to print a given string */
extern void print_string( uint8_t* buf );

/* Helper function to print a given file name (limited to 32 bytes) */
void print_filename(uint8_t* buf);

/* Prints all files in the "." directory */
/* Tests dir_open, dir_close, dir_read, and dir_write */
int print_all_files( void );

/* Tests read_dentry_by_name for a valid file name */
int fs_read_val_name_test( void );

/* Tests read_dentry_by_name for an invalid file name */
int fs_read_inval_name_test( void );

/* Tests read_dentry_by_name for a file name longer than 32 bytes */
int fs_read_long_name_test( void );

/* Tests read_dentry_by_index for a valid index */
int fs_read_val_index_test( void );

/* Tests read_dentry_by_index for an invalid index */
int fs_read_inval_index_test( void );

/* Prints out the contents of "frame0.txt" */
/* Tests file_open, file_close, file_read, and file_write */
int fs_print_small_file( void );

/* Prints out the contents of "ls" */
/* Tests file_open, file_close, file_read, and file_write */
int fs_print_executable_file( void );

/* Prints out the contents of "verylargetextwithverylongname.tx(t)" */
/* Tests file_open, file_close, file_read, and file_write */
int fs_print_large_file( void );

/* Tests change frequency function of RTC with different inputs */
int rtc_frequency_change_test( void );

/* Helper function used to change display messages depending on */
/* the input given to the RTC                                   */
void rtc_change_freq_test( unsigned int test_rate );

/* Tests if the terminal_open( ) function of the terminal		*/
/* drivers works as expected. 									*/
int32_t terminal_open_test( void );

/* Tests if the terminal_close( ) function of the terminal 		*/
/* drivers works as expected. 									*/
int32_t terminal_close_test( void );

/* Tests if the write function checks for NULL buffer. 			*/
int32_t terminal_write_NULL_test( void );

/* Tests if the size returns functions correctly, but also that	*/
/* the function handles cases where the number of characters in	*/
/* the buffer does NOT match the number of bytes passed in as 	*/
/* the argument. 												*/
int32_t terminal_write_size_test( void );

/* Tests if the read and write functions of the terminal driver	*/
/* work as expected. Test read and write by typing on the 		*/
/* keyboard and hitting ENTER. The characters typed should be 	*/
int terminal_read_write_test( void );

/* Tests the rtc_open function with the correct return values   */
/* and interrupt rate                                           */
int rtc_open_test( void );

/* Tests the rtc_close function should set the rate of the      */
/* periodic interrupt to 2                                      */
int rtc_close_test( void );

/* Tests the read and write functinality of the rtc drier       */
/* Writes a new periodic interrupt frequency to the rtc and     */
/* then tests read                                              */
int rtc_read_write_test( void );

void syscall_call_test( void );


#endif /* _TESTS_H */
