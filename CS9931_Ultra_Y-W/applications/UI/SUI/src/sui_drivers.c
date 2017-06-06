/*
 * File      : sui_drivers.c
 * This file is part of SUI
 * COPYRIGHT (C) 2014 - 2018, SSC
 *
 *  This program is sui_drivers.c
 *
 * Change Logs:
 * Date           Author       Notes
 * 2014-11-25     SSC          the first version
 */
 
#include "suidef.h"

#include "RA8875.h"

void _sui_lcd_graph(sui_uint16_t x,sui_uint16_t y,sui_uint16_t h,sui_uint16_t w,sui_uint16_t *data)
{
	RA8875_DrawBMP(x,y,h,w,data);
}
