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


#define PLL_SYS_KHZ (133 * 1000)

uint8_t g_w5k_status = 0;

wiznet_spi_config_t g_spi_config = {
    .data_in_pin = PIN_MISO,
    .data_out_pin = PIN_MOSI,
    .cs_pin = PIN_CS,
    .clock_pin = PIN_SCK,
    // /.irq_pin = 21u, // XXX IRQ?
    .reset_pin = PIN_RST,
    .clock_div_major = 4,
    .clock_div_minor = 0,
};

int wiznet_pio_init(int clockdiv)
{
    wiznet_spi_handle_t spi_handle;
    g_spi_config.clock_div_major = clockdiv;

    spi_handle = wiznet_spi_pio_open(&g_spi_config);

    (*spi_handle)->reset(spi_handle);
    (*spi_handle)->set_active(spi_handle);

    reg_wizchip_spi_cbfunc((*spi_handle)->read_byte, (*spi_handle)->write_byte);
    reg_wizchip_spiburst_cbfunc((*spi_handle)->read_buffer, (*spi_handle)->write_buffer);
    reg_wizchip_cs_cbfunc((*spi_handle)->frame_start, (*spi_handle)->frame_end);

    return 0;
}

void w5x00_init(char* szSPImode, char* szClockDiv)
{
    int clockdiv = (uint32_t) strtoul(szClockDiv, NULL, 10);

    if ( strcmp(szSPImode, "spinormal")==0 )
    {
        shprintf("Using SPI to talk to wiznet\r\n");
        wizchip_spi_initialize((PLL_SYS_KHZ / clockdiv) * 1000);
        wizchip_cris_initialize();
        wizchip_initialize();
    }
    else if ( strcmp(szSPImode, "spipio")==0 )
    {
        shprintf("Using PIO to talk to wiznet\n");
        if (wiznet_pio_init(clockdiv) != 0)   { shprintf("Failed to start with pio\n"); return; }
        wizchip_cris_initialize();
        void wizchip_initialize_pio(void);
        wizchip_initialize_pio();
    }
    else
    {
        shprintf("param error(spinormal/spipio) \r\n");
        return;
    }

uint8_t temp_mac[6] = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56};
setSHAR(temp_mac);

    // WIZ5X00 PHY configuration for link speed 100MHz
    wiz_PhyConf gPhyConf = {.by = PHY_CONFBY_SW,
                            .mode = PHY_MODE_MANUAL,
                            .speed = PHY_SPEED_100,
                            .duplex = PHY_DUPLEX_FULL};

    ctlwizchip(CW_SET_PHYCONF, &gPhyConf);
    ctlwizchip(CW_RESET_PHY, 0);

    g_w5k_status = 1;
}

void w5x00_info()
{   
    uint8_t mac[6]; 
    getSHAR(mac);
    assert((mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]) != 0);
    shprintf("mac address %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    wiz_PhyConf conf_in = {0};
    ctlwizchip(CW_GET_PHYCONF, &conf_in);
    shprintf("phy config mode: %s\r\n", conf_in.mode ? "auto" : "manual");
    shprintf("phy config speed: %s\r\n", conf_in.speed ? "100" : "10");
    shprintf("phy config duplex: %s\r\n", conf_in.duplex ? "full" : "half");    
}

struct netif g_netif;

ip_addr_t temp_ip;
ip_addr_t temp_mask;
ip_addr_t temp_gateway;

static struct dhcp g_dhcp_client;

void w5x00_lwipinit(char* szIp, char* szMask, char* szGateway)
{
    int dhcp_mode = 1;

    if ( szIp && szMask && szGateway )
    {
        if ( strlen(szIp)>0 && strlen(szMask)>0 && strlen(szGateway)>0 )
        {
            if ( ip4addr_aton(szIp,      &temp_ip)==0 )      {  shprintf("cmd_w5x00_lwipinit : Invalid IP address 1 \r\n");  return;   }
            if ( ip4addr_aton(szMask,    &temp_mask)==0 )    {  shprintf("cmd_w5x00_lwipinit : Invalid IP address 2 \r\n");  return;   }
            if ( ip4addr_aton(szGateway, &temp_gateway)==0 ) {  shprintf("cmd_w5x00_lwipinit : Invalid IP address 3 \r\n");  return;   }
            dhcp_mode = 0;
        }
    }

    lwip_init();

    netif_add(&g_netif, &temp_ip, &temp_mask, &temp_gateway, NULL, netif_initialize, netif_input);
    g_netif.name[0] = 'e';
    g_netif.name[1] = '0';

    // Assign callbacks for link and status
    netif_set_link_callback(&g_netif, netif_link_callback);
    netif_set_status_callback(&g_netif, netif_status_callback);

    int retval = socket(SOCKET_MACRAW, Sn_MR_MACRAW, 0, 0x00);

    if (retval < 0)
    {
        shprintf(" MACRAW socket open failed\r\n");
        return;
    }

    netif_set_link_up(&g_netif);
    netif_set_up(&g_netif);

    if ( dhcp_mode==1 )
    {
        dhcp_set_struct(&g_netif, &g_dhcp_client);
        dhcp_start(&g_netif);
    }
}

void w5x00_readbuff(char* param1, char* param2)
{
    uint8_t bufftemp[256] = {0,};
    uint32_t addr = (uint32_t) strtoul(param1, NULL, 16);
    addr = (addr << 8);
    uint32_t len = (uint32_t) strtoul(param2, NULL, 10);

    if ( len>sizeof(bufftemp) )
    {
        printf("cmd_w5x00_chipreadbuff : size error \n");
        return; 
    }
    
    WIZCHIP_READ_BUF(addr, bufftemp, len);
    //printf("W5x00(0x%08x) : ", addr);
    //dump_bytes(bufftemp, len);
    w5500_dump(addr, bufftemp, len);
}

void w5x00_read(char* param1)
{
    uint8_t bufftemp[6] = {0,};
    uint32_t addr = (uint32_t) strtoul(param1, NULL, 16);
    addr = (addr << 8);
    bufftemp[0] = WIZCHIP_READ(addr);
    //printf("W5x00(0x%08x) : %02x \n", addr, bufftemp[0]);
    w5500_dump(addr, bufftemp, 1);
}

uint8_t w5x00_read2(uint32_t addr)
{
    addr = (addr << 8);
    return WIZCHIP_READ(addr);
}

void w5x00_write2(uint32_t addr, uint8_t data)
{
    addr = (addr << 8);
    WIZCHIP_WRITE(addr, data);
}


void w5x00_writebuff(char* param1, char* szData)
{
    uint8_t bufftemp[256] = {0,};
    uint32_t addr = (uint32_t) strtoul(param1, NULL, 16);
    addr = (addr << 8);
    uint32_t len;

    len = convertStringToByteBuffer(szData, bufftemp, sizeof(bufftemp));

    if ( len>sizeof(bufftemp) )
    {
        printf("cmd_w5x00_chipreadbuff : size error \n");
        return; 
    }
    
    WIZCHIP_WRITE_BUF(addr, bufftemp, len);
}


