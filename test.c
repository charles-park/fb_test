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

#include "ui_parser.h"
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const char *OPT_DEVICE_NAME = "/dev/fb0";
const char *OPT_TEXT_STR = "FrameBuffer 테스트 프로그램입니다.";
unsigned int opt_x = 0, opt_y = 0, opt_width = 0, opt_height = 0, opt_color = 0;
unsigned char opt_red = 0, opt_green = 0, opt_blue = 0, opt_thckness = 1, opt_scale = 1;
unsigned char opt_clear = 0, opt_fill = 0, opt_info = 0, opt_font = 0;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage(const char *prog)
{
	printf("Usage: %s [-DrgbxywhfntscCi]\n", prog);
	puts("  -D --device    device to use (default /dev/fb0)\n"
	     "  -r --red       pixel red hex value.(default = 0)\n"
	     "  -g --green     pixel green hex value.(default = 0)\n"
	     "  -b --blue      pixel blue hex value.(default = 0)\n"
	     "  -x --x_pos     framebuffer memory x position.(default = 0)\n"
	     "  -y --y_pos     framebuffer memory y position.(default = 0)\n"
	     "  -w --width     reference width for drawing.\n"
	     "  -h --height    reference height for drawing.\n"
	     "  -f --fill      drawing fill box.(default empty box)\n"
	     "  -n --thickness drawing line thickness.(default = 1)\n"
	     "  -t --text      drawing text string.(default str = \"text\"\n"
	     "  -s --scale     scale of text.\n"
	     "  -c --color     background rgb(hex) color.(ARGB)\n"
         "  -C --clear     clear framebuffer(r = g = b = 0)\n"
	     "  -i --info      framebuffer info display.\n"
		 "  -F --font      Hangul font select\n"
		 "                 0 MYEONGJO\n"
		 "                 1 HANBOOT\n"
		 "                 2 HANGODIC\n"
		 "                 3 HANPIL\n"
		 "                 4 HANSOFT\n"
	);
	exit(1);
}

//------------------------------------------------------------------------------
static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  	1, 0, 'D' },
			{ "red",		1, 0, 'r' },
			{ "green",		1, 0, 'g' },
			{ "blue",		1, 0, 'b' },
			{ "x_pos",		1, 0, 'x' },
			{ "y_pos",		1, 0, 'y' },
			{ "width",		1, 0, 'w' },
			{ "height",		1, 0, 'h' },
			{ "fill",		0, 0, 'f' },
			{ "thickness",	1, 0, 'n' },
			{ "text",		1, 0, 't' },
			{ "scale",		1, 0, 's' },
			{ "color",		1, 0, 'c' },
			{ "clear",		0, 0, 'C' },
			{ "info",		0, 0, 'i' },
			{ "font",		1, 0, 'F' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:r:g:b:x:y:w:h:fn:t:s:c:CiF:", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			OPT_DEVICE_NAME = optarg;
			break;
		case 'r':
            opt_red = strtol(optarg, NULL, 16);
			opt_red = opt_red > 255 ? 255 : opt_red;
			break;
		case 'g':
            opt_green = strtol(optarg, NULL, 16);
			opt_green = opt_green > 255 ? 255 : opt_green;
			break;
		case 'b':
            opt_blue = strtol(optarg, NULL, 16);
			opt_blue = opt_blue > 255 ? 255 : opt_blue;
			break;
		case 'x':
            opt_x = abs(atoi(optarg));
			break;
		case 'y':
            opt_y = abs(atoi(optarg));
			break;
		case 'w':
            opt_width = abs(atoi(optarg));
			break;
		case 'h':
            opt_height = abs(atoi(optarg));
			break;
		case 'f':
            opt_fill = 1;
			break;
		case 'n':
            opt_thckness = abs(atoi(optarg));
            opt_thckness = opt_thckness > 0? opt_thckness : 1;
			break;
		case 't':
            OPT_TEXT_STR = optarg;
			break;
		case 's':
            opt_scale = abs(atoi(optarg));
            opt_scale = opt_scale ? opt_scale : 1;
			break;
		case 'c':
            opt_color = strtol(optarg, NULL, 16) & 0xFFFFFF;
			break;
		case 'C':
            opt_clear = 1;
			break;
		case 'i':
            opt_info = 1;
			break;
		case 'F':
			opt_font = abs(atoi(optarg));
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

//------------------------------------------------------------------------------
int disable_blink_cursor(void)
{
    int fd;
    char buf[10];
	/*
		echo 1 > /sys/class/graphics/fbcon/cursor_blink
	*/
    if((fd = open("/sys/class/graphics/fbcon/cursor_blink", O_RDWR)) <0 )   {
        perror("/sys/class/graphics/fbcon/cursor_blink open fail!\n");
        return -1;
    }
    memset(buf, 0x00, sizeof(buf));
    sprintf(buf, "%c\n", '0');
    write(fd, buf, sizeof(buf));
    close(fd);
    return 0;
}

//------------------------------------------------------------------------------
void dump_fb_info (fb_info_t *fb)
{
	printf("========== FB SCREENINFO ==========\n");
	printf("xres   : %d\n", fb->w);
	printf("yres   : %d\n", fb->h);
	printf("bpp    : %d\n", fb->bpp);
	printf("stride : %d\n", fb->stride);
	printf("bgr    : %d\n", fb->is_bgr);
	printf("fb_base     : %p\n", fb->base);
	printf("fb_data     : %p\n", fb->data);
	printf("==================================\n");
}

//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	fb_info_t	*pfb;
	int f_color, b_color;
	ui_grp_t 	*ui_grp;

    parse_opts(argc, argv);

    if (disable_blink_cursor ())
		exit(1);

    if ((pfb = fb_init (OPT_DEVICE_NAME)) == NULL) {
		err("frame buffer init fail!\n");
		exit(1);
	}

	if ((ui_grp = ui_init (pfb, "ui.cfg")) == NULL) {
		err("User interface create fail!\n");
		exit(1);
	}
	
	f_color = RGB_TO_UINT(opt_red, opt_green, opt_blue);
	b_color = COLOR_WHITE;

    if (opt_color)
        b_color = opt_color & 0x00FFFFFF;

    if (opt_clear)
        fb_clear(pfb);

	if (opt_info)
		dump_fb_info(pfb);

    if (OPT_TEXT_STR) {
		set_font(opt_font);
		switch(opt_font) {
			default :
			case eFONT_HAN_DEFAULT:
				draw_text(pfb, 0, pfb->h / 2, f_color, b_color, opt_scale,
					"한글폰트는 명조체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
			case eFONT_HANBOOT:
				draw_text(pfb, 0, pfb->h / 2, f_color, b_color, opt_scale,
					"한글폰트는 붓글씨체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
			case eFONT_HANGODIC:
				draw_text(pfb, 0, pfb->h / 2, f_color, b_color, opt_scale,
					"한글폰트는 고딕체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
			case eFONT_HANPIL:
				draw_text(pfb, 0, pfb->h / 2, f_color, b_color, opt_scale,
					"한글폰트는 필기체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
			case eFONT_HANSOFT:
				draw_text(pfb, 0, pfb->h / 2, f_color, b_color, opt_scale,
					"한글폰트는 한소프트체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
		}
        draw_text(pfb, opt_x, opt_y, f_color, b_color, opt_scale, "%s", OPT_TEXT_STR);
	}

    if (opt_width) {
        if (opt_height) {
            if (opt_fill)
                draw_fill_rect(pfb, opt_x, opt_y, opt_width, opt_height, f_color);
            else
                draw_rect(pfb, opt_x, opt_y, opt_width, opt_height, opt_thckness, f_color);
        }
        else
            draw_line(pfb, opt_x, opt_y, opt_width, f_color);
    }

	ui_update(pfb, ui_grp, -1);
	sleep(1);

#if 0
{
	int i = 0;
	for (i = 100; i > 0; i--) {
//void ui_set_str (fb_info_t *fb, ui_grp_t *ui_grp,
//                  int id, int x, int y, int scale, int font, char *fmt, ...)
		ui_set_str(pfb, ui_grp, 1, -1, -1, -1, 2, "한글 count=%d 중 입니다.", i);
		usleep(100000);
	}
}
#endif
	fb_close (pfb);
	ui_close(ui_grp);

	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
