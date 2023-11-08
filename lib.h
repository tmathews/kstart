#include <librsvg/rsvg.h>
#include "draw.h"
#include "map/hash.h"
#include "map/list.h"
#include "waywrap/waywrap.h"

#define KDATA_DIR "/usr/local/share/kallos/data/"

#define FREE_SVG(NAME) if (app->svg_##NAME != NULL)\
	g_object_unref(app->svg_##NAME);

struct shortcut {
	char *name;
	char *command;
	char *icon_filename;
	struct shortcut *next;
};

struct inputs {
	bool active;
	struct point pos;
	bool pressed, released;
	uint32_t last;
};

struct custom_event {
	int type;
	struct custom_event *next;
	
	// TODO unionize custom_event
	struct shortcut *shortcut;
};

struct hitzone {
	struct rect rect;
	struct custom_event event;
};

struct app {
	struct client_state *state;
	struct inputs input;
	struct custom_event *events;
	struct list *hitzones;
	struct color clr_main, clr_bg;
	char *fmt_time;
	struct shortcut *shortcut_head;
	struct shortcut *shortcut_first;
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
	struct map *images;
	char search_str[2048];
	float battery_percent;
	bool battery_charging;
};

struct app *app_new();
void app_free(struct app *app);
void app_process_inputs(struct app *app);
bool app_process_events(struct app *app);
bool is_valid_char(const char *);
RsvgHandle *load_svg(const char *);
void datestr(char *str, const char *fmt);
struct shortcut *read_shortcuts(char *filename);
bool shortcut_matches(struct shortcut *sc, const char *filter_str);
bool rect_contains(struct rect, struct point);
struct custom_event *add_cevent(struct custom_event *head, struct custom_event new);
void free_cevent(struct custom_event *ev, bool chain);
