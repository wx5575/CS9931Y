/*
 * File      : sui_window.h
 * This file is part of SUI
 * COPYRIGHT (C) 2014 - 2018, SSC
 *
 *  This program is window.h
 *
 * Change Logs:
 * Date           Author       Notes
 * 2014-11-25     SSC          the first version
 */

#include "suidef.h"

struct sui_panel
{
	sui_uint16_t	*data;
	struct sui_rect	*rect;
};

typedef struct sui_panel sui_panel_t;


struct panel_type *sui_window_create(const char *name, struct rect_type *rect);
void sui_window_update(struct panel_type *panel);
void sui_window_xor(struct panel_type *panel,struct rect_type *rect);
