#include <string.h>
#include <librsvg/rsvg.h>
#include <cairo/cairo.h>
#include "draw.h"

struct color hex2rgb(unsigned int val) {
	struct color c;
	c.r = ((val >> 16) & 0xFF) / 255.0;
	c.g = ((val >> 8) & 0xFF) / 255.0;
	c.b = ((val) & 0xFF) / 255.0;
	return c;
}

void path_rounded_rect(cairo_t *cr, double x, double y, double width, double height, double radius) {
	double degrees = M_PI / 180.0;
	cairo_new_sub_path(cr);
	cairo_arc(cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
	cairo_arc(cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
	cairo_arc(cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
	cairo_arc(cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
	cairo_close_path(cr);
}

void draw_svg_square(cairo_t *cr, RsvgHandle *handle, int x, int y, int size) {
	if (handle == NULL)
		return;
	RsvgRectangle rect = {
		.x = x,
		.y = y,
		.width = size,
		.height = size,
	};
	rsvg_handle_render_document(handle, cr, &rect, NULL);
}

void draw_img_square(cairo_t *cr, cairo_surface_t *img, int x, int y, int size) {
	int w = cairo_image_surface_get_width(img);
	float scale = (float)size / (float)w;
	cairo_save(cr);
	cairo_translate(cr, x, y);
	cairo_scale(cr, scale, scale);
	cairo_set_source_surface(cr, img, 0, 0);
	cairo_paint(cr);
	cairo_restore(cr);
}

double draw_text(cairo_t *cr, const char *str, int origin_x, int origin_y) {
	cairo_font_extents_t fe;
	cairo_text_extents_t te;
	char letter[2];
	double x = 0;
	int len = strlen(str);
	cairo_font_extents(cr, &fe);
	cairo_move_to(cr, 0, 0);
	for (int i=0; i < len; i++) {
	    *letter = '\0';
	    strncat(letter, str + i, 1);
	    cairo_text_extents(cr, letter, &te);
	    cairo_move_to(cr, origin_x + x, origin_y);
		x += te.x_advance;
	    cairo_show_text(cr, letter);
	}
	return x;
}

double draw_text_rtl(cairo_t *cr, const char *str, struct point origin) {
	cairo_text_extents_t te;
	char letter[2];
	double x = 0;
	int len = strlen(str);
	for (int i=0; i < len; i++) {
	    *letter = '\0';
	    strncat(letter, str + i, 1);
	    cairo_text_extents(cr, letter, &te);
		x += te.x_advance;
	}
	origin.x -= x;
	return draw_text(cr, str, origin.x, origin.y);
}

double draw_text_centered(cairo_t *cr, const char *str, struct point origin) {
	cairo_text_extents_t te;
	char letter[2];
	double x = 0;
	int len = strlen(str);
	for (int i=0; i < len; i++) {
	    *letter = '\0';
	    strncat(letter, str + i, 1);
	    cairo_text_extents(cr, letter, &te);
		x += te.x_advance;
	}
	origin.x -= x/2;
	return draw_text(cr, str, origin.x, origin.y);
}


