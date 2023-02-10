#include "fops.h"
#include "file_system.h"
#include "rtc.h"
#include "terminal.h"

/* Global table with address to return in the get_[specific]_table functions. */
fops_table_t table;

/* fops_table_t get_RTC_table;
 *   Inputs: None
 *   Return Value: fops_table_t
 *   Function: Assemble the open, read, write, and close functions for RTC in a table and return the address */
fops_table_t* get_RTC_table (void) {
    table.open = rtc_open;
    table.read = rtc_read;
    table.write = rtc_write;
    table.close = rtc_close;
    return &table;
}

/* fops_table_t get_dir_table;
 *   Inputs: None
 *   Return Value: fops_table_t
 *   Function: Assemble the open, read, write, and close functions for directory in a table and return the address */
fops_table_t* get_dir_table (void) {
    table.open = dir_open;
    table.read = dir_read;
    table.write = dir_write;
    table.close = dir_close;
    return &table;
}

/* fops_table_t get_file_table;
 *   Inputs: None
 *   Return Value: fops_table_t
 *   Function: Assemble the open, read, write, and close functions for file in a table and return the address */
fops_table_t* get_file_table (void) {
    table.open = file_open;
    table.read = file_read;
    table.write = file_write;
    table.close = file_close;
    return &table;
}

/* fops_table_t get_terminal_table;
 *   Inputs: None
 *   Return Value: fops_table_t
 *   Function: Assemble the open, read, write, and close functions for terminal in a table and return the address */
fops_table_t* get_terminal_table (void) {
    table.open = terminal_open;
    table.read = terminal_read;
    table.write = terminal_write;
    table.close = terminal_close;
    return &table;
}

/* fops_table_t get_stdin_table;
 *   Inputs: None
 *   Return Value: fops_table_t
 *   Function: Assemble the open, read, write, and close functions for stdin in a table and return the address */
fops_table_t* get_stdin_table (void) {
    table.open = terminal_open;
    table.read = terminal_read;
    table.write = NULL;
    table.close = terminal_close;
    return &table;
}

/* fops_table_t get_stdout_table;
 *   Inputs: None
 *   Return Value: fops_table_t
 *   Function: Assemble the open, read, write, and close functions for stdout in a table and return the address */
fops_table_t* get_stdout_table (void) {
    table.open = terminal_open;
    table.read = NULL;
    table.write = terminal_write;
    table.close = terminal_close;
    return &table;
}
