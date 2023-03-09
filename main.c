#define _POSIX_C_SOURCE 200112L
#include <cairo/cairo.h>
#include <librsvg/rsvg.h>
#include <wordexp.h>
#include "waywrap/waywrap.h"
#include "keyhold.h"
#include "draw.h"
#include "lib.h"

void draw(struct surface_state *, cairo_t *);
void draw_status(cairo_t *, struct app *, struct rect);
void draw_search(cairo_t *, struct app *, struct rect);
void draw_apps(cairo_t *, struct app *, struct rect);
void draw_options(cairo_t *, struct app *, struct rect);
void on_draw(struct surface_state *, unsigned char *);
void on_keyboard(uint32_t, xkb_keysym_t, const char *); 
void on_key_repeat(xkb_keysym_t);

struct app *app;
bool running = true;
char input_str[1024];
const char *placeholder_str = "Search applications...";
struct color clr_text, clr_placeholder;
struct keyhold *keyhold_root = NULL;

int main(int argc, char *argv[]) {
	clr_placeholder = hex2rgb(0x999999);
	clr_text = hex2rgb(0x313131);
	strcpy(input_str, "");

	app = app_new();

   	struct client_state *state = client_state_new();
	state->on_keyboard = on_keyboard;
	app->state = state;

	struct surface_state *a;
	int width = 600;
	int height = 400;
    a = surface_state_new(state, "Kallos Start Menu", width, height);
	a->on_draw = on_draw;

	// TODO if width is bigger than display, then make it smaller!
	xdg_toplevel_set_max_size(a->xdg_toplevel, width, height);
	xdg_toplevel_set_min_size(a->xdg_toplevel, width, height);
	xdg_toplevel_set_app_id(a->xdg_toplevel, "kallos-start");
	zxdg_toplevel_decoration_v1_set_mode(a->decos, 1);

	bool first = true;
    while (state->root_surface != NULL && running == true) {
		wl_display_dispatch(state->wl_display);
		keyhold_process(keyhold_root, state->key_repeat_delay, 
			state->key_repeat_rate, on_key_repeat);

		// If there is no active input let's bounce out of here!
		if (!first && state->active_surface_pointer == NULL && 
			state->active_surface_keyboard == NULL) {
			running = false;
		}
		first = false;
	}
	client_state_destroy(state);

	printf("freeing app... ");
	app_free(app);
	printf("freed\n");
	if (strlen(input_str) > 0) {
		printf("do app! %s\n", input_str);
	}
	printf("exit\n");
    return 0;
}

void on_draw(struct surface_state *state, unsigned char *data) {
	cairo_surface_t *csurf = cairo_image_surface_create_for_data(data,
		CAIRO_FORMAT_ARGB32, state->width, state->height, state->width * 4);
	cairo_t *cr = cairo_create(csurf);
	draw(state, cr);
	cairo_surface_flush(csurf);
	cairo_destroy(cr);
	cairo_surface_destroy(csurf);
}

void on_key_repeat(xkb_keysym_t sym) {
	//printf("got key repeat! %d\n", sym);
	if (sym == XKB_KEY_BackSpace)
		input_str[strlen(input_str)-1] = '\0';
}

void on_keyboard(uint32_t state, xkb_keysym_t sym, const char *utf8) {
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		keyhold_root = keyhold_add(keyhold_root, sym);
		if (sym == XKB_KEY_BackSpace) {
			int len = strlen(input_str);
			input_str[len-1] = '\0';
		} else if (sym == XKB_KEY_Escape) {
			running = false;
			input_str[0] = '\0';
			printf("quit application!\n");
		} else if (sym == XKB_KEY_Return) {
			printf("start app at index 0!\n");
			running = false;
		} else if (is_valid_char(utf8)) {
			//printf("char '%s'\n", utf8);
			strcat(input_str, utf8);
		}
	} else {
		keyhold_root = keyhold_remove(keyhold_root, sym);
	}
}

void draw(struct surface_state *state, cairo_t *cr) {
	struct color clr_main, clr_bg;
	
	int w = state->width;
	int h = state->height;
	
	clr_main = app->clr_main;
	clr_bg = app->clr_bg;

	// Draw app frame
	path_rounded_rect(cr, 1.5, 1.5, w-4, h-4, 2);
	cairo_set_source_rgb(cr, clr_bg.r, clr_bg.g, clr_bg.b);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr, clr_main.r, clr_main.g, clr_main.b);
	cairo_set_line_width(cr, 1.0);
	cairo_stroke(cr);

	// Set our font for the status section
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_select_font_face(cr, "Noto Sans", CAIRO_FONT_SLANT_NORMAL, 
		CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 14);

	// Draw our status and date indicators
	draw_status(cr, app, (struct rect){
		.x = 0,
		.y = 0,
		.width = w,
		.height = h,
	});	

	// Draw application search bar
	draw_search(cr, app, (struct rect){
		.x = 20, 
		.y = 50,
		.width = w - 40, 
		.height = 36, 
	});

	// TODO draw apps
	
	draw_options(cr, app, (struct rect){
		.x = 20,
		.y = h - 20 - 60,
		.width = w - 40,
		.height = 60, 
	});
}

void draw_status(cairo_t *cr, struct app *app, struct rect bounds) {
	char str[100];
	double offset, x, spacing, icon_size;
	x = 20;
	spacing = 30;
	icon_size = 16;
	// Draw Date
	datestr(str, app->fmt_time);
	draw_text_rtl(cr, str, (struct point){
		.x = bounds.width - 20, 
		.y = bounds.y + 30
	});
	// Draw Wifi
	draw_svg_square(cr, app->svg_wifi_full, x, 16, icon_size);
	x += icon_size + 8;
	offset = draw_text(cr, "-56dBm", x, 30);
	// Draw Battery
	x += offset + spacing;
	draw_svg_square(cr, app->svg_battery, x, 16, icon_size);
	x += icon_size + 8;
	offset = draw_text(cr, "99%", x, 30);
	// Draw Bluetooth
	x += offset + spacing;
	draw_svg_square(cr, app->svg_bluetooth, x, 16, icon_size);
	x += icon_size + 8;
	draw_text(cr, "On", x, 30);
}

void draw_search(cairo_t *cr, struct app *app, struct rect box) {
	struct color clr_main = app->clr_main;

	// Draw the border and container
	path_rounded_rect(cr, box.x, box.y, box.width, box.height, 2);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr, clr_main.r, clr_main.g, clr_main.b);
	cairo_set_line_width(cr, 2.0);
	cairo_stroke(cr);

	// Draw the search icon
	draw_svg_square(cr, app->svg_search, box.x+10, box.y+10, 16);

	// Draw the input text or placeholder
	const char *str;
	if (strlen(input_str) <= 0) {
		cairo_set_source_rgb(cr, clr_placeholder.r, clr_placeholder.g, 
			clr_placeholder.b);
		str = placeholder_str;
	} else {
		cairo_set_source_rgb(cr, clr_text.r, clr_text.g, clr_text.b);
		str = input_str;
	}
	cairo_select_font_face(cr, "Noto Sans", CAIRO_FONT_SLANT_NORMAL, 
		CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 16);
	draw_text(cr, str, box.x + 35, box.y + 5 + 19);
}

void draw_apps(cairo_t *cr, struct app *app, struct rect bounds) {

}

void draw_options(cairo_t *cr, struct app *app, struct rect bounds) {
	double x, y, spacing, icon_size;
	// We draw from right to left
	spacing = 15;
	icon_size = 24;
	x = bounds.x + bounds.width - icon_size;
	y = bounds.y + bounds.height - icon_size;

	RsvgHandle *icons[4] = {
		app->svg_power_off,
		app->svg_restart,
		app->svg_sleep,
		app->svg_exit,
	};
	for (int i = 0; i < 4; i++) {
		draw_svg_square(cr, icons[i], x, y, icon_size);
		x -= spacing + icon_size;
	}
}
