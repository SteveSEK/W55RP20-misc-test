#pragma once

#define SOCKET_MACRAW 0

extern struct netif g_netif;

int wiznet_pio_init(int clockdiv);
void w5x00_init(char* szSPImode, char* szClockDiv);
void w5x00_info();
void w5x00_lwipinit(char* szIp, char* szMask, char* szGateway);
void w5x00_readbuff(char* param1, char* param2);
void w5x00_read(char* param1);
uint8_t w5x00_read2(uint32_t addr);
void w5x00_write2(uint32_t addr, uint8_t data);
void w5x00_writebuff(char* param1, char* szData);

