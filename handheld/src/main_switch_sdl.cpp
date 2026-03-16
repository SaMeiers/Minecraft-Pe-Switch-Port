#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <stdio.h>
#include "App.h"
#include "AppPlatform_switch.h"
#include "NinecraftApp.h"
#include "client/sound/SoundEngine.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include "platform/input/Multitouch.h"

static AppPlatform_switch* s_platform = nullptr;
static NinecraftApp* s_app = nullptr;

SDL_Window* g_window = nullptr;
SDL_GLContext g_gl_context = nullptr;

void processInputs(PadState* pad) {
    padUpdate(pad);
    u64 kDown = padGetButtonsDown(pad);
    u64 kUp = padGetButtonsUp(pad);
    u64 kHeld = padGetButtons(pad);

    if (kDown & HidNpadButton_A) { Keyboard::feed(Keyboard::KEY_SPACE, 1); Keyboard::feed(Keyboard::KEY_RETURN, 1); }
    if (kUp & HidNpadButton_A) { Keyboard::feed(Keyboard::KEY_SPACE, 0); Keyboard::feed(Keyboard::KEY_RETURN, 0); }

    if (kDown & HidNpadButton_B) { Keyboard::feed(Keyboard::KEY_LSHIFT, 1); Keyboard::feed(Keyboard::KEY_BACKSPACE, 1); }
    if (kUp & HidNpadButton_B) { Keyboard::feed(Keyboard::KEY_LSHIFT, 0); Keyboard::feed(Keyboard::KEY_BACKSPACE, 0); }

    if (kDown & HidNpadButton_Y) Keyboard::feed(Keyboard::KEY_Q, 1);
    if (kUp & HidNpadButton_Y) Keyboard::feed(Keyboard::KEY_Q, 0);

    if (kDown & HidNpadButton_Plus) Keyboard::feed(Keyboard::KEY_ESCAPE, 1);
    if (kUp & HidNpadButton_Plus) Keyboard::feed(Keyboard::KEY_ESCAPE, 0);

    if (kDown & HidNpadButton_Minus) Keyboard::feed(Keyboard::KEY_E, 1);
    if (kUp & HidNpadButton_Minus) Keyboard::feed(Keyboard::KEY_E, 0);

    if (kDown & HidNpadButton_StickL) Keyboard::feed(Keyboard::KEY_LSHIFT, 1);
    if (kUp & HidNpadButton_StickL) Keyboard::feed(Keyboard::KEY_LSHIFT, 0);

    if (kDown & HidNpadButton_X) Keyboard::feed(Keyboard::KEY_T, 1);
    if (kUp & HidNpadButton_X) Keyboard::feed(Keyboard::KEY_T, 0);

    if (kDown & HidNpadButton_ZL) { Mouse::feed(MouseAction::ACTION_WHEEL, 0, 0, 0, 0, 1); }
    if (kDown & HidNpadButton_ZR) { Mouse::feed(MouseAction::ACTION_WHEEL, 0, 0, 0, 0, -1); }

    if (kDown & HidNpadButton_R) {
        Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_DOWN, 640, 360);
        Keyboard::feed(88, 1);
    }
    if (kUp & HidNpadButton_R) {
        Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_UP, 640, 360);
        Keyboard::feed(88, 0);
    }

    if (kDown & HidNpadButton_L) {
        Mouse::feed(MouseAction::ACTION_RIGHT, MouseAction::DATA_DOWN, 640, 360);
    }
    if (kUp & HidNpadButton_L) {
        Mouse::feed(MouseAction::ACTION_RIGHT, MouseAction::DATA_UP, 640, 360);
    }


    HidAnalogStickState l_stick = padGetStickPos(pad, 0);
    int left_x = l_stick.x;
    int left_y = l_stick.y;
    
    static bool was_w = false, was_s = false, was_a = false, was_d = false;
    
    bool is_w = (left_y > 10000 || (kHeld & HidNpadButton_Up));
    bool is_s = (left_y < -10000 || (kHeld & HidNpadButton_Down));
    bool is_a = (left_x < -10000 || (kHeld & HidNpadButton_Left));
    bool is_d = (left_x > 10000 || (kHeld & HidNpadButton_Right));

    if (is_w && !was_w) { Keyboard::feed(Keyboard::KEY_W, 1); Keyboard::feed(38, 1); }
    if (!is_w && was_w) { Keyboard::feed(Keyboard::KEY_W, 0); Keyboard::feed(38, 0); }
    was_w = is_w;

    if (is_s && !was_s) { Keyboard::feed(Keyboard::KEY_S, 1); Keyboard::feed(40, 1); }
    if (!is_s && was_s) { Keyboard::feed(Keyboard::KEY_S, 0); Keyboard::feed(40, 0); }
    was_s = is_s;

    if (is_a && !was_a) { Keyboard::feed(Keyboard::KEY_A, 1); Keyboard::feed(37, 1); }
    if (!is_a && was_a) { Keyboard::feed(Keyboard::KEY_A, 0); Keyboard::feed(37, 0); }
    was_a = is_a;

    if (is_d && !was_d) { Keyboard::feed(Keyboard::KEY_D, 1); Keyboard::feed(39, 1); }
    if (!is_d && was_d) { Keyboard::feed(Keyboard::KEY_D, 0); Keyboard::feed(39, 0); }
    was_d = is_d;

    HidAnalogStickState r_stick = padGetStickPos(pad, 1);
    int right_x = r_stick.x;
    int right_y = r_stick.y;
    
    short cam_dx = 0;
    short cam_dy = 0;
    
    if (abs(right_x) > 5000) cam_dx = right_x / 2000;
    if (abs(right_y) > 5000) cam_dy = -(right_y / 2000); 
    
    if (cam_dx != 0 || cam_dy != 0) {
        Mouse::feed(MouseAction::ACTION_MOVE, 0, Mouse::getX(), Mouse::getY(), cam_dx, cam_dy);
    }

    HidTouchScreenState state;
    hidGetTouchScreenStates(&state, 1);

    static bool wasTouching = false;
    static int last_x = 0;
    static int last_y = 0;

    if (state.count > 0) {
        last_x = state.touches[0].x;
        last_y = state.touches[0].y;
        
        char touchAction = wasTouching ? MouseAction::ACTION_MOVE : MouseAction::ACTION_LEFT; 
        char touchData = MouseAction::DATA_DOWN;

        Mouse::feed(touchAction, touchData, last_x, last_y);
        Multitouch::feed(touchAction, touchData, last_x, last_y, 0);
        
        wasTouching = true;
    } else if (wasTouching) {
        Mouse::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_UP, last_x, last_y);
        Multitouch::feed(MouseAction::ACTION_LEFT, MouseAction::DATA_UP, last_x, last_y, 0);
        wasTouching = false;
    }
}

bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    g_window = SDL_CreateWindow("Minecraft PE", 
                               SDL_WINDOWPOS_CENTERED, 
                               SDL_WINDOWPOS_CENTERED,
                               1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    
    if (!g_window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    
    g_gl_context = SDL_GL_CreateContext(g_window);
    
    if (!g_gl_context) {
        std::cerr << "GL Context creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return false;
    }
    
    SDL_GL_MakeCurrent(g_window, g_gl_context);
    
    SDL_GL_SetSwapInterval(1);
    
    if (IMG_Init(IMG_INIT_PNG) < 0) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
    }
    
    std::cout << "SDL initialized successfully" << std::endl;
    return true;
}

void cleanupSDL() {
    if (g_gl_context) {
        SDL_GL_DeleteContext(g_gl_context);
        g_gl_context = nullptr;
    }
    
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }
    
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char **argv) {
    FILE* log_file = freopen("minecraft_pe_debug.log", "w", stdout);
    if (!log_file) {
    
    }
    freopen("minecraft_pe_debug.log", "a", stderr);

    setvbuf(stdout, NULL, _IOLBF, 4096);
    setvbuf(stderr, NULL, _IOLBF, 4096);

    printf("--- MINECRAFT PE SWITCH PORT STARTING ---\n");

    printf("[1/9] Initializing Switch services (romfs, pl, sockets)... ");
    romfsInit();
    socketInitializeDefault();
    plInitialize(PlServiceType_User);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    
    PadState pad;
    padInitializeDefault(&pad);
    hidInitializeTouchScreen();
    printf("DONE\n");

    printf("[2/9] Initializing SDL and GL context... ");
    if (!initSDL()) {
        printf("FAILED! Exiting.\n");
        romfsExit();
        plExit();
        return -1;
    }
    printf("DONE\n");

    printf("[3/9] Creating AppPlatform_switch... ");
    s_platform = new AppPlatform_switch();
    printf("DONE\n");

    printf("[4/9] Creating NinecraftApp... ");
    s_app = new NinecraftApp();
    printf("DONE\n");


    printf("[6/9] Preparing AppContext... ");
    AppContext ctx;
#ifndef NO_EGL
    ctx.display = g_window;
    ctx.context = g_gl_context;
    ctx.surface = nullptr;
#endif
    ctx.platform = s_platform;
    ctx.doRender = true;
    printf("DONE\n");

    printf("[7/9] Calling s_app->App::init(ctx)... ");
    s_app->App::init(ctx);
    printf("DONE\n");

    printf("[9/9] Calling s_app->setSize()... ");
    s_app->setSize(1280, 720);
    printf("DONE. Entering Main Loop.\n");

    while (appletMainLoop() && !s_app->wantToQuit()) {

        processInputs(&pad);

        s_app->update();

        SDL_GL_SwapWindow(g_window);
    }

    s_app->destroy();

    delete s_app;
    delete s_platform;

    cleanupSDL();
    plExit();
    socketExit();
    romfsExit();
    return 0;
}
