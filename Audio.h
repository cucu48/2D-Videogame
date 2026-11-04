#ifndef _AUDIO_INCLUDE
#define _AUDIO_INCLUDE


#include <string>
#include <set>
#include <map>
#include <vector>


// Audio is a singleton that plays sounds with miniaudio (third_party/miniaudio.h).
// It has one music channel (playing a new track stops the previous one) and
// fire-and-forget sound effects that can overlap.
// Names without extension (e.g. "sounds/music_stage") are looked up as .wav,
// .mp3 and .flac, in that order (see sounds/LEEME.txt for the list of files).
// If the audio device or a file can't be opened it prints a warning once and
// the game keeps running without that sound.


class Audio
{
private:
	Audio();

public:
	static Audio &instance()
	{
		static Audio A;
		return A;
	}
	~Audio();

	// If the same track is already playing it is not restarted
	bool playMusic(const std::string &file, bool loop);
	void stopMusic();
	const std::string &currentMusic() const { return musicFile; }
	bool currentMusicLoops() const { return bMusicLoop; }

	void playSound(const std::string &file);
	// Loads a sound effect into memory now, so the first play has no delay
	void preload(const std::string &file);

private:
	bool init();
	bool resolve(const std::string &file, std::string &path); // adds the extension; false if missing
	void *newVoice(const std::string &file);                   // ma_sound* ready to play, or NULL

private:
	struct Impl;
	Impl *impl;
	bool bInitialized, bFailed;
	std::string musicFile;
	std::set<std::string> warned;   // missing files already reported
	// Sound effects decoded in memory: several voices per file so they can overlap
	std::map<std::string, std::vector<void *> > voices;
	bool bMusicLoop;

};


#endif // _AUDIO_INCLUDE
