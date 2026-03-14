# Control Adaptation Guide for Nintendo Switch

This guide explains how the original PC controls were adapted for Nintendo Switch hardware in the Minecraft PE port.

## Overview

The original PC version used keyboard and mouse input. For Switch, we mapped these to Joy-Con controls while maintaining the original game logic.

## Input System Architecture

### Original PC Input Flow
- Keyboard keys (WASD, Space, etc.) → KeyboardInput class
- Mouse movement → MouseTurnInput class (camera)
- Mouse clicks → MouseBuildInput class (build/destroy)

### Switch Adaptation
Instead of creating new input classes, we feed Joy-Con inputs into the existing PC input system:

1. **Left Stick** → Generates WASD key events
2. **Right Stick** → Generates mouse delta events
3. **Buttons** → Generate keyboard key events or mouse button events
4. **Touchscreen** → Generates mouse events (for UI)

## Hardware to Software Mapping

### Left Stick (Movement)
```
Physical Input    →    Generated Event    →    Game Action
Up               →    W key              →    Forward
Down             →    S key              →    Backward
Left             →    A key              →    Left
Right            →    D key              →    Right
```

### Right Stick (Camera)
```
Physical Input    →    Generated Event    →    Game Action
Horizontal axis   →    Mouse X delta      →    Camera yaw
Vertical axis     →    Mouse Y delta      →    Camera pitch
```

### Buttons
```
Physical Button   →    Generated Event    →    Game Action
A                 →    Space key          →    Jump
B                 →    Shift key          →    Sneak
X                 →    F5 key             →    Debug/Perspective toggle
Y                 →    Q key              →    Drop item
Minus (-)         →    E key              →    Inventory/Craft
Plus (+)          →    Escape key         →    Pause/Back
L3 (Left stick)   →    Shift key          →    Sneak
ZL                →    Mouse wheel left   →    Hotbar scroll left
ZR                →    Mouse wheel right  →    Hotbar scroll right
R                 →    Mouse left + 88 key →    Attack/Mine
L                 →    Mouse right        →    Use/Place
```

## Implementation Details

### Main Implementation File
`handheld/src/main_switch_sdl.cpp` - Contains the main input remapping logic

### Key Code Sections

#### Stick Input Processing
```cpp
// Left stick → WASD keys
if (gameController->axes[SDL_CONTROLLER_AXIS_LEFTX] < -8000) keys[KEY_A] = true;
if (gameController->axes[SDL_CONTROLLER_AXIS_LEFTX] > 8000) keys[KEY_D] = true;
if (gameController->axes[SDL_CONTROLLER_AXIS_LEFTY] < -8000) keys[KEY_W] = true;
if (gameController->axes[SDL_CONTROLLER_AXIS_LEFTY] > 8000) keys[KEY_S] = true;

// Right stick → Mouse delta
mouseX += gameController->axes[SDL_CONTROLLER_AXIS_RIGHTX] / 16384.0f * 8.0f;
mouseY += gameController->axes[SDL_CONTROLLER_AXIS_RIGHTY] / 16384.0f * 8.0f;
```

#### Button Input Processing
```cpp
// R trigger → Mouse left + keyboard 88 (for attack + mining)
if (gameController->buttons[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER]) {
    mouseButtons[SDL_BUTTON_LEFT] = true;
    keys[88] = true; // keyDestroy
}

// L trigger → Mouse right (for use/place)
if (gameController->buttons[SDL_CONTROLLER_BUTTON_LEFTSHOULDER]) {
    mouseButtons[SDL_BUTTON_RIGHT] = true;
}

// Face buttons
if (gameController->buttons[SDL_CONTROLLER_BUTTON_A]) keys[KEY_SPACE] = true;      // Jump
if (gameController->buttons[SDL_CONTROLLER_BUTTON_B]) keys[KEY_SHIFT] = true;     // Sneak
if (gameController->buttons[SDL_CONTROLLER_BUTTON_X]) keys[KEY_F5] = true;        // Debug toggle
if (gameController->buttons[SDL_CONTROLLER_BUTTON_Y]) keys[KEY_Q] = true;         // Drop item

// System buttons
if (gameController->buttons[SDL_CONTROLLER_BUTTON_BACK]) keys[KEY_ESCAPE] = true; // Plus button
if (gameController->buttons[SDL_CONTROLLER_BUTTON_START]) keys[KEY_E] = true;     // Minus button

// Stick clicks
if (gameController->buttons[SDL_CONTROLLER_BUTTON_LEFTSTICK]) keys[KEY_SHIFT] = true; // L3 sneak

// Triggers for hotbar
if (gameController->buttons[SDL_CONTROLLER_BUTTON_TRIGGERLEFT]) {
    // ZL - scroll hotbar left
    minecraft->getGui().getSelectedSlot()--;
    if (minecraft->getGui().getSelectedSlot() < 0) 
        minecraft->getGui().setSelectedSlot(8);
}
if (gameController->buttons[SDL_CONTROLLER_BUTTON_TRIGGERRIGHT]) {
    // ZR - scroll hotbar right
    minecraft->getGui().getSelectedSlot()++;
    if (minecraft->getGui().getSelectedSlot() > 8) 
        minecraft->getGui().setSelectedSlot(0);
}
```

#### Complete Input Loop Structure
```cpp
// Main input processing loop (executed each frame)
void processInput() {
    // Clear previous state
    memset(keys, 0, sizeof(keys));
    memset(mouseButtons, 0, sizeof(mouseButtons));
    mouseX = mouseY = 0;

    // Process controller if connected
    if (gameController) {
        // Read current controller state
        SDL_GameControllerUpdate();
        
        // Process sticks
        processSticks();
        
        // Process buttons
        processButtons();
        
        // Process triggers
        processTriggers();
    }

    // Feed events to game's input system
    feedKeyboardEvents(keys);
    feedMouseEvents(mouseX, mouseY, mouseButtons);
}
```

### Platform Configuration
`handheld/src/AppPlatform_switch_sdl.cpp` - Disables touchscreen support to ensure proper mouse/camera behavior

```cpp
bool AppPlatform_switch::supportsTouchscreen() {
    return false; // Forces PC-style input
}
```

#### Controller Initialization
```cpp
// Initialize SDL controller system
if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
    printf("SDL controller initialization failed: %s\n", SDL_GetError());
    return -1;
}

// Open first available controller
for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_IsGameController(i)) {
        gameController = SDL_GameControllerOpen(i);
        if (gameController) {
            printf("Controller opened: %s\n", SDL_GameControllerName(gameController));
            break;
        }
    }
}

if (!gameController) {
    printf("No controller found\n");
    return -1;
}
```

#### Key Constants Mapping
```cpp
// Key constants from KeyboardInput.h
#define KEY_W 87      // Forward
#define KEY_A 65      // Left
#define KEY_S 83      // Backward
#define KEY_D 68      // Right
#define KEY_SPACE 32  // Jump
#define KEY_SHIFT 16  // Sneak
#define KEY_ESCAPE 27 // Pause/Back
#define KEY_E 69      // Inventory/Craft
#define KEY_Q 81      // Drop item
#define KEY_F5 116    // Debug toggle
#define 88 88         // Destroy/Mine (custom key)
```

## Special Cases

### R Trigger Dual Function
The R trigger feeds both:
- Mouse left button (for entity attacks on first press)
- Keyboard key 88 (for continuous mining while held)

This ensures both attack and mining work correctly.

### Creative Flight
In creative mode, holding B (Shift) while jumping with A (Space) enables flight by feeding the original PC flight controls.

```cpp
// Creative flight detection in KeyboardInput.cpp
void KeyboardInput::tick(Minecraft *minecraft) {
    // Check for flight toggle (creative mode only)
    if (minecraft->player->getGameType() == GAMETYPE_CREATIVE) {
        if (keys[KEY_JUMP] && keys[KEY_SHIFT]) {
            // Toggle flight mode
            minecraft->player->setFlying(!minecraft->player->isFlying());
        }
    }
    
    // Handle flight movement
    if (minecraft->player->isFlying()) {
        if (keys[KEY_JUMP]) minecraft->player->moveUp(1.0f);     // Fly up
        if (keys[KEY_SHIFT]) minecraft->player->moveDown(1.0f);   // Fly down
    }
}
```

### UI Navigation
Touchscreen still works for UI elements, generating standard mouse events that the game's UI system already handles.

```cpp
// Touch input processing (fallback for UI)
void processTouchInput() {
    // Get touch state
    int numFingers = SDL_GetNumTouchFingers(SDL_TOUCH_MOUSEID);
    
    for (int i = 0; i < numFingers; i++) {
        SDL_Finger* finger = SDL_GetTouchFinger(SDL_TOUCH_MOUSEID, i);
        if (finger) {
            // Convert touch to mouse coordinates
            int mouseX = (int)(finger->x * windowWidth);
            int mouseY = (int)(finger->y * windowHeight);
            
            // Generate mouse event
            SDL_Event event;
            event.type = SDL_MOUSEMOTION;
            event.motion.x = mouseX;
            event.motion.y = mouseY;
            
            SDL_PushEvent(&event);
        }
    }
}
```

## Adapting for Other Platforms

To adapt these controls for other platforms:

1. **Identify your input API** - Get access to buttons, sticks, and touchscreen
2. **Map to PC equivalents** - Convert your platform's inputs to keyboard/mouse events
3. **Feed existing classes** - Use KeyboardInput, MouseTurnInput, and MouseBuildInput
4. **Disable native input** - Set supportsTouchscreen() to false if needed
5. **Test all actions** - Ensure movement, camera, building, and UI work

### Example: Adapting to Mobile Touch
```cpp
// Mobile virtual joystick implementation
class VirtualJoystick {
public:
    float getX() { return normalizedX; }
    float getY() { return normalizedY; }
    
    void update(int touchX, int touchY) {
        // Calculate distance from center
        float dx = touchX - centerX;
        float dy = touchY - centerY;
        float distance = sqrt(dx*dx + dy*dy);
        
        // Normalize to -1.0 to 1.0 range
        if (distance > deadZone) {
            normalizedX = dx / maxRadius;
            normalizedY = dy / maxRadius;
            
            // Clamp to prevent overflow
            normalizedX = std::max(-1.0f, std::min(1.0f, normalizedX));
            normalizedY = std::max(-1.0f, std::min(1.0f, normalizedY));
        }
    }
};

// Map virtual joystick to WASD
void updateMobileInput() {
    if (virtualJoystick.getX() < -0.5f) keys[KEY_A] = true;  // Left
    if (virtualJoystick.getX() > 0.5f) keys[KEY_D] = true;   // Right
    if (virtualJoystick.getY() < -0.5f) keys[KEY_W] = true;  // Forward
    if (virtualJoystick.getY() > 0.5f) keys[KEY_S] = true;   // Backward
}
```

### Example: Adapting to Keyboard Direct
```cpp
// Direct keyboard mapping (no controller)
void processKeyboardInput(SDL_Event* event) {
    switch(event->type) {
        case SDL_KEYDOWN:
            switch(event->key.keysym.scancode) {
                case SDL_SCANCODE_W: keys[KEY_W] = true; break;
                case SDL_SCANCODE_A: keys[KEY_A] = true; break;
                case SDL_SCANCODE_S: keys[KEY_S] = true; break;
                case SDL_SCANCODE_D: keys[KEY_D] = true; break;
                case SDL_SCANCODE_SPACE: keys[KEY_SPACE] = true; break;
                case SDL_SCANCODE_LSHIFT: keys[KEY_SHIFT] = true; break;
                // ect
            }
            break;
            
        case SDL_KEYUP:
            // Handle key release...
            break;
    }
}
```

## Dead Zones and Sensitivity

The implementation uses dead zones to prevent drift:
- Stick dead zone: 8000 (on 32768 range)
- Mouse sensitivity multiplier: 8.0f for right stick

Adjust these values in main_switch_sdl.cpp to fine-tune responsiveness.

```cpp
// Dead zone calculation
#define DEAD_ZONE 8000
#define MAX_AXIS_VALUE 32767
#define SENSITIVITY_MULTIPLIER 8.0f

// Apply dead zone and normalize
float applyDeadZone(int rawValue) {
    if (abs(rawValue) < DEAD_ZONE) return 0.0f;
    
    // Normalize remaining range
    float normalized = (float)(rawValue - (rawValue > 0 ? DEAD_ZONE : -DEAD_ZONE)) / 
                      (MAX_AXIS_VALUE - DEAD_ZONE);
    
    return normalized * SENSITIVITY_MULTIPLIER;
}

// Usage in stick processing
float rightX = applyDeadZone(gameController->axes[SDL_CONTROLLER_AXIS_RIGHTX]);
float rightY = applyDeadZone(gameController->axes[SDL_CONTROLLER_AXIS_RIGHTY]);

mouseX += rightX;
mouseY += rightY;
```

## Button Priority

Some buttons have priority handling:
- B button takes priority for sneaking over UI back
- Minus button opens inventory instead of crafting menu
- L3 stick press also acts as sneak (alternative to B)

This ensures the most important actions are always accessible.

```cpp
// Button priority example in UI handling
void handleUIInput() {
    // Check for B button (sneak) first
    if (gameController->buttons[SDL_CONTROLLER_BUTTON_B]) {
        if (isInGame()) {
            // In game: B = sneak
            keys[KEY_SHIFT] = true;
        } else {
            // In UI: B = back
            keys[KEY_ESCAPE] = true;
        }
    }
    
    // Minus button always opens inventory
    if (gameController->buttons[SDL_CONTROLLER_BUTTON_START]) {
        keys[KEY_E] = true; // Inventory
    }
}
```
