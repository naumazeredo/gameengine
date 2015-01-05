// TODO(naum): Change cerr to logging
#include <iostream>

// TODO(naum): Remove transcendental functions and reimplement them
#include <cmath>

#include "game.h"

namespace vsge
{

Game::
Game()
{
}

bool Game::
start(std::string name,
      int window_width, int window_height,
      int argc, char** argv)
{
  // TODO(naum): Read args

  name_ = name;

  // SDL initialization
  if (SDL_Init(SDL_INIT_VIDEO |
               SDL_INIT_JOYSTICK |
               SDL_INIT_GAMECONTROLLER |
               SDL_INIT_HAPTIC |
               // TODO(naum): Put Audio into another class
               SDL_INIT_AUDIO) != 0)
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not init: %s\n", SDL_GetError());
    return false;
  }

  // Window instantiation
  window_ = SDL_CreateWindow(
        name.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

  if (!window_)
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created: %s\n", SDL_GetError());
    return false;
  }

  // TODO(naum): Set VSync and linear interpolation

  // Renderer instantiation
  renderer_ = SDL_CreateRenderer(
        window_,
        -1,
        SDL_RENDERER_ACCELERATED);

  if (!renderer_)
  {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created: %s\n", SDL_GetError());
    return false;
  }

  SDL_SetRenderDrawColor(renderer_, 0xff, 0xff, 0xff, 0xff);
  SDL_RenderClear(renderer_);

  // TODO(naum): Don't use a texture as backbuffer (?)
  // Create Backbuffer
  backbuffer_ = SDL_CreateTexture(
      renderer_,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STREAMING,
      SCREEN_WIDTH, SCREEN_HEIGHT);

  backbufferPixels_ = new uint32[SCREEN_WIDTH * SCREEN_HEIGHT] {};
  for (int x = 0; x < SCREEN_WIDTH; ++x)
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
      backbufferPixels_[y * SCREEN_WIDTH + x] = ((y % 256) << 8) + (x % 256);

  // Audio
  // TODO(naum): Create a class for audio
  SDL_AudioSpec audioWant = {};
  SDL_AudioSpec audioHave;
  SDL_AudioDeviceID audioDevice;

  audioWant.freq = 48000;
  //audioWant.format = AUDIO_S16MSB; // NOTE(naum): Must be big-endian (why?)
  audioWant.format = AUDIO_S16LSB;
  audioWant.channels = 2;
  audioWant.samples = 4096;
  audioWant.callback = [] (void* userdata, uint8* stream, int audioLength)
  {
    static uint32 runningSampleIndex = 0;

    int samplesPerSecond = 48000;
    int toneHz = 256;
    int16 toneVolume = 50;
    int wavePeriod = samplesPerSecond / toneHz;
    int bytesPerSample = sizeof(int16) * 2;
    int sampleCount = audioLength / bytesPerSample;

    int16* audioStream = reinterpret_cast<int16*>(stream);
    for (int i = 0; i < sampleCount; ++i)
    {
      int16 sampleValue = 0;

      // NOTE(naum): Sine wave
      float angle = 2 * M_PI * runningSampleIndex++ / wavePeriod;
      while (angle >= 2 * M_PI)
        angle -= 2 * M_PI;
      float sineValue = std::sin(angle);
      sampleValue = static_cast<int16>(sineValue * toneVolume);

      // NOTE(naum): Square wave
      /*
      int halfWavePeriod = wavePeriod / 2;
      sampleValue = ((runningSampleIndex++ / halfWavePeriod) % 2)
        ? toneVolume : -toneVolume;
      */

      //*audioStream++ = sampleValue;
      //*audioStream++ = sampleValue;
      *stream++ = (sampleValue & 0xFF00) >> 16;
      *stream++ = (sampleValue & 0x00FF);

      *stream++ = (sampleValue & 0xFF00) >> 16;
      *stream++ = (sampleValue & 0x00FF);
    }
  };
  audioWant.userdata = 0;

  audioDevice = SDL_OpenAudioDevice(
      NULL, 0,
      &audioWant, &audioHave,
      SDL_AUDIO_ALLOW_FORMAT_CHANGE);

  if (audioDevice == 0)
    std::cout << "Failed to open audio: " << SDL_GetError() << std::endl;
  else
  {
    if (audioHave.format != audioWant.format)
      std::cout << "Could not get the wanted audio format!" << std::endl;
    SDL_PauseAudioDevice(audioDevice, 0);
  }


  // Success
  return true;
}

Game::
~Game()
{
  if (window_)
    SDL_DestroyWindow(window_);

  if (renderer_)
    SDL_DestroyRenderer(renderer_);

  // TODO(naum): Remove this when we remove the texture as backbuffer
  if (backbufferPixels_)
    delete backbufferPixels_;

  SDL_Quit();
}

void Game::
run()
{
  while (isRunning_)
  {
    // Handle Inputs
    handleInputs();

    // Game Logic
    gameUpdate();

    // Rendering
    render();
  }
}

void Game::
handleInputs()
{
  // Handle Inputs
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
    {
      isRunning_ = false;
    }

    handleKeyboard(event);
    handleMouse(event);
    handleJoysticks(event);
  }
}

void Game::
handleKeyboard(SDL_Event event)
{
  // Keyboard support
  // TODO(naum): Gather all information and handle
  // elsewhere. This should be only the API
  if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
  {
    auto key = event.key;
    auto keysym = key.keysym;

    bool isKeyDown = (key.state & SDL_PRESSED) == SDL_PRESSED;

    /*
    bool isShiftDown = (keysym.mod & KMOD_SHIFT) == KMOD_SHIFT;
    bool isAltDown = (keysym.mod & KMOD_ALT) == KMOD_ALT;
    bool isCtrlDown = (keysym.mod & KMOD_CTRL) == KMOD_CTRL;
    */

    if (isKeyDown && keysym.scancode == SDL_SCANCODE_ESCAPE)
      isRunning_ = false;
  }
}

void Game::
handleMouse(SDL_Event event)
{
  // Mouse support
  // NOTE(naum): Can support multiple mice
  if (event.type == SDL_MOUSEMOTION)
  {
  }

  if (event.type == SDL_MOUSEBUTTONDOWN ||
      event.type == SDL_MOUSEBUTTONUP)
  {
  }

  if (event.type == SDL_MOUSEWHEEL)
  {
  }
}

void Game::
handleJoysticks(SDL_Event event)
{
  // Joystick Support
  if (event.type == SDL_JOYDEVICEADDED)
  {
    auto jdevice = event.jdevice;
    std::cout << "Joystick connected: "
      << jdevice.which
      << std::endl;

    // TODO(naum): Add support for new controller
  }

  if (event.type == SDL_JOYDEVICEREMOVED)
  {
    SDL_JoystickUpdate();
    auto jdevice = event.jdevice;
    std::cout << "Joystick disconnected: "
      << jdevice.which
      << std::endl;
    // TODO(naum): Verify if joy is in use to do what is needed
  }

  if (event.type == SDL_JOYBUTTONDOWN ||
      event.type == SDL_JOYBUTTONUP)
  {
    auto jbutton = event.jbutton;
    std::cout << jbutton.which << ": " << jbutton.button << std::endl;
  }

  if (event.type == SDL_JOYAXISMOTION)
  {
  }

  if (event.type == SDL_JOYHATMOTION)
  {
  }
}

void Game::
render()
{
  // TODO(naum): Don't use this backbuffer texture
  SDL_UpdateTexture(backbuffer_, 0, backbufferPixels_, SCREEN_WIDTH * sizeof(uint32));
  SDL_RenderCopy(renderer_, backbuffer_, 0, 0);

  // Swap buffers and clear the backbuffer
  SDL_RenderPresent(renderer_);
  SDL_RenderClear(renderer_);
}

void Game::
gameUpdate()
{
  // Overload this with your game's logic
}

} // namespace vsge
