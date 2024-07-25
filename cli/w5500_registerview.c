#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "port_common.h"
#include "shell.h"

#include "wizchip_conf.h"

#include "w5500_func.h"
#include "w5500_misc.h"
#include "w5500_polltask.h"
#include "w5500_registerview.h"

#undef printf
#define printf shprintf2

void* g_check_w5500_register;

void init_Check_w5500_Register()
{
    Check_w5500_Register temp_check_w5500_register[CHECK_W5500_REGISTER_COUNT] = {
        { 0x00, 0, 0x00, 0x00, 0x00, 0x00/*, "[Mode    0x00]", " "*/ }, 
        { 0x01, 0, 0x00, 0x00, 0x00, 0x00/*, "[G/W     0x01]", " "*/ }, 
        { 0x02, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x02]", " "*/ }, 
        { 0x03, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x03]", " "*/ }, 
        { 0x04, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x04]", " "*/ }, 
        { 0x05, 0, 0x00, 0x00, 0x00, 0x00/*, "[Subnet  0x05]", " "*/ }, 
        { 0x06, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x06]", " "*/ }, 
        { 0x07, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x07]", " "*/ }, 
        { 0x08, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x08]", " "*/ }, 
        { 0x09, 0, 0x00, 0x00, 0x00, 0x00/*, "[HWAddr  0x09]", " "*/ }, 
        { 0x0A, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x0A]", " "*/ }, 
        { 0x0B, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x0B]", " "*/ }, 
        { 0x0C, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x0C]", " "*/ }, 
        { 0x0D, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x0D]", " "*/ }, 
        { 0x0E, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x0E]", " "*/ }, 
        { 0x0F, 0, 0x00, 0x00, 0x00, 0x00/*, "[SrcIP   0x0F]", " "*/ }, 
        { 0x10, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x10]", " "*/ }, 
        { 0x11, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x11]", " "*/ }, 
        { 0x12, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x12]", " "*/ }, 
        { 0x13, 0, 0x00, 0x00, 0x00, 0x00/*, "[IntLvT  0x13]", " "*/ }, 
        { 0x14, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x14]", " "*/ }, 
        { 0x15, 0, 0x00, 0x00, 0x00, 0x00/*, "[Int     0x15]", " "*/ }, 
        { 0x16, 0, 0x00, 0x00, 0x00, 0x00/*, "[IntMsk  0x16]", " "*/ }, 
        { 0x17, 0, 0x00, 0x00, 0x00, 0x00/*, "[SckInt  0x17]", " "*/ }, 
        { 0x18, 0, 0x00, 0x00, 0x00, 0x00/*, "[SckIntM 0x18]", " "*/ }, 
        { 0x19, 0, 0x00, 0x00, 0x00, 0x00/*, "[RtrTmr  0x19]", " "*/ }, 
        { 0x1A, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x1A]", " "*/ }, 
        { 0x1B, 0, 0x00, 0x00, 0x00, 0x00/*, "[RtrCnt  0x1B]", " "*/ }, 
        { 0x1C, 0, 0x00, 0x00, 0x00, 0x00/*, "[P-Tmr   0x1C]", " "*/ }, 
        { 0x1D, 0, 0x00, 0x00, 0x00, 0x00/*, "[P-Mgc   0x1D]", " "*/ }, 
        { 0x1E, 0, 0x00, 0x00, 0x00, 0x00/*, "[P-Mac   0x1E]", " "*/ }, 
        { 0x1F, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x1F]", " "*/ }, 
        { 0x20, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x20]", " "*/ }, 
        { 0x21, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x21]", " "*/ },
        { 0x22, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x22]", " "*/ },
        { 0x23, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x23]", " "*/ },
        { 0x24, 0, 0x00, 0x00, 0x00, 0x00/*, "[P-SId   0x24]", " "*/ },
        { 0x25, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x25]", " "*/ },
        { 0x26, 0, 0x00, 0x00, 0x00, 0x00/*, "[PMRU    0x26]", " "*/ },
        { 0x27, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x27]", " "*/ },
        { 0x28, 0, 0x00, 0x00, 0x00, 0x00/*, "[UnRchIP 0x28]", " "*/ },
        { 0x29, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x29]", " "*/ },
        { 0x2A, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x2A]", " "*/ }, 
        { 0x2B, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x2B]", " "*/ }, 
        { 0x2C, 0, 0x00, 0x00, 0x00, 0x00/*, "[UnRchPt 0x2C]", " "*/ }, 
        { 0x2D, 0, 0x00, 0x00, 0x00, 0x00/*, "[        0x2D]", " "*/ }, 
        { 0x2E, 0, 0x00, 0x00, 0x00, 0x00/*, "[PHYfgr  0x2E]", " "*/ }, 
        { 0x39, 0, 0x00, 0x00, 0x00, 0x00/*, "[ChipVer 0x39]", " "*/ } 
    };

    g_check_w5500_register = malloc(sizeof(temp_check_w5500_register));
    memcpy(g_check_w5500_register, temp_check_w5500_register, sizeof(temp_check_w5500_register));
}

void w5500_display_header()
{
    printf(MOVE_CURSOR(1, 1) "      <<<<< WIZnet Chip Ethernet PHY Register >>>>>");
    printf(MOVE_CURSOR(2, 1) "Link: -");
    printf(MOVE_CURSOR(2, 12) "Auto-Nego: -");
    printf(MOVE_CURSOR(2, 38) "Speed: -");
    printf(MOVE_CURSOR(2, 51) "Duplex: -");

    printf(MOVE_CURSOR( 7, 29) "Mode");
    printf(MOVE_CURSOR( 8, 29) "G/W");
    printf(MOVE_CURSOR(12, 29) "SubNet");
    printf(MOVE_CURSOR(16, 29) "MAC");
    printf(MOVE_CURSOR(22, 29) "SRC IP");

    printf(MOVE_CURSOR(16, 68) "RtrTmr");
    printf(MOVE_CURSOR(18, 68) "RtrCnt");
    printf(MOVE_CURSOR(19, 68) "P-Tmr");

    printf(MOVE_CURSOR(13, 108) "PMRU");
    
    printf(MOVE_CURSOR(21, 108) "PHY cfg");
    printf(MOVE_CURSOR(22, 108) "Chip Version");
}


void w5500_printBinaryWithColor(int index, Check_w5500_Register* pCheckRegister)
{
    int nRow, nCol;
    nRow = (index%16) + 7;
    nCol = (index/16) * 40;

    MOVE_CURSOR2(nRow, nCol);
    printf(RESET " " RESET BG_RESET);

    if ( pCheckRegister->old_value==pCheckRegister->new_value ) {   printf("[0x%04x] : 0x%02x(", pCheckRegister->addr, pCheckRegister->new_value); }
    else                                                        {   printf("[0x%04x] : " GREEN "0x%02x" RESET BG_RESET "(", pCheckRegister->addr, pCheckRegister->new_value); }

    static int8_t bg_index = 0;
    static char* bg_color[5] = { ESC "[42m", ESC "[43m", ESC "[44m", ESC "[45m", ESC "[46m" };

    for (int i = 7; i >= 0; i--) {
        int frtBit = getBit(pCheckRegister->frt_value, i);
        int oldBit = getBit(pCheckRegister->old_value, i);
        int newBit = getBit(pCheckRegister->new_value, i);

        if ( newBit==1 )
        {
            pCheckRegister->ever_setbit = pCheckRegister->ever_setbit | (uint16_t)(1 << (uint16_t)i);
        }
        
        if (oldBit != newBit) {
            if ( bg_index==5 ) bg_index = 0;
            printf("%s" RED "%d" RESET BG_RESET, bg_color[bg_index++], newBit);
        } else {
            if ( pCheckRegister->chkreg_state==0 )
            {
                printf("%d", newBit);
                pCheckRegister->chkreg_state = 1;
            }
            else
            {
                if ( getBit(pCheckRegister->ever_setbit, i)== 1 )   { printf(RED "%d" RESET, newBit); }
                else                                                { printf("%d", newBit); }
            }
        }        
    }
    printf(")");
}

void stringOPMDC(uint8_t data, char* szData)
{
    uint8_t tempBuff = (data & 0x38)>>3; 

    if ( tempBuff==0x0)         strcpy(szData, "10BT/H/Mann ");
    else if ( tempBuff==0x1)    strcpy(szData, "10BT/F/Mann ");
    else if ( tempBuff==0x2)    strcpy(szData, "100BT/H/Mann");
    else if ( tempBuff==0x3)    strcpy(szData, "100BT/F/Mann");
    else if ( tempBuff==0x4)    strcpy(szData, "100BT/H/Auto");
    else if ( tempBuff==0x5)    strcpy(szData, "Not Used    ");
    else if ( tempBuff==0x6)    strcpy(szData, "PowerDown   ");
    else if ( tempBuff==0x7)    strcpy(szData, "All/Auto    ");
    else                        strcpy(szData, "N/A         ");
}

void togglePhy()
{
    Check_w5500_Register* ppp = ((Check_w5500_Register*)g_check_w5500_register);
    uint8_t data = ppp[46].new_value;

    uint8_t extractedBits = (data & 0x38) >> 3;

    extractedBits = (extractedBits + 1) & 0x07;
    uint8_t incrementedBits = (extractedBits << 3);

    uint8_t data2 = (data & ~0x38) | incrementedBits;

    w5x00_write2(46, data2);

    ctlwizchip(CW_RESET_PHY, 0);
}

void w5500_display_registers()
{
    char szBuff[32];

    Check_w5500_Register* ppp = ((Check_w5500_Register*)g_check_w5500_register);
    
    if ( getBit(ppp[46].new_value, 0)==1 )  { printf(MOVE_CURSOR(2, 7)  BOLD BG_GREEN "UP  " RESET BG_RESET); }
    else                                    { printf(MOVE_CURSOR(2, 7)  BOLD BG_RED   "DOWN" RESET BG_RESET); }

    stringOPMDC(ppp[46].new_value, szBuff);
    printf(MOVE_CURSOR(2, 23) BOLD BG_GREEN "%s" RESET BG_RESET, szBuff);

    if ( getBit(ppp[46].new_value, 1)==1 )  { printf(MOVE_CURSOR(2, 45) BOLD BG_BLUE  "100M " RESET BG_RESET); }
    else                                    { printf(MOVE_CURSOR(2, 45) BOLD BG_BLACK "10M  " RESET BG_RESET); }
    if ( getBit(ppp[46].new_value, 2)==1 )  { printf(MOVE_CURSOR(2, 59) BOLD BG_BLUE  "Full " RESET BG_RESET); }
    else                                    { printf(MOVE_CURSOR(2, 59) BOLD BG_BLACK "Half  " RESET BG_RESET); }

    printf(MOVE_CURSOR( 4, 1) "================================================================================================================================");
    printf(MOVE_CURSOR( 5, 1) " Common Register ");
    printf(MOVE_CURSOR( 6, 1) "================================================================================================================================");

    static int iii = 0;
    MOVE_CURSOR2(1, 75); printf( GREEN "Monitor Cnt : %08x" RESET BG_RESET, iii++);

    for (int i=0; i<CHECK_W5500_REGISTER_COUNT; i++)
    {
        w5500_printBinaryWithColor(i, &ppp[i]);
    }
    printf(MOVE_CURSOR(23, 1) "================================================================================================================================");
    printf(MOVE_CURSOR(24, 1) " Socket Registers ");
    printf(MOVE_CURSOR(25, 1) "================================================================================================================================");

    //printf(MOVE_CURSOR(22, 1) "(#) SnMR(1) SnCR(1) SnIR(1) SnSR(1) SnPORT(2) SnDHAR(4) SnDIPR(4) SnDPORT(2) SnMSSR(2) SnTOS(1) SnTTL(1) Sn_RXBUF_SIZE(2) Sn_TXBUF_SIZE(2) ... ");
    printf(MOVE_CURSOR(26, 1) "(#) Mode CMD INT STA PORT Destination                      MSSR/TOS/TTL");
    printf("  BUF_SIZE TX_BUF_PTR      RX_BUF_PTR      IMR/FRAG/KPALV");
    
    uint8_t tmpline = 27;
    for (int i=0; i<_WIZCHIP_SOCK_NUM_; i++)
    {
        MOVE_CURSOR2(tmpline + i, 1);
        WIZCHIP_READ_BUF(Sn_MR(i), szBuff, 0x17);
        printf("(%d) %02x   %02x  %02x  %02x  %02x%02x %02x:%02x:%02x:%02x:%02x:%02x/%02x%02x%02x%02x/%02x%02x  %02x%02x/%02x/%02x", (i+1), 
                        szBuff[0], szBuff[1], szBuff[2], szBuff[3], szBuff[4], szBuff[5], szBuff[6], szBuff[7], szBuff[8], szBuff[9], 
                        szBuff[10], szBuff[11], szBuff[12], szBuff[13], /*szBuff[14],*/ szBuff[15], szBuff[16], szBuff[17], szBuff[18], szBuff[19], 
                        szBuff[20], szBuff[21], szBuff[22] );

        WIZCHIP_READ_BUF(Sn_RXBUF_SIZE(i), szBuff, 18);
        printf("    %02x/%02x    %02x%02x/%02x%02x/%02x%02x  %02x%02x/%02x%02x/%02x%02x  %02x/%02x%02x/%02x ", 
                        szBuff[0], szBuff[1], szBuff[2], szBuff[3], szBuff[4], szBuff[5], szBuff[6], szBuff[7], szBuff[8], szBuff[9], 
                        szBuff[10], szBuff[11], szBuff[12], szBuff[13], szBuff[14], szBuff[15], szBuff[16], szBuff[17] );
    }

    printf(MOVE_CURSOR(36, 1) "================================================================================================================================");
    MOVE_CURSOR2(37, 1);   printf("  Press Key => (x) eXit  (p) toggle PHY mode");
    printf(MOVE_CURSOR(38, 1) "================================================================================================================================");
}

extern uint8_t g_w5k_status;

void w5500_update_checkregister()
{
    Check_w5500_Register* ppp = ((Check_w5500_Register*)g_check_w5500_register);

    if ( g_w5k_status==0 ) return; 

    uint8_t temp_values[CHECK_W5500_REGISTER_COUNT];
    for (int i = 0; i < CHECK_W5500_REGISTER_COUNT; i++) {
        temp_values[i] = w5x00_read2(ppp[i].addr);
    }

    for (int i = 0; i < CHECK_W5500_REGISTER_COUNT; i++) {

        if ( ppp[i].chkreg_state==0 )
        {
            ppp[i].frt_value = temp_values[i];
            ppp[i].old_value = temp_values[i];
            ppp[i].new_value = temp_values[i];
        }
        else
        {
            ppp[i].old_value = ppp[i].new_value;
            ppp[i].new_value = temp_values[i];

            if (ppp[i].old_value != ppp[i].new_value) {
                ppp[i].chkreg_state = 3;
            }
            else {
                ppp[i].chkreg_state = 2;
            }
        }
    }
}


int w5500_viewregister()
{
    init_Check_w5500_Register();

    static char clear_msg[] =   "" USH_SHELL_CLEAR_SCREEN USH_SHELL_HOME;
    shell_print(clear_msg);
    
    CLEAR_FROM_CURSOR;

    w5500_display_header();
    w5500_display_registers();

    g_w5k_status = 11;
    while (1) {
        sleep_ms(100);

        w5500_update_checkregister();
        w5500_display_registers();

        char cli_usb_getc(void);
        char tempch = cli_usb_getc();
        if ( tempch=='x' )
            break;
        else if ( tempch=='p' )
            togglePhy();
    }
    g_w5k_status = 1;

    free(g_check_w5500_register);
    RESTORE_CURSOR;

    return 0;
}
