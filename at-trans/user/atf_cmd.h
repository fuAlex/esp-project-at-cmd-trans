/*
 * File	: at_cmd.h
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
#ifndef __AT_CMD_H
#define __AT_CMD_H

#include "at_custom.h"

#define AT_FAKE_NUM 8

typedef struct
{
  char *atf_cmdName;
  int atf_cmdLen;
  int (*atf_queryCmd)(uint8_t id);
  int (*atf_setupCmd)(uint8_t id, char *pPara);
  int (*atf_retCmd)(uint8_t id, char *pPara);
  char *atf_transCmd;
  int atf_transLen;
}atf_funcationType;


extern atf_funcationType atf_cmd[AT_FAKE_NUM];
int atf_cmdProcess(uint8_t *pAtRcvData, char *pAtRetCmd);
int at_data_str_get(char *p_dest, char **p_src, int32 max_len);

#endif
