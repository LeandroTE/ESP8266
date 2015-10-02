#include "espmissingincludes.h"
#include "c_types.h"

#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

#include "server.h"
#include "config.h"

struct espconn serverConn;
static esp_tcp serverTcp;

#define Debug

//Connection pool
static char txbuffer[MAX_CONN][MAX_TXBUFFER];
serverConnData connData[MAX_CONN];
static ETSTimer timer_1;


static void ICACHE_FLASH_ATTR timer_server(void *arg)
{
	static uint8_t state;
	os_timer_disarm(&timer_1);
#ifdef Debug
	ets_uart_printf("Tentando reconectar\n");
#endif
	if(!network_start()){
		return;
	}
}

static serverConnData ICACHE_FLASH_ATTR *serverFindConnData(void *arg) {
	int i;
	for (i=0; i<MAX_CONN; i++) {
		if (connData[i].conn==(struct espconn *)arg) 
			return &connData[i];
	}
	//ets_uart_printf("FindConnData: Huh? Couldn't find connection for %p\n", arg);
	return NULL; //WtF?
}


//send all data in conn->txbuffer
//returns result from espconn_sent if data in buffer or ESPCONN_OK (0)
//only internal used by espbuffsent, serverSentCb
static sint8  ICACHE_FLASH_ATTR sendtxbuffer(serverConnData *conn) {
	sint8 result = ESPCONN_OK;
	if (conn->txbufferlen != 0)	{
		conn->readytosend = false;
		result= espconn_sent(conn->conn, (uint8_t*)conn->txbuffer, conn->txbufferlen);
		conn->txbufferlen = 0;	

	}
	return result;
}


//send formatted output to transmit buffer and call sendtxbuffer, if ready (previous espconn_sent is completed)
sint8 ICACHE_FLASH_ATTR  espbuffsentprintf(serverConnData *conn, const char *format, ...) {
	uint16 len;
	va_list al;
	va_start(al, format);
	len = ets_vsnprintf(conn->txbuffer + conn->txbufferlen, MAX_TXBUFFER - conn->txbufferlen - 1, format, al);
	va_end(al);
	if (len <0)  {
		return len;
	}
	conn->txbufferlen += len;
	if (conn->readytosend)
		return sendtxbuffer(conn);
	return ESPCONN_OK;
}


//send string through espbuffsent
sint8 ICACHE_FLASH_ATTR espbuffsentstring(serverConnData *conn, const char *data) {
	return espbuffsent(conn, data, strlen(data));
}

//use espbuffsent instead of espconn_sent
//It solve problem: the next espconn_sent must after espconn_sent_callback of the pre-packet.
//Add data to the send buffer and send if previous send was completed it call sendtxbuffer and  espconn_sent
//Returns ESPCONN_OK (0) for success, -128 if buffer is full or error from  espconn_sent
sint8 ICACHE_FLASH_ATTR espbuffsent(serverConnData *conn, const char *data, uint16 len) {
	if (conn->txbufferlen + len > MAX_TXBUFFER) {
		//ets_uart_printf("espbuffsent: txbuffer full on conn %p\n", conn);
		return -128;
	}
	os_memcpy(conn->txbuffer + conn->txbufferlen, data, len);
	conn->txbufferlen += len;
	if (conn->readytosend) 
		return sendtxbuffer(conn);
	return ESPCONN_OK;
}

//callback after the data are sent
static void ICACHE_FLASH_ATTR serverSentCb(void *arg) {
	serverConnData *conn = serverFindConnData(arg);
	//ets_uart_printf("Sent callback on conn %p\n", conn);
	if (conn==NULL) return;
	conn->readytosend = true;
	sendtxbuffer(conn); // send possible new data in txbuffer
}

static void ICACHE_FLASH_ATTR serverRecvCb(void *arg, char *data, unsigned short len) {
	int x;
	char *p, *e;
	serverConnData *conn = serverFindConnData(arg);
	//ets_uart_printf("Receive callback on conn %p\n", conn);
	if (conn == NULL) return;


	uart0_tx_buffer(data, len);
}

static void ICACHE_FLASH_ATTR serverReconCb(void *arg, sint8 err) {
	serverConnData *conn=serverFindConnData(arg);
#ifdef Debug
	ets_uart_printf("ReconCb\n");
#endif
	if (conn==NULL) return;
	//Yeah... No idea what to do here. ToDo: figure something out.
}

static void ICACHE_FLASH_ATTR serverDisconCb(void *arg) {
	//Just look at all the sockets and kill the slot if needed.
	int i;
	static int counter;
#ifdef Debug
	ets_uart_printf("Desconectado\n");
#endif
	for (i=0; i<MAX_CONN; i++) {
		if (connData[i].conn!=NULL) {
			if (connData[i].conn->state==ESPCONN_NONE || connData[i].conn->state==ESPCONN_CLOSE) {
				connData[i].conn=NULL;
			}
		}
	}

		os_timer_setfn(&timer_1, (os_timer_func_t *)timer_server, NULL);
		os_timer_arm(&timer_1, 5000, 0);

}

static void ICACHE_FLASH_ATTR serverConnectCb(void *arg) {
	struct espconn *conn = arg;
	int i;
	//Find empty conndata in pool
#ifdef Debug
	ets_uart_printf("Conectado\n");
#endif
	for (i=0; i<MAX_CONN; i++) if (connData[i].conn==NULL) break;
	//ets_uart_printf("Con req, conn=%p, pool slot %d\n", conn, i);

	if (i==MAX_CONN) {
		//ets_uart_printf("Aiee, conn pool overflow!\n");
		espconn_disconnect(conn);
		return;
	}

	connData[i].conn=conn;
	connData[i].txbufferlen = 0;
	connData[i].readytosend = true;

	espconn_regist_sentcb(conn, serverSentCb);

}

void ICACHE_FLASH_ATTR serverInit(const char *name, ip_addr_t *ip, void *arg) {
	int result;
	int i;
	static esp_tcp tcp;
	if (ip==NULL) {
#ifdef Debug
		ets_uart_printf("Nslookup failed :/ Trying again...\n");
#endif
		//network_start();
	}

	for (i = 0; i < MAX_CONN; i++) {
		connData[i].conn = NULL;
		connData[i].txbuffer = txbuffer[i];
		connData[i].txbufferlen = 0;
		connData[i].readytosend = true;
	}

	serverConn.type=ESPCONN_TCP;
	serverConn.state=ESPCONN_NONE;
	serverConn.proto.tcp=&serverTcp;
	serverConn.proto.tcp->local_port=espconn_port();
	serverConn.proto.tcp->remote_port=8080;
	os_memcpy(serverConn.proto.tcp->remote_ip, &ip->addr, 4);
	espconn_regist_connectcb(&serverConn, serverConnectCb);
	espconn_regist_recvcb(&serverConn, serverRecvCb);
	espconn_regist_reconcb(&serverConn, serverReconCb);
	espconn_regist_disconcb(&serverConn, serverDisconCb);
	result = espconn_connect(&serverConn); // Start connection
#ifdef Debug
	ets_uart_printf("Resultado da tentativa de conexa: %d\n",result);
#endif
}

sint8 ICACHE_FLASH_ATTR network_start() {
	static ip_addr_t ip;
	sint8 result;
#ifdef Debug
	ets_uart_printf("Looking up server...\n");
#endif
	result = espconn_gethostbyname(&serverConn, "www.asgard.timeenergy.com.br", &ip, serverInit);
	return result;
}
