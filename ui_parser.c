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
   'D' : default data
   'L' : Line data

   모든 좌표는 비율값 (0%~100%), lw(out line width) 외곽라인 두께, 
   c/lc 컬러/외곽라인컬러는 ARBG값(32bits).

   [파일 내용]
   type(char), id(int), x(int), y(int), w(int), h(int), c(int), lw(int), lc(int)

   FB info : 800(x) x 480(y) 의 경우
   eg) 'R', 0, 0, 0, 10, 5, 0x00FFFFFF, 0, 0
   id - 0, 외곽라인이 없는 흰색 box. x = 0, w = 800 * 10 / 100, y = 0, h = 480 * 5 / 100
   eg) 'R', 1, 10, 5, 10, 5, 0x00FFFFFF, 2, 0x00FF0000
   id = 1, 2 pixel의 빨간 외곽라인이 있는 흰색 box.
   x = 800 * 10 / 100, y = 480 * 5 / 100,  w = 800 * 10 / 100, y = 0, h = 480 * 5 / 100
*/

//------------------------------------------------------------------------------
static void _ui_update (fb_info_t *fb, ui_grp_t *ui_grp, int id)
{
   int i;

   for (i = 0; i < ui_grp->r_cnt; i++) {
      if(id == ui_grp->r_item[i].id) {
         draw_fill_rect (fb,
            ui_grp->r_item[i].x, ui_grp->r_item[i].y,
            ui_grp->r_item[i].w, ui_grp->r_item[i].h,
            ui_grp->r_item[i].c.uint);

         if (ui_grp->r_item[i].lw)
            draw_rect (fb,
               ui_grp->r_item[i].x,  ui_grp->r_item[i].y,
               ui_grp->r_item[i].w,  ui_grp->r_item[i].h,
               ui_grp->r_item[i].lw, ui_grp->r_item[i].lc.uint);
      }
   }

   for (i = 0; i < ui_grp->s_cnt; i++) {
      if (id == ui_grp->s_item[i].r_id) {
         int x, y, color;

         x     = ui_grp->r_item[id].x;
         y     = ui_grp->r_item[id].y;
         color = ui_grp->r_item[id].c.uint;

         draw_text (fb,
            x + ui_grp->s_item[i].x_off, y + ui_grp->s_item[i].y_off,
            ui_grp->s_item[i].c.uint, color,
            ui_grp->s_item[i].scale, ui_grp->s_item[i].str);
      }
   }
}

//------------------------------------------------------------------------------
//void ui_update_str (fb_info_t *fb, ui_grp_t *ui_grp, int id, char *fmt, ...)
void ui_update_str (fb_info_t *fb, ui_grp_t *ui_grp, int id)
{
   int i;

   for (i = 0; i < ui_grp->s_cnt; i++) {
      if (id == ui_grp->s_item[i].r_id) {
         int x, y, color;

         x     = ui_grp->r_item[id].x;
         y     = ui_grp->r_item[id].y;
         color = ui_grp->r_item[id].c.uint;

         draw_text (fb,
            x + ui_grp->s_item[i].x_off, y + ui_grp->s_item[i].y_off,
            ui_grp->s_item[i].c.uint, color,
            ui_grp->s_item[i].scale, ui_grp->s_item[i].str);
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
ui_grp_t *ui_init (fb_info_t *fb, const char *cfg_filename)
{
	ui_grp_t	*ui_grp;
   FILE *pfd;
   char buf[80], r_cnt = 0, s_cnt = 0, *ptr;

   if ((pfd = fopen(cfg_filename, "r")) == NULL)
      return   NULL;

	if ((ui_grp = (ui_grp_t *)malloc(sizeof(ui_grp_t))) == NULL)
      return   NULL;

   memset (ui_grp, 0x00, sizeof(ui_grp_t));

   while(fgets(buf, sizeof(buf), pfd) != NULL) {
      ptr = strtok (buf, ",");
     
      if (*ptr == 'R') {
         ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].id      = atoi(ptr);
         ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].x       = atoi(ptr);
         ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].y       = atoi(ptr);
         ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].w       = atoi(ptr);
         ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].h       = atoi(ptr);
         ptr = strtok (NULL, ",");     

         ui_grp->r_item[r_cnt].c.uint  = strtoul(ptr, NULL, 16);
         ptr = strtok (NULL, ",");     ui_grp->r_item[r_cnt].lw      = atoi(ptr);

         ptr = strtok (NULL, ",");     
         ui_grp->r_item[r_cnt].lc.uint = strtoul(ptr, NULL, 16);

         ui_grp->r_item[r_cnt].x = (ui_grp->r_item[r_cnt].x * fb->w / 100);
         ui_grp->r_item[r_cnt].y = (ui_grp->r_item[r_cnt].y * fb->h / 100);
         ui_grp->r_item[r_cnt].w = (ui_grp->r_item[r_cnt].w * fb->w / 100);
         ui_grp->r_item[r_cnt].h = (ui_grp->r_item[r_cnt].h * fb->h / 100);

         r_cnt++;
      }
      else if (*ptr == 'S') {
         ptr = strtok (NULL, ",");     ui_grp->s_item[s_cnt].r_id   = atoi(ptr);
         ptr = strtok (NULL, ",");     ui_grp->s_item[s_cnt].x_off  = atoi(ptr);
         ptr = strtok (NULL, ",");     ui_grp->s_item[s_cnt].y_off  = atoi(ptr);
         ptr = strtok (NULL, ",");     ui_grp->s_item[s_cnt].scale  = atoi(ptr);

         ptr = strtok (NULL, ",");     
         ui_grp->s_item[s_cnt].c.uint = strtoul(ptr, NULL, 16);

         ptr = strtok (NULL, ",");
         strncpy(ui_grp->s_item[s_cnt].str, ptr, strlen(ptr)-1);
         s_cnt++;
      }
   }
   ui_grp->r_cnt = r_cnt;
   ui_grp->s_cnt = s_cnt;
   printf("%s\n", ui_grp->s_item[0].str);

   /* all item update */
   fb->is_bgr = true;
   if (ui_grp->r_cnt)
      ui_update (fb, ui_grp, -1);

   if (pfd)
      fclose (pfd);

	// file parser
	return	ui_grp;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
