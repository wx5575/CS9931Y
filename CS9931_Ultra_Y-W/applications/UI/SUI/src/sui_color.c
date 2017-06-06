/*
 * File      : sui_color.c
 * This file is part of SUI
 * COPYRIGHT (C) 2014 - 2018, SSC
 *
 *  This program is sui_color.c
 *
 * Change Logs:
 * Date           Author       Notes
 * 2014-11-25     SSC          the first version
 */

#include "sui_color.h"

sui_uint16_t sui_color_xor(sui_uint16_t color)
{
// 	sui_uint16_t r,g,b;
// 	r = 0xf800-(color & 0xf800);
// 	g = 0x07e0-(color & 0x07e0);
// 	b = 0x001f-(color & 0x001f);
	return (color ^ 0xffff);
}
