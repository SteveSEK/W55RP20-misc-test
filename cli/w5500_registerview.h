#pragma once

#define CHECK_W5500_REGISTER_COUNT 48

typedef struct {
    unsigned short addr;
    unsigned char  chkreg_state;
    unsigned char  frt_value;
    unsigned char  old_value;
    unsigned char  new_value;
    unsigned char  ever_setbit;
} Check_w5500_Register;

void init_Check_w5500_Register();
void w5500_display_header();
void w5500_printBinaryWithColor(int index, Check_w5500_Register* pCheckRegister);
void stringOPMDC(uint8_t data, char* szData);
void togglePhy();
void w5500_display_registers();
void w5500_update_checkregister();
int w5500_viewregister();




