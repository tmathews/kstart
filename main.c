// #define _POSIX_C_SOURCE 200112L
#include "audio.h"
#include "draw.h"
#include "keyhold.h"
#include "lib.h"
#include "sys/power.h"
#include "waywrap/waywrap.h"
#include <cairo/cairo.h>
#include <iwlib.h>
#include <librsvg/rsvg.h>
#include <wordexp.h>

void draw(struct surface_state *, cairo_t *);
void draw_status(cairo_t *, struct app *, struct rect);
void draw_search(cairo_t *, struct app *, struct rect);
void draw_apps(cairo_t *, struct app *, struct rect);
void draw_options(cairo_t *, struct app *, struct rect);
void on_draw(struct surface_state *, unsigned char *);
void on_keyboard(uint32_t, xkb_keysym_t, const char *);
void on_key_repeat(xkb_keysym_t);
void update_power(struct app *);
void update_wifi(struct app *);

struct app *app;
bool running = true;
const char *placeholder_str = "Search applications...";
struct color clr_text, clr_placeholder;
struct keyhold *keyhold_root = NULL;

cairo_surface_t *surf;

int main(int argc, char *argv[]) {
	clr_placeholder = hex2rgb(0x999999);
	clr_text = hex2rgb(0x313131);

	printf("new kstart\n");

	app = app_new();
	update_power(app);
	update_wifi(app);
	audio_process(&app->audio);

	printf("new client_state\n");

	struct client_state *state = client_state_new();
	state->on_keyboard = on_keyboard;
	app->state = state;

	printf("new surface \n");

	struct surface_state *a;
	int width = 600;
	int height = 440;
	int tick = 0;
	a = surface_state_new(state, "Kallos Start Menu", width, height);
	a->on_draw = on_draw;

	printf("setting decos \n");

	// TODO if width is bigger than display, then make it smaller!
	xdg_toplevel_set_max_size(a->xdg_toplevel, width, height);
	xdg_toplevel_set_min_size(a->xdg_toplevel, width, height);
	xdg_toplevel_set_app_id(a->xdg_toplevel, "kallos-start");
	zxdg_toplevel_decoration_v1_set_mode(a->decos, 1);

	printf("now run\n");

	bool first = true;
	while (state->root_surface != NULL && running == true) {
		audio_process(&app->audio);
		wl_display_dispatch(state->wl_display);
		keyhold_process(
			keyhold_root, state->key_repeat_delay, state->key_repeat_rate,
			on_key_repeat
		);
		// If there is no active input let's bounce out of here!
		// if (!first && state->active_surface_pointer == NULL &&
		//	state->active_surface_keyboard == NULL) {
		//	running = false;
		//}
		app_process_inputs(app);
		if (app_process_events(app)) {
			running = false;
		}
		if (tick > 180) {
			update_power(app);
			update_wifi(app);
			tick = 0;
		}
		tick++;
		first = false;
	}
	client_state_destroy(state);
	app_free(app);
	return 0;
}

void on_draw(struct surface_state *state, unsigned char *data) {
	cairo_surface_t *csurf = cairo_image_surface_create_for_data(
		data, CAIRO_FORMAT_ARGB32, state->width, state->height, state->width * 4
	);
	cairo_t *cr = cairo_create(csurf);
	draw(state, cr);
	cairo_surface_flush(csurf);
	cairo_destroy(cr);
	cairo_surface_destroy(csurf);
}

void on_key_repeat(xkb_keysym_t sym) {
	// printf("got key repeat! %d\n", sym);
	if (sym == XKB_KEY_BackSpace) {
		app->search_str[strlen(app->search_str) - 1] = '\0';
	}
}

void on_keyboard(uint32_t state, xkb_keysym_t sym, const char *utf8) {
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		keyhold_root = keyhold_add(keyhold_root, sym);
		if (sym == XKB_KEY_BackSpace) {
			int len = strlen(app->search_str);
			app->search_str[len - 1] = '\0';
		} else if (sym == XKB_KEY_Escape) {
			running = false;
			app->search_str[0] = '\0';
			printf("quit application!\n");
		} else if (sym == XKB_KEY_Return) {
			if (strlen(app->search_str) > 0 && app->shortcut_first != NULL) {
				app->events = add_cevent(
					app->events,
					(struct custom_event){
						.type = EV_RUN,
						.next = NULL,
					}
				);
			}
		} else if (is_valid_char(utf8)) {
			// printf("char '%s'\n", utf8);
			strcat(app->search_str, utf8);
			app->page = 0;
		}
	} else {
		keyhold_root = keyhold_remove(keyhold_root, sym);
	}
}

void draw(struct surface_state *state, cairo_t *cr) {
	list_iter(app->hitzones, free);
	list_empty(app->hitzones);

	struct color clr_main, clr_bg;

	int w = state->width;
	int h = state->height;

	clr_main = app->clr_main;
	clr_bg = app->clr_bg;

	// Draw app frame
	path_rounded_rect(cr, 0, 0, w, h, 0);
	cairo_set_source_rgba(cr, clr_bg.r, clr_bg.g, clr_bg.b, 0.45);
	cairo_fill(cr);
	// cairo_set_source_rgb(cr, clr_main.r, clr_main.g, clr_main.b);
	// path_rounded_rect(cr, 1.5, 1.5, w-4, h-4, 2);
	// cairo_set_source_rgb(cr, clr_bg.r, clr_bg.g, clr_bg.b);
	// cairo_fill_preserve(cr);
	// cairo_set_source_rgb(cr, clr_main.r, clr_main.g, clr_main.b);
	// cairo_set_line_width(cr, 1.0);
	// cairo_stroke(cr);

	// Set our font for the status section
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_select_font_face(
		cr, "Noto Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD
	);
	cairo_set_font_size(cr, 14);

	// Draw our status and date indicators
	draw_status(
		cr, app,
		(struct rect){
			.x = 0,
			.y = 0,
			.width = w,
			.height = h,
		}
	);

	// Draw application search bar
	draw_search(
		cr, app,
		(struct rect){
			.x = 20,
			.y = 50,
			.width = w - 40,
			.height = 36,
		}
	);
	draw_apps(
		cr, app,
		(struct rect){
			.x = 20,
			.y = 112,
			.width = w - 40,
			.height = h - 80 - 112 - 30,
		}
	);
	draw_options(
		cr, app,
		(struct rect){
			.x = 20,
			.y = h - 20 - 60,
			.width = w - 40,
			.height = 60,
		}
	);
}

int volume_to_radius(float vol) {
	int radius = vol * 16;
	if (radius > 16) {
		radius = 16;
	}
	return radius;
}

int signal_to_radius(int8_t sig) {
	// top end = -80, low end = -40
	// mimimum = 3, max = 16
	int high = 80;
	int low = 30;
	int val = (int)sig * -1;
	float x = 1.0 - ((float)(val - low) / (float)(high - low));
	int radius = (int)(x * 13);
	if (radius > 13)
		radius = 13;
	if (radius < 0)
		radius = 0;
	return 3 + radius;
}

void draw_status(cairo_t *cr, struct app *app, struct rect bounds) {
	char str[100];
	double offset, x, spacing, icon_size;
	x = 20;
	spacing = 30;
	icon_size = 16;
	// Draw Date
	datestr(str, app->fmt_time);
	draw_text_rtl(
		cr, str, (struct point){.x = bounds.width - 20, .y = bounds.y + 30}
	);
	// Draw Wifi
	cairo_save(cr);
	cairo_translate(cr, x, 24);
	// cairo_set_source_rgba(cr, .2, .2, .2, 0.1);
	cairo_set_source_rgba(cr, 1., 1., 1., 0.2);
	cairo_new_sub_path(cr);
	int radius = 16;
	cairo_arc(cr, 8, 8, radius, -0.75 * M_PI, -0.25 * M_PI);
	cairo_line_to(cr, 8, 8);
	cairo_close_path(cr);
	cairo_set_line_width(cr, 1.);
	cairo_stroke(cr);
	cairo_restore(cr);
	// TODO fix wifi_found drawing
	// if (app->wifi_found) {
	//	//cairo_fill(cr);
	//} else {
	//	cairo_stroke(cr);
	//}
	// Draw wifi range
	if (app->wifi_found) {
		cairo_save(cr);
		cairo_translate(cr, x, 24);
		cairo_set_source_rgba(cr, 1., 1., 1., 1.);
		cairo_new_sub_path(cr);
		radius = signal_to_radius(app->wifi_signal);
		cairo_arc(cr, 8, 8, radius, -0.75 * M_PI, -0.25 * M_PI);
		cairo_line_to(cr, 8, 8);
		cairo_close_path(cr);
		cairo_fill(cr);
		cairo_restore(cr);
	}
	cairo_save(cr);
	if (app->wifi_found) {
		// draw_svg_square(cr, app->svg_wifi_full, x, 16, icon_size);
		x += icon_size + 8;
		sprintf(str, "%ddBm", app->wifi_signal);
		offset = draw_text(cr, str, x, 30);
	} else {
		// draw_svg_square(cr, app->svg_wifi_na, x, 16, icon_size);
		x += icon_size + 8;
		offset = draw_text(cr, "N/A", x, 30);
	}
	cairo_restore(cr);
	// Draw Battery
	cairo_save(cr);
	x += offset + spacing;
	draw_svg_square(cr, app->svg_battery, x, 16, icon_size);
	// Draw battery square
	int fh = (int)(10 * (app->battery_percent / 100));
	path_rounded_rect(cr, x + 4, 20 + (10 - fh), 8, fh, 0);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);
	x += icon_size + 8;
	sprintf(str, "%0.0f%%", app->battery_percent);
	offset = draw_text(cr, str, x, 30);
	cairo_restore(cr);
	// Draw Bluetooth
	// x += offset + spacing;
	// draw_svg_square(cr, app->svg_bluetooth, x, 16, icon_size);
	// x += icon_size + 8;
	// draw_text(cr, "N/I", x, 30);
	//
	// Draw Volume
	x += offset + spacing;
	// draw_svg_square(cr, app->svg_bluetooth, x, 16, icon_size);
	{
		cairo_save(cr);
		cairo_translate(cr, x, 32);
		cairo_set_source_rgba(cr, 1., 1., 1., .2);
		cairo_new_sub_path(cr);
		cairo_line_to(cr, 0, 0);
		cairo_line_to(cr, 16, 0);
		cairo_line_to(cr, 16, -16);
		cairo_close_path(cr);
		cairo_fill(cr);
		cairo_restore(cr);
	}
	cairo_save(cr);
	cairo_translate(cr, x, 32);
	cairo_set_source_rgba(cr, 1., 1., 1., 1.);
	cairo_new_sub_path(cr);
	radius = volume_to_radius(app->audio.volume);
	// cairo_arc(cr, 8, 8, radius, -0.75*M_PI, -0.25*M_PI);
	cairo_line_to(cr, 0, 0);
	cairo_line_to(cr, radius, 0);
	cairo_line_to(cr, radius, -radius);
	cairo_close_path(cr);
	cairo_fill(cr);
	cairo_restore(cr);
	x += icon_size + 8; // icon_size + 8;
	if (app->audio.muted) {
		sprintf(str, "M");
	} else {
		sprintf(str, "%0.0f%%", app->audio.volume * 100);
	}
	draw_text(cr, str, x, 30);
}

void draw_search(cairo_t *cr, struct app *app, struct rect box) {
	struct color clr_main = app->clr_main;

	// Draw the border and container
	path_rounded_rect(cr, box.x, box.y, box.width, box.height, 3);
	cairo_set_source_rgba(cr, 1, 1, 1, 1);
	cairo_fill_preserve(cr);
	cairo_set_source_rgba(
		cr, 1, 1, 1, 0.8
	); // clr_main.r, clr_main.g, clr_main.b);
	cairo_set_line_width(cr, 4.0);
	cairo_stroke(cr);

	// Draw the search icon
	draw_svg_square(cr, app->svg_search, box.x + 10, box.y + 10, 16);

	// Draw the input text or placeholder
	const char *str;
	if (strlen(app->search_str) <= 0) {
		cairo_set_source_rgb(
			cr, clr_placeholder.r, clr_placeholder.g, clr_placeholder.b
		);
		str = placeholder_str;
	} else {
		cairo_set_source_rgb(cr, clr_text.r, clr_text.g, clr_text.b);
		str = app->search_str;
	}
	cairo_select_font_face(
		cr, "Noto Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD
	);
	cairo_set_font_size(cr, 16);
	draw_text(cr, str, box.x + 35, box.y + 5 + 19);
}

void draw_apps(cairo_t *cr, struct app *app, struct rect bounds) {
	int x, y, column_width, row_height, gap;
	struct color clr = hex2rgb(0xffffff);
	x = bounds.x;
	y = bounds.y;
	column_width = bounds.width / 5;
	row_height = bounds.height / 3;
	gap = 25;

	app->shortcut_first = NULL;
	struct rect zone;
	struct shortcut *scp = app->shortcut_head;
	int count, first, last = 0;
	first = app->page * 15;
	last = first + 15;
	for (; scp != NULL; scp = scp->next) {
		if (!shortcut_matches(scp, app->search_str))
			continue;
		if (count < first || count >= last) {
			count++;
			continue;
		}
		if (app->shortcut_first == NULL)
			app->shortcut_first = scp;
		struct shortcut sc = *scp;
		// Draw background
		zone = (struct rect
		){.x = x, .y = y, .width = column_width, .height = row_height};

		struct hitzone *hzone = malloc(sizeof(struct hitzone));
		hzone->rect = zone;
		hzone->event = (struct custom_event){
			.type = EV_SHORTCUT,
			.shortcut = scp,
		};
		list_append(app->hitzones, hzone);
		if (app->input.active && rect_contains(zone, app->input.pos)) {
			// is selected index OR pointer in area
			cairo_save(cr);
			path_rounded_rect(cr, x, y, column_width, row_height, 5.0);
			cairo_set_source_rgba(cr, clr.r, clr.g, clr.b, 0.05);
			cairo_fill_preserve(cr);
			cairo_set_line_width(cr, 2.0);
			cairo_stroke(cr);
			cairo_restore(cr);
		}
		// Draw icon
		if (sc.icon_filename != NULL) {
			cairo_surface_t *img =
				(cairo_surface_t *)map_get(app->images, sc.icon_filename);
			if (img != NULL) {
				draw_img_square(
					cr, img, x + (column_width - 32) / 2, y + 8, 32
				);
			}
		}
		// Draw text
		cairo_set_source_rgb(cr, clr.r, clr.g, clr.b);
		cairo_select_font_face(
			cr, "Noto Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL
		);
		cairo_set_font_size(cr, 14);
		draw_text_centered(
			cr, sc.name,
			(struct point){
				.x = x + (column_width / 2),
				.y = (y + 32 + 16 + 6) + 6,
			}
		);
		// Position adjustment
		if (x + column_width >= bounds.x + bounds.width) {
			y += row_height + gap;
			x = bounds.x;
		} else {
			x += column_width;
		}
		count++;
	}
}

void draw_options(cairo_t *cr, struct app *app, struct rect bounds) {
	double x, y, spacing, icon_size;
	// We draw from right to left
	spacing = 15;
	icon_size = 24;
	x = bounds.x + bounds.width - icon_size - 10;
	y = bounds.y + bounds.height - icon_size;

	RsvgHandle *icons[4] = {
		app->svg_power_off, app->svg_restart, app->svg_sleep, app->svg_lock,
		// app->svg_exit,
	};

	struct rect rect;
	int padding = 5;
	for (int i = 0; i < 4; i++) {
		rect = (struct rect){
			.x = x,
			.y = y,
			.width = icon_size,
			.height = icon_size,
		};
		// draw background circle highlight
		if (rect_contains(rect, app->input.pos)) {
			cairo_save(cr);
			path_rounded_rect(
				cr, rect.x - padding, rect.y - padding,
				rect.width + padding * 2, rect.height + padding * 2, 3
			);
			cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.2);
			cairo_fill_preserve(cr);
			cairo_stroke(cr);
			cairo_restore(cr);
		}
		draw_svg_square(cr, icons[i], x, y, icon_size);

		struct hitzone *hzone = malloc(sizeof(struct hitzone));
		hzone->rect = rect;
		hzone->event = (struct custom_event){
			.type = EV_BUILTIN_OPT,
			.option_type = i,
		};
		list_append(app->hitzones, hzone);

		x -= spacing + icon_size;
	}
	// Draw page count
	char str[100];
	sprintf(str, "Page %d/%d", app->page + 1, app->page_max + 1);
	cairo_select_font_face(
		cr, "Noto Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD
	);
	cairo_set_font_size(cr, 14);
	cairo_set_source_rgb(cr, 1, 1, 1);
	draw_text(cr, str, bounds.x + 10, bounds.y + bounds.height - 6);
}

void update_power(struct app *app) {
	app->battery_charging = false;
	unsigned int count, total, sum;
	total = 0;
	sum = 0;
	char **list = batteries_list(&count);
	// printf("got %d batteries\n", count);
	for (int i = 0; i < count; i++) {
		char *bat = list[i];
		int cap = battery_get_capacity(bat);
		if (cap >= 0) {
			total++;
			sum += cap;
			enum battery_status status = battery_get_status(bat);
			if (status == BS_CHARGING)
				app->battery_charging = true;
		}
		free(bat);
	}
	free(list);
	if (total != 0) {
		app->battery_percent = (float)sum / (float)total;
	}
}

void update_wifi(struct app *app) {
	app->wifi_found = false;
	iwrange range;
	char *device = "wlan0"; // TODO select first active wlan device
	int sock = iw_sockets_open();
	if (sock == -1 || iw_get_range_info(sock, device, &range) < 0) {
		return;
	}
	iwstats stats;
	wireless_config wc;
	if (iw_get_basic_config(sock, device, &wc) == 0) {
		app->wifi_found = true;
		strcpy(app->wifi_ssid, wc.essid); // TODO copy ssid str by max len...
		iw_get_stats(sock, device, &stats, &range, true);
		app->wifi_signal = (int8_t)stats.qual.level;
	}
	iw_sockets_close(sock);
}
