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

#define CHECK_PICO_GPIO_COUNT 30

typedef struct {
    uint8_t  no_gpio;
    uint8_t  func;
    uint8_t  mode;
    uint8_t  state;
    uint8_t  pull;
    uint8_t  slew;
    uint8_t  drv_str;
    uint32_t irq_msk;
    uint8_t  out_lvl;
    uint8_t  input_hyst;
} Check_Pico_GPIO;

Check_Pico_GPIO* g_check_gpio_old;
Check_Pico_GPIO* g_check_gpio_new;


void view_ionic_board()
{
    #define nRow  6
    MOVE_CURSOR2( nRow +   0, 120);  printf("                    +-----+                     \r\n");
    MOVE_CURSOR2( nRow +   1, 120);  printf("          .---------| USB |---------.           \r\n");
    MOVE_CURSOR2( nRow +   2, 120);  printf("         O|         +-----+         |O          \r\n");
    MOVE_CURSOR2( nRow +   3, 120);  printf("         O|                         |O          \r\n");
    MOVE_CURSOR2( nRow +   4, 120);  printf("     GND O|                         |O GND      \r\n");
    MOVE_CURSOR2( nRow +   5, 120);  printf("       X O|                         |O          \r\n");
    MOVE_CURSOR2( nRow +   6, 120);  printf("       X O|                         |O 3V3      \r\n");
    MOVE_CURSOR2( nRow +   7, 120);  printf("  GPIO 4 O|                         |O          \r\n");
    MOVE_CURSOR2( nRow +   8, 120);  printf("  GPIO 5 O|                         |O GPIO28   \r\n");
    MOVE_CURSOR2( nRow +   9, 120);  printf("     GND O|   +-----------------+   |O GND      \r\n");
    MOVE_CURSOR2( nRow +  10, 120);  printf("  GPIO 6 O|   |     W55RP20     |   |O GPIO27   \r\n");
    MOVE_CURSOR2( nRow +  11, 120);  printf("  GPIO 7 O|   |                 |   |O GPIO26   \r\n");
    MOVE_CURSOR2( nRow +  12, 120);  printf("  GPIO 8 O|   |  GPIO%02d - MISO  |   |O RUN      \r\n", PIN_MISO);
    MOVE_CURSOR2( nRow +  13, 120);  printf("  GPIO 9 O|   |  GPIO%02d - MOSI  |   |O MODE0    \r\n", PIN_MOSI);
    MOVE_CURSOR2( nRow +  14, 120);  printf("     GND O|   |  GPIO%02d - SCK   |   |O GND      \r\n", PIN_SCK);
    MOVE_CURSOR2( nRow +  15, 120);  printf("  GPIO10 O|   |  GPIO%02d - CS    |   |O MODE1    \r\n", PIN_CS);
    MOVE_CURSOR2( nRow +  16, 120);  printf("  GPIO11 O|   |  GPIO%02d - RST   |   |O MODE2    \r\n", PIN_RST);
    MOVE_CURSOR2( nRow +  17, 120);  printf("  GPIO12 O|   +-----------------+   |O GPIO19   \r\n");
    MOVE_CURSOR2( nRow +  18, 120);  printf("  GPIO13 O|                         |O GPIO18   \r\n");
    MOVE_CURSOR2( nRow +  19, 120);  printf("     GND O|                         |O GND      \r\n");
    MOVE_CURSOR2( nRow +  20, 120);  printf("  GPIO14 O|                         |O MODE3    \r\n");
    MOVE_CURSOR2( nRow +  21, 120);  printf("  GPIO15 O|                         |O MODE4    \r\n");
    MOVE_CURSOR2( nRow +  22, 120);  printf("          |                         |           \r\n");
    MOVE_CURSOR2( nRow +  23, 120);  printf("          |       +---------+       |           \r\n");
    MOVE_CURSOR2( nRow +  24, 120);  printf("          |       |         |       |           \r\n");
    MOVE_CURSOR2( nRow +  25, 120);  printf("          |       |   LAN   |       |           \r\n");
    MOVE_CURSOR2( nRow +  26, 120);  printf("          |-------|         |-------|           \r\n");
    MOVE_CURSOR2( nRow +  27, 120);  printf("                  +---------+                   \r\n");
    MOVE_CURSOR2( nRow +  28, 120);  printf("                                                \r\n");
}

const char* gpio_function_str(uint gpio_func)
{
    switch (gpio_func) {
        case GPIO_FUNC_SIO: return "GPIO";
        case GPIO_FUNC_XIP: return "XIP";
        case GPIO_FUNC_SPI: return "SPI";
        case GPIO_FUNC_UART: return "UART";
        case GPIO_FUNC_I2C: return "I2C";
        case GPIO_FUNC_PWM: return "PWM";
        case GPIO_FUNC_PIO0: return "PIO0";
        case GPIO_FUNC_PIO1: return "PIO1";
        case GPIO_FUNC_GPCK: return "GPCK";
        case GPIO_FUNC_USB: return "USB";
        case GPIO_FUNC_NULL: return "NULL";
        default: return "UNKNOWN";
    }
}

uint8_t gpio_pulls_value(uint pin)
{
    bool pull_up = gpio_is_pulled_up(pin);
    bool pull_down = gpio_is_pulled_down(pin);

    if (pull_up && pull_down)   return 3;
    else if (pull_up)           return 1;
    else if (pull_down)         return 2;
    else                        return 0; 
}

const char* gpio_pulls_str(uint data)
{
    if (data==0)         return "N/A"; 
    else if (data==1)    return "PULL_UP"; 
    else if (data==2)    return "PULL_DOWN"; 
    else if (data==3)    return "PULL_UPDN"; 
    else                 return "ERROR"; 
}

const char* gpio_drive_strength_str(uint data)
{
    switch (data) {
        case GPIO_DRIVE_STRENGTH_2MA: return "2mA";
        case GPIO_DRIVE_STRENGTH_4MA: return "4mA";
        case GPIO_DRIVE_STRENGTH_8MA: return "8mA";
        case GPIO_DRIVE_STRENGTH_12MA: return "12mA";
        default: return "UNKNOWN";
    }
}

void fill_pindata(Check_Pico_GPIO* p_gpio, uint8_t pin)
{
    p_gpio->no_gpio    = pin;  
    p_gpio->func       = gpio_get_function(pin);  
    p_gpio->mode       = gpio_is_dir_out(pin);              // 출력이면 1, 입력이면 0을 설정합니다.
    p_gpio->state      = gpio_get(pin);                     // HIGH(1) 또는 LOW(0)을 설정합니다.
    p_gpio->pull       = gpio_pulls_value(pin);             // 
    p_gpio->slew       = gpio_get_slew_rate(pin);   // SLOW(0) 또는 FAST(1)을 설정합니다.
    p_gpio->drv_str    = gpio_get_drive_strength(pin);      // 2mA, 4mA, 8mA, 12mA입니다.
    p_gpio->irq_msk    = gpio_get_irq_event_mask(pin);      // 
    p_gpio->out_lvl    = gpio_get_out_level(pin);           // HIGH(1) 또는 LOW(0)을 설정합니다.
    p_gpio->input_hyst = gpio_is_input_hysteresis_enabled(pin);  // ENABLED(1) 또는 DISABLED(0)을 설정합니다.
}


void update_pico_pins(int selected_gpio)
{
    static int iii = 0;
    MOVE_CURSOR2(1, 75); printf(GREEN BG_RESET "Monitor Cnt : %08x (%d)" RESET BG_RESET, iii++, selected_gpio);

    uint8_t tmpline = 7;
    for (int pin = 0; pin < 30; pin++) {
        Check_Pico_GPIO* old_gpio = &((Check_Pico_GPIO*)g_check_gpio_old)[pin];
        Check_Pico_GPIO* new_gpio = &((Check_Pico_GPIO*)g_check_gpio_new)[pin];

        fill_pindata(new_gpio, pin);

        MOVE_CURSOR2(tmpline + pin, 1);

        if ( selected_gpio==-1 )
        {
            printf(BG_BLUE "%02d     " BG_RESET, new_gpio->no_gpio);
        }
        else
        {
            if (pin == selected_gpio) 
            { 
                printf(BG_BLUE "%02d     " BG_RESET, new_gpio->no_gpio);
            } 
            else
            {
                printf("%02d     "     , new_gpio->no_gpio);
            }
        }

        #define TEMP_20240702_2(condition) if (condition) { printf(RED BG_GREEN); } else { printf(RESET BG_RESET); }

        TEMP_20240702_2( old_gpio->func      !=new_gpio->func       )   printf("%-8s   "       , gpio_function_str(new_gpio->func));
        TEMP_20240702_2( old_gpio->mode      !=new_gpio->mode       )   printf("%-7s "         , new_gpio->mode ? "OUTPUT" : "INPUT");
        TEMP_20240702_2( old_gpio->state     !=new_gpio->state      )   printf("%-5s   "       , new_gpio->state ? "HIGH" : "LOW");
        TEMP_20240702_2( old_gpio->pull      !=new_gpio->pull       )   printf("%-10s  "       , gpio_pulls_str(new_gpio->pull));
        TEMP_20240702_2( old_gpio->slew      !=new_gpio->slew       )   printf("%-5s      "    , new_gpio->slew ? "FAST" : "SLOW");
        TEMP_20240702_2( old_gpio->drv_str   !=new_gpio->drv_str    )   printf("%-6s         " , gpio_drive_strength_str(new_gpio->drv_str));
        TEMP_20240702_2( old_gpio->irq_msk   !=new_gpio->irq_msk    )   printf("0x%08x   "     , new_gpio->irq_msk);
        TEMP_20240702_2( old_gpio->out_lvl   !=new_gpio->out_lvl    )   printf("%-5s     "     , new_gpio->out_lvl ? "HIGH" : "LOW");
        TEMP_20240702_2( old_gpio->input_hyst!=new_gpio->input_hyst )   printf("%-10s "        , new_gpio->input_hyst ? "ENABLED" : "DISABLED");


        memcpy(old_gpio, new_gpio, sizeof(Check_Pico_GPIO));
    }
}

void init_pico_pins()
{
    Check_Pico_GPIO temp_Check_Pico_GPIO[CHECK_PICO_GPIO_COUNT] = {0,};

    g_check_gpio_old = malloc(sizeof(temp_Check_Pico_GPIO));
    g_check_gpio_new = malloc(sizeof(temp_Check_Pico_GPIO));
    memcpy(g_check_gpio_old, temp_Check_Pico_GPIO, sizeof(temp_Check_Pico_GPIO));
    memcpy(g_check_gpio_new, temp_Check_Pico_GPIO, sizeof(temp_Check_Pico_GPIO));

    MOVE_CURSOR2( 1, 1); printf("      <<<<< ioNIC Pins >>>>>");

    MOVE_CURSOR2( 4, 1); printf("=================================================================================================================");
    MOVE_CURSOR2( 5, 1); printf("GPIO   Function   Mode    State   Pull        Slew-Rate  Drive-Strength IRQ-Mask     Out-Level Input-Hysteresis ");
    MOVE_CURSOR2( 6, 1); printf("=================================================================================================================");

    MOVE_CURSOR2(37, 1); printf("=================================================================================================================");
    MOVE_CURSOR2(38, 1); printf("  Press Key => (x) eXit || (a) select all (u) up ((d) down                                                       ");
    MOVE_CURSOR2(39, 1); printf("                           (0) N/A (1) PULL_UP (2) PULL_DOWN (3) PULL_UPDN                                       ");
    MOVE_CURSOR2(40, 1); printf("                           (f) gpio_func (i) in/out (h) high/low  (s) slew rate (v) drv-strength                 ");
    MOVE_CURSOR2(41, 1); printf("=================================================================================================================");
}

void deinit_pico_pins()
{
    free(g_check_gpio_old);
    free(g_check_gpio_new);
}

void allpins_action()
{
    Check_Pico_GPIO temp_Check_Pico_GPIO[CHECK_PICO_GPIO_COUNT] = {
        { 0 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 1 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 2 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 3 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 4 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 5 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 6 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 7 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 8 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 9 , GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 10, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 11, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 12, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 13, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 14, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 15, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 16, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 17, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 18, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 19, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 20, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 21, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 22, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 23, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 24, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 25, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 26, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 27, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 28, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
        { 29, GPIO_FUNC_NULL, 0, 0, 1, 0, GPIO_DRIVE_STRENGTH_4MA, 0, 0, 1}, 
    };

    for ( int i=0; i<CHECK_PICO_GPIO_COUNT; i++)
    {
        gpio_set_function(i, temp_Check_Pico_GPIO[i].func);
        gpio_set_dir(i, temp_Check_Pico_GPIO[i].mode);
        gpio_put(i, temp_Check_Pico_GPIO[i].state);

        if ( temp_Check_Pico_GPIO[i].pull==0 )       {  gpio_set_pulls(i, 0, 0);  }
        else if ( temp_Check_Pico_GPIO[i].pull==1 )  {  gpio_set_pulls(i, 1, 0);  }
        else if ( temp_Check_Pico_GPIO[i].pull==2 )  {  gpio_set_pulls(i, 0, 1);  }
        else if ( temp_Check_Pico_GPIO[i].pull==3 )  {  gpio_set_pulls(i, 1, 1);  }

        gpio_set_slew_rate(i, temp_Check_Pico_GPIO[i].slew);
        gpio_set_drive_strength(i, temp_Check_Pico_GPIO[i].drv_str);
    }

}





void pico_pins_monitor()
{
    static char clear_msg[] = "" USH_SHELL_CLEAR_SCREEN USH_SHELL_HOME;
    shell_print(clear_msg);
    
    //CLEAR_FROM_CURSOR;
    printf(HIDE_CURSOR);

    init_pico_pins();
    view_ionic_board();

    uint8_t tmp_func = 0;
    uint8_t tmp_mode = 0;
    uint8_t tmp_level = 0;
    uint8_t tmp_pull = 0;
    uint8_t tmp_slew = 0;
    uint8_t tmp_drvstr = 0;
    int selected_gpio = 0;

#define TEMP_20240719_02(aa,bb)     if (selected_gpio==-1 ) { for (int jj=0; jj<CHECK_PICO_GPIO_COUNT; jj++) { aa; } } else { bb; }

    while (1)
    {
        sleep_ms(100);

        update_pico_pins(selected_gpio);

        char cli_usb_getc(void);
        char tempch = cli_usb_getc();
        if (tempch == 'x')
        {
            break;
        }
        else if (tempch == 'f')
        {
            if ( tmp_func==0 )         {  TEMP_20240719_02(gpio_set_function(jj, 0   ), gpio_set_function(selected_gpio, 0   ));  tmp_func++; }
            else if ( tmp_func==1   )  {  TEMP_20240719_02(gpio_set_function(jj, 1   ), gpio_set_function(selected_gpio, 1   ));  tmp_func++; }
            else if ( tmp_func==2   )  {  TEMP_20240719_02(gpio_set_function(jj, 2   ), gpio_set_function(selected_gpio, 2   ));  tmp_func++; }
            else if ( tmp_func==3   )  {  TEMP_20240719_02(gpio_set_function(jj, 3   ), gpio_set_function(selected_gpio, 3   ));  tmp_func++; }
            else if ( tmp_func==4   )  {  TEMP_20240719_02(gpio_set_function(jj, 4   ), gpio_set_function(selected_gpio, 4   ));  tmp_func++; }
            else if ( tmp_func==5   )  {  TEMP_20240719_02(gpio_set_function(jj, 5   ), gpio_set_function(selected_gpio, 5   ));  tmp_func++; }
            else if ( tmp_func==6   )  {  TEMP_20240719_02(gpio_set_function(jj, 6   ), gpio_set_function(selected_gpio, 6   ));  tmp_func++; }
            else if ( tmp_func==7   )  {  TEMP_20240719_02(gpio_set_function(jj, 7   ), gpio_set_function(selected_gpio, 7   ));  tmp_func++; }
            else if ( tmp_func==8   )  {  TEMP_20240719_02(gpio_set_function(jj, 8   ), gpio_set_function(selected_gpio, 8   ));  tmp_func++; }
            else if ( tmp_func==9   )  {  TEMP_20240719_02(gpio_set_function(jj, 9   ), gpio_set_function(selected_gpio, 9   ));  tmp_func=0x1f; }
            else if ( tmp_func==0x1f)  {  TEMP_20240719_02(gpio_set_function(jj, 0x1f), gpio_set_function(selected_gpio, 0x1f));  tmp_func=0; }

        }
        else if (tempch == 'i')
        {
            if ( tmp_mode==0 )       {  TEMP_20240719_02(gpio_set_dir(jj, GPIO_IN), gpio_set_dir(selected_gpio, GPIO_IN));  tmp_mode++; }
            else if ( tmp_mode==1 )  {  TEMP_20240719_02(gpio_set_dir(jj, GPIO_OUT), gpio_set_dir(selected_gpio, GPIO_OUT)); tmp_mode = 0; }
        }
        else if (tempch == 'h')
        {
            if ( tmp_level==0 )       {  TEMP_20240719_02(gpio_put(jj, 0), gpio_put(selected_gpio, 0)); tmp_level++; }
            else if ( tmp_level==1 )  {  TEMP_20240719_02(gpio_put(jj, 1), gpio_put(selected_gpio, 1)); tmp_level = 0; }
        }
        /*
        else if (tempch == 'p')
        {
            if ( tmp_pull==0 )       {  TEMP_20240719_02(gpio_set_pulls(jj, 0, 0), gpio_set_pulls(selected_gpio, 0, 0)); tmp_pull++; }
            else if ( tmp_pull==1 )  {  TEMP_20240719_02(gpio_set_pulls(jj, 1, 0), gpio_set_pulls(selected_gpio, 1, 0)); tmp_pull++; }
            else if ( tmp_pull==2 )  {  TEMP_20240719_02(gpio_set_pulls(jj, 0, 1), gpio_set_pulls(selected_gpio, 0, 1)); tmp_pull++; }
            else if ( tmp_pull==3 )  {  TEMP_20240719_02(gpio_set_pulls(jj, 1, 1), gpio_set_pulls(selected_gpio, 1, 1)); tmp_pull = 0; }
        }
        */
        else if (tempch == '0')
        {
            TEMP_20240719_02(gpio_set_pulls(jj, 0, 0), gpio_set_pulls(selected_gpio, 0, 0));
        }
        else if (tempch == '1')
        {
            TEMP_20240719_02(gpio_set_pulls(jj, 1, 0), gpio_set_pulls(selected_gpio, 1, 0));
        }
        else if (tempch == '2')
        {
            TEMP_20240719_02(gpio_set_pulls(jj, 0, 1), gpio_set_pulls(selected_gpio, 0, 1));
        }
        else if (tempch == '3')
        {
            TEMP_20240719_02(gpio_set_pulls(jj, 1, 1), gpio_set_pulls(selected_gpio, 1, 1));
        }
        else if (tempch == 's')
        {
            if ( tmp_slew==0 )       {  TEMP_20240719_02(gpio_set_slew_rate(jj, 0), gpio_set_slew_rate(selected_gpio, 0)); tmp_slew++; }
            else if ( tmp_slew==1 )  {  TEMP_20240719_02(gpio_set_slew_rate(jj, 1), gpio_set_slew_rate(selected_gpio, 1)); tmp_slew = 0; }
        }
        else if (tempch == 'v')
        {
            if ( tmp_drvstr==0 )       {  TEMP_20240719_02(gpio_set_drive_strength(jj, 0), gpio_set_drive_strength(selected_gpio, 0)); tmp_drvstr++; }
            else if ( tmp_drvstr==1 )  {  TEMP_20240719_02(gpio_set_drive_strength(jj, 1), gpio_set_drive_strength(selected_gpio, 1)); tmp_drvstr++; }
            else if ( tmp_drvstr==2 )  {  TEMP_20240719_02(gpio_set_drive_strength(jj, 2), gpio_set_drive_strength(selected_gpio, 2)); tmp_drvstr++; }
            else if ( tmp_drvstr==3 )  {  TEMP_20240719_02(gpio_set_drive_strength(jj, 3), gpio_set_drive_strength(selected_gpio, 3)); tmp_drvstr = 0; }
        }
        else if (tempch == 'r')
        {
            // XXXX
        }
        else if (tempch == 'a') // 
        {
            if ( selected_gpio==-1 )    selected_gpio = 0;
            else                        selected_gpio= -1;
        }
        else if (tempch == 'u') // Arrow Up
        {
            if ( selected_gpio==-1 )    continue;
            if (selected_gpio > 0) selected_gpio--;
            if (selected_gpio==0)  selected_gpio = 29;
        }
        else if (tempch == 'd') // Arrow Down
        {
            if ( selected_gpio==-1 )    continue;
            if (selected_gpio < 29) selected_gpio++;
            if (selected_gpio==29)  selected_gpio = 0;
        }
    }

    deinit_pico_pins();
    MOVE_CURSOR2(40, 1); printf("\r\n");
    
    printf(SHOW_CURSOR);
    //RESTORE_CURSOR;
}




// GPIO 핀의 상태를 출력하는 함수
int get_gpio_status(uint gpio)
{
    // GPIO를 입력 모드로 설정하고 풀다운 활성화
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_down(gpio);
    sleep_ms(10); // 약간의 딜레이를 줘서 안정화
    bool is_pulled_down = gpio_get(gpio);

    // GPIO를 입력 모드로 설정하고 풀업 활성화
    gpio_pull_up(gpio);
    sleep_ms(10); // 약간의 딜레이를 줘서 안정화
    bool is_pulled_up = gpio_get(gpio);

    // GPIO를 입력 모드로 설정하고 풀업/풀다운 비활성화
    gpio_disable_pulls(gpio);
    sleep_ms(10); // 약간의 딜레이를 줘서 안정화
    bool is_floating = gpio_get(gpio);

    // 결과 출력
    printf("GPIO %d: ", gpio);
    if (is_pulled_down && !is_pulled_up) {
        return 1; // GND
    } else if (!is_pulled_down && is_pulled_up) {
        return 2; // VCC
    } else if (!is_pulled_down && !is_pulled_up && !is_floating) {
        return 3; // Float
    } else {
        return 4; // N/A
    }
}



void pico_pin_test()
{
    uint8_t tmp_value[12];
    printf("================================================================================================================================================================\r\n");
    printf("      |                    Input                                |                 Output (High)                 |                  Output (Low)                 \r\n");
    printf("      |---------------------------------------------------------|-----------------------------------------------|-----------------------------------------------\r\n");
    printf("GPIO  |   NO-Pull    PullUP    PullUPDN   PullDN    PullUPDN    |   NO-Pull    PullUP    PullDN    PullUPDN     |   NO-Pull    PullUP    PullDN    PullUPDN     \r\n");
    printf("================================================================================================================================================================\r\n");
    
    for ( int i=0; i<CHECK_PICO_GPIO_COUNT; i++)
    {
        gpio_set_function(i, GPIO_FUNC_SIO);

        gpio_set_dir(i, GPIO_IN);

        gpio_set_pulls(i, 0, 0); sleep_ms(10); tmp_value[0] = gpio_get(i);
        gpio_set_pulls(i, 1, 0); sleep_ms(10); tmp_value[1] = gpio_get(i);
        gpio_set_pulls(i, 1, 1); sleep_ms(10); tmp_value[2] = gpio_get(i);
        gpio_set_pulls(i, 0, 1); sleep_ms(10); tmp_value[3] = gpio_get(i);        
        gpio_set_pulls(i, 1, 1); sleep_ms(10); tmp_value[4] = gpio_get(i);

        
        gpio_set_dir(i, GPIO_OUT);

        gpio_put(i, 1);
        gpio_set_pulls(i, 0, 0); sleep_ms(10); tmp_value[5] = gpio_get(i);
        gpio_set_pulls(i, 1, 0); sleep_ms(10); tmp_value[6] = gpio_get(i);
        gpio_set_pulls(i, 0, 1); sleep_ms(10); tmp_value[7] = gpio_get(i);
        gpio_set_pulls(i, 1, 1); sleep_ms(10); tmp_value[8] = gpio_get(i);

        gpio_put(i, 0);
        gpio_set_pulls(i, 0, 0); sleep_ms(10); tmp_value[9] = gpio_get(i);
        gpio_set_pulls(i, 1, 0); sleep_ms(10); tmp_value[10] = gpio_get(i);
        gpio_set_pulls(i, 0, 1); sleep_ms(10); tmp_value[11] = gpio_get(i);
        gpio_set_pulls(i, 1, 1); sleep_ms(10); tmp_value[12] = gpio_get(i);


        #define TEMP_20240722_1(a)  (a==1)?"HIGH":"LOW "

        printf("%02d        %-4s       %-4s      %-4s       %-4s      %-4s            %-4s       %-4s      %-4s      %-4s             %-4s       %-4s      %-4s      %-4s\r\n", i, 
                                TEMP_20240722_1(tmp_value[0]), TEMP_20240722_1(tmp_value[1]), TEMP_20240722_1(tmp_value[2]), TEMP_20240722_1(tmp_value[3]), 
                                TEMP_20240722_1(tmp_value[4]), TEMP_20240722_1(tmp_value[5]), TEMP_20240722_1(tmp_value[6]), TEMP_20240722_1(tmp_value[7]), 
                                TEMP_20240722_1(tmp_value[8]), TEMP_20240722_1(tmp_value[9]), TEMP_20240722_1(tmp_value[10]), TEMP_20240722_1(tmp_value[11]), TEMP_20240722_1(tmp_value[12]));
    }

}


