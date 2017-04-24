/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#if 0
#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"

// test :AT+TEST=1,"abc"<,3>
void ICACHE_FLASH_ATTR
at_setupCmdTest(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    uint8 buffer[32] = {0};
    pPara++; // skip '='

    //get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    if (flag == FALSE) {
        at_response_error();
        return;
    }

    if (*pPara++ != ',') { // skip ','
        at_response_error();
        return;
    }

    os_sprintf(buffer, "the first parameter:%d\r\n", result);
    at_port_print(buffer);

    //get the second parameter
    // string
    at_data_str_copy(buffer, &pPara, 10);
    at_port_print("the second parameter:");
    at_port_print(buffer);
    at_port_print("\r\n");

    if (*pPara == ',') {
        pPara++; // skip ','
        result = 0;
        //there is the third parameter
        // digit
        flag = at_get_next_int_dec(&pPara, &result, &err);
        // we donot care of flag
        os_sprintf(buffer, "the third parameter:%d\r\n", result);
        at_port_print(buffer);
    }

    if (*pPara != '\r') {
        at_response_error();
        return;
    }

    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_testCmdTest(uint8_t id)
{
    uint8 buffer[32] = {0};

    os_sprintf(buffer, "%s\r\n", "at_testCmdTest");
    at_port_print(buffer);
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_queryCmdTest(uint8_t id)
{
    uint8 buffer[32] = {0};

    os_sprintf(buffer, "%s\r\n", "at_queryCmdTest");
    at_port_print(buffer);
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_exeCmdTest(uint8_t id)
{
    uint8 buffer[32] = {0};

    os_sprintf(buffer, "%s\r\n", "at_exeCmdTest");
    at_port_print(buffer);
    at_response_ok();
}

extern void at_exeCmdCiupdate(uint8_t id);
at_funcationType at_custom_cmd[] = {
    {"+TEST", 5, at_testCmdTest, at_queryCmdTest, at_setupCmdTest, at_exeCmdTest},
#ifdef AT_UPGRADE_SUPPORT
    {"+CIUPDATE", 9,       NULL,            NULL,            NULL, at_exeCmdCiupdate}
#endif
};



void ICACHE_FLASH_ATTR
user_init(void)
{
    char buf[64] = {0};
    at_customLinkMax = 5;
    at_init();
    os_sprintf(buf,"compile time:%s %s",__DATE__,__TIME__);
    at_set_custom_info(buf);
    at_port_print("\r\nready\r\n");
    at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_custom_cmd[0]));
}
#endif

#include "osapi.h"
#include "mem.h"
#include "at_custom.h"
#include "user_interface.h"
#include "gpio.h"
#include "driver/gpio16.h"

#include "at_trans_main.h"
#include "spi_flash.h"
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}
#if 0
#define EINT_REGEN_MIN_TIME 80 //80ms
bool g_eint_generented = false;
static os_timer_t ptimer;
static void ICACHE_FLASH_ATTR gpio_revise_timer_cb(void *timer_arg)
{
	if(g_eint_generented)
	{
		GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 0);
		g_eint_generented = false;
	}
}

static void ICACHE_FLASH_ATTR wakeup_external_host(void)
{	
	if( g_eint_generented == false)
	{
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);
		GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 1);
		os_timer_disarm(&ptimer);
		os_timer_setfn(&ptimer,gpio_revise_timer_cb,NULL);
		os_timer_arm(&ptimer,EINT_REGEN_MIN_TIME,false);
		
		g_eint_generented = true;
	}
}

static void ICACHE_FLASH_ATTR at_exeCmd_STLightSleep(uint8_t id)
{
	at_response_ok();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(5));
	gpio_pin_wakeup_enable(PERIPHS_IO_MUX_GPIO5_U,GPIO_PIN_INTR_HILEVEL);

	wifi_set_sleep_type(LIGHT_SLEEP_T);
}

struct rsp_type {
	uint8* data;
	uint32 head;
	uint32 size;
	struct rsp_type* next;
};
#define AT_LINK_MAX  5
static int32 current_link = -1;
static int8 flag = 0;
static struct rsp_type* rsp[AT_LINK_MAX + 1];
static void ICACHE_FLASH_ATTR at_fake_uart_tx_test1_func(const uint8*data,uint32 length)
{
	uint8* p = NULL;
	int32 link_id = 0;
	int32 data_len;
	struct rsp_type* rsp_ptr = NULL;
	if (os_memcmp(data,"\r\n+IPD,",os_strlen("\r\n+IPD,")) == 0) {
		p = (uint8*)data + os_strlen("\r\n+IPD,");
		link_id = atoi(p);
		p++;
		if (*p == ',') {
			p++;
			data_len = atoi(p);
		} else {
			data_len = link_id;
			link_id = 0;
		}
		if ((link_id >= 0) && (link_id < AT_LINK_MAX)) {
			p = os_strchr(p,':');
			if (p) {
				p++;
				if (length == (p - data + data_len)) {
					if (rsp[link_id] == NULL) {
						rsp[link_id] = (struct rsp_type*)os_malloc(length + sizeof(struct rsp_type));
						if (rsp[link_id]) {
							uint8 tmp[32];
							os_memset(rsp[link_id],0x0,length + sizeof(struct rsp_type));
							rsp[link_id]->next = NULL;
							rsp[link_id]->data = (uint8*)(rsp[link_id] + 1);
							rsp[link_id]->size = length;
							rsp[link_id]->head = 0;
							os_memcpy(rsp[link_id]->data,data,length);
							set_tcp_block(link_id);
							
							os_sprintf(tmp,"+IPDATAIND:%d\r\n",link_id);
							if (flag == 0) {
								at_port_print(tmp);
							} else {
								length = os_strlen(tmp);
								if (rsp[AT_LINK_MAX]) {
									for(rsp_ptr = rsp[AT_LINK_MAX];rsp_ptr->next;rsp_ptr = rsp_ptr->next){
									}
									rsp_ptr->next = (struct rsp_type*)os_malloc(length + sizeof(struct rsp_type));
									if (rsp_ptr->next) {
										rsp_ptr = rsp_ptr->next;
									}
								} else {
									rsp[AT_LINK_MAX] = (struct rsp_type*)os_malloc(length + sizeof(struct rsp_type));
									rsp_ptr = rsp[AT_LINK_MAX];
								}
								
								if (rsp_ptr) {
									os_memset(rsp_ptr,0x0,length + sizeof(struct rsp_type));
									rsp_ptr->next = NULL;
									rsp_ptr->data = (uint8*)(rsp_ptr + 1);
									rsp_ptr->size = length;
									rsp_ptr->head = 0;
									os_memcpy(rsp_ptr->data,tmp,length);
								}
							}
						}
					} else {
						for(rsp_ptr = rsp[link_id];rsp_ptr->next;rsp_ptr = rsp_ptr->next){
							
						}
						rsp_ptr->next = (struct rsp_type*)os_malloc(length + sizeof(struct rsp_type));
						if (rsp_ptr->next) {
							rsp_ptr = rsp_ptr->next;
							os_memset(rsp_ptr,0x0,length + sizeof(struct rsp_type));
							rsp_ptr->next = NULL;
							rsp_ptr->data = (uint8*)(rsp_ptr + 1);
							rsp_ptr->size = length;
							rsp_ptr->head = 0;
							os_memcpy(rsp_ptr->data,data,length);
						}
					}
				}
			}
		}
	} else {
		if (flag == 0) 
		{
			int len = 0;
			current_link = -1;
			while (len < length) {
				wakeup_external_host();
				len += at_uart_tx_data((uint8*)data + len,length - len);
			}
		}
		else
		{
			if (rsp[AT_LINK_MAX]) {
				for(rsp_ptr = rsp[AT_LINK_MAX];rsp_ptr->next;rsp_ptr = rsp_ptr->next){
				}
				rsp_ptr->next = (struct rsp_type*)os_malloc(length + sizeof(struct rsp_type));
				if (rsp_ptr->next) {
					rsp_ptr = rsp_ptr->next;
				}
			} else {
				rsp[AT_LINK_MAX] = (struct rsp_type*)os_malloc(length + sizeof(struct rsp_type));
				rsp_ptr = rsp[AT_LINK_MAX];
			}
			
			if (rsp_ptr) {
				os_memset(rsp_ptr,0x0,length + sizeof(struct rsp_type));
				rsp_ptr->next = NULL;
				rsp_ptr->data = (uint8*)(rsp_ptr + 1);
				rsp_ptr->size = length;
				rsp_ptr->head = 0;
				os_memcpy(rsp_ptr->data,data,length);
			}
		}
	}
}
static bool g_at_fake_at_fake_uart_tx_test1_func_start = false;
static bool g_at_fake_need_close = false;
static bool g_at_fake_status = FALSE; 
static void ICACHE_FLASH_ATTR at_uart_tx_filter_func(const uint8*data,uint32 length)
{
//    os_printf("tx_filter_func,len:%d\r\n",length);
	wakeup_external_host();
	if(g_at_fake_at_fake_uart_tx_test1_func_start)
	{
		at_fake_uart_tx_test1_func(data,length);
	}
	else
	{
		int len = 0;
		while (len < length) {
			wakeup_external_host();
			len += at_uart_tx_data((uint8*)data + len,length - len);
		}
	}
}

static void ICACHE_FLASH_ATTR at_custom_uart_tx_complete_cb(void)
{
//    os_printf("at_custom_uart_tx_complete_cb\r\n");
	if(g_at_fake_at_fake_uart_tx_test1_func_start)
	{	
		int len = 0;
		if (current_link == -1) {
			return;
		}
		
		wakeup_external_host();
		len = at_uart_tx_data(rsp[current_link]->data + rsp[current_link]->head,rsp[current_link]->size - rsp[current_link]->head);
		rsp[current_link]->head += len;
		if (rsp[current_link]->head == rsp[current_link]->size) {
			struct rsp_type* p = rsp[current_link];
			rsp[current_link] = rsp[current_link]->next;
			os_free(p);
		}	
		
		if (rsp[current_link] == NULL) {
			at_leave_special_state();
			clr_tcp_block(current_link);
			current_link = -1;
			at_response_ok();
			flag = 0;
			
			while (rsp[AT_LINK_MAX]) {
				wakeup_external_host();
				len = at_uart_tx_data(rsp[AT_LINK_MAX]->data + rsp[AT_LINK_MAX]->head,rsp[AT_LINK_MAX]->size - rsp[AT_LINK_MAX]->head);
				rsp[AT_LINK_MAX]->head += len;
				if (rsp[AT_LINK_MAX]->head == rsp[AT_LINK_MAX]->size) {
					struct rsp_type* p = rsp[AT_LINK_MAX];
					rsp[AT_LINK_MAX] = rsp[AT_LINK_MAX]->next;
					os_free(p);
				}	
			}
		}
		
		if  (g_at_fake_status == false) {
			uint32 loop = 0;
			for(loop = 0;loop <  AT_LINK_MAX + 1;loop++) {
				if (rsp[loop]) {
					break;
				}
			}

			if (loop ==  AT_LINK_MAX + 1) {
				g_at_fake_at_fake_uart_tx_test1_func_start = false;
			}
		}
	}
}

static void ICACHE_FLASH_ATTR at_custom_uart_rx_intr_cb(uint8* data,int32 len)
{
    char *buf = (char*)os_malloc(len);
    os_memcpy(buf, (char *)data, len);
    os_printf("rx:%s#\r\n", buf);
	at_fake_uart_rx(data, len);
}

static void ICACHE_FLASH_ATTR at_setupCmdFake(uint8_t id, char *pPara)
{
    int result = 0, err = 0;
    pPara++; // skip '='
    //get the first parameter
    // digit
    at_get_next_int_dec(&pPara, &result, &err);


    if ((result != 0) && (result != 1)) { // skip ','
        at_response_error();
        return;
    }

    if (*pPara != '\r') {
        at_response_error();
        return;
    }
	at_response_ok();

	if (result == 1) {
		g_at_fake_at_fake_uart_tx_test1_func_start = true;
		g_at_fake_status = true;
	} else {
		g_at_fake_status = false;
		uint32 loop = 0;
		for(loop = 0;loop <  AT_LINK_MAX + 1;loop++) {
			if (rsp[loop]) {
				break;
			}
		}

		if (loop ==  AT_LINK_MAX + 1) {
			g_at_fake_at_fake_uart_tx_test1_func_start = false;
		}
	}
}

static  void ICACHE_FLASH_ATTR at_setupCmdGetData(uint8_t id, char *pPara)
{
    int result = 0, err = 0;
	int len = 0;
    pPara++; // skip '='

    //get the first parameter
    // digit
    at_get_next_int_dec(&pPara, &result, &err);

    if (*pPara != '\r') {
        at_response_error();
        return;
    }

	if ((result >= 0) && (result < AT_LINK_MAX)) {
		if (rsp[result] == NULL) {
			at_response_ok();
		} else {
			at_leave_special_state();
			current_link = result;
			wakeup_external_host();
			len = at_uart_tx_data(rsp[current_link]->data + rsp[current_link]->head,rsp[current_link]->size - rsp[current_link]->head);
			rsp[current_link]->head += len;
			if (rsp[current_link]->head == rsp[current_link]->size) {
				struct rsp_type* p = rsp[current_link];
				rsp[current_link] = rsp[current_link]->next;
				os_free(p);
			}	
			
			if (rsp[current_link] == NULL) {
				at_leave_special_state();
				clr_tcp_block(current_link);
				current_link = -1;
				at_response_ok();
			} else {
				flag = 1;
			}
		}
	} else {
		at_response_ok();
	}
}
/********************************************************
*/
/*********************************************************/

extern void at_exeCmdCiupdate(uint8_t id);
static at_funcationType at_custom_cmd[] = {
	{"+FAKE", 5, NULL, NULL, at_setupCmdFake, NULL},
	{"+IPD", 4, NULL, NULL, at_setupCmdGetData, NULL},
	{"+STLIGHTSLEEP", 13, NULL, NULL,NULL , at_exeCmd_STLightSleep},
#ifdef AT_UPGRADE_SUPPORT
	{"+CIUPDATE", 9,       NULL,            NULL,            NULL, at_exeCmdCiupdate}
#endif
};
#endif
void user_rf_pre_init(void)
{
    
}

#define MAGIC_NUM 0X23455FC3
#define MAGIC_SEC 0XFB
typedef struct {
	uint32_t is_open;
} save_para_t;

void ICACHE_FLASH_ATTR system_param_update(void)
{
	save_para_t flag; 
	flag.is_open = MAGIC_NUM;
	spi_flash_erase_sector(MAGIC_SEC);
    spi_flash_write((MAGIC_SEC) * SPI_FLASH_SEC_SIZE, (uint32 *)&flag, sizeof(save_para_t));
}

void ICACHE_FLASH_ATTR system_init_down_call(void)
{
	save_para_t flag; 
	spi_flash_read((MAGIC_SEC) * SPI_FLASH_SEC_SIZE,(uint32 *)&flag, sizeof(save_para_t));
	if (flag.is_open != MAGIC_NUM) {
		is_first_flag = 1;
	}
}

extern void at_exeCmdCiupdate(uint8_t id);
at_funcationType at_custom_cmd[] = {
    {"+TEST", 5, NULL, NULL, NULL, NULL},
#ifdef AT_UPGRADE_SUPPORT
    {"+CIUPDATE", 9,       NULL,            NULL,            NULL, at_exeCmdCiupdate}
#endif
};

void user_init(void)
{
    char buf[64];
	os_memset(buf, 0, 64);
    at_customLinkMax = 5;
	ATF_WAIT_TX_CLEAR();
    at_init();
	change_uart_reg();
	system_init_down_call();
//    os_printf("\r\nescape:%s\r\n", at_set_escape_character(0x0d)?"OK":"FAIL");
    os_sprintf(buf,"compile time:%s %s",__DATE__,__TIME__);
    at_set_custom_info(buf);
    /* enable fake and regiest uart tx */
    //at_fake_uart_enable(TRUE, at_uart_tx_filter_func);
//    at_register_uart_rx_intr(at_custom_uart_rx_intr_cb);
//    at_register_uart_tx_complete_func(at_custom_uart_tx_complete_cb);
/////////////////////////////////////////////
    at_fake_uart_enable(TRUE, at_uart_tx_byte_filter_call);
    at_register_uart_rx_intr(at_uart_rx_intr_call);
    at_register_uart_tx_complete_func(at_uart_tx_complete_call);
////////////////////////////////////////
    at_port_print("\r\nready\r\n");
	at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_custom_cmd[0]));
}

