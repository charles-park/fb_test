//-----------------------------------------------------------------------------
//
// 2022.03.23 framebuffer graphic library(chalres-park)
//
//-----------------------------------------------------------------------------
#ifndef __FB_LIB_H__
#define __FB_LIB_H__

//-----------------------------------------------------------------------------
// Color table & convert macro
//-----------------------------------------------------------------------------
#define RGB_TO_UINT(r,g,b)  (((r << 16) | (g << 8) | b) & 0xFFFFFF)
#define UINT_TO_R(i)        ((i >> 16) & 0xFF)
#define UINT_TO_G(i)        ((i >>  8) & 0xFF)
#define UINT_TO_B(i)        ((i      ) & 0xFF)
/*
    https://www.rapidtables.com/web/color/RGB_Color.html
*/
#define COLOR_BLACK         RGB_TO_UINT(0, 0, 0)
#define COLOR_WHITE         RGB_TO_UINT(255,255,255)
#define COLOR_RED	        RGB_TO_UINT(255,0,0)
#define COLOR_LIME	        RGB_TO_UINT(0,255,0)
#define COLOR_BLUE	        RGB_TO_UINT(0,0,255)
#define COLOR_YELLOW        RGB_TO_UINT(255,255,0)
#define COLOR_GREEN	        RGB_TO_UINT(0,128,0)
#define COLOR_CYAN          RGB_TO_UINT(0,255,255)
#define COLOR_MAGENTA       RGB_TO_UINT(255,0,255)
#define COLOR_SILVER        RGB_TO_UINT(192,192,192)
#define COLOR_DIM_GRAY      RGB_TO_UINT(105,105,105)
#define COLOR_GRAY          RGB_TO_UINT(128,128,128)
#define COLOR_DARK_GRAY     RGB_TO_UINT(169,169,169)
#define COLOR_LIGHT_GRAY    RGB_TO_UINT(211,211,211)
#define COLOR_MAROON        RGB_TO_UINT(128,0,0)
#define COLOR_OLIVE         RGB_TO_UINT(128,128,0)
#define COLOR_PURPLE        RGB_TO_UINT(128,0,128)
#define COLOR_TEAL          RGB_TO_UINT(0,128,128)
#define COLOR_NAVY          RGB_TO_UINT(0,0,128)

//-----------------------------------------------------------------------------
// Frame buffer struct
//-----------------------------------------------------------------------------
union fb_color {
    struct {
        unsigned int    b:8;    // lsb
        unsigned int    g:8;
        unsigned int    r:8;
        unsigned int    a:8;
    } bits;
    unsigned int uint;
};

struct fb_config
{
	int fd;
	int width;
	int height;
	int height_virtual;
	int bpp;
	int stride;
	int red_offset;
	int red_length;
	int green_offset;
	int green_length;
	int blue_offset;
	int blue_length;
	int transp_offset;
	int transp_length;
	int buffer_num;
	char *data;
	char *base;

    union fb_color bg_color, fg_color;
    int line_thickness, font_scale;
};

//-----------------------------------------------------------------------------
#define FONT_HANGUL_WIDTH   16
#define FONT_ASCII_WIDTH    8
#define FONT_HEIGHT         16

enum eFONTS_HANGUL {
    eFONT_HAN_DEFAULT = 0,
    eFONT_HANBOOT,
    eFONT_HANGODIC,
    eFONT_HANPIL,
    eFONT_HANSOFT,
    eFONT_END
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern void draw_text      (struct fb_config *fb, int x, int y, unsigned int color, char *fmt, ...);
extern void draw_line      (struct fb_config *fb, int x, int y, int w);
extern void draw_rect      (struct fb_config *fb, int x, int y, int w, int h);
extern void draw_fill_rect (struct fb_config *fb, int x, int y, int w, int h);

extern void set_fg_color   (struct fb_config *fb, unsigned int color);
extern void set_bg_color   (struct fb_config *fb, unsigned int color);
extern void set_line_width (struct fb_config *fb, int thickness);
extern void set_font       (enum eFONTS_HANGUL s_font);
extern void set_font_scale (struct fb_config *fb, unsigned int scale);

extern void fb_clear       (struct fb_config *fb);
extern int  fb_init        (struct fb_config *fb, const char *DEVICE_NAME);
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
#endif  // #define __FB_LIB_H__
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
