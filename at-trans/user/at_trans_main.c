#include "osapi.h"
#include "mem.h"
#include "at_custom.h"
#include "user_interface.h"
#include "gpio.h"
#include "driver/gpio16.h"

#include "at_trans_main.h"
#include "uart.h"

#define UART_RX_PARSE_CONFIG 1

uint8_t g_is_though = 1;

#if 0
at_leave_special_state();
void at_enter_special_state(void);

/**
  * @brief  get digit form at cmd line.the maybe alter pSrc
  * @param  p_src: at cmd line string
  *         result:the buffer to be placed result
  *         err : err num
  * @retval TRUE:
  *         FALSE:
  */
bool at_get_next_int_dec(char **p_src,int*result,int* err);
/**
  * @brief  get string form at cmd line.the maybe alter pSrc
  * @param  p_dest: the buffer to be placed result
  *         p_src: at cmd line string
  *         max_len :max len of string excepted to get
  * @retval None
  */
int32 at_data_str_copy(char *p_dest, char **p_src, int32 max_len);


void at_port_print(const char *str);

#endif

uint8_t is_first_flag = 0;
extern void ICACHE_FLASH_ATTR system_param_update(void);

void ICACHE_FLASH_ATTR init_cmd()
{
	wifi_set_opmode(STATION_MODE);
	uint8_t uart[32] = "AT+UART_DEF=4800,8,1,0,0\r\n";
	uint8_t net[55] = "AT+SAVETRANSLINK=1,\"116.62.50.90\",8899,\"TCP\"\r\n";
	at_fake_uart_rx(uart, os_strlen(uart));
	change_uart_reg();
	at_fake_uart_rx(net, os_strlen(net));
	is_first_flag = 0;
	g_is_though = 0;
	ATF_WAIT_TX_OVER();
	system_param_update();
}

void ICACHE_FLASH_ATTR change_uart_reg(void)
{
	//clear all interrupt
    CLEAR_PERI_REG_MASK(UART_INT_ENA(0), UART_RXFIFO_FULL_INT_ENA|UART_RXFIFO_TOUT_INT_ENA);
	WRITE_PERI_REG(UART_CONF1(0),
        ((100 & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S) |
        (0x05 & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S | UART_RX_TOUT_EN |
        ((0x10 & UART_TXFIFO_EMPTY_THRHD)<<UART_TXFIFO_EMPTY_THRHD_S));//wjl 
    //enable rx_interrupt
    SET_PERI_REG_MASK(UART_INT_ENA(0), UART_RXFIFO_FULL_INT_ENA|UART_RXFIFO_TOUT_INT_ENA);
}


/****************************wifi mode***********************************/
//AT+WMODE=STA
int ICACHE_FLASH_ATTR at_ret_wifi_mode(uint8_t id, char *pPara)
{

}

int ICACHE_FLASH_ATTR
at_query_wifi_mode(uint8_t id)
{
	char buf[5];
	uint8_t mode = wifi_get_opmode_default();
	if(SOFTAP_MODE == mode) {
		atf_ret_ok("=AP");
	} else {
		atf_ret_ok("=STA");
	}
	return 0;
}
    
int ICACHE_FLASH_ATTR
at_setup_wifi_mode(uint8_t id, char *pPara)
{
	char *sta = "STA";
	char *ap = "AP";
    uint8 buffer[32];
	os_memset(buffer, 0, 32);
    pPara++; // skip '='
    //get the second parameter
	if(0 == os_memcmp(pPara, sta, os_strlen(sta))) {
		wifi_set_opmode(STATION_MODE);
		atf_ret_ok();
	} else if (0 == os_memcmp(pPara, ap, os_strlen(ap))) {
		wifi_set_opmode(SOFTAP_MODE);
		atf_ret_ok();
	} else {
		atf_ret_err();
	}
	
	return 0;
}

/***************************************************************/


/****************************wifi mode***********************************/
//AT+WMODE=STA
int ICACHE_FLASH_ATTR atf_ret_uart(uint8_t id, char *pPara)
{
	return -1;
}

int ICACHE_FLASH_ATTR
atf_query_uart(uint8_t id)
{
	return -1;
}
    
int ICACHE_FLASH_ATTR
atf_setup_uart(uint8_t id, char *pPara)
{
	int result = 0, err = 0, flag = 0;
	int baudrate;
	uint8_t data_bits, stop_bits, pari, flow;
	uint8_t buffer[32];
	char parity[5];
	char flowctrl[4];
	os_memset(parity, 0, 5);
	os_memset(flowctrl, 0, 4);
	char *pStr = pPara;
    pStr++; // skip '='

    //get the first parameter
    // baudrate
    flag = at_get_next_int_dec(&pStr, &result, &err);
    // flag must be ture because there are more parameter
    if (flag == FALSE) {
        atf_ret_err();
        return ESP_FAIL;
    }
	baudrate = result;
    if (*pStr++ != ',') { // skip ','
        atf_ret_err();
        return ESP_FAIL;
    }

	// data_bits
    flag = at_get_next_int_dec(&pStr, &result, &err);
    // flag must be ture because there are more parameter
    if (flag == FALSE) {
        atf_ret_err();
        return ESP_FAIL;
    }
	data_bits = result;
    if (*pStr++ != ',') { // skip ','
        atf_ret_err();
        return ESP_FAIL;
    }

	// stop_bits
    flag = at_get_next_int_dec(&pStr, &result, &err);
    // flag must be ture because there are more parameter
    if (flag == FALSE) {
        atf_ret_err();
        return ESP_FAIL;
    }
	stop_bits = result;
    if (*pStr++ != ',') { // skip ','
        atf_ret_err();
        return ESP_FAIL;
    }

    //get the second parameter
    // string
    at_data_str_get(parity, &pStr, 5);
	switch(*parity) {
		case 'N': pari = 0; break;
		case 'E': pari = 2; break;
		case 'O': pari = 1; break;
		default:  pari = 0; break;
	}
	if (*pStr++ != ',') { // skip ','
        atf_ret_err();
        return ESP_FAIL;
    }
	//get the second parameter
    // string
    at_data_str_get(flowctrl, &pStr, 4);
	switch(*flowctrl) {
		case 'N': flow = 0; break;
		case 'F': flow = 3; break;
		default:  flow = 0; break;
	}
	
    if (*pStr != '\r') {
        atf_ret_err();
        return ESP_FAIL;
    }

	char * pRet = (char *)os_malloc(40);
	os_memset(pRet, 0, 40);
	if(pRet == NULL) {
		atf_ret_err();
        return ESP_FAIL;
	}
	os_sprintf(pRet, "AT+UART_DEF=%d,%d,%d,%d,%d\r\n",baudrate,data_bits,stop_bits,pari,flow);
	at_fake_uart_rx(pRet, os_strlen(pRet));
	os_free(pRet);
	atf_ret_ok();
	return (int)pRet;
}

/***************************************************************/

/****************************wifi mac***********************************/
//AT+WSMAC
int ICACHE_FLASH_ATTR 
atf_ret_mac(uint8_t id, char *pPara)
{
	return -1;
}

int ICACHE_FLASH_ATTR
atf_query_mac(uint8_t id)
{
    uint8 buffer[32];
	uint8_t mac[6];
	os_memset(buffer, 0, 32);
	wifi_get_macaddr(STATION_IF, mac);
	os_sprintf(buffer, "+ok=%02x%02x%02x%02x%02x%02x\r\n\r\n",MAC2STR(mac));
	at_port_print(buffer);
	return ESP_OK;
}
    
int ICACHE_FLASH_ATTR
atf_setup_mac(uint8_t id, char *pPara)
{
	return -1;
}
/***************************************************************/

/****************************wifi mac***********************************/
int ICACHE_FLASH_ATTR atf_query_link_status(uint8_t id)
{
    uint8 buffer[42];
	os_memset(buffer, 0, 42);
	struct station_config config;
	if(STATION_GOT_IP == wifi_station_get_connect_status()){
		wifi_station_get_config(&config);
		if(0 != config.ssid[0]) {
			os_sprintf(buffer, "+ok=%s\r\n\r\n",config.ssid);
			at_port_print(buffer);
		} else {
			if(config.bssid_set){
				os_sprintf(buffer, "+ok=%02x%02x%02x%02x%02x%02x\r\n\r\n",MAC2STR(config.bssid));
				at_port_print(buffer);
			} else {
				atf_ret_err();
			}
		}
	} else {
		at_port_print("+ok=Disconnected\r\n\r\n");
	}
	return ESP_OK;
}

/***************************************************************/

/****************************smartconfig***********************************/
//AT+QJOINE
int ICACHE_FLASH_ATTR 
atf_ret_qjoine(uint8_t id, char *pPara)
{
	return -1;
}

int ICACHE_FLASH_ATTR atf_query_qjoine(uint8_t id)
{
	char * pRet = (char *)os_malloc(25);
	os_memset(pRet, 0, 25);
	if(pRet == NULL) {
		atf_ret_err();
        return ESP_FAIL;
	}
	wifi_station_disconnect();
	os_sprintf(pRet, "AT+CWSTOPSMART\r\n");
	at_fake_uart_rx(pRet, os_strlen(pRet));
	
	os_sprintf(pRet, "AT+CWSTARTSMART=1\r\n");
	at_fake_uart_rx(pRet, os_strlen(pRet));
	atf_ret_ok();
	os_free(pRet);
	return 0;
}

/***************************************************************/

/****************************through***********************************/
static uint8_t is_enter_through_process = 0;
#define U0_SAVE_MAX 128
static uint8_t uart_save_buf[U0_SAVE_MAX] = {0};
void ICACHE_FLASH_ATTR atf_query_enter_through_wait(uint8_t *data, int length)
{
	char pRet[64];
	char esp_ok[2] = ">";
	char esp_err[6] = "ERROR";
	char* through_ar = "ALREADY CONNECT";
	char* through_ok = "CONNECT";
	char* through_err = "ERROR";
	char entm_str[15] = "AT+CIPSEND\r\n";
	
	if (1 == is_enter_through_process) {
		if (length == 1) {
			int len = os_strlen((char*)uart_save_buf);
			if (*data != '\r' && *data != '\n' && len < U0_SAVE_MAX) {
				uart_save_buf[len] = *data;
			}
		} else {
			os_memcpy(uart_save_buf, data, length);
		}
		if (0 == os_memcmp((char*)uart_save_buf, through_ok, os_strlen(through_ok))) {
			at_fake_uart_rx((uint8_t*)entm_str, os_strlen(entm_str));
			os_printf("\r\n********enter1**********\r\n");
			is_enter_through_process = 3;
		} else if (0 == os_memcmp((char*)uart_save_buf, through_ar, os_strlen(through_ar))) { 
			at_fake_uart_rx((uint8_t*)entm_str, os_strlen(entm_str));
			is_enter_through_process = 2;
		} else if (0 == os_memcmp((char*)uart_save_buf, through_err, os_strlen(through_err))) {
			atf_ret_err();
			is_enter_through_process = 0;
		} 
		os_memset(uart_save_buf, 0, U0_SAVE_MAX);
	}
	else if (2 == is_enter_through_process) {
		if (length == 1) {
			int len = os_strlen((char*)uart_save_buf);
			if (*data != '\r' && *data != '\n' && len < U0_SAVE_MAX) {
				uart_save_buf[len] = *data;
			}
		} else {
			os_memcpy(uart_save_buf, data, length);
		}
		if(NULL != os_strstr((char*)uart_save_buf, esp_err)){
			is_enter_through_process = 3;
			os_printf("\r\n********enter1**********\r\n");
		}
		os_memset(uart_save_buf, 0, U0_SAVE_MAX);
	}
	else if (3 == is_enter_through_process) {
		if (length == 1) {
			int len = os_strlen((char*)uart_save_buf);
			if (*data != '\r' && *data != '\n' && len < U0_SAVE_MAX) {
				uart_save_buf[len] = *data;
			}
		} else {
			os_memcpy(uart_save_buf, data, length);
		}
		if (NULL != os_strstr((char*)uart_save_buf, esp_ok)) {
			atf_ret_ok();
			g_is_though = 1;
			is_enter_through_process = 0;
			os_printf("\r\n********enter2**********\r\n");
		} else if(NULL != os_strstr((char*)uart_save_buf, esp_err)){
			atf_ret_err();
			is_enter_through_process = 0;
		}
		os_memset(uart_save_buf, 0, U0_SAVE_MAX);
	}
}

//AT+ENTM
int ICACHE_FLASH_ATTR atf_query_enter_through(uint8_t id)
{
	int ret = 0;
	if (STATION_GOT_IP != wifi_station_get_connect_status()){
		atf_ret_ok();
		return 0;
	}
	char * pRet = (char *)os_malloc(50);
	os_memset(pRet, 0, 50);
	if(pRet == NULL) {
		atf_ret_err();
        return ESP_FAIL;
	}
	
	os_sprintf(pRet, "AT+CIPMODE=1\r\n");
	at_fake_uart_rx(pRet, os_strlen(pRet));
	
	os_sprintf(pRet, "AT+CIPSTART=\"TCP\",\"116.62.50.90\",8899\r\n");
	at_fake_uart_rx(pRet, os_strlen(pRet));
	is_enter_through_process = 1;
	os_printf("[%d] process\r\n",__LINE__);
	os_free(pRet);
	return 0;
}

int ICACHE_FLASH_ATTR atf_ret_enter_through(uint8_t id, char *pPara)
{
	return 0;
}
/***************************************************************/

/****************************through***********************************/

int ICACHE_FLASH_ATTR atf_query_exit_through(uint8_t id)
{
	char * pRet = (char *)os_malloc(5);
	os_memset(pRet, 0, 5);
	if(pRet == NULL) {
		atf_ret_err();
        return ESP_FAIL;
	}
	at_port_print("a");
	os_sprintf(pRet, "+++");
	at_fake_uart_rx(pRet, os_strlen(pRet));
	os_free(pRet);
	g_is_though = 0;
	os_printf("\r\n********exit2**********\r\n");
	return 0;
}

/***************************************************************/
/****************************net info***********************************/
//AT+NETP=TCP,CLIENT,8899,116.62.50.90
int ICACHE_FLASH_ATTR at_ret_net_info(uint8_t id, char *pPara)
{
	return 0;
}
int ICACHE_FLASH_ATTR at_setup_net_info(uint8_t id, char *pPara)
{
	int result = 0, err = 0, flag = 0;
	uint8_t buffer[32];
	char prot[5];
	char type[8];
	char ip_addr[15];
	os_memset(prot, 0, 5);
	os_memset(type, 0, 8);
	os_memset(type, 0, 15);
	int port;
	char *pStr = pPara;
    pStr++; // skip '='

	//get the second parameter
    // string
    at_data_str_get(prot, &pStr, 5);
	if (*pStr++ != ',') { // skip ','
        atf_ret_err();
        return ESP_FAIL;
    }

	//get the second parameter
    // string
    at_data_str_get(type, &pStr, 8);
	if (*pStr++ != ',') { // skip ','
        atf_ret_err();
        return ESP_FAIL;
    }

    //get the first parameter
    // baudrate
    flag = at_get_next_int_dec(&pStr, &result, &err);
    // flag must be ture because there are more parameter
    if (flag == FALSE) {
        atf_ret_err();
        return ESP_FAIL;
    }
	port = result;
    if (*pStr++ != ',') { // skip ','
        atf_ret_err();
        return ESP_FAIL;
    }

	//get the second parameter
    // string
    at_data_str_get(ip_addr, &pStr, 15);
	
    if (*pStr != '\r') {
        atf_ret_err();
        return ESP_FAIL;
    }

	char * pRet = (char *)os_malloc(64);
	os_memset(pRet, 0, 64);
	if(pRet == NULL) {
		atf_ret_err();
        return ESP_FAIL;
	}
	os_sprintf(pRet, "AT+CIPSTART=\"%s\",\"%s\",%d\r\nAT+CIPMODE=1\r\n",prot,ip_addr,port);
	at_fake_uart_rx(pRet, os_strlen(pRet));
	os_free(pRet);
	atf_ret_ok();
	return 0;
}
/***************************************************************/

void ICACHE_FLASH_ATTR at_uart_tx_byte_filter_call(const uint8*data,uint32 length)
{
	int i, len = 0;;
	char* trans_ok = "+ok";
	char* trans_err = "+ERR";
	
	if(is_first_flag == 1) {
		os_printf("tx_complete init_cmd\r\n");
		init_cmd();
	}
	ets_printf("_");
	for(i=0; i<length; i++){
		ets_printf("%c.",data[i]);
	}
	
	if (g_is_though & STATION_GOT_IP == wifi_station_get_connect_status()) {
		while (len < length) {
			len += at_uart_tx_data((uint8*)data + len,length - len);
		}
		return;
	}

	atf_query_enter_through_wait((uint8_t *)data, length);

	if(length > 3){
		if(0 == os_memcmp((char*)data, trans_ok, 3) || 0 == os_memcmp((char*)data, trans_err, 4)){
			while (len < length) {
				len += at_uart_tx_data((uint8*)data + len,length - len);
			}
		}
	}
}

void ICACHE_FLASH_ATTR at_uart_rx_intr_call(uint8* data,int32 len)
{
#if UART_RX_PARSE_CONFIG
    int i,ret = 0;;
	uint8_t *retData = NULL;
	uint8_t thou[4] = "+++";
	uint8_t buf[32];
	
	os_memcpy(buf, data, (len>31)?31:len);
	buf[(len>31)?31:len] = 0;
	os_printf("[%d] %s\r\n",__LINE__, buf);
	
	if (g_is_though) {
		if (0 == os_memcmp(data, thou, 3)){
			at_fake_uart_rx(data, len);
			at_port_print("a");
			ATF_WAIT_TX_OVER();
			g_is_though = 0;
			os_printf("\r\n********exit1**********\r\n");
		} else {
			at_fake_uart_rx(data, len);
		}
	} else {
	    //add custom parse
	   	ret = atf_cmdProcess(data, retData);
		if (ret > 0) {
			retData = (char*)ret;
			os_printf("[%d] %s #\r\n", __LINE__, retData);
			at_fake_uart_rx(retData, os_strlen(retData));
			os_free(retData);
		} else if(ret == 0) {
			//return them self
		} else {
			os_printf("[%d] esp cmd\r\n", __LINE__);
			at_fake_uart_rx(data, len);
		}
	    //end
	}
#else
    at_fake_uart_rx(data, len);
#endif
}

void ICACHE_FLASH_ATTR at_uart_tx_complete_call(void)
{
	
}


