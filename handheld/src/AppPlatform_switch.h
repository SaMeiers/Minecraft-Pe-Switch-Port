#ifndef APPPLATFORM_SWITCH_H__
#define APPPLATFORM_SWITCH_H__

#include "AppPlatform.h"
#include "platform/log.h"
#include <cmath>
#include <fstream>
#include <sstream>

#ifdef __SWITCH__
#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

class AppPlatform_switch: public AppPlatform
{
public:
    AppPlatform_switch();
    virtual ~AppPlatform_switch();

	BinaryBlob readAssetFile(const std::string& filename) override;
    void saveScreenshot(const std::string& filename, int glWidth, int glHeight) override;
    TextureData loadTexture(const std::string& filename_, bool textureFolder) override;
    std::string getDateString(int s) override;
	int checkLicense() override;
	bool supportsTouchscreen() override;
	bool isPowerVR() override;
	int getScreenWidth() override;
	int getScreenHeight() override;
	
	virtual void createUserInput() override;
	virtual int getUserInputStatus() override;
	virtual StringVector getUserInput() override;
	
	static SDL_Window* getSDLWindow();
	static SDL_Renderer* getSDLRenderer();
	static void cleanupSDL();
private:
	int p_userInputStatus;
	StringVector p_userInput;
};

#endif /*APPPLATFORM_SWITCH_H__*/
