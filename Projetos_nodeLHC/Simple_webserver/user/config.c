

// this is the normal build target ESP include set
#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "driver/uart.h"

#include "server.h"


#include "config.h"


void config_gpio(void) {
	// Initialize the GPIO subsystem.
	gpio_init();
	//Set GPIO2 to output mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	//Set GPIO2 high
	gpio_output_set(BIT2, 0, BIT2, 0);
}





