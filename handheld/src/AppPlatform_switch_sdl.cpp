#include "AppPlatform_switch.h"
#include "client/renderer/Textures.h"
#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int getRemainingFileSize(FILE* fp);

extern SDL_Window* g_window;

AppPlatform_switch::AppPlatform_switch() : p_userInputStatus(-1)
{
    LOGI("AppPlatform_switch initialized");
}

AppPlatform_switch::~AppPlatform_switch()
{

}

BinaryBlob AppPlatform_switch::readAssetFile(const std::string& filename) {

    FILE* fp = fopen(("romfs:/" + filename).c_str(), "rb");
    if (!fp)
        return BinaryBlob();

    int size = getRemainingFileSize(fp);

    BinaryBlob blob;
    blob.size = size;
    blob.data = new unsigned char[size];

    fread(blob.data, 1, size, fp);
    fclose(fp);

    return blob;
}

void AppPlatform_switch::saveScreenshot(const std::string& filename, int glWidth, int glHeight) {

}

TextureData AppPlatform_switch::loadTexture(const std::string& filename_, bool textureFolder)
{
    TextureData out;
    
    std::string filename = textureFolder ? "romfs:/images/" + filename_ : "romfs:/" + filename_;
    
    SDL_Surface* surface = IMG_Load(filename.c_str());
    
    if (surface) {
        out.w = surface->w;
        out.h = surface->h;
        out.data = new unsigned char[4 * out.w * out.h];
        out.memoryHandledExternally = false;
        
        SDL_Surface* rgba_surface = SDL_CreateRGBSurfaceWithFormat(0, out.w, out.h, 32, SDL_PIXELFORMAT_RGBA32);
        
        if (rgba_surface) {
            SDL_BlitSurface(surface, nullptr, rgba_surface, nullptr);
            
            memcpy(out.data, rgba_surface->pixels, 4 * out.w * out.h);
            
            SDL_FreeSurface(rgba_surface);
        }
        
        SDL_FreeSurface(surface);
        LOGI("Loaded texture: %s (%dx%d)", filename.c_str(), out.w, out.h);
    } else {
        LOGI("Couldn't load texture: %s - %s", filename.c_str(), IMG_GetError());
        
        out.w = 64;
        out.h = 64;
        out.data = new unsigned char[4 * out.w * out.h];
        out.memoryHandledExternally = false;
        
        for (int y = 0; y < out.h; y++) {
            for (int x = 0; x < out.w; x++) {
                int index = (y * out.w + x) * 4;
                bool checker = ((x / 8) + (y / 8)) % 2;
                out.data[index + 0] = checker ? 255 : 128; 
                out.data[index + 1] = checker ? 192 : 64;  
                out.data[index + 2] = checker ? 203 : 255;  
                out.data[index + 3] = 255;                 
            }
        }
    }
    
    return out;
}

std::string AppPlatform_switch::getDateString(int s) {
    std::stringstream ss;
    ss << s << " s (UTC)";
    return ss.str();
}

void AppPlatform_switch::createUserInput() {
    SwkbdConfig kbd;
    Result rc = swkbdCreate(&kbd, 0);
    if (R_SUCCEEDED(rc)) {
        swkbdConfigMakePresetDefault(&kbd);
        swkbdConfigSetHeaderText(&kbd, "Enter World Name:");
        
        char tmpoutstr[256];
        tmpoutstr[0] = '\0';
        
        rc = swkbdShow(&kbd, tmpoutstr, sizeof(tmpoutstr));
        
        if (R_SUCCEEDED(rc)) {
            p_userInputStatus = 1;
            p_userInput.clear();
            p_userInput.push_back(std::string(tmpoutstr));
            p_userInput.push_back(""); 
        } else {
            p_userInputStatus = 0; 
        }
        swkbdClose(&kbd);
    } else {
        p_userInputStatus = 0; 
    }
}

int AppPlatform_switch::getUserInputStatus() {
	int status = p_userInputStatus;
	if (status != -1) {
		p_userInputStatus = -1; 
	}
	return status;
}

StringVector AppPlatform_switch::getUserInput() {
	return p_userInput;
}

int AppPlatform_switch::checkLicense() {
    static int _z = 0;
    _z++;
    if (_z > 2) return 1;
    return 0;
}

bool AppPlatform_switch::supportsTouchscreen() {
    return false; 
}

bool AppPlatform_switch::isPowerVR() {
    return false;
}

int AppPlatform_switch::getScreenWidth() {
    return 1280;
}

int AppPlatform_switch::getScreenHeight() {
    return 720;
}

SDL_Window* AppPlatform_switch::getSDLWindow() {
    return g_window;
}

SDL_Renderer* AppPlatform_switch::getSDLRenderer() {
    return nullptr; 
}

void AppPlatform_switch::cleanupSDL() {
}
