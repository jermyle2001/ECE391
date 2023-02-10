#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "types.h"
#include "file_system.h"
#include "terminal.h"
#include "syscall.h"
#include "paging.h"

#define PASS 1
#define FAIL 0
#define FREQ_SET 4

/* Define Macros to enable Checkpoint Tests... */
#define RUN_CHECKPOINT1_TESTS 1
#define RUN_CHECKPOINT2_TESTS 1
#define RUN_CHECKPOINT3_TESTS 0
#define RUN_CHECKPOINT4_TESTS 0
#define RUN_CHECKPOINT5_TESTS 0



/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

/* Test suite entry point */
void launch_tests( void ){
	unsigned int i;
	/* Line added here to get rid of warnings						*/
	i = 0;
	/* Clear the screen so we can see more clearly what is going on */
	/* when we try to run the tests. Some tests will need to be 	*/
	/* commented out in order to run other tests. For 				*/
	/* Checkpoint 3.1, this means that we will need to comment out 	*/
	/* or change the exception vector test if we want to test 		*/
	/* multiple vectors, since the handler is configured to loop 	*/
	/* infinitely once an exception is called. 						*/
	/* The #define statements at the top allow the checkpoint 		*/
	/* tests to be easily included or excluded, at least for each	*/
	/* separate checkpoint. Change as needed. 						*/
	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;
	// printf("Running tests!\n");

#if RUN_CHECKPOINT1_TESTS
	/* //////////////////////////////////////////////////////////// */
	/* -------------------- CHECKPOINT 1 TESTS -------------------- */
	/* //////////////////////////////////////////////////////////// */
	printf("<----------------------------- CHECKPOINT 1 TESTS ----------------------------->");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
	clear_and_reset_screen();	
	screen_x = 0;
	screen_y = 0;	

	/* ------------------ RTC INITIALIZATION TEST ----------------- */
	// TEST_OUTPUT("rtc_init_test", rtc_init_test( void ));	

	/* ---------------------- BASIC IDT TEST ---------------------- */
	/* Basic IDT test. Checks if entries have been set correctly.   */
	TEST_OUTPUT("idt_test", idt_test( ));
	printf("\n");
	
	/* ----------------- BASIC PIC ERROR CHECKING ----------------- */
	TEST_OUTPUT("enable_irq_inval_test", enable_irq_inval_test( ));
	printf("\n");
	TEST_OUTPUT("disable_irq_inval_test", disable_irq_inval_test( ));
	printf("\n");
	TEST_OUTPUT("send_eoi_inval_test", send_eoi_inval_test( ));
	printf("\n");

    /* --------------- PAGING INITIALIZATION TESTS  --------------- */
    TEST_OUTPUT("paging_init_test", paging_init_test( ));
	printf("\n");

	/* The paging bounds tests accepst an integer from 1-4 to test  */
	/* the upper and lower bounds of the kernel and video page      */
	/* initialization (SHOULD CAUSE PAGE FAULT)						*/
	/* 1 - Lower bound kernel memory, 2 - Upper bound kernel memory */
	/* 3 - Lower bound video memory, 4 - Upper bound video memory   */
	// TEST_OUTPUT("paging_bounds_test", paging_bounds_test(1));

	/* The following two functions test the exception vectors.      */
	/* They should be commented out if we want to test the RTC or   */
	/* the keyboard, since the handler for Checkpoint 3.1 is 	    */
	/* configured to make the handler loop infinitely and unable    */
	/* to process any more input. 								    */

	/* --------------------- EXCEPTION TESTS ---------------------- */
	/*            EXCEPTION TEST #1: exception_DE_test              */
	/* Function that tests whether we can raise the DE exception,   */
	/* WITHOUT explicitly calling an INT instruction. Comment out   */
	/* if we want to test other vectors, or other code in general   */
	// TEST_OUTPUT( "exception_DE_test", exception_DE_test( ));

	/*----------EXCEPTION TEST #2: exception_vector_test------------*/
	/* Function that tests that each exception can be raised via    */
	/* an INT instruction. More or less checks that the handler     */
	/* works properly, and raises the right exception when the      */
	/* INT instruction is invoked. 								    */
	// TEST_OUTPUT("exception_vector_test", exception_vector_test( 0x80 ));

	printf("Continuing...\n");
	for (i = 0; i < 2 * VERY_LARGE_NUM_SLEEP; i++) {}
	clear_and_reset_screen();	
	screen_x = 0;
	screen_y = 0;	
#endif

#if RUN_CHECKPOINT2_TESTS
	/* //////////////////////////////////////////////////////////// */
	/* -------------------- CHECKPOINT 2 TESTS -------------------- */
	/* //////////////////////////////////////////////////////////// */
	printf("<----------------------------- CHECKPOINT 2 TESTS ----------------------------->");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
	clear_and_reset_screen();	
	screen_x = 0;
	screen_y = 0;	
	
	/*--------------------RTC FREQUENCY TEST------------------------*/
	/* Function will pass multiple different values into for 		*/
	/* setting the RTC clock rate. Will show ones that are not 		*/
	/* Accepted and ones that are accepted will be set and 			*/
	/* demonstrate that the clock was set succesfully 				*/
	/* defines in tests.h an be set to test different vaules to test */
	rtc_init();
	
	TEST_OUTPUT("rtc_frequency_change_test", rtc_frequency_change_test());
	
	printf("Continuing...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	clear_and_reset_screen();
	screen_x = 0;
	screen_y = 0;

	/*----------RTC OPEN, CLOSE, READ, and Write TEST---------------*/
	/* Tests the read, open, close, and write functions of the rtc  */
	TEST_OUTPUT("rtc_open_test", rtc_open_test( ));
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
	clear_and_reset_screen();
	screen_x = 0;
	screen_y = 0;

	TEST_OUTPUT("rtc_close_test", rtc_close_test( ));	
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
	clear_and_reset_screen();
	screen_x = 0;
	screen_y = 0;

	TEST_OUTPUT("rtc_read_write_test", rtc_read_write_test( ));
	

	printf("Testing File Systems Next...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;
	disable_irq(RTC_IRQ_NUM);

	/* -------------------- FILE SYSTEM TESTS --------------------- */
	TEST_OUTPUT("print_all_files", print_all_files( ));
	
	printf("Continuing...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP * 2; i++) {}
	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;

	TEST_OUTPUT("fs_read_val_name_test", fs_read_val_name_test( ));
	printf("\n");
	TEST_OUTPUT("fs_read_inval_name_test", fs_read_inval_name_test( ));
	printf("\n");
	TEST_OUTPUT("fs_read_long_name_test", fs_read_long_name_test( ));
	printf("\n");
	TEST_OUTPUT("fs_read_val_index_test", fs_read_val_index_test( ));
	printf("\n");
	TEST_OUTPUT("fs_read_inval_index_test", fs_read_inval_index_test( ));
	printf("\n");

	printf("Continuing...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	clear_and_reset_screen();
	screen_x = 0;
	screen_y = 0;

	TEST_OUTPUT("fs_print_small_file", fs_print_small_file());

	printf("Continuing...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	clear_and_reset_screen();
	screen_x = 0;
	screen_y = 0;

	TEST_OUTPUT("fs_print_executable_file", fs_print_executable_file());
	
	printf("Continuing...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	clear_and_reset_screen();
	screen_x = 0;
	screen_y = 0;
	
	TEST_OUTPUT("fs_print_large_file", fs_print_large_file());
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;	

	printf("Testing Terminal Next...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;
	#endif
	/* -------------------- TERMINAL OPEN TEST -------------------- */
	/* Tests if the terminal_open( ) function of the terminal		*/
	/* drivers works as expected. 									*/
	TEST_OUTPUT("terminal_open_test", terminal_open_test( ));
	
	/* -------------------- TERMINAL CLOSE TEST ------------------- */
	/* Tests if the terminal_close( ) function of the terminal 		*/
	/* drivers works as expected. 									*/
	TEST_OUTPUT("terminal_close_test", terminal_close_test( ));


	/* ------------------ TERMINAL WRITE NULL TEST ---------------- */
	/* Tests if the write function checks for NULL buffer. 			*/
	TEST_OUTPUT("terminal_write_NULL_test", terminal_write_NULL_test( ));

	/* ------------------ TERMINAL WRITE SIZE TEST ---------------- */
	/* Tests if the size returns functions correctly, but also that	*/
	/* the function handles cases where the number of characters in	*/
	/* the buffer does NOT match the number of bytes passed in as 	*/
	/* the argument. 												*/
	TEST_OUTPUT("terminal_write_size_test", terminal_write_size_test( ));	

	/* -------------------- TERMINAL DRIVER TEST ------------------ */
	/* Function that tests that the terminal driver works. Type		*/
	/* keys into the keyboard when the test is being run, and press	*/
	/* ENTER to complete the test. The keys should be echoed onto 	*/
	/* the screen. 													*/
	TEST_OUTPUT("terminal_driver_test", terminal_read_write_test( ));

	printf("Continuing...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;

#if RUN_CHECKPOINT3_TESTS
	/* Test if the specified system call vector calls properly...	*/
	syscall_call_test( );
#endif

#if RUN_CHECKPOINT4_TESTS

#endif

#if RUN_CHECKPOINT5_TESTS

#endif


}

/* //////////////////////////////////////////////////////////// */
/* -------------------- CHECKPOINT 1 TESTS -------------------- */
/* //////////////////////////////////////////////////////////// */

/* IDT Test
 * 
 * Asserts that the exception vector entries have been set properly.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test( void )
{
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < NUM_EXCEPTIONS; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			printf("Offset not set. Failing test.\n");
			assertion_failure();
			result = FAIL;
		}
		if( idt[ i ].present != 1 )
		{
			printf("Present bit not set correctly (Set to %d). Failing test...\n", idt[ i ].present );
			assertion_failure();
			result = FAIL;
		}
		if( idt[ i ].dpl != 0)
		{
			printf("dpl not set correctly (Set to %d). Failing test...\n", idt[ i ].dpl );
			assertion_failure();
			result = FAIL;
		}
		if( idt[ i ].reserved0 != 0 )
		{
			printf("reserved0 not set correctly (Set to %d). Failing test...\n", idt[ i ].reserved0 );
			assertion_failure();
			result = FAIL;
		}
		if( idt[ i ].reserved1 != 1 )
		{
			printf("reserved1 not set correctly (Set to %d). Failing test...\n", idt[ i ].reserved1 );
			assertion_failure();
			result = FAIL;
		}
		if( idt[ i ].reserved2 != 1 )
		{
			printf("reserved 2 not set correctly (Set to %d). Failing test...\n", idt[ i ].reserved2 );
			assertion_failure();
			result = FAIL;
		}
		if( idt[ i ].reserved3 != 0 )
		{
			printf("reserved 3 not set correctly (Set to %d). Failing test...\n", idt[ i ].reserved3 );
			assertion_failure();
			result = FAIL;
		}
		if( idt[ i ].reserved4 != 0 )
		{
			printf("reserved 4 not set correctly (Set to %d). Failing test...\n", idt[ i ].reserved4 );
			assertion_failure();
			result = FAIL;
		}
		if( i != 0x80 )
		{
			if( idt[ i ].dpl != 0 )
			{
				assertion_failure();
				result = FAIL;
			}
		}
		else
		{
			if( idt[ i ].dpl != 3 )
			{
				assertion_failure();
				result = FAIL;
			}
		}
		if( idt[ i ].seg_selector != KERNEL_CS )
		{
			assertion_failure();
			result = FAIL;
		}
		if( idt[ i ].size != 1 )
		{
			assertion_failure();
			result = FAIL;
		}

	}

	return result;
}

/* EXCEPTION VECTOR TESTS */
/* Tests whether the INT instruction invokes the corresponding */
/* exception vector. Freezes the kernel afterwards. uncomment  */
/* the vector that the program wants to test. Includes system  */
/* calls. 													   */
/* Inputs: Vector ID number which we want to test. 			   */
/* Outputs: None. 											   */
/* Side Effects: Calls the exception handler corresponding to  */
/*   the corresponding vector. Loops the program infinitely    */
/*   once the exception has been handled. 					   */
/* Coverage: Load IDT, Exception Handlers, System Call for 3.1 */
int exception_vector_test( int arg )
{
	/* Print Test Header with macro*/
	TEST_HEADER;

	/* Set result to PASS for now */
	int result = PASS;

	/* Invoke the interrupt corresponding to the vector number */
	/* desired. For Checkpoint 3.1, will not quash the user    */
	/* program but rather loop infinitely instead. 			   */
	switch( arg )
	{
		case EXCEPTION_VECTOR_DE:
			asm volatile("int $0");
			break;
		case EXCEPTION_VECTOR_DB:
			asm volatile("int $1");
			break;
		case EXCEPTION_VECTOR_NMI:
			asm volatile("int $2");
			break;
		case EXCEPTION_VECTOR_BP:
			asm volatile("int $3");
			break;
		case EXCEPTION_VECTOR_OF:
			asm volatile("int $4");
			break;
		case EXCEPTION_VECTOR_BR:
			asm volatile("int $5");
			break;
		case EXCEPTION_VECTOR_UD:
			asm volatile("int $6");
			break;
		case EXCEPTION_VECTOR_NM:
			asm volatile("int $7");
			break;
		case EXCEPTION_VECTOR_DF:
			asm volatile("int $8");
			break;
		case EXCEPTION_VECTOR_CSO:
			asm volatile("int $9");
			break;
		case EXCEPTION_VECTOR_TS:
			asm volatile("int $10");
			break;
		case EXCEPTION_VECTOR_NP:
			asm volatile("int $11");
			break;
		case EXCEPTION_VECTOR_SS:
			asm volatile("int $12");
			break;
		case EXCEPTION_VECTOR_GP:
			asm volatile("int $13");
			break;
		case EXCEPTION_VECTOR_PF:
			asm volatile("int $14");
			break;
		case EXCEPTION_VECTOR_15:
			asm volatile("int $15");
			break;
		case EXCEPTION_VECTOR_MF:
			asm volatile("int $16");
			break;
		case EXCEPTION_VECTOR_AC:
			asm volatile("int $17");
			break;
		case EXCEPTION_VECTOR_MC:
			asm volatile("int $18");
			break;
		case EXCEPTION_VECTOR_XF:
			asm volatile("int $19");
			break;
		case VECTOR_SYS_CALL:
			asm volatile("int $128");
			break;

		default:
			asm volatile("int $0");
			break;
	}

	return result;
}

/* EXCEPTION DE TEST */
/* Tests whether the divide by zero exception gets raised when */
/* the corresponding reason is passed through. Freezes the     */
/* kernel afterwards. 										   */
/* Inputs: None. 											   */
/* Outputs: None. 											   */
/* Side Effects: Calls the handler, which should loop the code */
/*   for Checkpoint 3.1 									   */
/* Coverage: Load IDT, Exception Handlers, Exception #0 (DE)   */
int exception_DE_test( void )
{
	TEST_HEADER;

	int result = PASS;
	int dividend;
	int divisor;
	int test_DE;

	dividend = 1;
	divisor = 0;
	test_DE = dividend / divisor;

	return result;
}

/* ENABLE_IRQ INVALID TEST */
/* Tests whether passing in a negative IRQ number and an IRQ   */
/* number greater than 15 when trying to enable that specific  */
/* IRQ number will cause the PIC to break   		           */
/* Inputs: None. 											   */
/* Outputs: None. 											   */
/* Side Effects: If the test fails, nothing should show on 	   */
/*				 on the screen when typing					   */
/* Coverage: Enable IRQ for the PIC							   */
int enable_irq_inval_test( void )
{
	TEST_HEADER;

	enable_irq(NEGATIVE_IRQ_NUM);
	enable_irq(VERY_LARGE_IRQ_NUM);

	return PASS;
}

/* DISABLE_IRQ INVALID TEST */
/* Tests whether passing in a negative IRQ number and an IRQ   */
/* number greater than 15 when trying to disable that specific */
/* IRQ number will cause the PIC to break   		           */
/* Inputs: None. 											   */
/* Outputs: None. 											   */
/* Side Effects: If the test fails, nothing should show on 	   */
/*				 on the screen when typing					   */
/* Coverage: Disable IRQ for the PIC					       */
int disable_irq_inval_test( void )
{
	TEST_HEADER;

	disable_irq(NEGATIVE_IRQ_NUM);
	disable_irq(VERY_LARGE_IRQ_NUM);

	return PASS;
}

/* SEND_EOI INVALID TEST */
/* Tests whether passing in a negative IRQ number and an IRQ   */
/* number greater than 15 when trying to send an 			   */
/* end-of-interrupt signal for that specific IRQ number will   */
/* cause the PIC to break   		          				   */ 
/* Inputs: None. 											   */
/* Outputs: None. 											   */
/* Side Effects: If the test fails, nothing should show on 	   */
/*				 on the screen when typing					   */
/* Coverage: Send EOI for the PIC						       */
int send_eoi_inval_test( void )
{
	TEST_HEADER;

	send_eoi(NEGATIVE_IRQ_NUM);
	send_eoi(VERY_LARGE_IRQ_NUM);

	return PASS;
}

/* RTC INIT TEST */
/* Tests whether RTC is being properly initialized to 2Hz      */ 
/* Also then tests settig the RTC to different frequencies     */
/* Inputs: None. 											   */
/* Outputs: None. 											   */
/* Side Effects: Should increment video memory such that a 	   */
/*				 bunch of characters will flip through screen  */
/* Coverage: RTC initialization							       */
int rtc_init_test( void )
{
	TEST_HEADER;
	int i;
	for (i = 0; i < (VERY_LARGE_NUM_SLEEP); i++) {}

	rtc_init();

	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}

	disable_irq(RTC_IRQ_NUM);

	clear();

	return PASS;
}

/* PAGING INIT TEST */
/* Tests whether kernel and video memory pages are being set   */
/*  up correctly      										   */ 
/* Inputs: None. 											   */
/* Outputs: None. 											   */
/* Side Effects: Will page fault if not initialized correctly  */
/* Coverage: Kernel and Video Memory Paging initialization     */
int paging_init_test( void )
{
	
	TEST_HEADER;
	uint8_t dereferenced;

	uint8_t* kernal_start_pointer = (uint8_t*) KERNEL_START_ADDR;
	dereferenced = *kernal_start_pointer;

	uint8_t* kernel_end_pointer = (uint8_t*) KERNEL_END_ADDR;
	dereferenced = *kernel_end_pointer;

	uint8_t* video_start_pointer = (uint8_t*) VIDEO_START_ADDR;
	dereferenced = *video_start_pointer;

	uint8_t* video_end_pointer = (uint8_t*) VIDEO_END_ADDR;
	dereferenced = *video_end_pointer;

	return PASS;
}

/* PAGING BOUNDS TEST */
/* Tests the bounds of the  kernel and video memory pages      */
/*  up correctly      										   */ 
/* Inputs: unsigned int test_num							   */
/*  	    --> 1 - Lower bound kernel memory                  */
/*  	    --> 2 - Upper bound kernel memory                  */
/*  	    --> 3 - Lower bound video memory                   */
/*  	    --> 4 - Lower bound video memory                   */
/* Outputs: None. 											   */
/* Side Effects: Should page fault for each case since we are  */
/*			     testing upper and lower out-of-bounds cases   */
/* Coverage: Kernel and Video Memory Paging initialization     */
int paging_bounds_test(unsigned int test_num)
{
	TEST_HEADER;
	uint8_t dereferenced;
	uint8_t* kernel_before_pointer;
	uint8_t* kernel_after_pointer;
	uint8_t* video_before_pointer;
	uint8_t* video_after_pointer;

	switch(test_num) {
		case 1:
			kernel_before_pointer = (uint8_t*) (KERNEL_START_ADDR - 1);
			dereferenced = *kernel_before_pointer;
			break;

		case 2:
			kernel_after_pointer = (uint8_t*) (KERNEL_END_ADDR + 1);
			dereferenced = *kernel_after_pointer;
			break;

		case 3:
			video_before_pointer = (uint8_t*) (VIDEO_START_ADDR - 1);
			dereferenced = *video_before_pointer;
			break;

		case 4:
			video_after_pointer = (uint8_t*) (VIDEO_END_ADDR + 1);
			dereferenced = *video_after_pointer;
			break;

		default:
			break;
	}

	return FAIL;
}



/* //////////////////////////////////////////////////////////// */
/* -------------------- CHECKPOINT 2 TESTS -------------------- */
/* //////////////////////////////////////////////////////////// */

/* PRINT STRING */
/* Helper function to print out a given string 				   */ 
/* Inputs: uint8_t* buf - A given buffer to print out		   */
/* Outputs: None. 											   */
/* Side Effects: Prints the given buffer out to the console    */
/* Coverage: Helper function for file_system.c tests	       */
void print_string( uint8_t* buf ) {
	unsigned int i = 0;
	unsigned int length = strlen((const int8_t*) buf);

	for (i = 0; i < length; i++) {
		putc(buf[i]);
	}
	return;
}

/* PRINT FILENAME */
/* Helper function to print out a given filename (32 bytes)	   */ 
/* Inputs: uint8_t* buf - A given buffer to print out		   */
/* Outputs: None. 											   */
/* Side Effects: Prints the given buffer out to the console    */
/* Coverage: Helper function for file_system.c tests	       */
void print_filename( uint8_t* buf ) {
    int i =0;
	while(buf[i] != NULL && i < MAX_FILE_LENGTH) {
		putc(buf[i]);
		i++;
	}
	while(i < MAX_FILE_LENGTH) { /*TODO: Max file length is 32*/
		printf(" ");
		i++;
	}
}

/* READ DENTRY FOR VALID FILE NAME TEST */
/* Tests read_dentry_by_name for valid file name "frame0.txt"  */ 
/* Inputs: None									   			   */
/* Outputs: Will return PASS is the file is found			   */
/* Side Effects: None								           */
/* Coverage: read_dentry_by_name() in file_system.c		       */
int fs_read_val_name_test( void )
{
	TEST_HEADER;
	dentry_t test_dentry;
	unsigned int result;
	char test_name[] = "frame0.txt";
	result = read_dentry_by_name((const uint8_t*) test_name, &test_dentry);

	if (result == -1) {
		return FAIL;
	} else {
		return PASS;
	}
}

/* READ DENTRY FOR INVALID FILE NAME TEST */
/* Tests read_dentry_by_name for an invalid file name 		   */
/* "invalid_name.txt "										   */ 
/* Inputs: None									   			   */
/* Outputs: Will return PASS is the file is not found		   */
/* Side Effects: None								           */
/* Coverage: read_dentry_by_name() in file_system.c		       */
int fs_read_inval_name_test( void )
{
	TEST_HEADER;
	dentry_t test_dentry;
	unsigned int result;
	char test_name[] = "invalid_name.txt";
	result = read_dentry_by_name((const uint8_t*) test_name, &test_dentry);

	if (result == -1) {
		return PASS;
	} else {
		return FAIL;
	}
}

/* READ DENTRY FOR LONG FILE NAME TEST */
/* Tests read_dentry_by_name for an file name longer than 32B  */ 
/* Inputs: None									   			   */
/* Outputs: Will return PASS is the file is not found		   */
/*			(Invalid file name / over 32 bytes)				   */
/* Side Effects: None								           */
/* Coverage: read_dentry_by_name() in file_system.c		       */
int fs_read_long_name_test( void )
{
	TEST_HEADER;
	dentry_t test_dentry;
	unsigned int result;
	char test_name[] = "verylargetextwithverylongname.txt";
	result = read_dentry_by_name((const uint8_t*) test_name, &test_dentry);

	if (result == -1) {
		return FAIL;
	} else {
		return PASS;
	}	
}

/* READ DENTRY FOR VALID INDEX TEST */
/* Tests read_dentry_by_name for valid index 0				   */ 
/* Inputs: None									   			   */
/* Outputs: Will return PASS is the file is found			   */
/* Side Effects: None								           */
/* Coverage: read_dentry_by_index() in file_system.c	       */
int fs_read_val_index_test( void )
{
	TEST_HEADER;
	dentry_t test_dentry;
	unsigned int result;
	unsigned int test_index = 0;
	result = read_dentry_by_index(test_index, &test_dentry);

	if (result == -1) {
		return FAIL;
	} else {
		return PASS;
	}
}

/* READ DENTRY FOR INVALID INDEX TEST */
/* Tests read_dentry_by_name for an invalid file index -1	   */ 
/* Inputs: None									   			   */
/* Outputs: Will return PASS is the file is not found		   */
/* Side Effects: None								           */
/* Coverage: read_dentry_by_index() in file_system.c	       */
int fs_read_inval_index_test( void )
{
	TEST_HEADER;
	dentry_t test_dentry;
	unsigned int result;
	unsigned int test_index = -1;
	result = read_dentry_by_index(test_index, &test_dentry);

	if (result == -1) {
		return PASS;
	} else {
		return FAIL;
	}
}

/* PRINT ALL FILES TEST */
/* Prints all files in the given directory "."	   			   */ 
/* Inputs: None									   			   */
/* Outputs: Will return PASS if no failures occur from 		   */
/*			dir_open, dir_read, dir_write, and dir_close	   */
/* Side Effects: None								           */
/* Coverage: dir_open(), dir_close(), dir_read(), dir_write()  */
/*			 in file_system.c	       						   */
int print_all_files( void )
{
	dentry_t curr_dentry;
	inode_t curr_inode;	
    uint8_t test_buf[MAX_FILE_LENGTH];
	char test_dir[] = ".";
	int32_t test_fd = 2;
	int32_t num_bytes_to_read = MAX_FILE_LENGTH; 

	clear_and_reset_screen();
	dir_open((const uint8_t*) test_dir);

	while(dir_read(test_fd, test_buf, num_bytes_to_read) > 0) {
		printf("file_name: ");
		print_filename(test_buf);
		read_dentry_by_name(test_buf, &curr_dentry);
		curr_inode = *((inode_t*)(p_inode_addr + (curr_dentry.index_node_num)));
		printf("  file_type: %d, file_size: %d", curr_dentry.file_type, curr_inode.file_size);
		printf("\n");
	}

	if(dir_write(test_fd, test_buf, num_bytes_to_read) != -1) {
		return FAIL;
	}

	dir_close(test_fd);
	
	return PASS;
}

/* PRINT SMALL FILE TEST */
/* Reads and outputs the contents of the file "frame0.txt"	   */ 
/* Inputs: None									   			   */
/* Outputs: Will return PASS if no failures occur from 		   */
/*			file_open, file_read, file_write, and file_close   */
/* Side Effects: None								           */
/* Coverage: file_open(), file_close(), file_read(), 		   */
/*			 file_write(), and read_data() in file_system.c	   */
int fs_print_small_file( void )
{
	char test_file[] = "frame0.txt";
	char test_buf[SIZE_DATA_BLOCK];
	unsigned int num_bytes_to_read = SIZE_DATA_BLOCK;
	int32_t test_fd = 2;
	unsigned int result = file_open((const uint8_t*) test_file);

	if (result == -1) {
		return FAIL;
	}

	printf("START READING FILE: ");
	print_string((uint8_t*) test_file);
	printf("\n");
	while ((file_read(test_fd, test_buf, num_bytes_to_read)) > 0) {
		print_string((uint8_t*) test_buf);
	}
	printf("\n");

	if (file_write(test_fd, test_buf, num_bytes_to_read) != -1) {
		return FAIL;
	}

	file_close(test_fd);

	return PASS;
}

/* PRINT EXECUTABLE TEST */
/* Reads and outputs the contents of the file "ls"	   		   */ 
/* Inputs: None									   			   */
/* Outputs: Will return PASS if no failures occur from 		   */
/*			file_open, file_read, file_write, and file_close   */
/* Side Effects: None								           */
/* Coverage: file_open(), file_close(), file_read(), 		   */
/*			 file_write(), and read_data() in file_system.c	   */
int fs_print_executable_file( void )
{
	char test_file[] = "shell";
	char test_buf[SIZE_DATA_BLOCK*2];
	unsigned int num_bytes_to_read = SIZE_DATA_BLOCK*2;
	int32_t test_fd = 2;
	unsigned int result = file_open((const uint8_t*) test_file);

	if (result == -1) {
		return FAIL;
	}

	printf("START READING FILE: ");
	print_string((uint8_t*) test_file);
	printf("\n");
	while ((file_read(test_fd, test_buf, num_bytes_to_read)) > 0) {
		print_string((uint8_t*) test_buf);
	}
	printf("\n");

	if (file_write(test_fd, test_buf, num_bytes_to_read) != -1) {
		return FAIL;
	}

	file_close(test_fd);

	return PASS;
}

/* PRINT LARGE FILE TEST */
/* Reads and outputs the contents of the file 				   */
/* verylargetextwithverylongname.tx(t) 						   */
/* Inputs: None									   			   */
/* Outputs: Will return PASS if no failures occur from 		   */
/*			file_open, file_read, file_write, and file_close   */
/* Side Effects: None								           */
/* Coverage: file_open(), file_close(), file_read(), 		   */
/*			 file_write(), and read_data() in file_system.c	   */
int fs_print_large_file( void )
{
	char test_file[] = "verylargetextwithverylongname.tx";
	char test_buf[SIZE_DATA_BLOCK * 2];
	unsigned int num_bytes_to_read = SIZE_DATA_BLOCK * 2;
	int32_t test_fd = 2;
	unsigned int result = file_open((const uint8_t*) test_file);
	unsigned int num_bytes_read = 0;

	if (result == -1) {
		return FAIL;
	}

	printf("START READING FILE: ");
	print_string((uint8_t*) test_file);
	printf("\n");
	num_bytes_read = file_read(test_fd, test_buf, num_bytes_to_read);
	print_string((uint8_t*) test_buf);
	printf("\n");

	printf("\n*BEGIN* NUM BYTES READ: %d *END*\n", num_bytes_read);

	if (file_write(test_fd, test_buf, num_bytes_to_read) != -1) {
		return FAIL;
	}

	file_close(test_fd);

	return PASS;
}

/* TERMINAL OPEN TEST */
/* Tests if the terminal_open( ) function of the terminal		*/
/* drivers works as expected. 									*/
/* Inputs: None. 												*/
/* Outputs: -1. Technically the file always fails to be read, 	*/
/* 	        since we are just looking at the terminal driver. 	*/
/* Side Effects: None. 											*/
int32_t terminal_open_test( void )
{
	int i;

	/* Test if NULL will cause unexpected output.	*/
	i = terminal_open( ( uint8_t* ) NULL );
	if( i != -1 )
	{
		return FAIL;
	}

	/* Test if 0 will cause unexpected output. 		*/
	i = terminal_open( ( uint8_t* ) 0x00 );
	if (i != -1 )
	{
		return FAIL;
	}

	/* Test if a random input will cause 			*/
	/* unexpected output. 							*/
	i = terminal_open( ( uint8_t* ) 0xFF );
	if( i != 0 )
	{
		return FAIL;
	}

	/* All tests passed. Return PASS. */
	return PASS;
}

/* TERMINAL CLOSE TEST							   				*/
/* Tests if the terminal_close( ) function of the terminal 		*/
/* drivers works as expected. 									*/
/* Inputs: int32_t fd -> File Descriptor. 						*/
/* Outputs: -1. Despite any input, should output -1, since we 	*/
/* 			aren't actually closing any files, we are "failing"	*/
/* 			to close the file. Closing files aren't relevant to	*/
/* 			the terminal driver. 								*/
/* Side Effects: None. 											*/
int32_t terminal_close_test( void )
{
	int i;

	/* Test if NULL will cause unexpected output.	*/
	i = terminal_close( ( int32_t ) NULL );
	if( i != -1 )
	{
		return FAIL;
	}

	/* Test if 0 will cause unexpected output. 		*/
	i = terminal_close( ( int32_t ) 0x00 );
	if (i != -1 )
	{
		return FAIL;
	}

	/* Test if a random input will cause 			*/
	/* unexpected output. 							*/
	i = terminal_close( ( int32_t )  0xFF );
	if( i != -1 )
	{
		return FAIL;
	}

	/* All tests passed. Return PASS. */
	return PASS;
}


/* TERMINAL WRITE NULL TEST */
/* Tests if the write function checks for NULL buffer. 			*/
/* Inputs: int32_t fd -> File Descriptor - argument doesn't		*/
/* 						 matter in context of function. 		*/
/* 		   const uint8_t* buf -> string to be printed. Set to 	*/
/* 								 NULL for testing. 				*/
/*  	   int32_t nbytes -> number of bytes to be written. 	*/
/*	 						 Doesn't matter for this test. 		*/
/* Outputs: -1. Technically failed to write buffer because it 	*/
/* 				was NULL. 										*/
/* Side Effects: None. 											*/
int32_t terminal_write_NULL_test( void )
{
	int i;
	uint8_t string[] = "abcdef";
	i = terminal_write( NULL, NULL, 3 );
	if( i != -1 )
	{
		return FAIL;
	}
	i = terminal_write( NULL, string, NULL );
	if( i != -1 )
	{
		return FAIL;
	}
	i = terminal_write( NULL, string, 0 );
	if( i != -1 )
	{
		return FAIL;
	}

	/* Also reset keyboard buffer to prevent printing issues	*/
	reset_keyboard_buffer( );
	return PASS;
}


/* TERMINAL_WRITE_SIZE_TEST */
/* Tests if the size returns functions correctly, but also that	*/
/* the function handles cases where the number of characters in	*/
/* the buffer does NOT match the number of bytes passed in as 	*/
/* the argument. 												*/
/* Inputs: int32_t fd -> File Descriptor - argument doesn't		*/
/* 						 matter in context of function. 		*/
/* 		   const uint8_t* buf -> string to be printed. Set to 	*/
/* 						 "abcdef", and should be visually 		*/
/* 						 confirmed to work correctly. It should	*/
/* 						 print abc abcdef abcdef			*/
/*  	   int32_t nbytes -> number of bytes to be written. 	*/
/* 						 Varies per testing goal. 				*/

int32_t terminal_write_size_test( void )
{
	int i;
	uint8_t c = 0x0A;
	uint8_t string[] = "abcdef";
	/* Synchronize custom terminal screen tracking with given	*/
	/* screen tracking in lib.c									*/
	terminal_y[ display_terminal ] = screen_y;
	terminal_x[ display_terminal ] = screen_x;


	/* The following should print "abc"	*/
	keyboard_putc( c );
	i = terminal_write( NULL, string, 3 );
	if( i != 3 )
	{
		return FAIL;
	}
	/* The following should print "abcdef"	*/
	keyboard_putc( c );
	i = terminal_write( NULL, string, 6 );
	if( i != 6 )
	{
		return FAIL;
	}
	/* The following should print "abcdef"	*/
	keyboard_putc( c );
	i = terminal_write( NULL, string, 9 );
	if( i != 6 )
	{
		return FAIL;
	}
	keyboard_putc( c );

	/* Also reset keyboard buffer to prevent printing issues	*/
	reset_keyboard_buffer( );

	/* Synchronize screen coordinates with terminal coordinates	*/
	screen_y = terminal_y[ display_terminal ];
	screen_x = terminal_x[ display_terminal ];

	printf("\n");
	return PASS;
}



/* TERMINAL READ/WRITE TEST */
/* Tests if the read and write functions of the terminal driver	*/
/* work as expected. Test read and write by typing on the 		*/
/* keyboard and hitting ENTER. The characters typed should be 	*/
/* echoed onto the keyboard. 									*/
/* Inputs: None. 												*/
/* Outputs: None. 												*/
/* Side Effects: Reads the keyboard buffer until ENTER pressed, */
/* and then writes the contents to screen. See terminal_write	*/
/* for additional details. 										*/
int terminal_read_write_test( void )
{
	TEST_HEADER;

	int i;

	while( 1 )
	{
		/* Declare a buffer to be written to. */
		uint8_t buf[ BUFFER_SIZE ];

		/* Enable the terminal to read. Press ENTER to	*/
		/* finish reading, and write to terminal. 		*/
		i = terminal_read( NULL, buf, NULL );

		/* Write the contents of buf to the screen.		*/
		/* Loop for the duration of this test. 			*/
		i = terminal_write( NULL, buf, 128 );
	}

	/* If somehow we ever reach here, we have technically	*/
	/* failed the test. 									*/
	return FAIL;

}

/* RTC FREQUENCY CHANGE TEST */
/* First initilizes the RTC and then clears the screen			*/
/* after it will then test 3 different cases that should not    */
/* change the frequency of the periodic interrupts. After		*/
/* three valid inputs will be passed and then will cause the    */
/* screen to change at the desired intervals. 					*/
/* Inputs: None. 												*/
/* Outputs: None. 												*/
/* Side Effects: Will change and test different clock speed. 	*/
/* inputs after the test the periodic interrupts will be set 	*/
/* back to 2 Hz													*/
int rtc_frequency_change_test( void ){
	TEST_HEADER;

	printf("\nDefault clock speed is 2 Hz\n");
	int i;
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	rtc_init();
	disable_irq(RTC_IRQ_NUM);

	/* Reset the screen */
	clear_and_reset_screen();
	screen_x = 0;
	screen_y = 0;
	printf("Testing invalid inputs to change interrupt rate:\n\n");

	/* 3 different invalid cases */
	rtc_change_freq_test(SMALL_RATE);
	rtc_change_freq_test(LARGE_RATE);
	rtc_change_freq_test(NON_2_RATE);

	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
	
	clear_and_reset_screen();
	screen_x = 0;
	screen_y = 0;	

	printf("Testing valid inputs to change interrupt rate:\n\n");
	/* 3 different valid cases */
	rtc_change_freq_test(ACCEPTED_RATE_1);	
	printf("\n\n");
	rtc_change_freq_test(ACCEPTED_RATE_2);		
	printf("\n\n");
	rtc_change_freq_test(ACCEPTED_RATE_3);
	printf("\n\n");	

	printf("Completed RTC input tests\n");
	return PASS;
}

/* RTC CHANGE FREQUENCY TEST */
/* Helper function for the rtc_frequency_change_test used too 	*/
/* Set the frequency of the periodic interrupts and give a		*/
/* different response depending on the validity of the input 	*/
/* Inputs: test_rate: Frequency to set the clock to  			*/
/* Outputs: None. 												*/
/* Side Effects: Will set the interrupt speed and send a 	 	*/
/* response depending on if the speed is a valid input		 	*/
void rtc_change_freq_test( unsigned int test_rate ) 
{
	int i;
	int check =  rtc_set_freq( test_rate );
	/* Check to see if number that we passed is valid */
	if ( check ){
		printf("Received input of %d is valid \n", test_rate);
		for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
		enable_irq(RTC_IRQ_NUM);
		for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
		disable_irq(RTC_IRQ_NUM);
	}
	else {
		printf("Invalid Input of %d:\n", test_rate);
		printf("Must be a power of 2 and within [2, 1024] Hertz\n\n");
		for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
	}
}

/* rtc_open_test												*/
/* Tests the open functionality of the rtc driver 				*/
/* Inputs:None										  			*/
/* Outputs: None. 												*/
/* Side Effects: Test open for rtc and sets interrupt to 2 hz	*/
int rtc_open_test( void ) {
	TEST_HEADER;

	/* Open should just intialize the RTC and set frequency to 2 */
	int result;
	int i;
	/* Testing rtc_open with NULL*/
	printf("\nTesting invalid rtc_open (NULL case 1): ");
	result = rtc_open( ( uint8_t* ) NULL);
	if (result != -1) {
		return FAIL;
	}
	printf("PASSED \n");
	printf("Testing invalid rtc_open (NULL case 2): ");

	/* Testing rtc_open with another invalid case */
	result = rtc_open( ( uint8_t* ) 0x00 );
	if (result != -1 ) {
		return FAIL;
	}
	printf("PASSED \n");
	printf("\nTesting valid rtc_open should be set to 2 Hz:\n");

	/* Testing rtc_open with a valid case */
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
	result = rtc_open( ( uint8_t* )  0xFF );
	if( result != 0 ) {
		return FAIL;
	}
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	disable_irq(RTC_IRQ_NUM);
	/* Return pass if we made it here */
	printf("\n");
	return PASS;

}

/* rtc_close_test												*/
/* Tests the close functionality of the rtc driver 				*/
/* Inputs:None										  			*/
/* Outputs: None. 												*/
/* Side Effects: tests close for rtc and sets interrupt to 2	*/
int rtc_close_test( void ) {
	TEST_HEADER; 

	int result;
	int i;
	/* Test if NULL will cause unexpected output.	*/
	result = rtc_close( ( int32_t ) NULL );
	if ( result != 0 ) {
		return FAIL;
	}
	// printf("TESTING rtc_close\n");
	printf("\nShowing that close set the interrupt rate to 2 Hz\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}

	/* Show that the interrupts are occuring at correct frequency */
	enable_irq(RTC_IRQ_NUM);
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	disable_irq(RTC_IRQ_NUM);
	printf("\n");

	/* All tests passed. Return PASS. */
	return PASS;
}

/* rtc_read_write_test											*/
/* Tests the read_write functionality of the rtc driver 		*/
/* Inputs:None										  			*/
/* Outputs: None. 												*/
/* Side Effects: Sets interrupt to 4 hz and makes sure read 	*/
/* works properly 												*/
int rtc_read_write_test( void ) {
	TEST_HEADER;

	int result;
	int i;
	/* Set a vlid buffer to write to rtc*/
	uint8_t buf[4];
	buf[0] = 0x04;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;

	/* Invalid NULL case */
	result = rtc_write(NULL, NULL, 4);
	if (result != -1){
		return FAIL;
	}
	/* Testing valid case */
	printf("\nTesting rtc_write with 4hz input: ");
	result = rtc_write(NULL, buf, 4);
	if (result != 0){
		return FAIL;
	}
	/* Testing the read function */
	printf("PASSED\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}

	printf("Testing rtc_read: ");
	result = rtc_read(NULL, buf, 4);
	if (result != 0){
		return FAIL;
	}
	/* Testing that interrupt rate is correct after read */
	printf("PASSED\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}

	printf("Testing interrupt rate after read (should be 4 Hz): \n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	enable_irq(RTC_IRQ_NUM);
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	disable_irq(RTC_IRQ_NUM);
	printf("\n\n");
	return PASS;
}



/* //////////////////////////////////////////////////////////// */
/* -------------------- CHECKPOINT 3 TESTS -------------------- */
/* //////////////////////////////////////////////////////////// */
#if 0
/* syscall_call_test											*/
/* This test is intended to show the function of the system		*/
/* calls read, write, open, and close, WITHOUT a functioning	*/
/* execute or halt as a backup. The test will use open to open	*/
/* several files, and read to read one file. Because the file	*/
/* system is write only, we will test write by opening the rtc	*/
/* file and writing, or setting, the rtc value. Finally, we		*/
/* test close by closing a file and attempting to read from it,	*/
/* which should fail. 											*/
/* Inputs: none. 												*/
/* Outputs: none.												*/
/* Side Effects: As described above. Contents will be printed	*/
/* to the screen and should be visually confirmed.				*/
void syscall_call_test( void )
{
	/* Print Test Header with macro*/
	TEST_HEADER;
	int i;

	/* Set up a PCB to simulate a program, also map to page.	*/
	curr_pid = 0;
    pcb_t* program_pcb = (pcb_t*)( EIGHT_MB - EIGHT_KB * ( curr_pid + 1 ) );

	/* Declare a vector for syscalls.	*/
	uint32_t vector;

	/* Also set the arg to get the appropriate vector	*/
	uint32_t vector_open = 5;
	/* This arg is set because inline assembly is finicky. For	*/
	/* some reason, the last register used gets clobbered, so	*/
	/* we add an extra line in our inline assembly to clobber a	*/
	/* unused register.											*/
	uint32_t random = 0x00;

	uint32_t user_page = 32;
	uint32_t eight_mb_offset = 0x00800000;

    uint32_t pcb_addr = FOUR_MB * curr_pid + eight_mb_offset;
	page_directory[ user_page ].present         = 1;
    page_directory[ user_page ].read_write      = 1;
    page_directory[ user_page ].user_supervisor = 0;
    page_directory[ user_page ].write_through   = 0;
    page_directory[ user_page ].cache_disable   = 0;
    page_directory[ user_page ].accessed        = 0;
    page_directory[ user_page ].available_1     = 0;
    page_directory[ user_page ].page_size       = 1;
    page_directory[ user_page ].available_3     = 0;
    page_directory[ user_page ].virtual_address = ( unsigned int ) ( pcb_addr >> 12 );
    flush_tlb( );

	/* Verify the flags in the file array are set to 0. 			*/
	printf("Verifying flags...\n");
	for( i = 0; i < MAX_NUM_FILES; i++ )
	{
		printf("Flag %d: %d\n", i, program_pcb->fd_array[ i ].flags );
	}

	printf("\nOpening files. Nothing should print onscreen...\n");
	/* Open the file first. Set the vector accordingly. Open three	*/
	/* times to simulate placement of stdin and stout.				*/
	for( i = 0; i < 3; i++ )
	{
		vector = 5;
		/* Set the filename to simulate opening small file 	*/
		uint8_t file_small[] = "frame0.txt";
		asm volatile("movl %[vector], %%eax \n\
					movl %[file_small], %%ebx \n\
					movl %[random], %%ecx "
					:: [ vector ] "r" ( vector ), [ file_small ] "r" ( file_small ), [ random ] "r" (random) );

		/* Now open the file with system call. We open the file 3 	*/
		/* times because the first two file opens are technically	*/
		/* not valid.												*/
		asm volatile("int $128");
	}

	printf("Verifying flags...\n");
	for( i = 0; i < MAX_NUM_FILES; i++ )
	{
		printf("Flag %d: %d\n", i, program_pcb->fd_array[ i ].flags );
	}


	printf("\nFiles opened! Now attempting to read one of the open files...\n");
	/* Arbitrarily set the arguments to read. We really only	*/
	/* need to make sure fd and nbytes are valid, and buf is	*/
	/* somewhere we can print back to. 							*/
	int32_t fd;
	uint8_t buf[ 128 ];
	int32_t nbytes;
	uint32_t vector_read;

	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	printf("\nProceeding with test...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;
	terminal_x[ display_terminal ] = 0;
	terminal_y[ display_terminal ] = 0;

	printf("Printing file frame0.txt...\n");
	
	int result = 1;
	while( result > 0 )
	{
		/* Set the values of arguments into read.					*/
		fd = 0x02;
		buf[ 0 ] = 0;
		nbytes = SIZE_DATA_BLOCK;
		vector_read = 3;

		/* Simulate a system call by setting the arguments in eax,	*/
		/* ebx, ecx, and edx.										*/
		asm volatile("movl 	%[vector_read], %%eax;"
					"movl 	%[fd], %%ebx;"
					"movl 	%[buf], %%ecx;"
					"movl 	%[nbytes], %%edx;"
					"movl  %[random], %%esi;"
					: : [ vector_read ] "r" ( vector_read ),
					[ fd ] "r" ( fd ),
					[ buf ] "r" ( buf ),
					[ nbytes ] "r" ( nbytes ),
					[ random ] "r" ( random )
					);
		
		/* Invoke the interrupt corresponding to the vector number 	*/
		/* desired. For Checkpoint 3.1, will not quash the user    	*/
		/* program but rather loop infinitely instead. 			   	*/
		asm volatile("int $128");

		print_string((uint8_t*) buf);

		/* Move the result to the eax register. 					*/
		asm volatile("movl %%eax, %[result]" :[result] "=r" ( result ) );

	}
	/* Force the thread to loop so we can have visually confirm the	*/
	/* contents of the file.										*/
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}

	/* Clear the screen to indicate moving on to next test			*/
	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;
	terminal_x[ display_terminal ] = 0;
	terminal_y[ display_terminal ] = 0;

	printf("Continuing...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}

	/* Clear the screen to indicate moving on to next test			*/
	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;
	terminal_x[ display_terminal ] = 0;
	terminal_y[ display_terminal ] = 0;

	/* Also open the "." file, so that we can try opening a dir	*/
	/* type file. We'll try to print out all the files listed	*/
	/* in that directory.										*/
	printf("Now attempting to read a directory file. Files should print onscreen.\n");
	printf("Opening file and reading...\n");
	vector = 5;
	uint8_t dir[] = ".";
	asm volatile("movl %[vector], %%eax \n\
				 movl %[dir], %%ebx \n\
				 movl %[random], %%ecx "
				 :: [ vector ] "r" ( vector ), [ dir ] "r" ( dir ), [ random ] "r" (random) );
	asm volatile("int $128");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP / 2; i++) {}
	result = 1;
	while( result > 0 )
	{
		/* Set the values of arguments into read.					*/
		fd = 0x03;
		buf[ 0 ] = 0;
		nbytes = MAX_FILE_LENGTH;
		vector_read = 3;

		/* Simulate a system call by setting the arguments in eax,	*/
		/* ebx, ecx, and edx.										*/
		asm volatile("movl 	%[vector_read], %%eax;"
					"movl 	%[fd], %%ebx;"
					"movl 	%[buf], %%ecx;"
					"movl 	%[nbytes], %%edx;"
					"movl  %[random], %%esi;"
					: : [ vector_read ] "r" ( vector_read ),
					[ fd ] "r" ( fd ),
					[ buf ] "r" ( buf ),
					[ nbytes ] "r" ( nbytes ),
					[ random ] "r" ( random )
					);
		
		/* Invoke the interrupt corresponding to the vector number 	*/
		/* desired. For Checkpoint 3.1, will not quash the user    	*/
		/* program but rather loop infinitely instead. 			   	*/
		asm volatile("int $128");

		/* Move the result from the eax register. 					*/
		asm volatile("movl %%eax, %[result]" :[result] "=r" ( result ) );

		if( result <= 0 )
		{
			break;
		}

		dentry_t curr_dentry;
		inode_t curr_inode;
		
		printf("file_name: ");
		print_filename( buf );
		read_dentry_by_name( buf, &curr_dentry );
		curr_inode = *((inode_t*)(p_inode_addr + (curr_dentry.index_node_num)));
		printf("  file_type: %d, file_size: %d", curr_dentry.file_type, curr_inode.file_size);
		printf("\n");

	}

	/* Force the thread to loop so we can have visually confirm the	*/
	/* contents of the file.										*/
	printf("All files printed! Confirm contents of print on screen.\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}

	clear_and_reset_screen( );
	screen_x = 0;
	screen_y = 0;
	terminal_x[ display_terminal ] = 0;
	terminal_y[ display_terminal ] = 0;

	printf("\nTesting write. Opening RTC and writing...\n");
	/* Open rtc as new file so that we can prove that write works	*/
	uint8_t rtc_name[] = "rtc";
	vector_open = 5;
	/* Open the RTC file before we can do anything.					*/
	asm volatile("movl %[vector_open], %%eax \n\
				 movl %[rtc_name], %%ebx \n\
				 movl %[random], %%ecx "
				 :: [ vector_open ] "r" ( vector_open ), [ rtc_name ] "r" ( rtc_name ), [ random ] "r" (random) );
	asm volatile("int $128");

	/* RTC opened. Now write to the RTC file, changing the freq. 	*/
	printf("\nSetting RTC value to 0x04!\n");
	uint32_t vector_write = 4;
	fd = 4;
	buf[0] = 0x04;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	nbytes = 4;
	asm volatile("movl 	%[vector_write], %%eax;"
				 "movl 	%[fd], %%ebx;"
				 "movl 	%[buf], %%ecx;"
				 "movl 	%[nbytes], %%edx;"
				 "movl  %[random], %%esi;"
				 : : [ vector_write ] "r" ( vector_write ),
				   [ fd ] "r" ( fd ),
				   [ buf ] "r" ( buf ),
				   [ nbytes ] "r" ( nbytes ),
				   [ random ] "r" ( random )
				);
	/* Use syscall interrupt to write to rtc */
	asm volatile("int $128");

	printf("\nWaiting for RTC to print...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}
	
	/* Now set the RTC again to confirm the write works.	*/
	printf("\nSetting RTC value to 0x0400\n");
	nbytes = 4;
	fd = 4;
	vector_write = 4;
	buf[0] = 0x00;
	buf[1] = 0x04;
	buf[2] = 0x00;
	buf[3] = 0x00;

	asm volatile("movl 	%[vector_write], %%eax;"
				 "movl 	%[fd], %%ebx;"
				 "movl 	%[buf], %%ecx;"
				 "movl 	%[nbytes], %%edx;"
				 "movl  %[random], %%esi;"
				 : : [ vector_write ] "r" ( vector_write ),
				   [ fd ] "r" ( fd ),
				   [ buf ] "r" ( buf ),
				   [ nbytes ] "r" ( nbytes ),
				   [ random ] "r" ( random )
				);
	asm volatile("int $128");

	printf("\nWaiting for RTC to print...\n");
	for (i = 0; i < VERY_LARGE_NUM_SLEEP; i++) {}

	/* Now let's close some files and see if close works!	*/
	/* Close file #2. Then try to read it - the function 	*/
	/* should not be able to read from the file.			*/
	
	/* Close File #2 */
	printf("\nClosing file 2 so that we can try to read it later...\n");
	uint32_t vector_close = 6;
	fd = 2;
	asm volatile("movl 	%[vector_close], %%eax;"
				 "movl 	%[fd], %%ebx;"
				 "movl 	%[buf], %%ecx;"
				 "movl 	%[nbytes], %%edx;"
				 "movl  %[random], %%esi;"
				 : : [ vector_close ] "r" ( vector_close ),
				   [ fd ] "r" ( fd ),
				   [ buf ] "r" ( buf ),
				   [ nbytes ] "r" ( nbytes ),
				   [ random ] "r" ( random )
				);
	asm volatile("int $128");

	/* Now try to read from file 2. Should fail and 	*/
	/* print nothing.									*/
	printf("\nReading file 2. Should fail (no contents of file should print onscreen)\n");
	asm volatile("movl 	%[vector_read], %%eax;"
				 "movl 	%[fd], %%ebx;"
				 "movl 	%[buf], %%ecx;"
				 "movl 	%[nbytes], %%edx;"
				 "movl  %[random], %%esi;"
				 : : [ vector_read ] "r" ( vector_read ),
				   [ fd ] "r" ( fd ),
				   [ buf ] "r" ( buf ),
				   [ nbytes ] "r" ( nbytes ),
				   [ random ] "r" ( random )
				);
	asm volatile("int $128");

	/* Check if value returned is failure. */
	asm volatile("movl %%eax, %[result]" :[result] "=r" ( result ) );

	printf("\nRead called. Confirm contents on screen.\n");
	printf("\nEnding test!\n");
}
#endif

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */
