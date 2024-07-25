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

#include "port_common.h"

#include "wizchip_conf.h"
#include "socket.h"
#include "w5x00_spi.h"
#include "w5x00_lwip.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"

#include "lwip/apps/lwiperf.h"
#include "lwip/etharp.h"
#include "lwip/dhcp.h"

#include "pico/wiznet_spi_pio.h"

#include "w5500_func.h"
#include "w5500_misc.h"
#include "w5500_polltask.h"
#include "w5500_registerview.h"

#undef printf
#define printf shprintf2

#define LINE_SIZE 16
#define PRINT_THRESHOLD 5

typedef enum {
    TYPE_NONE,
    TYPE_ZERO,
    TYPE_FF,
    TYPE_MIXED
} DataType;

void PrintLine(char* pBuff, uint32_t address, uint8_t* data, DataType type)
{
    char lineBuffer[80];
    int offset = 0;

    offset += sprintf(pBuff + offset, "[%08X] ", address);
    for (int i = 0; i < LINE_SIZE; ++i) {
        offset += sprintf(pBuff + offset, "%02X ", data[i]);
    }
    offset += sprintf(pBuff + offset, " | ");
    for (int i = 0; i < LINE_SIZE; ++i) {
        //if (data[i] >= 0x21 && data[i] <= 0x7E) {
        if (data[i] >= 0x21 && data[i] <= 0x7E && data[i] !=0x25 ) {
            offset += sprintf(pBuff + offset, "%c", data[i]);
        } else {
            offset += sprintf(pBuff + offset, ".");
        }
    }
    offset += sprintf(pBuff + offset, " | ");
    switch (type) {
        case TYPE_ZERO:
            offset += sprintf(pBuff + offset, "0x00");
            break;
        case TYPE_FF:
            offset += sprintf(pBuff + offset, "0xFF");
            break;
        case TYPE_MIXED:
            offset += sprintf(pBuff + offset, "Mixed");
            break;
    }
    sprintf(pBuff + offset, "\r\n");
}

DataType AnalyzeData(uint8_t* data)
{
    int all_zero = 1;
    int all_ff = 1;
    char lineBuffer[80];

    for (int i = 0; i < LINE_SIZE; ++i) {
        if (data[i] != 0x00) {
            all_zero = 0;
        }
        if (data[i] != 0xFF) {
            all_ff = 0;
        }
        if (!all_zero && !all_ff) {
            return TYPE_MIXED;
        }
    }
    if (all_zero) {
        return TYPE_ZERO;
    }
    if (all_ff) {
        return TYPE_FF;
    }
    return TYPE_MIXED; // Should never reach here
}


void Memory_Summary(uint32_t start, uint32_t end, uint8_t mode)
{
    if (start >= end) {
        printf("Invalid range.\r\n");
        return;
    }

    uint32_t size = end - start;
    uint8_t *memory = (uint8_t *)start; // 직접 메모리 접근을 가정
    DataType old1_type = TYPE_NONE;
    DataType old2_type = TYPE_NONE;
    int line_count = 0;
    char lineBuffer[80];
    int skip_count = 0;

    for (uint32_t address = start; address < end; address += LINE_SIZE)
    {
        uint8_t *data = &memory[address - start];
        DataType type = AnalyzeData(data);

        int should_skip = 0;

        if (mode == 1) {
            if (type == TYPE_ZERO || type == TYPE_FF) {
                if (type == old1_type && old1_type == old2_type) {
                    should_skip = 1;
                }
            }
        } else if (mode == 2) {
            if (type == TYPE_ZERO || type == TYPE_FF || type == TYPE_MIXED) {
                if (type == old1_type && old1_type == old2_type) {
                    should_skip = 1;
                }
            }
        }

        if (should_skip)
        {
            if (skip_count == 0)
            {
                printf("...\r\n");
            }
            skip_count++;
        } else
        {
            line_count++;
            PrintLine(lineBuffer, address, data, type);
            printf(lineBuffer);
            skip_count = 0;
        }

        old2_type = old1_type;
        old1_type = type;
        
        if (line_count > 20) {
            line_count = 0;
            sleep_ms(100);
        }
    }
}



//void pico_memory_dump(char* param1, char* param2, uint8_t mode)
void pico_memory_dump(char* param1, char* param2, char* param3)
{
    uint8_t mode = (uint8_t) strtoul(param1, NULL, 16);
    uint32_t addr = (uint32_t) strtoul(param2, NULL, 16);
    uint32_t end = (uint32_t) strtoul(param3, NULL, 16);

    if ( addr==0 && end==0 )
    {
        shprintf2("\r\n < ROM > \r\n");
        Memory_Summary(0x00000000, 0x00000050, 0);
        shprintf2("\r\n < XIP(Flash) > \r\n");
        Memory_Summary(0x10000000, 0x10000050, 0);
        shprintf2("\r\n < RAM > \r\n");
        Memory_Summary(0x20000000, 0x20000050, 0);
        shprintf2("\r\n < APB Peripherals > \r\n");
        Memory_Summary(0x40000000, 0x40000050, 0);
        shprintf2("\r\n < AHB-Lite Peripherals > \r\n");
        Memory_Summary(0x50000000, 0x50000050, 0);
        shprintf2("\r\n < IOPORT Registers > \r\n");
        Memory_Summary(0xd0000000, 0xd0000050, 0);
        shprintf2("\r\n < CM0+ Internal Registers > \r\n");
        Memory_Summary(0xe0000000, 0xe0000050, 0);
    }
    else
    {
        Memory_Summary(addr, end, mode);
    }
    
    return;
}

void pico_memory_map()
{
    uint8_t *memory = (uint8_t *)0x00000000;
    char lineBuffer[80];
    uint8_t *data = &memory[0x00000000];
    DataType type;

    shprintf2("\r\n" BOLD "RP2040 Memory Map" RESET "\r\n\r\n");

    #define TEMP_20240715_2(aa) { data = &memory[aa + 0x00];  type = AnalyzeData(data);   PrintLine(lineBuffer, aa + 0x00, data, type);   printf(lineBuffer); \
         shprintf2("%-74s", " "); data = &memory[aa + 0x10];  type = AnalyzeData(data);   PrintLine(lineBuffer, aa + 0x10, data, type);   printf(lineBuffer); \
         shprintf2("%-74s", " "); data = &memory[aa + 0x20];  type = AnalyzeData(data);   PrintLine(lineBuffer, aa + 0x20, data, type);   printf(lineBuffer); \
         shprintf2("%-74s", " "); data = &memory[aa + 0x30];  type = AnalyzeData(data);   PrintLine(lineBuffer, aa + 0x30, data, type);   printf(lineBuffer); \
         shprintf2("%-74s", " "); data = &memory[aa + 0x40];  type = AnalyzeData(data);   PrintLine(lineBuffer, aa + 0x40, data, type);   printf(lineBuffer); \
         shprintf2("\r\n");}

    shprintf2(BG_GREEN " %-14s           " RESET " " BG_GREEN " %-20s " RESET " " BG_GREEN " %-19s " RESET "\r\n",  "Address Range",        "Memory Area",          "Description"        );  
    shprintf2(BG_BLUE  " 0x%08X - 0x%08X " RESET " "          " %-20s "       " "          " %-19s "       "    ",  0x00000000, 0x00003FFF, "ROM",                  "Read-Only"          ); TEMP_20240715_2(0x00000000);  
    shprintf2(BG_BLUE  " 0x%08X - 0x%08X " RESET " "          " %-20s "       " "          " %-19s "       "    ",  0x10000000, 0x1FFFFFFF, "XIP",                  "External Flash"     ); TEMP_20240715_2(0x10000000); 
    shprintf2(BG_BLUE  " 0x%08X - 0x%08X " RESET " "          " %-20s "       " "          " %-19s "       "    ",  0x20000000, 0x2003FFFF, "SRAM",                 "Internal SRAM"      ); TEMP_20240715_2(0x20000000); 
    shprintf2(BG_BLUE  " 0x%08X - 0x%08X " RESET " "          " %-20s "       " "          " %-19s "       "    ",  0x40000000, 0x400FFFFF, "APB Peripherals",      "APB Registers"      ); TEMP_20240715_2(0x40000000); 
    shprintf2(BG_BLUE  " 0x%08X - 0x%08X " RESET " "          " %-20s "       " "          " %-19s "       "    ",  0x50000000, 0x503FFFFF, "AHB-Lite Peripherals", "AHB Registers"      ); TEMP_20240715_2(0x50000000); 
    shprintf2(BG_BLUE  " 0x%08X - 0x%08X " RESET " "          " %-20s "       " "          " %-19s "       "    ",  0xD0000000, 0xDFFFFFFF, "IO Registers",         "Single-Cycle IO"    ); TEMP_20240715_2(0xD0000000); 
    shprintf2(BG_BLUE  " 0x%08X - 0x%08X " RESET " "          " %-20s "       " "          " %-19s "       "    ",  0xE0000000, 0xE00FFFFF, "Cortex-M0+ Internal",  "Processor Registers"); TEMP_20240715_2(0xE0000000); 
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CLOCK_PIN 10
#define DATA_PIN 13


void swd_xxrequest(char* param)
{
    // 클럭 주기와 모니터 주기 초기화
    uint clock_period_ms = 0;
    uint monitor_cycles = 0;
    bool input_mode = false;
    char buffparam1[100];
    char buffparam2[100];
    strcpy(buffparam1, param);
    strcpy(buffparam2, param);

    // 파라미터 분석
    char* token = strtok(buffparam1, ",");
    while (token != NULL) {
        if (token[0] == 'c') {
            clock_period_ms = atoi(token + 1);
        } else if (token[0] == 'm') {
            monitor_cycles = atoi(token + 1);
        }
        token = strtok(NULL, ",");
    }

    // 동적 메모리 할당
    char* pCLK = (char*)malloc(monitor_cycles + 1);
    char* pTx = (char*)malloc(monitor_cycles + 1);
    char* pRx = (char*)malloc(monitor_cycles + 1);
    memset(pCLK, 0, monitor_cycles + 1);
    memset(pTx, 0, monitor_cycles + 1);
    memset(pRx, 0, monitor_cycles + 1);

    // 클럭 시퀀스 생성
    for (int i = 0; i < monitor_cycles; i++) {
        pCLK[i] = (i % 2) ? '1' : '0';
    }

    // 파라미터 분석 및 pTx 설정
    token = strtok(buffparam2, ",");
    int tx_index = 0;
    while (token != NULL)
    {
        if (token[0] == 't' && token[1] == 'l') {
            int cycles = atoi(token + 2);
            for (int i = 0; i < cycles; i++) {
                pTx[tx_index++] = '0';
            }
        } else if (token[0] == 't' && token[1] == 'h') {
            int cycles = atoi(token + 2);
            for (int i = 0; i < cycles; i++) {
                pTx[tx_index++] = '1';
            }
        } else if (token[0] == 't' && token[1] == 'b') {
            for (int i = 2; token[i] != '\0'; i++) {
                pTx[tx_index++] = token[i];
            }
        } else if (token[0] == 'i') {
            input_mode = true;
            while (tx_index < monitor_cycles) {
                pTx[tx_index++] = 'x';
            }
        }
        token = strtok(NULL, ",");
    }

    // GPIO 초기화
    uint8_t inputmode = 0;
    gpio_init(CLOCK_PIN);
    gpio_init(DATA_PIN);
    gpio_set_dir(CLOCK_PIN, GPIO_OUT);
    gpio_set_dir(DATA_PIN, GPIO_OUT); inputmode = 0;
    gpio_set_pulls(CLOCK_PIN, 1, 0);
    gpio_set_pulls(CLOCK_PIN, 1, 0);

    // SWD 송수신
    for (int i = 0; i < monitor_cycles; i++) {
        
        gpio_put(CLOCK_PIN, pCLK[i] - '0');
        //gpio_put(CLOCK_PIN, !(pCLK[i] - '0'));
        //for (int jj=0; jj<5; jj++);

        // 데이터 출력 및 입력 처리
        if (pTx[i] == 'x')
        {
            //if (inputmode==0)
            {
                gpio_set_dir(DATA_PIN, GPIO_IN); inputmode = 1;
            }
            pRx[i] = gpio_get(DATA_PIN) ? '1' : '0';
        }
        else
        {
            gpio_put(DATA_PIN, pTx[i] - '0');
            pRx[i] = 'x';  // 입력 모드가 아니면 'x'로 채움
        }

        sleep_us(clock_period_ms);
    }

    // 결과 출력 (디버그 용)    
    void print_string_in_chunks(const char *input_string);
    printf("pCLK: ");   print_string_in_chunks(pCLK);    printf("\r\n");
    printf("pTx : ");   print_string_in_chunks(pTx );    printf("\r\n");
    printf("pRx : ");   print_string_in_chunks(pRx );    printf("\r\n");

    free(pCLK);
    free(pTx);
    free(pRx);
}


void print_string_in_chunks(const char *input_string)
{
    int length = strlen(input_string);
    int chunk_size = 100;
    char buffer[chunk_size + 1]; // +1 for null terminator

    for (int i = 0; i < length; i += chunk_size) {
        // Copy chunk_size characters from input_string to buffer
        strncpy(buffer, &input_string[i], chunk_size);
        buffer[chunk_size] = '\0'; // Null-terminate the buffer

        printf("%s", buffer);
    }
}

