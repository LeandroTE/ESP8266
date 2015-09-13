#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CONFIG_STATIC
#ifdef CONFIG_STATIC




void config_execute(void);

#endif



#define CONFIG_GPIO
#ifdef CONFIG_GPIO
#include <gpio.h>

void config_gpio(void);

#endif

#endif /* __CONFIG_H__ */
