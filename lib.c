#include <stdbool.h>
#include <wordexp.h>
#include "lib.h"

bool is_valid_char(const char *str) {
	return strlen(str) > 0;
}

RsvgHandle *load_svg(const char *filename) {
	RsvgHandle *handle = rsvg_handle_new_from_file(filename, NULL);
	// Set the default DPI, you should set at draw time for the particular 
	// surface.
	if (handle != NULL)
		rsvg_handle_set_dpi(handle, 72.0);
	return handle;
}

struct app *app_new() {
	struct app *app = malloc(sizeof(struct app));
	app->events = NULL;
	strcpy(app->search_str, "");

	// TODO read colors from kallos config file
	app->clr_main = hex2rgb(0xAB8449);
	app->clr_bg = hex2rgb(0x100F0D);

	app->fmt_time = malloc(128);
	strcpy(app->fmt_time, "%B %e, %Y %H:%M:%S");

	// If these icons don't load, don't worry - we won't draw them.	
	app->svg_battery = load_svg(KDATA_DIR "battery.svg");
	app->svg_bluetooth = load_svg(KDATA_DIR "bluetooth.svg");
	app->svg_exit = load_svg(KDATA_DIR "exit.svg");
	app->svg_power_off = load_svg(KDATA_DIR "shutdown.svg");
	app->svg_restart = load_svg(KDATA_DIR "reboot.svg");
	app->svg_search = load_svg(KDATA_DIR "search.svg");
	app->svg_sleep = load_svg(KDATA_DIR "sleep-mode.svg");
	app->svg_wifi_full = load_svg(KDATA_DIR "wifi-full.svg");
	app->images = map_new(1024);
	// TODO get fallback if env_home is empty, also properly check string size
    //char *env_home = getenv("HOME");
	char *homepath = malloc(2048);
	strcpy(homepath, getenv("HOME"));
	strcat(homepath, "/.config/kallos/shortcuts");
	printf("HOME: %s\n", homepath);
	app->shortcut_head = read_shortcuts(homepath);
	app->shortcut_first = NULL;

	// Load all images :)
	struct shortcut *scp = app->shortcut_head;
	for (;scp != NULL; scp = scp->next) {
		if (scp->icon_filename == NULL)
			continue;
		char *url = scp->icon_filename;
		cairo_surface_t *image = (cairo_surface_t *)map_get(app->images, url);
		if (image != NULL)
			continue;
		// TODO load SVG if ends in ".svg"
		image = cairo_image_surface_create_from_png(url);
		if (image != NULL) {
			map_set(app->images, url, image);
		}
	}

	return app;
}

void app_free(struct app *app) {
	FREE_SVG(battery);
	FREE_SVG(bluetooth);
	FREE_SVG(exit);
	FREE_SVG(power_off);
	FREE_SVG(search);
	FREE_SVG(sleep);
	FREE_SVG(wifi_full);
	// TODO free map images
	// TODO free shortcuts
	free(app->fmt_time);
	free(app);
}

void datestr(char *str, const char *fmt) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(str, 100, fmt, t);
}

char *new_str_from(const char *str, int offset, int size) {
	char *nstr = malloc(size-offset);
	memcpy(nstr, str+offset, size-offset);
	return nstr;
}

struct shortcut *new_shortcut() {
	struct shortcut *sc = malloc(sizeof(struct shortcut));
	sc->name = NULL;
	sc->icon_filename = NULL;
	sc->command = NULL;
	sc->next = NULL;
	return sc;
}

struct shortcut *read_shortcuts(char *filename) {
	// 	'prop value of prop here' ->> Key "prop" = "value of proper here"
	// example file:
	// name Steam
	// icon /usr/share/icons/hicolor/256x256/apps/steam.png 
	// command steam
	//
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	struct shortcut *sc, *head;
	sc = NULL;
	head = NULL;
	fp = fopen(filename, "r");
	if (fp == NULL)
		return NULL;
	while ((read = getline(&line, &len, fp)) != -1) {
		line[strcspn(line, "\n")] = 0; // Remove terminator
		if (strstr(line, "name") != NULL) {
			if (sc != NULL) {
				sc->next = new_shortcut();
				sc = sc->next;
			} else {
				sc = new_shortcut();
				head = sc;
			}
			sc->name = new_str_from(line, 5, len);
		} else if (sc != NULL) {
			if (strstr(line, "icon") != NULL) {
				sc->icon_filename = new_str_from(line, 5, len);
			} else if (strstr(line, "command") != NULL) {
				sc->command = new_str_from(line, 8, len);
			}
		}
	}
	fclose(fp);
	if (line)
		free(line);
	return head;
}

bool shortcut_matches(struct shortcut *sc, const char *filter_str) {
	if (strlen(filter_str) == 0)
		return true;
	char *pos = strcasestr(sc->name, filter_str);
	if (pos != NULL) {
		return true;
	}
	return false;
}

struct point get_pointer_position(struct app *app) {
	struct point p;
	if (app->state->active_surface_pointer == NULL)
		return p;
	struct pointer_event pe = *app->state->active_surface_pointer->pointer;
	p.x = wl_fixed_to_int(pe.surface_x);
	p.y = wl_fixed_to_int(pe.surface_y);
	//printf("%d %d\n", p.x, p.y);
	return p;
}

void app_process_inputs(struct app *app) {
	struct inputs *i = &app->input;
	i->released = false;
	if (app->state->active_surface_pointer == NULL) {
		i->active = false;
		return;
	}
	struct pointer_event pe = *app->state->active_surface_pointer->pointer;
	if (pe.time == i->last) {
		return;
	}
	i->active = true;
	i->last = pe.time;
	if (pe.event_mask&POINTER_EVENT_BUTTON) {
		if (pe.button == 272) {
			bool was_pressed = i->pressed;
			i->pressed = pe.state == 1;
			if (was_pressed && !i->pressed) {	
				i->released = true;
			}
		}
	}
	if (pe.event_mask&POINTER_EVENT_MOTION) {
		i->pos.x = wl_fixed_to_int(pe.surface_x);
		i->pos.y = wl_fixed_to_int(pe.surface_y);
	}
}

int run_cmd(const char *str) {
	wordexp_t w;
	switch (wordexp(str, &w, WRDE_NOCMD)) {
		case 0:
			break;
		case WRDE_NOSPACE:
		case WRDE_CMDSUB:
		case WRDE_BADCHAR:
		default:
			return -1;
	}
	if (w.we_wordc < 1) {
		return -1;
	}
	const char *bin = w.we_wordv[0];
	if (!bin || !*bin) {
		return -1;
	}
	if (strchr(bin, '/'))
		execv(bin, w.we_wordv);
	else
		execvp(bin, w.we_wordv);
	return 0;
}

bool app_process_events(struct app *app) {
	bool exit = false;
	struct custom_event *ev = app->events;
	while (ev != NULL) {
		switch (ev->type) {
		case 1:
			run_cmd(ev->shortcut->command);	
			exit = true;
		case 2:
			if (app->shortcut_first != NULL) {
				run_cmd(app->shortcut_first->command);
				exit = true;
			}
		}
		ev = ev->next;
	}
	free_cevent(app->events, true);
	return exit;
}

bool rect_contains(struct rect rect, struct point pos) {
	if (pos.x >= rect.x && pos.x < rect.x + rect.width &&
		pos.y >= rect.y && pos.y < rect.y + rect.height) {
		return true;
	}
	return false;
}

struct custom_event *add_cevent(struct custom_event *head, struct custom_event new) {
	struct custom_event *ev = malloc(sizeof(struct custom_event));
	*ev = new;
	if (head == NULL) {
		return ev;
	}
	struct custom_event *tail = head;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = ev;
	return head;
}

void free_cevent(struct custom_event *ev, bool chain) {
	if (ev == NULL)
		return;
	if (chain) {
		free_cevent(ev->next, chain);
	}
	free(ev);
}
