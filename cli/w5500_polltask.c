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

void poll_w5x00macraw(int check_linkup)
{
    uint8_t pack[ETHERNET_MTU];
    uint16_t pack_len = 0;
    struct pbuf *p = NULL;

    if ( check_linkup==1 )
    {
        if ( !netif_is_link_up(&g_netif) )
        {
            return;
        }
    }

    getsockopt(SOCKET_MACRAW, SO_RECVBUF, &pack_len);
    if (pack_len > 0)
    {
        pack_len = recv_lwip(SOCKET_MACRAW, (uint8_t *)pack, pack_len);
        if (pack_len)
        {
            p = pbuf_alloc(PBUF_RAW, pack_len, PBUF_POOL);
            if (p==0)
            {
                shprintf("poll_w5x00macraw  pbuf_alloc error");
            }
            pbuf_take(p, pack, pack_len);
        }
        else
        {
            shprintf(" No packet received\n");
        }

        if (pack_len && p != NULL)
        {
            LINK_STATS_INC(link.recv);
            if (g_netif.input(p, &g_netif) != ERR_OK)
                pbuf_free(p);
        }
    }
    sys_check_timeouts();
}

uint8_t g_link_status_link = 0x00;
void check_link_status()
{
    uint8_t regaddr;

    uint8_t temp_link_status_link = 0x00;

    extern uint8_t g_w5k_status;
    if ( g_w5k_status==0 ) return; 

    if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp_link_status_link) == -1)
    {
        shprintf("Unknown PHY link status\r\n");
        return;
    }

    if ( temp_link_status_link==g_link_status_link )
    {
        return;
    }

    g_link_status_link = temp_link_status_link;

    if (g_w5k_status!=11)   shprintf("\r\n PHY link status changed [0X%02x]", g_link_status_link);
}

// 20240620 add W5x00Poll_service
static void prvW5x00PollTask(void *pvParameters); // entry function for this service, implementation below
TaskHandle_t xW5x00PollTask; // FreeRTOS task handle for this service
SemaphoreHandle_t g_w5x00_mutex = NULL;

BaseType_t W5x00Poll_service(void)
{
    BaseType_t xReturn;

    xReturn = xTaskCreate(
        prvW5x00PollTask,             // main function of this service, defined below
        xstr(wiznetpoll),             // name defined in services.h
        1024,                          // STACK_HEARTBEAT, stack size defined in services.h
        NULL,                         // parameters passed to created task (not used)
        1,                            // priority of this service, defined in services.h
        &xW5x00PollTask               // FreeRTOS task handle
    );

    if (xReturn == pdPASS) {
        cli_print_raw("W5x00Poll service started");
    }
    else {
        cli_print_raw("Error starting the W5x00Poll service");
    }

    return xReturn;
}

static void prvW5x00PollTask(void *pvParameters)
{
    g_w5x00_mutex = xSemaphoreCreateMutex();

    while(true) {
        // XXXXX prvW5x00PollTask
        task_delay_ms(1);
        check_link_status();
        poll_w5x00macraw(1);
    }
}

