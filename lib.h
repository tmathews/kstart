#include <librsvg/rsvg.h>
#include "draw.h"

#define KDATA_DIR "/usr/local/share/kallos/data/"

#define FREE_SVG(NAME) if (app->svg_##NAME != NULL)\
	g_object_unref(app->svg_##NAME);

struct shortcut {
	char *name;
	char *command;
	char *icon_filename;
};

struct app {
	struct client_state *state;
	struct color clr_main, clr_bg;
	char *fmt_time;
	struct shortcut **shortcuts;
	RsvgHandle *svg_battery;
	RsvgHandle *svg_bluetooth;
	RsvgHandle *svg_bluetooth_off;
	RsvgHandle *svg_exit;
	RsvgHandle *svg_power_off;
	RsvgHandle *svg_restart;
	RsvgHandle *svg_search;
	RsvgHandle *svg_sleep;
	RsvgHandle *svg_wifi_full;
	RsvgHandle *svg_wifi_nosig;
	RsvgHandle *svg_wifi_off;
	RsvgHandle *svg_wifi_ok;
	RsvgHandle *svg_wifi_weak;
};

struct app *app_new();
void app_free(struct app *app);
bool is_valid_char(const char *);
RsvgHandle *load_svg(const char *);
void datestr(char *str, const char *fmt);
