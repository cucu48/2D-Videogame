#ifndef _AUDIO_INCLUDE
#define _AUDIO_INCLUDE


#include <string>


// Audio is a singleton that plays sounds with miniaudio (third_party/miniaudio.h).
// For now it has a single music channel: playing a new track stops the previous one.
// If the audio device or the file can't be opened it just prints a warning and
// the game keeps running without sound.


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

	bool playMusic(const std::string &file, bool loop);
	void stopMusic();

private:
	bool init();

private:
	struct Impl;
	Impl *impl;
	bool bInitialized, bFailed;

};


#endif // _AUDIO_INCLUDE
