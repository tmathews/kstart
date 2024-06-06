#ifndef FILE_audio
#define FILE_audio

#include <pulse/mainloop-signal.h>
#include <stdbool.h>
#include <pulse/mainloop.h>
#include <pulse/pulseaudio.h>

struct audio {
	pa_mainloop *loop;
	pa_mainloop_api *api;
	pa_context *ctx;
	float volume;
	bool muted;
};

bool audio_init(struct audio *a);
int audio_process(struct audio *a);
void audio_destory(struct audio *a);

#endif
