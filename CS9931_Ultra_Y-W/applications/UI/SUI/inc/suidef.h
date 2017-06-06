/*
 * File      : suidef.h
 * This file is part of SUI
 * COPYRIGHT (C) 2014 - 2018, SSC
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2014-11-25     SSC          the first version
 */

/* SUI basic data type definitions */
typedef signed   char                   sui_int8_t;      /**<  8bit integer type */
typedef signed   short                  sui_int16_t;     /**< 16bit integer type */
typedef signed   long                   sui_int32_t;     /**< 32bit integer type */
typedef unsigned char                   sui_uint8_t;     /**<  8bit unsigned integer type */
typedef unsigned short                  sui_uint16_t;    /**< 16bit unsigned integer type */
typedef unsigned long                   sui_uint32_t;    /**< 32bit unsigned integer type */
typedef int                             sui_bool_t;      /**< boolean type */

/* boolean type definitions */
#define SUI_TRUE                         1               /**< boolean true  */
#define SUI_FALSE                        0               /**< boolean fails */

/* struct type definitions */
struct sui_rect
{
	sui_uint16_t x1;
	sui_uint16_t y1;
	
	sui_uint16_t x2;
	sui_uint16_t y2;
};


