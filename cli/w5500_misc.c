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

#include "pico/wiznet_spi_pio.h"

#include "w5500_func.h"
#include "w5500_misc.h"
#include "w5500_polltask.h"
#include "w5500_registerview.h"

char w5x00_buf[200] = {0,};

void add_crlf()
{
    uint32_t len = strlen(w5x00_buf);

    if ( w5x00_buf[len-1]=='\r' || w5x00_buf[len-1]=='\n'  )  w5x00_buf[len-1] = 0;
    if ( w5x00_buf[len-2]=='\r' || w5x00_buf[len-2]=='\n'  )  w5x00_buf[len-2] = 0;

    strcat(w5x00_buf, "\r\n");

    uint32_t len2 = strlen(w5x00_buf);
}

int is_printable(char c)
{
    if ( c<' ' || c>'~' )     return 0;    
    return 1;
}


void w5500_dump(uint32_t address, unsigned char *data, uint32_t length)
{
    uint32_t i;
    unsigned char ascii[17];
    memset(ascii, 0, sizeof(ascii));

    shprintf2("Address      Hex Data                                          ASCII\r\n");
    shprintf2("--------------------------------------------------------------------------\r\n");

    for (i = 0; i < length; i++) {
        if (i % 16 == 0) {
            shprintf2("[0x%08x]  ", (unsigned char*)address + i);
        }
        shprintf2("%02x ", data[i]);

        if (is_printable(data[i])) {
            ascii[i % 16] = data[i];
        } else {
            ascii[i % 16] = '.';
        }

        if ((i + 1) % 16 == 0) {
            shprintf2("  %s\n", ascii);
        }
    }

    if (i % 16 != 0) {
        for (uint32_t j = i % 16; j < 16; j++) {
            shprintf2("   ");
        }
        shprintf2("  %s\r\n", ascii);
    }
}

int convertStringToByteBuffer(const char* str, unsigned char* buffer, uint32_t bufferLen)
{
    const char* p = str;
    unsigned int byte;
    int len = 0;

    while (*p && len < bufferLen)
    {
        char hex[3] = {*p, *(p+1), '\0'};
        if (hex[1]==0)  break;
        byte = strtoul(hex, NULL, 16);
        buffer[len] = (unsigned char)byte;
        p += 2;
        len++;
    }

    return len;
}

uint16_t getBit(uint16_t value, int position) {
    return (value >> position) & 1;
}

uint16_t setBit(uint16_t value, int position) {
    if (position < 0 || position > 15) {
        printf("Error: Position out of range (0-15).\n");
        return value;
    }
    return value | (1 << position);
}




