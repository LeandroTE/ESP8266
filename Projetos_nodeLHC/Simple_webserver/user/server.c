#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

#include "server.h"
#include "config.h"

#define print_debug

static struct espconn serverConn_telnet;
static esp_tcp serverTcp_telnet;

static struct espconn serverConn_http;
static esp_tcp serverTcp_http;

//Connection pool
static char txbuffer[MAX_CONN][MAX_TXBUFFER];
serverConnData connData_telnet[MAX_CONN];
serverConnData connData_http[MAX_CONN];


const char html_page[] = "HTTP/1.0 200 OK\r\n\r\n<!DOCTYPE html>\r\n<html>\r\n<body>\r\n<form>\r\n  Nome Rede Wifi:<br>\r\n  <input type=\"text\" name=\"SSID\">\r\n  <br>\r\n  Senha:<br>\r\n  <input type=\"text\" name=\"senha\"><br><br>\r\n  <input type=\"submit\">\r\n</form>\r\n</body>\r\n</html>\r\n";

static serverConnData ICACHE_FLASH_ATTR *serverFindConnData_telnet(void *arg) {
	int i;
	for (i=0; i<MAX_CONN; i++) {
		if (connData_telnet[i].conn==(struct espconn *)arg)
			return &connData_telnet[i];
	}
	ets_uart_printf("FindConnData_telnet");
	return NULL; //WtF?
}

static serverConnData ICACHE_FLASH_ATTR *serverFindConnData_http(void *arg) {
	int i;
	for (i=0; i<MAX_CONN; i++) {
		if (connData_http[i].conn==(struct espconn *)arg)
			return &connData_http[i];
	}
	ets_uart_printf("FindConnData_http");
	return NULL;
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
		if (result != ESPCONN_OK)
			ets_uart_printf("sendtxbuffer: espconn_sent error %d on conn %p\n", result, conn);
	}
	return result;
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
#ifdef print_debug
		ets_uart_printf("espbuffsent: txbuffer full on conn %p\n", conn);
#endif
		return -128;
	}
	os_memcpy(conn->txbuffer + conn->txbufferlen, data, len);
	conn->txbufferlen += len;
	if (conn->readytosend) 
		return sendtxbuffer(conn);
	return ESPCONN_OK;
}


//------------------Callback Telnet---------------------------------------------------
//callback after the data are sent
static void ICACHE_FLASH_ATTR serverSentCb_telnet(void *arg) {
	serverConnData *conn = serverFindConnData_telnet(arg);
	if (conn==NULL) return;
	conn->readytosend = true;
	sendtxbuffer(conn); // send possible new data in txbuffer
}

static void ICACHE_FLASH_ATTR serverRecvCb_telnet(void *arg, char *data, unsigned short len) {
	int x;
	char *p, *e;
	serverConnData *conn = serverFindConnData_telnet(arg);
	if (conn == NULL) return;

	uart0_tx_buffer(data, len);
}

static void ICACHE_FLASH_ATTR serverReconCb_telnet(void *arg, sint8 err) {
	serverConnData *conn=serverFindConnData_telnet(arg);
	if (conn==NULL) return;
	//Yeah... No idea what to do here. ToDo: figure something out.
}

static void ICACHE_FLASH_ATTR serverDisconCb_telnet(void *arg) {
	//Just look at all the sockets and kill the slot if needed.
	int i;
#ifdef print_debug
	ets_uart_printf("Telnet:Disconnectado TCP\n");
#endif
	for (i=0; i<MAX_CONN; i++) {
		if (connData_telnet[i].conn!=NULL) {
			if (connData_telnet[i].conn->state==ESPCONN_NONE || connData_telnet[i].conn->state==ESPCONN_CLOSE) {
				connData_telnet[i].conn=NULL;
			}
		}
	}
}

static void ICACHE_FLASH_ATTR serverConnectCb_telnet(void *arg) {
	struct espconn *conn = arg;
	int i;
#ifdef print_debug
	ets_uart_printf("Telnet: Connectado TCP\n");
#endif
	for (i=0; i<MAX_CONN; i++) if (connData_telnet[i].conn==NULL) break;

	if (i==MAX_CONN) {
		espconn_disconnect(conn);
		return;
	}
	connData_telnet[i].conn=conn;
	connData_telnet[i].txbufferlen = 0;
	connData_telnet[i].readytosend = true;

	espconn_regist_recvcb(conn, serverRecvCb_telnet);
	espconn_regist_reconcb(conn, serverReconCb_telnet);
	espconn_regist_disconcb(conn, serverDisconCb_telnet);
	espconn_regist_sentcb(conn, serverSentCb_telnet);
}
//------------------Callback HTTP---------------------------------------------------
static void ICACHE_FLASH_ATTR serverSentCb_http(void *arg) {
	serverConnData *conn = serverFindConnData_http(arg);
#ifdef print_debug
	ets_uart_printf("Http: Sent Data\n");
#endif
	if (conn==NULL) return;
	conn->readytosend = true;
	sendtxbuffer(conn); // send possible new data in txbuffer
}

static void ICACHE_FLASH_ATTR serverRecvCb_http(void *arg, char *data, unsigned short len) {
	uint8_t i;
#ifdef print_debug
	ets_uart_printf("Http: Receive data\n");
#endif
	for (i = 0; i < MAX_CONN; ++i)if (connData_http[i].conn)
	espbuffsentstring(&connData_http[i],html_page);

	uart0_tx_buffer(html_page, sizeof(html_page));
}

static void ICACHE_FLASH_ATTR serverReconCb_http(void *arg, sint8 err) {
	serverConnData *conn=serverFindConnData_http(arg);

#ifdef print_debug
	ets_uart_printf("Http: serverReconCb_http\n");
#endif
	if (conn==NULL) return;
	//Yeah... No idea what to do here. ToDo: figure something out.
}

static void ICACHE_FLASH_ATTR serverDisconCb_http(void *arg) {
	//Just look at all the sockets and kill the slot if needed.
	int i;
#ifdef print_debug
	ets_uart_printf("Http: Disconnectado TCP\n");
#endif
	for (i=0; i<MAX_CONN; i++) {
		if (connData_http[i].conn!=NULL) {
			if (connData_http[i].conn->state==ESPCONN_NONE || connData_http[i].conn->state==ESPCONN_CLOSE) {
				connData_http[i].conn=NULL;
			}
		}
	}
}

static void ICACHE_FLASH_ATTR serverConnectCb_http(void *arg) {
	struct espconn *conn = arg;
	int i;
#ifdef print_debug
	ets_uart_printf("Http: Connectado TCP\n");
#endif
	for (i=0; i<MAX_CONN; i++) if (connData_http[i].conn==NULL) break;

	if (i==MAX_CONN) {
		espconn_disconnect(conn);
		return;
	}
	connData_http[i].conn=conn;
	connData_http[i].txbufferlen = 0;
	connData_http[i].readytosend = true;

	espconn_regist_recvcb(conn, serverRecvCb_http);
	espconn_regist_reconcb(conn, serverReconCb_http);
	espconn_regist_disconcb(conn, serverDisconCb_http);
	espconn_regist_sentcb(conn, serverSentCb_http);
}


//-------------------servers initialization-----------------------------------------
void ICACHE_FLASH_ATTR serverInit_telnet() {
	int i;
#ifdef print_debug
	ets_uart_printf("Telnet: ServerInit\n");
#endif
	for (i = 0; i < MAX_CONN; i++) {
		connData_telnet[i].conn = NULL;
		connData_telnet[i].txbuffer = txbuffer[i];
		connData_telnet[i].txbufferlen = 0;
		connData_telnet[i].readytosend = true;
	}
	serverConn_telnet.type=ESPCONN_TCP;
	serverConn_telnet.state=ESPCONN_NONE;
	serverTcp_telnet.local_port=23;
	serverConn_telnet.proto.tcp=&serverTcp_telnet;

	espconn_regist_connectcb(&serverConn_telnet, serverConnectCb_telnet);
	espconn_accept(&serverConn_telnet);
	espconn_regist_time(&serverConn_telnet, SERVER_TIMEOUT, 0);
}

void ICACHE_FLASH_ATTR serverInit_http() {
	int i;
#ifdef print_debug
	ets_uart_printf("Http: ServerInit\n");
#endif
	for (i = 0; i < MAX_CONN; i++) {
		connData_http[i].conn = NULL;
		connData_http[i].txbuffer = txbuffer[i];
		connData_http[i].txbufferlen = 0;
		connData_http[i].readytosend = true;
	}
	serverConn_http.type=ESPCONN_TCP;
	serverConn_http.state=ESPCONN_NONE;
	serverTcp_http.local_port=80;
	serverConn_http.proto.tcp=&serverTcp_http;

	espconn_regist_connectcb(&serverConn_http, serverConnectCb_http);
	espconn_accept(&serverConn_http);
	espconn_regist_time(&serverConn_http, SERVER_TIMEOUT + 15, 0);
}
