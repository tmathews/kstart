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
	free(app->fmt_time);
	free(app);
}

void datestr(char *str, const char *fmt) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(str, 100, fmt, t);
}


