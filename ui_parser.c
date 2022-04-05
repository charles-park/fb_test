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

/*
   UI Config file 형식

   [ type ]
   '#' : commant
   'R' : Rect data
   'S' : string data
   'C' : default config data
   'L' : Line data
   'G' : Rect group data

   Rect data x, y, w, h는 fb의 비율값 (0%~100%), 모든 컬러값은 32bits rgb data.

   ui.cfg file 참조
*/

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int _ui_str_scale (int w, int h, int lw, int slen)
{
   int as, w_len, h_len;

   for (as = 1; as < 5; as++) {
      w_len = slen * FONT_ASCII_WIDTH * as;
      h_len = FONT_HEIGHT * as + lw * 2;

      if ((slen > w) || (h_len > h))
         return (as -1);
   }
   return 1;
}

//------------------------------------------------------------------------------
static void _ui_str_pos_xy (ui_grp_t *ui_grp, int id, int *px, int *py)
{
   int i, x, y, w, h, lw;

   for (i = 0; i < ui_grp->r_cnt; i++) {
      if (id == ui_grp->r_item[i].id) {
         x  = ui_grp->r_item[i].x;  y = ui_grp->r_item[i].y;
         w  = ui_grp->r_item[i].w;  h = ui_grp->r_item[i].h;
         lw = ui_grp->r_item[i].lw;
      }
   }

   for (i = 0; i < ui_grp->s_cnt; i++) {
      if (id == ui_grp->s_item[i].r_id) {

         int slen = strlen(ui_grp->s_item[i].str);

         /* scale = -1 이면 최대 스케일을 구하여 표시한다 */
         if (ui_grp->s_item[i].scale < 0)
            ui_grp->s_item[i].scale = _ui_str_scale (w, h, lw, slen);

         if (ui_grp->s_item[i].x_off < 0) {
            int slen = strlen(ui_grp->s_item[i].str);

            slen = slen * FONT_ASCII_WIDTH * ui_grp->s_item[i].scale;
            *px = x + ((w - slen) / 2);
         }
         else
            *px = x + ui_grp->s_item[i].x_off;

         if (ui_grp->s_item[i].y_off < 0) {
            *py = y + ((h - FONT_HEIGHT * ui_grp->s_item[i].scale)) / 2;
         }
         else
            *py = y + ui_grp->s_item[i].y_off;
      }
   }
}

//------------------------------------------------------------------------------
static void _ui_update (fb_info_t *fb, ui_grp_t *ui_grp, int id)
{
   int i, color;

   for (i = 0; i < ui_grp->r_cnt; i++) {
      if(id == ui_grp->r_item[i].id) {
         draw_fill_rect (fb,
            ui_grp->r_item[i].x, ui_grp->r_item[i].y,
            ui_grp->r_item[i].w, ui_grp->r_item[i].h,
            ui_grp->r_item[i].rc.uint);

         if (ui_grp->r_item[i].lw)
            draw_rect (fb,
               ui_grp->r_item[i].x,  ui_grp->r_item[i].y,
               ui_grp->r_item[i].w,  ui_grp->r_item[i].h,
               ui_grp->r_item[i].lw, ui_grp->r_item[i].lc.uint);

         color = ui_grp->r_item[i].rc.uint;
      }
   }

   for (i = 0; i < ui_grp->s_cnt; i++) {
      if (id == ui_grp->s_item[i].r_id) {
         int x, y;

         _ui_str_pos_xy(ui_grp, id, &x, &y);
         draw_text (fb,
            x, y, ui_grp->s_item[i].fc.uint, color,
            ui_grp->s_item[i].scale, ui_grp->s_item[i].str);
      }
   }
}

//------------------------------------------------------------------------------
void ui_set_str (fb_info_t *fb, ui_grp_t *ui_grp,
                  int id, int scale, char *fmt, ...)
{
   int i;

   for (i = 0; i < ui_grp->s_cnt; i++) {
      if (id == ui_grp->s_item[i].r_id) {
         va_list va;
         int x, y;
         char buf[ITEM_STR_MAX];

         /* 받아온 string 변환 하여 buf에 저장 */
         memset(buf, 0x00, sizeof(buf));
         va_start(va, fmt);
         vsprintf(buf, fmt, va);
         va_end(va);

         ui_grp->s_item[i].scale = scale;

         if (strlen(ui_grp->s_item[i].str) > strlen(buf)) {
            /* 기존 String을 배경색으로 다시 그림(텍스트 지움) */
            /* string x, y 좌표 연산 */
            _ui_str_pos_xy(ui_grp, id, &x, &y);
            draw_text (fb,
               x, y, ui_grp->r_item[id].rc.uint, ui_grp->r_item[id].rc.uint,
               ui_grp->s_item[i].scale, ui_grp->s_item[i].str);
            memset (ui_grp->s_item[i].str, 0x00, ITEM_STR_MAX);
         }

         /* 새로운 string 표시 */
         /* string x, y 좌표 연산 */
         strncpy(ui_grp->s_item[i].str, buf, strlen(buf));
         _ui_str_pos_xy(ui_grp, id, &x, &y);
         draw_text (fb,
            x, y, ui_grp->s_item[i].fc.uint, ui_grp->r_item[id].rc.uint,
            ui_grp->s_item[i].scale, ui_grp->s_item[i].str);
      }
   }
}

//------------------------------------------------------------------------------
void ui_set_color (fb_info_t *fb, ui_grp_t *ui_grp,
                  int id, int fc, int rc, int lc)
{
   int i;

   for (i = 0; i < ui_grp->r_cnt; i++) {
      if (id == ui_grp->r_item[i].id) {
         ui_grp->r_item[i].rc.uint = (rc < 0) ? ui_grp->rc.uint : rc;
         ui_grp->r_item[i].lc.uint = (lc < 0) ? ui_grp->lc.uint : lc;
      }
   }
   for (i = 0; i < ui_grp->s_cnt; i++) {
      if (id == ui_grp->s_item[i].r_id) {
         ui_grp->s_item[i].fc.uint = (fc < 0) ? ui_grp->fc.uint : fc;
      }
   }
}

//------------------------------------------------------------------------------
void ui_update (fb_info_t *fb, ui_grp_t *ui_grp, int id)
{
   int i;

   if (id < 0) {
      for (i = 0; i < ui_grp->r_cnt; i++)
         _ui_update (fb, ui_grp, i);
   }
   else
      _ui_update (fb, ui_grp, id);
}

//------------------------------------------------------------------------------
void ui_close (ui_grp_t *ui_grp)
{
   if (ui_grp)
      free (ui_grp);
}

//------------------------------------------------------------------------------
static int _ui_file_sign_check (char *buf, fb_info_t *fb, ui_grp_t *ui_grp)
{
   char *ptr = strtok (buf, ",");

   if(strncmp("ODROID-UI-CONFIG", buf, strlen(ptr)))
      return -1;

   return 0;
}

//------------------------------------------------------------------------------
static void _ui_parser_cmd_C (char *buf, fb_info_t *fb, ui_grp_t *ui_grp)
{
   char *ptr = strtok (buf, ",");

   ptr = strtok (NULL, ",");     fb->is_bgr  = (atoi(ptr) != 0) ? 1: 0;
   ptr = strtok (NULL, ",");     ui_grp->fc.uint = strtol(ptr, NULL, 16);
   ptr = strtok (NULL, ",");     ui_grp->rc.uint = strtol(ptr, NULL, 16);
   ptr = strtok (NULL, ",");     ui_grp->lc.uint = strtol(ptr, NULL, 16);
   ptr = strtok (NULL, ",");     ui_grp->f_type = atoi(ptr);

   set_font(ui_grp->f_type);
}

//------------------------------------------------------------------------------
static void _ui_parser_cmd_R (char *buf, fb_info_t *fb, ui_grp_t *ui_grp)
{
   int r_cnt = ui_grp->r_cnt;
   char *ptr = strtok (buf, ",");

   ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].id   = atoi(ptr);
   ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].x    = atoi(ptr);
   ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].y    = atoi(ptr);
   ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].w    = atoi(ptr);
   ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].h    = atoi(ptr);
   ptr = strtok (NULL, ",");

   ui_grp->r_item[r_cnt].rc.uint  = strtol(ptr, NULL, 16);
   ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].lw   = atoi(ptr);

   ptr = strtok (NULL, ",");
   ui_grp->r_item[r_cnt].lc.uint = strtol(ptr, NULL, 16);

   ui_grp->r_item[r_cnt].x = (ui_grp->r_item[r_cnt].x * fb->w / 100);
   ui_grp->r_item[r_cnt].y = (ui_grp->r_item[r_cnt].y * fb->h / 100);
   ui_grp->r_item[r_cnt].w = (ui_grp->r_item[r_cnt].w * fb->w / 100);
   ui_grp->r_item[r_cnt].h = (ui_grp->r_item[r_cnt].h * fb->h / 100);

   if ((signed)ui_grp->r_item[r_cnt].rc.uint < 0)
      ui_grp->r_item[r_cnt].rc.uint = ui_grp->rc.uint;

   if ((signed)ui_grp->r_item[r_cnt].lc.uint < 0)
      ui_grp->r_item[r_cnt].lc.uint = ui_grp->lc.uint;

   r_cnt++;
   ui_grp->r_cnt = r_cnt;
}

//------------------------------------------------------------------------------
static void _ui_parser_cmd_S (char *buf, fb_info_t *fb, ui_grp_t *ui_grp)
{
   int s_cnt = ui_grp->s_cnt;
   char *ptr = strtok (buf, ",");

   ptr = strtok (NULL, ",");     ui_grp->s_item[s_cnt].r_id   = atoi(ptr);
   ptr = strtok (NULL, ",");     ui_grp->s_item[s_cnt].x_off  = atoi(ptr);
   ptr = strtok (NULL, ",");     ui_grp->s_item[s_cnt].y_off  = atoi(ptr);
   ptr = strtok (NULL, ",");     ui_grp->s_item[s_cnt].scale  = atoi(ptr);

   ptr = strtok (NULL, ",");
   ui_grp->s_item[s_cnt].fc.uint = strtoul(ptr, NULL, 16);

   if ((signed)ui_grp->s_item[s_cnt].fc.uint < 0)
      ui_grp->s_item[s_cnt].fc.uint = ui_grp->fc.uint;

   ptr = strtok (NULL, ",");
   strncpy(ui_grp->s_item[s_cnt].str, ptr, strlen(ptr)-1);
   s_cnt++;
   ui_grp->s_cnt = s_cnt;
}

//------------------------------------------------------------------------------
static void _ui_parser_cmd_G (char *buf, fb_info_t *fb, ui_grp_t *ui_grp)
{
   int pos = ui_grp->r_cnt;
   int s_h, r_h, sid, r_cnt, g_cnt, rc, lw, lc, i, j, y_s;
   char *ptr = strtok (buf, ",");

   ptr = strtok (NULL, ",");     sid   = atoi(ptr);
   ptr = strtok (NULL, ",");     r_cnt = atoi(ptr);
   ptr = strtok (NULL, ",");     s_h   = atoi(ptr);
   ptr = strtok (NULL, ",");     r_h   = atoi(ptr);
   ptr = strtok (NULL, ",");     g_cnt = atoi(ptr);
   ptr = strtok (NULL, ",");     rc    = strtol(ptr, NULL, 16);
   ptr = strtok (NULL, ",");     lw    = atoi(ptr);
   ptr = strtok (NULL, ",");     lc    = strtol(ptr, NULL, 16);

   for (i = 0; i < g_cnt; i++) {
      for (j = 0; j < r_cnt; j++) {
         pos = ui_grp->r_cnt + j + i * r_cnt;

         ui_grp->r_item[pos].w = (fb->w / r_cnt);
         y_s                   = (fb->h * s_h) / 100;
         ui_grp->r_item[pos].h = (fb->h * r_h) / 100;

         ui_grp->r_item[pos].id = sid + j + i * r_cnt;
         ui_grp->r_item[pos].x  = ui_grp->r_item[pos].w * j;
         ui_grp->r_item[pos].y  = ui_grp->r_item[pos].h * i + y_s;
         ui_grp->r_item[pos].lw = lw;

         ui_grp->r_item[pos].rc.uint = rc < 0 ? ui_grp->rc.uint : rc;
         ui_grp->r_item[pos].lc.uint = lc < 0 ? ui_grp->lc.uint : lc;
      }
   }
   ui_grp->r_cnt = pos +1;
}

//------------------------------------------------------------------------------
ui_grp_t *ui_init (fb_info_t *fb, const char *cfg_filename)
{
	ui_grp_t	*ui_grp;
   FILE *pfd;
   char buf[256], r_cnt = 0, s_cnt = 0, *ptr, is_cfg_file = 0;

   if ((pfd = fopen(cfg_filename, "r")) == NULL)
      return   NULL;

	if ((ui_grp = (ui_grp_t *)malloc(sizeof(ui_grp_t))) == NULL)
      return   NULL;

   memset (ui_grp, 0x00, sizeof(ui_grp_t));
   memset (buf,    0x00, sizeof(buf));

   while(fgets(buf, sizeof(buf), pfd) != NULL) {
      if (!is_cfg_file) {
         is_cfg_file = strncmp ("ODROID-UI-CONFIG", buf, strlen(buf)-1) == 0 ? 1 : 0;
         memset (buf, 0x00, sizeof(buf));
         continue;
      }
      switch(buf[0]) {
         case  'C':  _ui_parser_cmd_C (buf, fb, ui_grp); break;
         case  'R':  _ui_parser_cmd_R (buf, fb, ui_grp); break;
         case  'S':  _ui_parser_cmd_S (buf, fb, ui_grp); break;
         case  'G':  _ui_parser_cmd_G (buf, fb, ui_grp); break;
         default :
            err("Unknown parser command! cmd = %c\n", buf[0]);
         case  '#':  case  '\n':
         break;
      }
      memset (buf, 0x00, sizeof(buf));
   }

   if (!is_cfg_file) {
      err("UI Config File not found! (filename = %s)\n", cfg_filename);
      free (ui_grp);
      return NULL;
   }

   /* all item update */
   if (ui_grp->r_cnt)
      ui_update (fb, ui_grp, -1);

   if (pfd)
      fclose (pfd);

	// file parser
	return	ui_grp;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
