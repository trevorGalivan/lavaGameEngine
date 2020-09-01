#include "soundSystem.h"

#include "fileUtils.h"

#define WITH_MINIAUDIO
#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>

#include <map>

SoundSystem::SoundSystem() 
{
	gSoloudptr = std::make_unique< SoLoud::Soloud >();
	gSoloudptr->init();
}

SoundSystem::~SoundSystem()
{
	sources.clear();
	gSoloudptr->deinit();
}

void SoundSystem::loadSound(std::string path)
{
	if (!sources.contains(path)) { // check to make sure sound not already loaded
		auto dotIndex = path.rfind('.');
		std::string extension;
		if (dotIndex != std::string::npos) {
			extension = path.substr(dotIndex + 1);
		}

		if (extension == "wav") {
			std::unique_ptr<SoLoud::Wav> wav = std::make_unique<SoLoud::Wav>();
			wav->load((fileUtils::getAssetsDirectory() + path).c_str());

			sources.insert(std::pair(path, std::move(wav)));
		}
	}
}

void SoundSystem::playSound(std::string path) // start playing a sound file, with no way to stop or modify the sound
{
	loadSound(path);
	gSoloudptr->play(*sources.at(path));
}