/******************************************************************************
 * @file node_bin.c
 *
 * @brief /bin folder for the CLI, contains system executables
 *
 * @author Cavin McKinley (MCKNLY LLC)
 *
 * @date 02-14-2024
 * 
 * @copyright Copyright (c) 2024 Cavin McKinley (MCKNLY LLC)
 *            Released under the MIT License
 * 
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <microshell.h>
#include "hardware_config.h"
#include "shell.h"
#include "services.h"
#include "service_queues.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"


/**
* @brief '/bin/ps' executable callback function.
*
* Prints out FreeRTOS task list information in a similar style to *nix 'ps'.
*
* @param ush_file_execute_callback Params given by typedef ush_file_execute_callback. see ush_types.h
*
* @return nothing
*/
static void ps_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    // initialize array to hold task list ASCII output. Assume 40 bytes per task + header
    const char *tasks_header =  USH_SHELL_FONT_STYLE_BOLD
                                USH_SHELL_FONT_COLOR_BLUE
                                "                                Min\r\n"
                                "Task            State   Pri     Stack   No\r\n"
                                "------------------------------------------\r\n"
                                USH_SHELL_FONT_STYLE_RESET;
    int header_len = strlen(tasks_header);
    int tasks_maxlen = 40 * uxTaskGetNumberOfTasks();
    char *ps_msg = pvPortMalloc(header_len + tasks_maxlen);
    strcpy(ps_msg, tasks_header);

    // call FreeRTOS vTaskList API and print to CLI
    vTaskListTasks(ps_msg + header_len, tasks_maxlen); // note this is a blocking, processor intensive function
    shell_print(ps_msg);
    vPortFree(ps_msg);
}

/**
* @brief '/bin/top' executable callback function.
*
* Prints out FreeRTOS task runtime stats information in a similar style to *nix 'top'.
*
* @param ush_file_execute_callback Params given by typedef ush_file_execute_callback. see ush_types.h
*
* @return nothing
*/
static void top_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    // initialize char array to hold task stats ASCII output. Assume 40 bytes per task + header
    const char *tasks_header =  USH_SHELL_FONT_STYLE_BOLD
                                USH_SHELL_FONT_COLOR_BLUE
                                "Task            Runtime(us)     Percentage\r\n"
                                "------------------------------------------\r\n"
                                USH_SHELL_FONT_STYLE_RESET;
    int header_len = strlen(tasks_header);
    int tasks_maxlen = 40 * uxTaskGetNumberOfTasks();
    char *top_msg = pvPortMalloc(header_len + tasks_maxlen);
    strcpy(top_msg, tasks_header);

    // call FreeRTOS vTaskGetRunTimeStats API and print to CLI
    vTaskGetRunTimeStatistics(top_msg + header_len, tasks_maxlen);
    shell_print(top_msg);
    vPortFree(top_msg);
}

/**
* @brief '/bin/free' executable callback function.
*
* Prints out the FreeRTOS heap memory manager usage statistics in a similar style to *nix 'free'.
*
* @param ush_file_execute_callback Params given by typedef ush_file_execute_callback. see ush_types.h
*
* @return nothing
*/
static void free_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    HeapStats_t *heap_stats = pvPortMalloc(sizeof(HeapStats_t)); // structure to hold heap stats results
    vPortGetHeapStats(heap_stats);  // get the heap stats
    const int heap_stats_maxlen = 300; // just a rough estimate for dynamic mem needed
    char *heap_stats_msg = pvPortMalloc(heap_stats_maxlen);
    unsigned int total_heap_size = configTOTAL_HEAP_SIZE;
    
    // format the heap stats
    snprintf(heap_stats_msg, heap_stats_maxlen,
            USH_SHELL_FONT_STYLE_BOLD
            USH_SHELL_FONT_COLOR_BLUE
            "Memory Statistics       Bytes\r\n"
            "------------------------------\r\n"
            USH_SHELL_FONT_STYLE_RESET
            "Total heap:\t\t%u\r\n"
            "Used heap:\t\t%u\r\n"
            "Available heap:\t\t%u\r\n"
            "Largest free block:\t%u\r\n"
            "Smallest free block:\t%u\r\n"
            "Num free blocks:\t%u\r\n"
            "Min ever heap:\t\t%u\r\n"
            "Num mallocs:\t\t%u\r\n"
            "Num frees:\t\t%u\r\n",
            total_heap_size,
            (total_heap_size - heap_stats->xAvailableHeapSpaceInBytes),
            heap_stats->xAvailableHeapSpaceInBytes,
            heap_stats->xSizeOfLargestFreeBlockInBytes,
            heap_stats->xSizeOfSmallestFreeBlockInBytes,
            heap_stats->xNumberOfFreeBlocks,
            heap_stats->xMinimumEverFreeBytesRemaining,
            heap_stats->xNumberOfSuccessfulAllocations,
            heap_stats->xNumberOfSuccessfulFrees
    );
    
    shell_print(heap_stats_msg);
    vPortFree(heap_stats);
    vPortFree(heap_stats_msg);
}

/**
* @brief '/bin/df' executable callback function.
*
* Prints out the internal/onboard (program) flash memory usage in a similar style to *nix 'df'.
*
* @param ush_file_execute_callback Params given by typedef ush_file_execute_callback. see ush_types.h
*
* @return nothing
*/
static void df_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    flash_usage_t flash_usage;
    const int flash_usage_msg_maxlen = 300;
    char *flash_usage_msg = pvPortMalloc(flash_usage_msg_maxlen);

    // get the flash usage data struct
    flash_usage = onboard_flash_usage();

    // format the flash usage printout
    snprintf(flash_usage_msg, flash_usage_msg_maxlen,
            USH_SHELL_FONT_STYLE_BOLD
            USH_SHELL_FONT_COLOR_BLUE
            "Flash Statistics                KBytes\r\n"
            "--------------------------------------\r\n"
            USH_SHELL_FONT_STYLE_RESET
            "Total flash size:\t\t%u\r\n"
            "Program binary size:\t\t%u\r\n"
            "Filesystem reserved size:\t%u\r\n"
            "Free flash space:\t\t%u\r\n",
            flash_usage.flash_total_size  / 1024,
            flash_usage.program_used_size / 1024,
            flash_usage.fs_reserved_size  / 1024,
            flash_usage.flash_free_size   / 1024
    );

    shell_print(flash_usage_msg);
    vPortFree(flash_usage_msg);
}

/**
* @brief '/bin/kill' executable callback function.
*
* Kill an RTOS task using the name given by 'ps'.
*
* @param ush_file_execute_callback Params given by typedef ush_file_execute_callback. see ush_types.h
*
* @return nothing
*/
static void kill_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    static char err_msg[100]; // buffer for holding the error output message

    if (argc == 2) {
        struct taskman_item_t tmi;
        tmi.task = xTaskGetHandle(argv[1]);
        tmi.action = DELETE;

        // put the kill request into the taskmanager queue
        if (tmi.task != NULL) {
            taskman_request(&tmi);
        }
        else {
            sprintf(err_msg, "%s is not a currently running task", argv[1]);
            shell_print(err_msg);
        }
    }
    else {
        shell_print("command requires exactly one argument, see 'help <kill>'");
    }
}

/**
* @brief '/bin/service' executable callback function.
*
* Interact with system services (list/start/suspend/resume). Services are defined in services.h
* Note that stopping services is performed with '/bin/kill'.
*
* @param ush_file_execute_callback Params given by typedef ush_file_execute_callback. see ush_types.h
*
* @return nothing
*/
static void service_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    char *err_msg = pvPortMalloc(100);    // buffer for holding the error output message
    char *service_msg = pvPortMalloc(32); // buffer for holding the service success message

    if (argc == 3) {
        struct taskman_item_t tmi;
        tmi.task = xTaskGetHandle(argv[2]);
        if (strcmp(argv[1], "start") == 0) { // START a service (task)
            if (tmi.task != NULL) {
                sprintf(err_msg, "%s is already running", argv[2]);
                shell_print(err_msg);
            }
            else {
                // Iterate through service_strings array to find the matching service index
                int i;
                for (i = 0; i < (sizeof(service_strings)/sizeof(service_strings[0])); i++) {
                    if (strcmp(argv[2], service_strings[i]) == 0) {
                        service_functions[i](); // call the function pointer at the same index of the matched service string
                        break;
                    }
                    if (i == ((sizeof(service_strings)/sizeof(service_strings[0]))-1)) {
                        sprintf(err_msg, "%s is not an available service, try 'service list'", argv[2]);
                        shell_print(err_msg);
                        break;
                    }
                }
            }
        }
        else if (strcmp(argv[1], "suspend") == 0) { // SUSPEND a running service (task)
            if (tmi.task == NULL) {
                sprintf(err_msg, "%s is not a running service, try '/bin/ps'", argv[2]);
                shell_print(err_msg);
            }
            else {
                tmi.action = SUSPEND;
                taskman_request(&tmi);
                sprintf(service_msg, "%s service suspended", argv[2]);
                shell_print(service_msg);
            }
        }
        else if (strcmp(argv[1], "resume") == 0 && tmi.task != NULL) { // RESUME a suspended service (task)
            if (tmi.task == NULL) {
                sprintf(err_msg, "%s is not a running service, try '/bin/ps'", argv[2]);
                shell_print(err_msg);
            }
            else {
                tmi.action = RESUME;
                taskman_request(&tmi);
                sprintf(service_msg, "%s service resumed", argv[2]);
                shell_print(service_msg);
            }
        }
        else {
            shell_print("command syntax error, see 'help <service>'");
        }
    }
    else if (argc == 2 && strcmp(argv[1], "list") == 0) { // LIST available services and their current state
        int i;
        const char *service_list_header =   USH_SHELL_FONT_STYLE_BOLD
                                            USH_SHELL_FONT_COLOR_BLUE
                                            "Available Services\tStatus\r\n"
                                            "------------------------------------\r\n"
                                            USH_SHELL_FONT_STYLE_RESET;
        char *service_list_msg = pvPortMalloc(strlen(service_list_header) + (sizeof(service_strings) * (configMAX_TASK_NAME_LEN + 16)));
        TaskHandle_t service_taskhandle;
        char service_state[12];

        strcpy(service_list_msg, service_list_header);

        // interate through available services, get their states from RTOS
        for (i = 0; i < (sizeof(service_strings)/sizeof(service_strings[0])); i++) {
            service_taskhandle = xTaskGetHandle(service_strings[i]);
            if (service_taskhandle == NULL) {
                strcpy(service_state, "not started");
            }
            else {
                switch (eTaskGetState(service_taskhandle)) {
                    case (eRunning):
                    case (eBlocked):
                        strcpy(service_state, "running");
                        break;
                    case (eSuspended):
                        strcpy(service_state, "suspended");
                        break;
                    case (eReady):
                        strcpy(service_state, "not started");
                        break;
                    default:
                        break;
                }
            }
            
            // copy current service name into table
            strcpy(service_list_msg + strlen(service_list_msg), service_strings[i]);
            // add an extra tab to short service names to make the table look better
            if (strlen(service_strings[i]) < (configMAX_TASK_NAME_LEN - 8)) {
                strcpy(service_list_msg + strlen(service_list_msg), "\t");
            }
            // copy current service state into table
            sprintf(service_list_msg + strlen(service_list_msg), "\t\t%s\r\n", service_state);
        }

        shell_print(service_list_msg);
        vPortFree(service_list_msg);
    }
    else {
        shell_print("command syntax error, see 'help <service>'");
    }

    // free heap used for the string buffers
    vPortFree(err_msg);
    vPortFree(service_msg);
}

/**
* @brief '/bin/reboot' executable callback function.
*
* Reboot the MCU by forcing a watchdog timeout.
*
* @param ush_file_execute_callback Params given by typedef ush_file_execute_callback. see ush_types.h
*
* @return nothing
*/
static void reboot_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    if (argc == 1) { // if there are no arguments, perform a normal watchdog reboot
        shell_print("rebooting system...");
        // stop the watchdog service so we can force a watchdog reboot
        struct taskman_item_t tmi; // create a taskmanager action item
        tmi.task = xTaskGetHandle("watchdog");
        tmi.action = SUSPEND;
        if (tmi.task != NULL) { // only try to suspend watchdog if it is actually running
            taskman_request(&tmi); // put the action in taskmanager queue
            // wait here until watchdog task state is "suspended"
            while(eTaskGetState(tmi.task) != eSuspended) {}
        }

        // now we can force a watchdog reboot
        force_watchdog_reboot();
    }
    else if (argc == 2 && strcmp(argv[1], "bootloader") == 0) { // reboot to bootloader
        shell_print("rebooting to bootloader...");
        wait_here_us(1E6); // wait for a second so user can see output
        reset_to_bootloader();
    }
    else {
        shell_print("command syntax error, see 'help <reboot>'");
    }
}


// 20240610 add w5500 test
#include "w5500_func.h"
static void w5500_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    if (argc == 1) {        
    }
    else if (argc == 4 && strcmp(argv[1], "init") == 0) {
        void w5x00_init(char* szSPImode, char* szClockDiv);
        w5x00_init(argv[2], argv[3]);
    }
    else if (argc == 2 && strcmp(argv[1], "info") == 0) {
        void w5x00_info();
        w5x00_info();
    }
    else if (argc == 3 && strcmp(argv[1], "read") == 0) {
        void w5x00_read(char* szaddr);
        w5x00_read(argv[2]);
    }
    else if (argc == 4 && strcmp(argv[1], "readbuff") == 0) {
        void w5x00_readbuff(char* szaddr, char* szlen);
        w5x00_readbuff(argv[2], argv[3]);
    }
    else if (argc == 4 && strcmp(argv[1], "writebuff") == 0) {
        void w5x00_writebuff(char* szaddr, char* szlen);
        w5x00_writebuff(argv[2], argv[3]);
    }
    else if (argc == 2 && strcmp(argv[1], "register") == 0) {
        int w5500_viewregister();
        w5500_viewregister();
    }
    else if (argc == 2 && strcmp(argv[1], "lwip") == 0) {
        void w5x00_lwipinit(char* szIp, char* szMask, char* szGateway);
        w5x00_lwipinit(0, 0, 0);
    }
    else {
        shell_print("command syntax error, see 'help /bin/pico'");
    }

}

static void pico_exec_callback(struct ush_object *self, struct ush_file_descriptor const *file, int argc, char *argv[])
{
    if (argc == 1) {        
    }
    else if (argc == 2 && strcmp(argv[1], "memory") == 0) {
        void pico_memory_map();
        pico_memory_map();
    }
    else if (argc == 5 && strcmp(argv[1], "memory") == 0) {
        void pico_memory_dump(char* param1, char* param2, char* param3);
        pico_memory_dump(argv[2], argv[3], argv[4]);
    }
    else if (argc == 2 && strcmp(argv[1], "pins") == 0) {
        void pico_pins_monitor();
        pico_pins_monitor();
    }
    else if (argc == 2 && strcmp(argv[1], "gpiotest") == 0) {
        void pico_pin_test();
        pico_pin_test();
    }
    else if (argc == 3 && strcmp(argv[1], "swd") == 0) {
        void swd_xxrequest(char* param);
        swd_xxrequest(argv[2]);
    }
    else {
        shell_print("command syntax error, see 'help /bin/pico'");
    }

}


// bin directory files descriptor
static const struct ush_file_descriptor bin_files[] = {
    {
        .name = "ps",                                   // file name (required)
        .description = "print running service info",    // optional file description
        .help = NULL,                                   // optional help manual
        .exec = ps_exec_callback,                       // optional execute callback
        .get_data = NULL,                               // optional get data (cat) callback
        .set_data = NULL                                // optional set data (echo) callback
    },
    {
        .name = "top",
        .description = "print runtime stats for services",
        .help = NULL,
        .exec = top_exec_callback,
        .get_data = NULL,
        .set_data = NULL 
    },
    {
        .name = "free",
        .description = "print heap memory (RAM) usage stats",
        .help = NULL,
        .exec = free_exec_callback,
        .get_data = NULL,
        .set_data = NULL 
    },
    {
        .name = "df",
        .description = "print flash memory usage stats",
        .help = NULL,
        .exec = df_exec_callback,
        .get_data = NULL,
        .set_data = NULL 
    },
    {
        .name = "kill",
        .description = "kill the service name given by 'bin/ps'",
        .help = "usage: kill <\e[3mservicename\e[0m>\r\n",
        .exec = kill_exec_callback,
        .get_data = NULL,
        .set_data = NULL 
    },
    {
        .name = "service",
        .description = "interact with available services",
        .help = "usage: service <list|start|suspend|resume> <\e[3mservicename\e[0m>\r\n",
        .exec = service_exec_callback,
        .get_data = NULL,
        .set_data = NULL 
    },
    {
        .name = "reboot",
        .description = "reboot device",
        .help = "usage: reboot            - normal mode\r\n"     
                "       reboot bootloader - UF2 mode\r\n",
        .exec = reboot_exec_callback,
        .get_data = NULL,
        .set_data = NULL 
    }

    ,
    {
        .name = "w5500",
        .description = "w5500 test",
        .help = "usage: w5500 \r\n"     
                "       w5500 init spinormal 8           \r\n"
                "       w5500 init spipio 4              \r\n"
                "       w5500 info                       \r\n"
                "       w5500 readbuff 0009 16           \r\n"
                "       w5500 writebuff 0009 0008dcaabbee\r\n"
                "       w5500 register                   \r\n"
                "       w5500 lwip                       \r\n", 
        .exec = w5500_exec_callback,
        .get_data = NULL,
        .set_data = NULL 
    }
    ,
    {
        .name = "pico",
        .description = "pico command",
        .help = "usage: pico \r\n"     
                "       pico memory 0 00003400 00003800 \r\n"
                "       pico memory 1 D0000000 D0000100 \r\n"
                "       pico memory 2 D0000000 D0000100 \r\n"
                "       pico memory \r\n"
                "       pico gpiotest \r\n"
                "       pico pins \r\n",

        .exec = pico_exec_callback,
        .get_data = NULL,
        .set_data = NULL 
    }

};

// bin directory handler
static struct ush_node_object bin;

void shell_bin_mount(void)
{
    // mount bin directory
    ush_node_mount(&ush, "/bin", &bin, bin_files, sizeof(bin_files) / sizeof(bin_files[0]));
}
