//------------------------------------------------------------------------------
//
// 2022.03.23 framebuffer graphic library(chalres-park)
//
//------------------------------------------------------------------------------
/*
  Simple framebuffer testing program
  Set the screen to a given color. For use in developing Linux
  display drivers.
  Copyright (c) 2014, Jumpnow Technologies, LLC
  All rights reserved.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  Ideas taken from the Yocto Project psplash program.
 */

//------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <getopt.h>

#include "typedefs.h"
#include "fblib/fblib.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define	ITEM_COUNT_MAX	64
#define	ITEM_STR_MAX	64

typedef struct background_item__t {
	int				id, x, y, w, h, lw;
	fb_color_u		c, lc;
}	b_item_t;

typedef struct string_item__t {
	int				id, x_off, y_off, scale;
	fb_color_u		c;
	char			str[ITEM_STR_MAX];
}	s_item_t;

typedef struct ui_group__t {
	int				cnt;
	b_item_t		b_item[ITEM_COUNT_MAX];
	s_item_t		s_item[ITEM_COUNT_MAX];
}	ui_grp_t;

//------------------------------------------------------------------------------
ui_grp_t *ui_init (const char *cfg_filename, fb_info_t *f_info)
{
	ui_grp_t	*ui_grp = (ui_grp_t *)malloc(sizeof(ui_grp_t));
	// file parser
	return	ui_grp;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
