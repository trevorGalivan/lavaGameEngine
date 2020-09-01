#pragma once
#include "system.h"
#include <memory>

namespace SoLoud {
	class Soloud;
	class AudioSource;
}
class Engine;
class SoundSystem : System
{
	std::map<std::string, std::unique_ptr<SoLoud::AudioSource>> sources;
	std::unique_ptr<SoLoud::Soloud> gSoloudptr;
public:
	void playSound(std::string path); // start playing a sound file, with no way to stop or modify the sound. Tries to load sound if not alraedy loaded
	void loadSound(std::string path); // load a sound without playing (useful to avoid lag spike on first playing). Does nothing if sound already loaded
	SoundSystem();
	~SoundSystem();
	void update(Engine& engine, double dt);
	void draw(Engine& engine, double dt); // Does NOT swap buffers
};

