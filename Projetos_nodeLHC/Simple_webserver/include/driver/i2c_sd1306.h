/******************************************************************************
 * File Name   :  i2c_sd1306.h
 * Author      :  Leandro
 * Description :  Header file for i2c_sd1306.c
 * Date        :  july 30, 2015.
 *****************************************************************************/

#ifndef __I2C_SD1306_H
#define	__I2C_SD1306_H

#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

//#define INA219_ADDRESS 0x80
#define SD1306_ADDRESS 0x78

// SSD1306 PARAMETERS
#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 64
#define SSD1306_MAXROWS 7
#define SSD1306_MAXCONTRAST 0xFF

	// command table
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

	// scrolling commands
#define SSD1306_SCROLL_RIGHT 0x26
#define SSD1306_SCROLL_LEFT 0X27
#define SSD1306_SCROLL_VERT_RIGHT 0x29
#define SSD1306_SCROLL_VERT_LEFT 0x2A
#define SSD1306_SET_VERTICAL 0xA3

	// address setting
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDRESS 0x21
#define SSD1306_COLUMNADDRESS_MSB 0x00
#define SSD1306_COLUMNADDRESS_LSB 0x7F
#define SSD1306_PAGEADDRESS	0x22
#define SSD1306_PAGE_START_ADDRESS 0xB0

	// hardware configuration
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_SEGREMAP 0xA1
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

	// timing and driving
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_NOP 0xE3

	// power supply configuration
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x10
#define SSD1306_SWITCHCAPVCC 0x20

// PROTOTYPES
void ICACHE_FLASH_ATTR SSD1306PinSetup( void );
void ICACHE_FLASH_ATTR SSD1306Init( void );
char* ICACHE_FLASH_ATTR itoa(int i, char b[]);
uint8_t ICACHE_FLASH_ATTR SSD1306SendCommand( char *data, int i );
void ICACHE_FLASH_ATTR SSD1306SendData( char *data, int i );
void ICACHE_FLASH_ATTR setAddress( char page, char column );
void ICACHE_FLASH_ATTR clearScreen(void);
void ICACHE_FLASH_ATTR charDraw(char row, char column, int data);
void ICACHE_FLASH_ATTR stringDraw( char row, char column, char *word);
void ICACHE_FLASH_ATTR pixelDraw(char x, char y);
void ICACHE_FLASH_ATTR horizontalLine(char xStart, char xStop, char y);
void ICACHE_FLASH_ATTR verticalLine(char x, char yStart, char yStop);
void ICACHE_FLASH_ATTR imageDraw(const char IMAGE[], char row, char column);
//void circleDraw(char x, char y, char radius);
void ICACHE_FLASH_ATTR circleDraw(register int x, register int y, int r);

void ICACHE_FLASH_ATTR Set_Contrast_Control(unsigned char d);
void ICACHE_FLASH_ATTR Set_Inverse_Display(unsigned char d);
void ICACHE_FLASH_ATTR Set_Display_On_Off(unsigned char d);

void ICACHE_FLASH_ATTR Fill_RAM( char data);
void ICACHE_FLASH_ATTR Fill_RAM_PAGE(unsigned char page, char data);



#endif
