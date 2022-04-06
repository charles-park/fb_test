//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifndef __UI_PARSER_H__
#define __UI_PARSER_H__

//------------------------------------------------------------------------------
#define	ITEM_COUNT_MAX	64
#define	ITEM_STR_MAX	64

//------------------------------------------------------------------------------
typedef struct rect_item__t {
	int				id, x, y, w, h, lw;
	fb_color_u		rc, lc;
}	r_item_t;

typedef struct string_item__t {
	int				r_id, x_off, y_off, scale, f_type;
	fb_color_u		fc, bc;
	char            str[ITEM_STR_MAX];
}	s_item_t;

typedef struct ui_group__t {
	int             r_cnt, s_cnt, f_type;
    fb_color_u      fc, rc, lc;
	r_item_t		r_item[ITEM_COUNT_MAX];
	s_item_t		s_item[ITEM_COUNT_MAX];
}	ui_grp_t;

//------------------------------------------------------------------------------
extern void         ui_update   (fb_info_t *fb, ui_grp_t *ui_grp, int id);
extern void         ui_close    (ui_grp_t *ui_grp);
extern ui_grp_t     *ui_init    (fb_info_t *fb, const char *cfg_filename);

//------------------------------------------------------------------------------

#endif  // #define __UI_PARSER_H__
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
