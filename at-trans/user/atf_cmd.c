/*
 * File	: at_cmd.c
 * This file is part of Espressif's AT+ command set program.
 * Copyright (C) 2013 - 2016, Espressif Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "atf_cmd.h"
#include "user_interface.h"
#include "osapi.h"
#include <stdlib.h>
#include "at_trans_main.h"


/** @defgroup AT_BASECMD_Functions
  * @{
  */ 

atf_funcationType atf_cmd[AT_FAKE_NUM] = {
	{"AT+WMODE", 8, at_query_wifi_mode, at_setup_wifi_mode, at_ret_wifi_mode, NULL, 0},
	{"AT+UART", 7, NULL, atf_setup_uart, atf_ret_uart, NULL, 0},
	{"AT+WSMAC", 8, atf_query_mac, atf_setup_mac, NULL, NULL, 0},
	{"AT+WSLK", 7, atf_query_link_status, NULL, NULL, NULL, 0},
	{"AT+QJOINE", 9, atf_query_qjoine, NULL, atf_ret_qjoine, NULL, 0},
	{"AT+ENTM", 7, atf_query_enter_through, NULL, atf_ret_enter_through, NULL, 0},
	{"AT+NETP", 7, NULL, at_setup_net_info, at_ret_net_info, NULL, 0},
	{"+++", 3, atf_query_exit_through, NULL, NULL, NULL, 0},
};

/**
  * @brief  get string form at cmd line.the maybe alter pSrc
  * @param  p_dest: the buffer to be placed result
  *         p_src: at cmd line string
  *         max_len :max len of string excepted to get
  * @retval None
  */
int ICACHE_FLASH_ATTR at_data_str_get(char *p_dest, char **p_src, int32 max_len)
{
	int i = 0;
	for (i=0; i<max_len; i++){
		if('\r' == *((*p_src)+i) || ',' == *((*p_src)+i)) {
			os_memcpy(p_dest, *p_src, i);
			*p_src += i;
			return i;
		}
	}
	return -1;
}

/**
  * @brief  Query and localization one commad.
  * @param  cmdLen: received length of command
  * @param  pCmd: point to received command 
  * @retval the id of command
  *   @arg -1: failure
  */
static int16_t ICACHE_FLASH_ATTR
at_cmdSearch(int8_t cmdLen, uint8_t *pCmd)
{
  int16_t i;

  if(cmdLen == 0)
  {
    return 0;
  }
  else if(cmdLen > 0)
  {
    for(i=0; i<AT_FAKE_NUM; i++)
    {
      //os_printf("%d len %d\r\n", cmdLen, atf_cmd[i].atf_cmdLen);
      if(cmdLen == atf_cmd[i].atf_cmdLen)
      {
        //os_printf("%s cmp %s\r\n", pCmd, atf_cmd[i].atf_cmdName);
        if(os_memcmp(pCmd, atf_cmd[i].atf_cmdName, cmdLen) == 0) //think add cmp len first
        {
          return i;
        }
      }
    }
  }
  return -1;
}

/**
  * @brief  Get the length of commad.
  * @param  pCmd: point to received command 
  * @retval the length of command
  *   @arg -1: failure
  */
static int8_t ICACHE_FLASH_ATTR
at_getCmdLen(uint8_t *pCmd)
{
  uint8_t n,i;

  n = 0;
  i = 128;

  while(i--)
  {
    if((*pCmd == '\r') || (*pCmd == '=') || (*pCmd == '?') || ((*pCmd >= '0')&&(*pCmd <= '9')))
    {
      return n;
    }
    else
    {
      pCmd++;
      n++;
    }
  }
  return -1;
}

static uint8_t* ICACHE_FLASH_ATTR
at_getCmdHart(uint8_t *pCmd)
{
  uint8_t n,i;

  n = 0;
  i = 128;

  while(i--)
  {
    if ((*pCmd == 'A') && (*(pCmd+1) == 'T') )
    {
      return pCmd;
    }
    else
    {
      pCmd++;
      n++;
    }
  }
  return NULL;
}
/**
  * @brief  Distinguish commad and to execution.
  * @param  pAtRcvData: point to received (command) 
  * @retval ret cmd lenth
  */
int ICACHE_FLASH_ATTR
atf_cmdProcess(uint8_t *pAtRcvData, char *pAtRetCmd)
{
  int16_t cmdId;
  int8_t cmdLen = -1;
  uint16_t i;
  int ret = ESP_FAIL;
  uint8_t * pData = at_getCmdHart(pAtRcvData);
  if(pData != NULL) {
  	cmdLen = at_getCmdLen(pData);
  }
  if(cmdLen != -1)
  {
    cmdId = at_cmdSearch(cmdLen, pData);
  }
  else 
  {
  	cmdId = -1;
  }
  if(cmdId != -1)
  {
    pData += cmdLen;
    if(*pData == '\r')
    {
      if(atf_cmd[cmdId].atf_queryCmd)
      {
        ret = atf_cmd[cmdId].atf_queryCmd(cmdId);
      }
    }
    else if((*pData >= '0') && (*pData <= '9') || (*pData == '='))
    {
      if(atf_cmd[cmdId].atf_setupCmd)
      {
        ret = atf_cmd[cmdId].atf_setupCmd(cmdId, pData);
      }
    }
  }
  if(ret > 0) {
  	pAtRetCmd = (char*)ret;
  }
  return ret;
}

/**
  * @}
  */
