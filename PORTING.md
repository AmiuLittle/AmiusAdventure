# Porting
As mentioned in [README.md](README.md), all of the game logic is in [amius_adventure](amius_adventure/), and it can be imported into your port with a few includes and compiler flags. Your port of the game will need to follow specific criteria to accurately represent the game. Keep in mind, these rules are set in place for those who want to make a port that is as faithful to the game as possible, I'm not going to sacrifice you to the 3DS gods if you wanna backport and cannot fulfill every detail

## Required Prerequesites (The game engine requires the hardware to fulfill this to even run)
- 3d graphics / Psudo 3d graphics capabilities (The game engine will specify 3d model locations and textures it wants, but whether you choose to use 3d models or something like SNES mode 7 is up to you)
- Ability to represent two "screens" concurrently (not necessarily at the same time, you could add a hotkey to switch between them instead. This is because the 3ds has two physical screens and they are both used by my game)
- Ability to capture mouse like behaviour on the bottom "screen" (The 3ds has a touchscreen on its bottom screen, this could be emulated with a cursor controlled by a thumbstick, actual mouse input, or a physical touch screen)
- Input for the DPad, Shoulder buttons, Start, Select, and ABXY buttons
- Implementations for a few common GNU C++17 libraries (std::vector, std::map, std::chrono to mention a few)
- A panic function (I don't care what it does as long as it stops execution and/or exits the program)
- I'm not sure for memory requirements at the moment but this is being designed to run in 128MB of RAM

## Optional Prerequesites (All of these are fulfilled by the 3DS platform but aren't required to make the game run)
- 3d graphics capabilities similar to, if not better than the feature set provided by Citro3D (basically OpenGL ES 2.0)
- input for an analogue stick (Preferabably for the left hand / thumb)
- Sound handling of some sort (The game engine will just say what sounds it wants playing, whether it should loop, and when it should stop. I don't mind if you rip the samples from n3ds_backend or if you come up with some midi, be creative with it).

## What you need to know
&nbsp;&nbsp;&nbsp;&nbsp;The game engine will do its own thing until it needs to talk to the hardware, in your initialization stage you are expected to initialize a scene class for the top and bottom screen, initialize the engine, then run `engine.update()` in a loop. Technically you could stop here and say the game is ported because its code is running on the target platform, but nothing is happening on the screen, there's no sound, and the game won't recieve any input which isn't much of a game.
&nbsp;&nbsp;&nbsp;&nbsp;During the game's execution it will call hardware functions and edit the arrangement of both scenes, it is your job to implement these hardware functions and write routines for rendering the scenes. Along with that, assets are kept in the platform's folder, not in the game engine's folder. Whether you rip the assets from another port or make your own is your choice. This includes sounds, models, textures, and shaders.

## Steps to a Full Port
1. Make a panic function that takes a string, I recommend this makes the target platform close the app and open a window to report the error. If thats not possible, like how on the 3ds the only thing that is really running at one time is the game itself, you could make the system do something like a bluescreen.
2. Set up the engine and run `engine.update()` in a loop (see [main.cpp](n3ds_platform/main.cpp))
3. In the loop, gather input and convert to `AmiusAdventure::Input::InputState` so that the engine can use it
4. Make your own assets or copy the assets from another port
5. Write a rendering engine that uses both scenes, call a grapics update every frame after the engine tick
6. Implement functions to handle sound, i have set up the 3ds to handle sound asyncronously on another thread, hence functions are used to 'talk' to the sound engine, even if the sound engine is going to be on the same thread in your port.
