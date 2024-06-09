#include <pulse/def.h>
#include <pulse/mainloop.h>
#include <stdio.h>
#include "audio.h"

static void audio_sinkinfo_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
	if (!i) return;
	struct audio *a = (struct audio *)userdata;
	a->volume = (float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM;
	a->muted = i->mute;
}

static void audio_serverinfo_cb(pa_context *c, const pa_server_info *i, void *userdata) {
	//printf("default sink name = %s\n", i->default_sink_name);
	pa_context_get_sink_info_by_name(c, i->default_sink_name, audio_sinkinfo_cb, userdata);
}

static void audio_subscribe_cb(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata) {
	pa_operation *op = NULL;
	unsigned facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
	switch (facility) {
	    case PA_SUBSCRIPTION_EVENT_SINK:
	        op = pa_context_get_sink_info_by_index(c, idx, audio_sinkinfo_cb, userdata);
	        break;
	}
	if (op) pa_operation_unref(op);
}

static void audio_ctxstate_cb(pa_context *c, void *userdata) {
	//struct audio* a = (struct audio*)userdata;
	switch (pa_context_get_state(c)) {
	    case PA_CONTEXT_CONNECTING:
	    case PA_CONTEXT_AUTHORIZING:
	    case PA_CONTEXT_SETTING_NAME:
	        break;
	    case PA_CONTEXT_READY:
	        printf("PulseAudio connection established.\n");
	        pa_context_get_server_info(c, audio_serverinfo_cb, userdata);
	        pa_context_set_subscribe_callback(c, audio_subscribe_cb, userdata);
	        pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
	        break;
	    case PA_CONTEXT_TERMINATED:
	    case PA_CONTEXT_FAILED:
	    default:
	        break;
	}
}

static void audio_exit_cb(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata) {
	struct audio *a = (struct audio*)userdata;
	if (a) a->api->quit(a->api, 0);
}

int audio_process(struct audio *a) {
	int val;
	pa_mainloop_iterate(a->loop, 0, &val);
	return val;
}

bool audio_init(struct audio *a) {
	a->loop = pa_mainloop_new();
	if (!a->loop) {
		return false;
	}
	a->api = pa_mainloop_get_api(a->loop);
	if (!a->api) {
		return false;
	}
	a->ctx = pa_context_new(a->api, "kstart");
	if (!a->ctx) {
		return false;
	}
    if (pa_context_connect(a->ctx, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
		return false;
	}
	pa_context_set_state_callback(a->ctx, audio_ctxstate_cb, a);
	printf("audio init success!\n");
	return true;
}

void audio_destory(struct audio *a) {
	if (a->ctx) {
		pa_context_unref(a->ctx);
	}
	if (a->loop) {
		pa_mainloop_free(a->loop);
	}
}
