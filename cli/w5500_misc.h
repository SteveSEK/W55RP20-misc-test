#pragma once

// ANSI escape codes for text formatting
#define ESC "\x1b"
#define RESET ESC "[0m"
#define BOLD ESC "[1m"
#define UNDERLINE ESC "[4m"
#define RED ESC "[31m"
#define GREEN ESC "[32m"
#define YELLOW ESC "[33m"
#define BLUE ESC "[34m"
#define MAGENTA ESC "[35m"
#define CYAN ESC "[36m"
#define WHITE ESC "[37m"

#define BG_RESET ESC "[49m"
#define BG_RED ESC "[41m"
#define BG_GREEN ESC "[42m"
#define BG_CYAN ESC "[46m"
#define BG_BLACK ESC "[40m"
#define BG_BLUE ESC "[44m"
#define BG_GRAY1 ESC "[47m"
#define BG_GRAY2 ESC "[48m"

#define CLEAR_FROM_CURSOR ESC "[J"
#define MOVE_CURSOR(row, col) ESC "[" #row ";" #col "H"
#define MOVE_CURSOR2(row, col) { printf(ESC "[%d;%dH", row, col); }
#define SAVE_CURSOR ESC "[s"
#define RESTORE_CURSOR ESC "[u"

#define HIDE_CURSOR "\x1B[?25l"
#define SHOW_CURSOR "\x1B[?25h"

extern char w5x00_buf[200];
extern void shell_print(char *buf);
extern void sys_check_timeouts(void);
void task_delay_ms(uint32_t delay_ms);

//  XXXX  add shprintf
#define shprintf(fmt, ...)  { sprintf(w5x00_buf, fmt __VA_OPT__(,) __VA_ARGS__); add_crlf(); shell_print(w5x00_buf); task_delay_ms(10); }
//#define shprintf(fmt, ...)  { sprintf(w5x00_buf, fmt __VA_OPT__(,) __VA_ARGS__); add_crlf(); shell_print(w5x00_buf); }
#define shprintf2(fmt, ...)  { sprintf(w5x00_buf, fmt __VA_OPT__(,) __VA_ARGS__); shell_print(w5x00_buf); }

void add_crlf();
int is_printable(char c);
void w5500_dump(uint32_t address, unsigned char *data, uint32_t length);
int convertStringToByteBuffer(const char* str, unsigned char* buffer, uint32_t bufferLen);
uint16_t getBit(uint16_t value, int position);
uint16_t setBit(uint16_t value, int position);

