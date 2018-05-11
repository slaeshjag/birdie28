#include "sfx.h"
#include "main.h"

void sfx_init() {
	s->sfx.sound[SFX_WALK] = d_sound_streamed_load("res/sfx/walk.ogg", DARNIT_SOUND_PRELOAD, 1);
	s->sfx.sound[SFX_APPLE] = d_sound_streamed_load("res/sfx/apple.ogg", DARNIT_SOUND_PRELOAD, 1);
}


void sfx_play(SfxSound sound) {
	d_sound_play(s->sfx.sound[sound], -1, 100, 100, 0);
	return;
}
