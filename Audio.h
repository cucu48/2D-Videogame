#ifndef _AUDIO_INCLUDE
#define _AUDIO_INCLUDE


#include <string>
#include <set>
#include <map>
#include <vector>



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
	void pushMusic(const std::string &file, bool loop);
	void popMusic();
	const std::string &currentMusic() const { return musicFile; }
	bool currentMusicLoops() const { return bMusicLoop; }

	void setVolume(float v);
	float getVolume() const { return volume; }
	void setMusicDucked(bool ducked);
	void setMusicHurry(bool hurry);

	void playSound(const std::string &file);
	void preload(const std::string &file);

private:
	bool init();
	void dropPaused();
	bool resolve(const std::string &file, std::string &path);
	void *newVoice(const std::string &file);
	void applyMusicVolume();
	static float volumeGain(float v);

private:
	struct Impl;
	Impl *impl;
	bool bInitialized, bFailed;
	std::string musicFile;
	std::set<std::string> warned;
	std::map<std::string, std::vector<void *> > voices;
	bool bMusicLoop;
	float volume;
	bool bDucked;
	bool bHurry;

};


#endif
