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

#include "fblib/fblib.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const char *OPT_DEVICE_NAME = "/dev/fb0";
const char *OPT_TEXT_STR = "FrameBuffer 테스트 프로그램입니다.";
unsigned int opt_x = 0, opt_y = 0, opt_width = 0, opt_height = 0, opt_color = 0;
unsigned char opt_red = 0, opt_green = 0, opt_blue = 0, opt_thckness = 1, opt_scale = 1;
unsigned char opt_clear = 0, opt_fill = 0, opt_info = 0, opt_font = 0;

//------------------------------------------------------------------------------
#if defined(__DEBUG__)
	#define	dbg(fmt, args...)	fprintf(stderr,"%s(%d) : " fmt, __func__, __LINE__, ##args)
#else
	#define	dbg(fmt, args...)
#endif

//------------------------------------------------------------------------------
void dump_vscreeninfo(struct fb_var_screeninfo *fvsi)
{
	printf("======= FB VAR SCREENINFO =======\n");
	printf("xres: %d\n", fvsi->xres);
	printf("yres: %d\n", fvsi->yres);
	printf("yres_virtual: %d\n", fvsi->yres_virtual);
	printf("buffer number: %d\n", fvsi->yres_virtual / fvsi->yres);
	printf("bpp : %d\n", fvsi->bits_per_pixel);
	printf("red bits    :\n");
	printf("    offset   : %d\n", fvsi->red.offset);
	printf("    length   : %d\n", fvsi->red.length);
	printf("    msb_right: %d\n", fvsi->red.msb_right);
	printf("green bits  :\n");
	printf("    offset   : %d\n", fvsi->green.offset);
	printf("    length   : %d\n", fvsi->green.length);
	printf("    msb_right: %d\n", fvsi->green.msb_right);
	printf("blue bits   :\n");
	printf("    offset   : %d\n", fvsi->blue.offset);
	printf("    length   : %d\n", fvsi->blue.length);
	printf("    msb_right: %d\n", fvsi->blue.msb_right);
	printf("transp bits :\n");
	printf("    offset   : %d\n", fvsi->transp.offset);
	printf("    length   : %d\n", fvsi->transp.length);
	printf("    msb_right: %d\n", fvsi->transp.msb_right);

	printf("=================================\n");
}

//------------------------------------------------------------------------------
void dump_fscreeninfo(struct fb_fix_screeninfo *ffsi)
{
	printf("======= FB FIX SCREENINFO =======\n");
	printf("id          : %s\n", ffsi->id);
	printf("smem_start  : 0x%08lX\n", ffsi->smem_start);
	printf("smem_len    : %u\n", ffsi->smem_len);
	printf("line_length : %u\n", ffsi->line_length);
	printf("=================================\n");
}

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
void dump_fb_info (struct fb_config *fb)
{
	printf("========== FB SCREENINFO ==========\n");
	printf("xres: %d\n", fb->width);
	printf("yres: %d\n", fb->height);
	printf("bpp : %d\n", fb->bpp);
	printf("stride : %d\n", fb->stride);
	printf("yres_virtual: %d\n", fb->height_virtual);
	printf("buffer number: %d\n", fb->buffer_num);
	printf("red bits    :\n");
	printf("    offset   : %d\n", fb->red_offset);
	printf("    length   : %d\n", fb->red_length);
	printf("green bits  :\n");
	printf("    offset   : %d\n", fb->green_offset);
	printf("    length   : %d\n", fb->green_length);
	printf("blue bits   :\n");
	printf("    offset   : %d\n", fb->blue_offset);
	printf("    length   : %d\n", fb->blue_length);
	printf("transp bits :\n");
	printf("    offset   : %d\n", fb->transp_offset);
	printf("    length   : %d\n", fb->transp_length);
	printf("fb_base     : %p\n", fb->base);
	printf("smem_start  : %p\n", fb->data);
	printf("==================================\n");
	printf("fg_color : 0x%08X\n", fb->fg_color.uint);
	printf("          R : 0x%02X\n", fb->fg_color.bits.r);
	printf("          G : 0x%02X\n", fb->fg_color.bits.g);
	printf("          B : 0x%02X\n", fb->fg_color.bits.b);
	printf("bg_color : 0x%08X\n", fb->bg_color.uint);
	printf("          R : 0x%02X\n", fb->fg_color.bits.r);
	printf("          G : 0x%02X\n", fb->fg_color.bits.g);
	printf("          B : 0x%02X\n", fb->fg_color.bits.b);
	printf("font_scale  : 0x%02d\n" , fb->font_scale);
	printf("line thickness : 0x%02d\n", fb->line_thickness);
	printf("==================================\n");
}

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	struct fb_config fb;

    parse_opts(argc, argv);

    if (disable_blink_cursor ())
		exit(1);

    fb_init (&fb, OPT_DEVICE_NAME);

    set_fg_color  (&fb, RGB_TO_UINT(opt_red, opt_green, opt_blue));
    set_bg_color  (&fb, COLOR_WHITE);
    set_font_scale(&fb, opt_scale);

    if (opt_color)
        set_bg_color (&fb, opt_color & 0xFFFFFF);

    if (opt_clear)
        fb_clear(&fb);

    fb.line_thickness = opt_thckness;

	if (opt_info)
		dump_fb_info(&fb);

    if (OPT_TEXT_STR) {
		set_font(opt_font);
		switch(opt_font) {
			default :
			case eFONT_HAN_DEFAULT:
				draw_text(&fb, 0, fb.height / 2, fb.fg_color.uint, 
					"한글폰트는 명조체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
			case eFONT_HANBOOT:
				draw_text(&fb, 0, fb.height / 2, fb.fg_color.uint, 
					"한글폰트는 붓글씨체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
			case eFONT_HANGODIC:
				draw_text(&fb, 0, fb.height / 2, fb.fg_color.uint, 
					"한글폰트는 고딕체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
			case eFONT_HANPIL:
				draw_text(&fb, 0, fb.height / 2, fb.fg_color.uint, 
					"한글폰트는 필기체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
			case eFONT_HANSOFT:
				draw_text(&fb, 0, fb.height / 2, fb.fg_color.uint, 
					"한글폰트는 한소프트체 이며, Font Scale은 %d배 입니다.", opt_scale);
			break;
		}
        draw_text(&fb, opt_x, opt_y, fb.fg_color.uint, "%s", OPT_TEXT_STR);
	}

    if (opt_width) {
        if (opt_height) {
            if (opt_fill)
                draw_fill_rect(&fb, opt_x, opt_y, opt_width, opt_height);
            else
                draw_rect(&fb, opt_x, opt_y, opt_width, opt_height);
        }
        else
            draw_line(&fb, opt_x, opt_y, opt_width);
    }
	close (fb.fd);

	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
