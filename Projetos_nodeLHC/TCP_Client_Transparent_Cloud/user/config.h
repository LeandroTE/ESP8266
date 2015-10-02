#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CONFIG_STATIC
#ifdef CONFIG_STATIC

#define STA_SSID	"Time Energy"
#define STA_PASSWORD	"@cessorestrito!"
#define AP_SSID		"ESP_Time_Energy"
#define AP_PASSWORD	"seila258"
// over ride in this file
#include "config_wifi.h"

void config_execute(void);

#endif

//#define CONFIG_DYNAMIC
#ifdef CONFIG_DYNAMIC

typedef struct config_commands {
	char *command;
	void(*function)(serverConnData *conn, uint8_t argc, char *argv[]);
} config_commands_t;


void config_parse(serverConnData *conn, char *buf, int len);

#endif

#define CONFIG_GPIO
#ifdef CONFIG_GPIO
#include <gpio.h>

void config_gpio(void);

#endif

#endif /* __CONFIG_H__ */
