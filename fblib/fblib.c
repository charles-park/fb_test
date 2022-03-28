//-----------------------------------------------------------------------------
//
// 2022.03.23 framebuffer graphic library(chalres-park)
//
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Fonts
//-----------------------------------------------------------------------------
#include "FontHangul.h"
#include "FontHansoft.h"
#include "FontHanboot.h"
#include "FontHangodic.h"
#include "FontHanpil.h"
#include "FontHangodic.h"
#include "FontAscii_8x16.h"

#include "fblib.h"

//-----------------------------------------------------------------------------
#if defined(__DEBUG__)
	#define	dbg(fmt, args...)	fprintf(stderr,"%s(%d) : " fmt, __func__, __LINE__, ##args)
#else
	#define	dbg(fmt, args...)
#endif

//-----------------------------------------------------------------------------
static  void    make_image  (   unsigned char is_first,
                                unsigned char *dest,
                                unsigned char *src);
static  unsigned char *get_hangul_image(unsigned char HAN1,
                                        unsigned char HAN2,
                                        unsigned char HAN3);
static  void    put_pixel           (struct fb_config *fb, int x, int y, unsigned int color);
static  void    draw_hangul_bitmap  (struct fb_config *fb,
                                        int x, int y, unsigned char *p_img, unsigned int color);
static  void    draw_ascii_bitmap   (struct fb_config *fb,
                                        int x, int y, unsigned char *p_img, unsigned int color);
static  void    _draw_text          (struct fb_config *fb,
                                        int x, int y, char *p_str, unsigned int color);
        void    draw_text           (struct fb_config *fb, int x, int y, unsigned int color, char *fmt, ...);
        void    draw_line           (struct fb_config *fb, int x, int y, int w);
        void    draw_rect           (struct fb_config *fb, int x, int y, int w, int h);
        void    draw_fill_rect      (struct fb_config *fb, int x, int y, int w, int h);
        void    set_fg_color        (struct fb_config *fb, unsigned int color);
        void    set_bg_color        (struct fb_config *fb, unsigned int color);
        void    set_line_width      (struct fb_config *fb, int thickness);
        void    set_font_scale      (struct fb_config *fb, unsigned int scale);
        void    set_font            (enum eFONTS_HANGUL s_font);
        void    fb_clear            (struct fb_config *fb);
        int     fb_init             (struct fb_config *fb, const char *DEVICE_NAME);

//-----------------------------------------------------------------------------
// hangul image base 16x16
//-----------------------------------------------------------------------------
unsigned char HANFontImage[32] = {0,};

const char D_ML[22] = { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 																	};
const char D_FM[40] = { 1, 3, 0, 2, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 0, 2, 1, 3, 1, 3, 1, 3 			};
const char D_MF[44] = { 0, 0, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 1, 6, 3, 7, 3, 7, 3, 7, 1, 6, 2, 6, 4, 7, 4, 7, 4, 7, 2, 6, 1, 6, 3, 7, 0, 5 };

unsigned char *HANFONT1 = (unsigned char *)FONT_HANGUL1;
unsigned char *HANFONT2 = (unsigned char *)FONT_HANGUL2;
unsigned char *HANFONT3 = (unsigned char *)FONT_HANGUL3;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void make_image  (unsigned char is_first,
                        unsigned char *dest,
                        unsigned char *src)
{
    int i;
    if (is_first)   for (i = 0; i < 32; i++)    dest[i]  = src[i];
    else            for (i = 0; i < 32; i++)    dest[i] |= src[i];
}

//-----------------------------------------------------------------------------
static unsigned char *get_hangul_image( unsigned char HAN1,
                                        unsigned char HAN2,
                                        unsigned char HAN3)
{
    unsigned char f, m, l;
    unsigned char f1, f2, f3;
    unsigned char first_flag = 1;
    unsigned short utf16 = 0;

    /*------------------------------
    UTF-8 을 UTF-16으로 변환한다.

    UTF-8 1110xxxx 10xxxxxx 10xxxxxx
    ------------------------------*/
    utf16 = ((unsigned short)HAN1 & 0x000f) << 12 |
            ((unsigned short)HAN2 & 0x003f) << 6  |
            ((unsigned short)HAN3 & 0x003f);
    utf16 -= 0xAC00;

    /* 초성 / 중성 / 종성 분리 */
    l = (utf16 % 28);
    utf16 /= 28;
    m = (utf16 % 21) +1;
    f = (utf16 / 21) +1;

    /* 초성 / 중성 / 종성 형태에 따른 이미지 선택 */
    f3 = D_ML[m];
    f2 = D_FM[(f * 2) + (l != 0)];
    f1 = D_MF[(m * 2) + (l != 0)];

    memset(HANFontImage, 0, sizeof(HANFontImage));
    if (f)  {   make_image(         1, HANFontImage, HANFONT1 + (f1*16 + f1 *4 + f) * 32);    first_flag = 0; }
    if (m)  {   make_image(first_flag, HANFontImage, HANFONT2 + (        f2*22 + m) * 32);    first_flag = 0; }
    if (l)  {   make_image(first_flag, HANFontImage, HANFONT3 + (f3*32 - f3 *4 + l) * 32);    first_flag = 0; }

    return HANFontImage;
}

//-----------------------------------------------------------------------------
static void put_pixel (struct fb_config *fb, int x, int y, unsigned int color)
{
    if ((x < fb->width) && (y < fb->height)) {
        union fb_color c;
        c.uint = color;

        int offset = (y * fb->stride) + (x * (fb->bpp >> 3));
        int r_o = fb->red_offset   / fb->red_length;
        int g_o = fb->green_offset / fb->green_length;
        int b_o = fb->blue_offset  / fb->blue_length;

        *(fb->data + offset + r_o) = c.bits.r;
        *(fb->data + offset + g_o) = c.bits.g;
        *(fb->data + offset + b_o) = c.bits.b;

        if (fb->transp_length != 0)
            *(fb->data + offset + fb->transp_offset/fb->transp_length) = 255;
    } else {
        dbg("Out of range.(width = %d, x = %d, height = %d, y = %d)\n", 
            fb->width, x, fb->height, y);
    }
}

//-----------------------------------------------------------------------------
static void draw_hangul_bitmap (struct fb_config *fb,
                    int x, int y, unsigned char *p_img, unsigned int color)
{
    int pos, i, j, mask, x_off, y_off, scale_y, scale_x;

    dbg("font scale = %d\n", fb->font_scale);

    for (i = 0, y_off = 0, pos = 0; i < 16; i++) {
        for (scale_y = 0; scale_y < fb->font_scale; scale_y++) {
            if (scale_y)
                pos -= 2;
            for (x_off = 0, j = 0; j < 2; j++) {
                for (mask = 0x80; mask > 0; mask >>= 1) {
                    for (scale_x = 0; scale_x < fb->font_scale; scale_x++) {
                        int c;
                        if (!color)
                            c = (p_img[pos] & mask) ? fb->fg_color.uint : fb->bg_color.uint;
                        else
                            c = (p_img[pos] & mask) ? color : fb->bg_color.uint;

                        put_pixel(fb, x + x_off, y + y_off, c);
                        x_off++;
                    }
                }
                pos++;
            }
            y_off++;
        }
    }
}

//-----------------------------------------------------------------------------
static void draw_ascii_bitmap (struct fb_config *fb,
                    int x, int y, unsigned char *p_img, unsigned int color)
{
    int pos, mask, x_off, y_off, scale_y, scale_x;

    dbg("font scale = %d\n", fb->font_scale);

    for (pos = 0, y_off = 0; pos < 16; pos++) {
        for (scale_y = 0; scale_y < fb->font_scale; scale_y++) {
            for (x_off = 0, mask = 0x80; mask > 0; mask >>= 1) {
                for (scale_x = 0; scale_x < fb->font_scale; scale_x++) {
                    int c;
                    if (!color)
                        c = (p_img[pos] & mask) ? fb->fg_color.uint : fb->bg_color.uint;
                    else
                        c = (p_img[pos] & mask) ? color : fb->bg_color.uint;

                    put_pixel(fb, x + x_off, y + y_off, c);
                    x_off++;
                }
            }
            y_off++;
        }
    }
}

//-----------------------------------------------------------------------------
static void _draw_text (struct fb_config *fb,
                    int x, int y, char *p_str, unsigned int color)
{
    unsigned char *p_img;
    unsigned char c1, c2, c3;

    while(*p_str) { 
        c1 = *(unsigned char *)p_str++;

        //---------- 한글 ---------
        /* 모든 문자는 기본적으로 UTF-8형태로 저장되며 한글은 3바이트를 가진다. */
        /* 한글은 3바이트를 일어 UTF8 to UTF16으로 변환후 초/중/종성을 분리하여 조합형으로 표시한다. */
        if (c1 >= 0x80){
            c2 = *(unsigned char *)p_str++;
            c3 = *(unsigned char *)p_str++;

            p_img = get_hangul_image(c1, c2, c3);
            draw_hangul_bitmap(fb, x, y, p_img, color);
            x = x + FONT_HANGUL_WIDTH * fb->font_scale;
        }
        //---------- ASCII ---------
        else {
            p_img = (unsigned char *)FONT_ASCII[c1];
            draw_ascii_bitmap(fb, x, y, p_img, color);
            x = x + FONT_ASCII_WIDTH * fb->font_scale;
        }
    }  
}

//-----------------------------------------------------------------------------
void draw_text (struct fb_config *fb, int x, int y, unsigned int color, char *fmt, ...)
{
    char buf[256];
    va_list va;

    memset(buf, 0x00, sizeof(buf));

    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    va_end(va);

    _draw_text(fb, x, y, buf, color);
}

//-----------------------------------------------------------------------------
void draw_line (struct fb_config *fb, int x, int y, int w)
{
    int dx;

    for (dx = 0; dx < w; dx++)
        put_pixel(fb, x + dx, y, fb->fg_color.uint);
}

//-----------------------------------------------------------------------------
void draw_rect (struct fb_config *fb, int x, int y, int w, int h)
{
	int dy, i;

	for (dy = 0; dy < h; dy++) {
        if (dy < fb->line_thickness || (dy > (h - fb->line_thickness -1)))
            draw_line (fb, x, y + dy, w);
        else {
            for (i = 0; i < fb->line_thickness; i++) {
                put_pixel (fb, x + 0    +i, y + dy, fb->fg_color.uint);
                put_pixel (fb, x + w -1 -i, y + dy, fb->fg_color.uint);
            }
        }
	}
}

//-----------------------------------------------------------------------------
void draw_fill_rect (struct fb_config *fb, int x, int y, int w, int h)
{
	int dy;

	for (dy = 0; dy < h; dy++)
        draw_line(fb, x, y + dy, w);
}

//-----------------------------------------------------------------------------
void set_fg_color (struct fb_config *fb, unsigned int color)
{
    union fb_color c;
    c.uint = color;
    fb->fg_color.bits.r = c.bits.r;
    fb->fg_color.bits.g = c.bits.g;
    fb->fg_color.bits.b = c.bits.b;
    dbg("color = %d, r = %d, g = %d, b = %d\n", color, c.bits.r, c.bits.g, c.bits.b);
}

//-----------------------------------------------------------------------------
void set_bg_color (struct fb_config *fb, unsigned int color)
{
    union fb_color c;
    c.uint = color;
    fb->bg_color.bits.r = c.bits.r;
    fb->bg_color.bits.g = c.bits.g;
    fb->bg_color.bits.b = c.bits.b;
    dbg("color = %d, r = %d, g = %d, b = %d\n", color, c.bits.r, c.bits.g, c.bits.b);
}

//-----------------------------------------------------------------------------
void set_line_width (struct fb_config *fb, int thickness)
{
    fb->line_thickness = thickness;
}

//-----------------------------------------------------------------------------
void set_font_scale (struct fb_config *fb, unsigned int scale)
{
    fb->font_scale = scale;
}

//-----------------------------------------------------------------------------
void set_font(enum eFONTS_HANGUL s_font)
{
    switch(s_font)
    {
        case    eFONT_HANBOOT:
            HANFONT1 = (unsigned char *)FONT_HANBOOT1;
            HANFONT2 = (unsigned char *)FONT_HANBOOT2;
            HANFONT3 = (unsigned char *)FONT_HANBOOT3;
        break;
        case    eFONT_HANGODIC:
            HANFONT1 = (unsigned char *)FONT_HANGODIC1;
            HANFONT2 = (unsigned char *)FONT_HANGODIC2;
            HANFONT3 = (unsigned char *)FONT_HANGODIC3;
        break;
        case    eFONT_HANPIL:
            HANFONT1 = (unsigned char *)FONT_HANPIL1;
            HANFONT2 = (unsigned char *)FONT_HANPIL2;
            HANFONT3 = (unsigned char *)FONT_HANPIL3;
        break;
        case    eFONT_HANSOFT:
            HANFONT1 = (unsigned char *)FONT_HANSOFT1;
            HANFONT2 = (unsigned char *)FONT_HANSOFT2;
            HANFONT3 = (unsigned char *)FONT_HANSOFT3;
        break;
        case    eFONT_HAN_DEFAULT:
        default :
            HANFONT1 = (unsigned char *)FONT_HANGUL1;
            HANFONT2 = (unsigned char *)FONT_HANGUL2;
            HANFONT3 = (unsigned char *)FONT_HANGUL3;
        break;
    }
}

//-----------------------------------------------------------------------------
void fb_clear (struct fb_config *fb)
{
    memset(fb->data, 0x00, (fb->width * fb->height * fb->bpp) / 8);
}

//-----------------------------------------------------------------------------
int fb_init (struct fb_config *fb, const char *DEVICE_NAME)
{
	struct fb_var_screeninfo fvsi;
	struct fb_fix_screeninfo ffsi;
    int fd;

	memset(fb, 0, sizeof(struct fb_config));

	if ((fd = open(DEVICE_NAME, O_RDWR)) < 0) {
		perror("open");
        return -1;
	}
	if (ioctl(fd, FBIOGET_VSCREENINFO, &fvsi) < 0) {
		perror("ioctl(FBIOGET_VSCREENINFO)");
        goto out;
	}
	if (ioctl(fd, FBIOGET_FSCREENINFO, &ffsi) < 0) {
		perror("ioctl(FBIOGET_FSCREENINFO)");
        goto out;
	}

	fb->fd      = fd;
	fb->width   = fvsi.xres;
	fb->height  = fvsi.yres;
	fb->bpp     = fvsi.bits_per_pixel;
	fb->stride  = ffsi.line_length;

	fb->red_offset      = fvsi.red.offset;
	fb->red_length      = fvsi.red.length;
	fb->green_offset    = fvsi.green.offset;
	fb->green_length    = fvsi.green.length;
	fb->blue_offset     = fvsi.blue.offset;
	fb->blue_length     = fvsi.blue.length;
	fb->transp_offset   = fvsi.transp.offset;
	fb->transp_length   = fvsi.transp.length;

	if (fb->red_length != 8 || fb->green_length != 8 || fb->blue_length != 8) {
		perror("mmap");
        goto out;
	}

	fb->base = (char *)mmap((caddr_t) NULL, ffsi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (fb->base == (char *)-1) {
		perror("mmap");
        goto out;
	}
    fb->data = fb->base + ((unsigned long) ffsi.smem_start % (unsigned long) getpagesize());

    return 0;
out:
    close(fd);
    return -1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


















