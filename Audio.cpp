#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>
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
	ma_sound *music;        // current track, NULL if none
	ma_sound *paused;       // track put aside by pushMusic, kept at its position
	std::string pausedFile;
	bool pausedLoop;
};


Audio::Audio()
{
	impl = NULL;
	bInitialized = false;
	bFailed = false;
	volume = 1.f;
	bDucked = false;
	bMusicLoop = false;
}

Audio::~Audio()
{
	if(impl != NULL)
	{
		stopMusic();
		dropPaused();
		if(bInitialized)
		{
			for(map<string, vector<void *> >::iterator it = voices.begin(); it != voices.end(); ++it)
				for(size_t i = 0; i < it->second.size(); i++)
				{
					ma_sound_uninit((ma_sound *)it->second[i]);
					delete (ma_sound *)it->second[i];
				}
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
	impl->music = NULL;
	impl->paused = NULL;
	impl->pausedLoop = false;

	// Only real outputs: without this, miniaudio silently falls back to its
	// "null" backend (plays nothing) when it can't reach the sound server.
	// In WSL the sound goes through PulseAudio (WSLg).
	ma_backend backends[] = { ma_backend_pulseaudio, ma_backend_alsa, ma_backend_jack,
	                          ma_backend_wasapi, ma_backend_coreaudio };
	// PulseAudio (WSLg) buffers a lot by default; ask it for low latency
#if !defined(_WIN32)
	setenv("PULSE_LATENCY_MSEC", "30", 0);
#endif
	if(ma_context_init(backends, sizeof(backends) / sizeof(backends[0]), NULL, &impl->context) != MA_SUCCESS)
	{
		cerr << "Audio: no audio output found (PulseAudio/ALSA), playing without sound" << endl;
		bFailed = true;
		return false;
	}
	ma_engine_config config = ma_engine_config_init();
	config.pContext = &impl->context;
	// Small buffer so the sound comes out right when it's played
	config.periodSizeInMilliseconds = 10;
	if(ma_engine_init(&config, &impl->engine) != MA_SUCCESS)
	{
		cerr << "Audio: could not open the audio device (" << ma_get_backend_name(impl->context.backend)
		     << "), playing without sound" << endl;
		ma_context_uninit(&impl->context);
		bFailed = true;
		return false;
	}
	cerr << "Audio: using " << ma_get_backend_name(impl->context.backend) << endl;
	ma_engine_set_volume(&impl->engine, volumeGain(volume));
	bInitialized = true;
	return true;
}

// Finds the file on disk: as given, or adding .wav / .mp3 / .flac
bool Audio::resolve(const string &file, string &path)
{
	const char *exts[] = { "", ".wav", ".mp3", ".flac" };
	for(int i = 0; i < 4; i++)
	{
		path = file + exts[i];
		ifstream f(path.c_str());
		if(f.good())
			return true;
	}
	if(warned.insert(file).second)
		cerr << "Audio: missing " << file << " (.wav/.mp3/.flac)" << endl;
	return false;
}

bool Audio::playMusic(const string &file, bool loop)
{
	if(impl != NULL && impl->music != NULL && file == musicFile && ma_sound_is_playing(impl->music))
		return true;
	string path;
	if(!init())
		return false;
	stopMusic();
	dropPaused();   // another track takes over: the paused one won't come back
	musicFile = file;
	bMusicLoop = loop;
	if(!resolve(file, path))
		return false;
	ma_sound *s = new ma_sound;
	if(ma_sound_init_from_file(&impl->engine, path.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, s) != MA_SUCCESS)
	{
		delete s;
		if(warned.insert(path).second)
			cerr << "Audio: could not load " << path << endl;
		return false;
	}
	impl->music = s;
	applyMusicVolume();
	ma_sound_set_looping(s, loop ? MA_TRUE : MA_FALSE);
	// Optional loop points: "<file>.loop" holds the first and the end frame of
	// the part that repeats, so the intro only sounds once and the loop is seamless
	if(loop)
	{
		ifstream lf((file + ".loop").c_str());
		unsigned long long loopBegin, loopEnd;
		if(lf >> loopBegin >> loopEnd && loopEnd > loopBegin)
			ma_data_source_set_loop_point_in_pcm_frames(ma_sound_get_data_source(s), loopBegin, loopEnd);
	}
	ma_sound_start(s);
	return true;
}

void Audio::stopMusic()
{
	musicFile.clear();
	if(impl == NULL || impl->music == NULL)
		return;
	ma_sound_stop(impl->music);
	ma_sound_uninit(impl->music);
	delete impl->music;
	impl->music = NULL;
}

void Audio::dropPaused()
{
	if(impl == NULL || impl->paused == NULL)
		return;
	ma_sound_uninit(impl->paused);
	delete impl->paused;
	impl->paused = NULL;
}

// Pauses the current track where it is and plays another one on top
void Audio::pushMusic(const string &file, bool loop)
{
	if(!init())
		return;
	dropPaused();
	string prevFile = musicFile;
	bool prevLoop = bMusicLoop;
	ma_sound *prev = impl->music;
	if(prev != NULL)
		ma_sound_stop(prev);   // stop keeps the position: start resumes from there
	impl->music = NULL;
	musicFile.clear();
	playMusic(file, loop);
	impl->paused = prev;
	impl->pausedFile = prevFile;
	impl->pausedLoop = prevLoop;
}

// Stops the pushed track and resumes the paused one from where it was
void Audio::popMusic()
{
	if(impl == NULL)
		return;
	stopMusic();
	if(impl->paused == NULL)
		return;
	impl->music = impl->paused;
	impl->paused = NULL;
	musicFile = impl->pausedFile;
	bMusicLoop = impl->pausedLoop;
	applyMusicVolume();
	ma_sound_start(impl->music);
}

void Audio::setVolume(float v)
{
	volume = v < 0.f ? 0.f : (v > 1.f ? 1.f : v);
	if(bInitialized)
		ma_engine_set_volume(&impl->engine, volumeGain(volume));
}

// El oido es logaritmico: con el volumen lineal casi no se nota la diferencia
// hasta llegar abajo del todo. Cada paso del 10% son 4 dB (100% = 0 dB, 10% = -36 dB).
float Audio::volumeGain(float v)
{
	if(v <= 0.001f)
		return 0.f;
	return powf(10.f, (v - 1.f) * 2.f);
}

void Audio::setMusicDucked(bool ducked)
{
	bDucked = ducked;
	applyMusicVolume();
}

void Audio::applyMusicVolume()
{
	if(impl != NULL && impl->music != NULL)
		ma_sound_set_volume(impl->music, bDucked ? 0.5f : 1.f);
}

// Creates one more voice for a sound effect, fully decoded in memory. The
// first one reads the file (synchronously, not in the background like
// ma_engine_play_sound does, which is what delayed the sounds); the next ones
// share its decoded data, so they are instant.
void *Audio::newVoice(const string &file)
{
	vector<void *> &v = voices[file];
	ma_sound *s = new ma_sound;
	ma_result r;
	if(v.empty())
	{
		string path;
		if(!resolve(file, path))
		{
			delete s;
			return NULL;
		}
		r = ma_sound_init_from_file(&impl->engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, s);
		if(r != MA_SUCCESS && warned.insert(path).second)
			cerr << "Audio: could not load " << path << endl;
	}
	else
		r = ma_sound_init_copy(&impl->engine, (ma_sound *)v[0], 0, NULL, s);
	if(r != MA_SUCCESS)
	{
		delete s;
		return NULL;
	}
	v.push_back(s);
	return s;
}

void Audio::preload(const string &file)
{
	if(init() && voices[file].empty())
		newVoice(file);
}

// Sound effect: plays once and can overlap with others and with the music
void Audio::playSound(const string &file)
{
	const size_t MAX_VOICES = 8;   // the same effect sounding at once
	if(!init())
		return;
	vector<void *> &v = voices[file];
	ma_sound *s = NULL;
	for(size_t i = 0; i < v.size() && s == NULL; i++)
		if(!ma_sound_is_playing((ma_sound *)v[i]))
			s = (ma_sound *)v[i];
	if(s == NULL && v.size() < MAX_VOICES)
		s = (ma_sound *)newVoice(file);
	if(s == NULL)   // missing file, or all voices busy: restart the oldest one
	{
		if(v.empty())
			return;
		s = (ma_sound *)v[0];
	}
	ma_sound_seek_to_pcm_frame(s, 0);
	ma_sound_start(s);
}
