/*
 * File	: user_main.c
 * This file is part of Espressif's AT+ command set program.
 * Copyright (C) 2013 - 2016, Espressif Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.	If not, see <http://www.gnu.org/licenses/>.
 */
#include <user_interface.h>
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "driver/uart.h"
#include "../include/task.h"

#include "server.h"
#include "config.h"
#include "flash_param.h"
#include "../include/driver/i2c_sd1306.h"
#include "driver/i2c.h"

//#define WIFI_CLIENTSSID		"Time Energy"
//#define WIFI_CLIENTPASSWORD	"@cessorestrito!"
//
//#define WIFI_CLIENTSSID		"God Save The Queen"
//#define WIFI_CLIENTPASSWORD	"@cessorestrito"

#define WIFI_CLIENTSSID		"LHC"
#define WIFI_CLIENTPASSWORD	"tijolo22"

#define WIFI_CLIENTSSID_AP		"Medidor TE"
#define WIFI_CLIENTPASSWORD_AP	"@cessorestrito!"

os_event_t		recvTaskQueue[recvTaskQueueLen];
extern  serverConnData connData[MAX_CONN];
struct rst_info * reset_info;


typedef enum {
	WIFI_CONNECTING,
	WIFI_CONNECTING_ERROR,
	WIFI_CONNECTED,
} tConnState;

const char *WiFiMode[] =
{
		"NULL",		// 0x00
		"STATION",	// 0x01
		"SOFTAP", 	// 0x02
		"STATIONAP"	// 0x03
};

#define MAX_UARTBUFFER (MAX_TXBUFFER/4)
static uint8 uartbuffer[MAX_UARTBUFFER];


static ETSTimer WiFiLinker;

static struct ip_info ipConfig;
static tConnState connState = WIFI_CONNECTING;

unsigned char *p = (unsigned char*)&ipConfig.ip.addr;

LOCAL void ICACHE_FLASH_ATTR setup_wifi_st_mode(void)
{
	wifi_set_opmode(STATION_MODE);
	struct station_config stconfig;
	wifi_station_disconnect();
	wifi_station_dhcpc_stop();
	if(wifi_station_get_config(&stconfig))
	{
				os_memcpy(&stconfig.ssid, WIFI_CLIENTSSID, sizeof(WIFI_CLIENTSSID));
				os_memcpy(&stconfig.password, WIFI_CLIENTPASSWORD, sizeof(WIFI_CLIENTPASSWORD));
		wifi_station_set_config(&stconfig);
		//ets_uart_printf("SSID: %s\n",stconfig.ssid);
		stringDraw(5, 1, (char*)stconfig.ssid);
	}
	wifi_station_connect();
	wifi_station_dhcpc_start();
	wifi_station_set_auto_connect(1);
	//ets_uart_printf("STA mode\n");

}

LOCAL void ICACHE_FLASH_ATTR setup_wifi_ap_mode(void)
{
	ets_uart_printf("\n\nMudando Wifi para modo AP\n");
	wifi_set_opmode(SOFTAP_MODE);
	struct softap_config apconfig;
	wifi_softap_dhcps_stop();
	if(wifi_softap_get_config(&apconfig))
	{
		apconfig.authmode = AUTH_WPA_PSK;
		os_memcpy(&apconfig.ssid, WIFI_CLIENTSSID_AP, sizeof(WIFI_CLIENTSSID));
		os_memcpy(&apconfig.password, WIFI_CLIENTPASSWORD_AP, sizeof(WIFI_CLIENTPASSWORD));
		wifi_softap_set_config(&apconfig);
		//ets_uart_printf("SSID: %s\n",apconfig.ssid);

		stringDraw(5, 1, (char*)apconfig.ssid);
		//ets_uart_printf("Password: %s\n",apconfig.password);

	}
	wifi_softap_dhcps_start();


}

void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
	char IP[15];
	char IP_temp[3];
	uint8_t wifi_opmode;

	system_soft_wdt_feed();
	os_timer_disarm(&WiFiLinker);

	wifi_opmode = wifi_get_opmode();
	if(wifi_opmode==STATION_MODE){
		switch(wifi_station_get_connect_status())
		{
		case STATION_GOT_IP:

			wifi_get_ip_info(STATION_IF, &ipConfig);
			if(ipConfig.ip.addr != 0 && connState !=WIFI_CONNECTED) {
				connState = WIFI_CONNECTED;
				serverInit(23);
				//ets_uart_printf("%d.%d.%d.%d",p[0],p[1],p[2],p[3]);
				uart0_sendStr("\r");
				itoa(p[0],IP_temp);
				IP[0]=IP_temp[0];
				IP[1]=IP_temp[1];
				IP[2]=IP_temp[2];
				IP[3]='.';
				itoa(p[1],IP_temp);
				IP[4]=IP_temp[0];
				IP[5]=IP_temp[1];
				IP[6]=IP_temp[2];
				IP[7]='.';
				itoa(p[2],IP_temp);
				IP[8]=IP_temp[0];
				IP[9]='.';
				itoa(p[3],IP_temp);
				IP[10]=IP_temp[0];
				IP[11]=IP_temp[1];
				IP[12]=IP_temp[2];
				IP[13]=0;
				IP[14]=0;
				stringDraw(6, 1, "IP:");
				stringDraw(6, 18, IP);
			}
			break;
		case STATION_WRONG_PASSWORD:
			connState = WIFI_CONNECTING_ERROR;
			//ets_uart_printf("WiFi connecting error, wrong password\n");
			break;
		case STATION_NO_AP_FOUND:
			connState = WIFI_CONNECTING_ERROR;
			//ets_uart_printf("WiFi connecting error, ap not found\n");
			setup_wifi_ap_mode();
			break;
		case STATION_CONNECT_FAIL:
			connState = WIFI_CONNECTING_ERROR;
			//			ets_uart_printf("WiFi connecting fail\n");

			break;
		default:
			connState = WIFI_CONNECTING;
			//			ets_uart_printf("WiFi connecting...\n");
						stringDraw(6, 1, "WiFi connecting");
		}
		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 2000, 0);
	}else{
		//		ets_uart_printf("Conectar na rede SSID: %s\n",WIFI_CLIENTSSID_AP);
	}
}

static void ICACHE_FLASH_ATTR recvTask(os_event_t *events)
{
	uint8_t i;

	while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
	{
		WRITE_PERI_REG(0X60000914, 0x73); //WTD
		uint16 length = 0;
		while ((READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) && (length<MAX_UARTBUFFER))
			uartbuffer[length++] = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
		for (i = 0; i < MAX_CONN; ++i)
			if (connData[i].conn) 
				espbuffsent(&connData[i], uartbuffer, length);		
	}

	if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
	}
	else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
	}
	ETS_UART_INTR_ENABLE();
}


// UartDev is defined and initialized in rom code.
extern UartDevice    UartDev;

void user_rf_pre_init(void)
{
	reset_info = system_get_rst_info();
}

void user_init(void)
{
	uint8_t i;



	UartDev.data_bits = EIGHT_BITS;
	UartDev.parity = NONE_BITS;
	UartDev.stop_bits = ONE_STOP_BIT;
	uart_init(BIT_RATE_9600, BIT_RATE_9600);


	i2c_init();
	SSD1306Init();
	clearScreen();
	stringDraw(4, 1, "SDK ver:");
	stringDraw(4, 48, (char*)system_get_sdk_version());


	//	ets_uart_printf("reset reason: %d\n", reset_info->reason);
	//	ets_uart_printf("Booting...\n");
	//	ets_uart_printf("SDK version:%s\n", system_get_sdk_version());

	setup_wifi_st_mode();
	if(wifi_get_phy_mode() != PHY_MODE_11N)
		wifi_set_phy_mode(PHY_MODE_11N);
	if(wifi_station_get_auto_connect() == 0)
		wifi_station_set_auto_connect(1);



#ifdef CONFIG_DYNAMIC
	flash_param_t *flash_param;
	flash_param_init();
	flash_param = flash_param_get();
	UartDev.data_bits = GETUART_DATABITS(flash_param->uartconf0);
	UartDev.parity = GETUART_PARITYMODE(flash_param->uartconf0);
	UartDev.stop_bits = GETUART_STOPBITS(flash_param->uartconf0);
	uart_init(flash_param->baud, BIT_RATE_115200);
#else

#endif
	//ets_uart_printf("size flash_param_t %d\n", sizeof(flash_param_t));


	serverInit(23);


#ifdef CONFIG_GPIO
	config_gpio();
#endif



	//		os_timer_disarm(&timer_1);
	//		os_timer_setfn(&timer_1, (os_timer_func_t *)timer_1_int, NULL);
	//		os_timer_arm(&timer_1, 1000, 1);

	// Wait for Wi-Fi connection
	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);




	system_os_task(recvTask, recvTaskPrio, recvTaskQueue, recvTaskQueueLen);
}
