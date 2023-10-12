#include <stdbool.h>
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

	// TODO read colors from kallos config file
	app->clr_main = hex2rgb(0xAB8449);
	app->clr_bg = hex2rgb(0x100F0D);

	app->fmt_time = malloc(128);
	strcpy(app->fmt_time, "%B %e, %Y %H:%M");

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
	app->shortcut_head = read_shortcuts("./shortcuts");

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
