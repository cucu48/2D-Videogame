#include <iostream>
#include "Audio.h"

// miniaudio's implementation is compiled only in this file, without our strict warnings
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#include "third_party/miniaudio.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


using namespace std;


struct Audio::Impl
{
	ma_context context;
	ma_engine engine;
	ma_sound music;
	bool bMusicLoaded;
};


Audio::Audio()
{
	impl = NULL;
	bInitialized = false;
	bFailed = false;
}

Audio::~Audio()
{
	if(impl != NULL)
	{
		stopMusic();
		if(bInitialized)
		{
			ma_engine_uninit(&impl->engine);
			ma_context_uninit(&impl->context);
		}
		delete impl;
	}
}

// The engine is created the first time a sound is needed
bool Audio::init()
{
	if(bInitialized)
		return true;
	if(bFailed)
		return false;
	impl = new Impl();
	impl->bMusicLoaded = false;

	// Only real outputs: without this, miniaudio silently falls back to its
	// "null" backend (plays nothing) when it can't reach the sound server.
	// In WSL the sound goes through PulseAudio (WSLg).
	ma_backend backends[] = { ma_backend_pulseaudio, ma_backend_alsa, ma_backend_jack,
	                          ma_backend_wasapi, ma_backend_coreaudio };
	if(ma_context_init(backends, sizeof(backends) / sizeof(backends[0]), NULL, &impl->context) != MA_SUCCESS)
	{
		cerr << "Audio: no audio output found (PulseAudio/ALSA), playing without sound" << endl;
		bFailed = true;
		return false;
	}
	ma_engine_config config = ma_engine_config_init();
	config.pContext = &impl->context;
	if(ma_engine_init(&config, &impl->engine) != MA_SUCCESS)
	{
		cerr << "Audio: could not open the audio device (" << ma_get_backend_name(impl->context.backend)
		     << "), playing without sound" << endl;
		ma_context_uninit(&impl->context);
		bFailed = true;
		return false;
	}
	cerr << "Audio: using " << ma_get_backend_name(impl->context.backend) << endl;
	bInitialized = true;
	return true;
}

bool Audio::playMusic(const string &file, bool loop)
{
	if(!init())
		return false;
	stopMusic();
	if(ma_sound_init_from_file(&impl->engine, file.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &impl->music) != MA_SUCCESS)
	{
		cerr << "Audio: could not load " << file << endl;
		return false;
	}
	impl->bMusicLoaded = true;
	ma_sound_set_looping(&impl->music, loop ? MA_TRUE : MA_FALSE);
	ma_sound_start(&impl->music);
	return true;
}

void Audio::stopMusic()
{
	if(impl == NULL || !impl->bMusicLoaded)
		return;
	ma_sound_stop(&impl->music);
	ma_sound_uninit(&impl->music);
	impl->bMusicLoaded = false;
}
