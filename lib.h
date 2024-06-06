#include <librsvg/rsvg.h>

#include "audio.h"
#include "draw.h"
#include "map/hash.h"
#include "map/list.h"
#include "waywrap/waywrap.h"

#define KDATA_DIR "/usr/local/share/kallos/data/"

#define FREE_SVG(NAME)                                                         \
	if (app->svg_##NAME != NULL)                                               \
		g_object_unref(app->svg_##NAME);

enum EV_TYPE {
	EV_RUN,
	EV_SHORTCUT,
	EV_BUILTIN_OPT,
};

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
	int32_t scroll;
	int32_t scroll_scan;
};

struct custom_event {
	enum EV_TYPE type;
	struct custom_event *next;

	// TODO unionize custom_event
	struct shortcut *shortcut;
	int option_type;
};

struct hitzone {
	struct rect rect;
	struct custom_event event;
};

struct app {
	struct audio audio;
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
	RsvgHandle *svg_lock;
	RsvgHandle *svg_power_off;
	RsvgHandle *svg_restart;
	RsvgHandle *svg_search;
	RsvgHandle *svg_sleep;
	struct map *images;
	char search_str[2048];
	float battery_percent;
	bool battery_charging;
	bool wifi_found;
	int8_t wifi_signal;
	char wifi_ssid[512];
	int page, page_max;
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
struct custom_event *
add_cevent(struct custom_event *head, struct custom_event new);
void free_cevent(struct custom_event *ev, bool chain);
