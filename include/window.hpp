// -----------------------------------------------------------------------------
// window.hpp
// Developed by Dextron12 (C) 2025
//
// This header defines the WindowContext class, which wraps SDL_Window and 
// SDL_Renderer with additional support for:
//
// - Rendering to a fixed-resolution internal render target (e.g., 1280x720)
// - Automatically scaling that target to fit the actual window size
// - Enabling resolution-independent drawing via a virtual screen space
// - A simple Virtual File System (VFS) with:
//     - Automatic detection of the project working directory
//     - Resolution of relative paths inside the working directory
//     - A utility function VFS::exists() to check file existence
//
// This system emulates behavior seen in many 2D games 
// (e.g., Stardew Valley, Celeste), enabling consistent rendering across 
// different window sizes and display resolutions � without requiring manual 
// camera or scaling logic in game code.
//
// Note: This is *not* a direct wrapper over SDL's default renderer. 
// It handles offscreen rendering, scaling, and presentation internally.
// The end user can render sprites, UI, etc. to WindowContext::renderer as 
// if it were a normal SDL_Renderer � the wrapper handles the rest.
// -----------------------------------------------------------------------------


#ifndef WINDOW_HPP
#define WINDOW_HPP

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <string>
#include <filesystem>
#include <system_error>
#include <iostream>


#ifdef _WIN32
#include <Windows.h> // Used to resolve exec path.
#endif

// -----------------------------------------------------------------------------------
// The logical (virtual) resolution used for all internal rendering.
// This defines the coordinate space the game logic operates in, 
// regardless of the actual window size.
//
// Typically, you'd set this to match your game's target aspect ratio.
// For example, 1280x720 maintains a 16:9 aspect ratio and is a common baseline
// used by many modern 2D games.
//
// The renderer will automatically scale this logical space to fit the 
// actual window dimensions, preserving the aspect ratio and maintaining 
// consistent visuals across different screen sizes.
extern int LOGICAL_WIDTH;
extern int LOGICAL_HEIGHT;
// -----------------------------------------------------------------------------------


enum PathType {
	ABS_PATH,
	REL_PATH
};

enum errorType {
	FATAL_ERROR, //	A error with this code will resultin abort() being called.
	Error,		// Outputs always. oNLY POSTS LOG TO TERMINAL
	WARNING,	// WILL OUTPUT ON RELEASE( etc, path's not found)
	INFO		// Only outputs logs on debug builds
};

struct ResolvedPath {
	std::filesystem::path path;
	bool success;

	ResolvedPath(std::filesystem::path ckPath, bool suc) : path(ckPath), success(suc) {}
};

class WindowContext {
private:
	SDL_Event event;

	Uint32 lastTime;
	Uint32 currentTime;

	// The vritual renderTarget etxture that we blit to for scaling and resising pruposes. This sia  god-send
	SDL_Texture* virtScreen;

public:
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool appState = false;
	bool windowResized = false;
	float width; float height;

	float deltaTime;

	WindowContext(std::string windowTitle, SDL_Point windowSize, SDL_WindowFlags windowFlags, const SDL_Point logicalWindowSize = {1280,720}); // By default sets the logical render size to 1280x720 (16:9) aspect ratio
	~WindowContext();
	
	void startFrame(); // Call at the start of your game-loop.
	void endFrame();   // Call this at the enf of your loop (after rendering) Will call SDL_RenderPresnet() for you.
};

class VFS {
public:
	//Configures the basePath for exec working dir.
	static void init();
	static ResolvedPath resolve(const std::string& relativePath);

	//Checks if either a relative or absolute path exists
	static bool exists(const std::string& path, const PathType);

	static std::filesystem::path basePath;
};


void log(errorType type, const char* fmt, ...);

#endif