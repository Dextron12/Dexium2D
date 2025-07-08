#include "window.hpp"

int LOGICAL_WIDTH = 1280;
int LOGICAL_HEIGHT = 720;

std::filesystem::path VFS::basePath;

WindowContext::WindowContext(std::string windowTitle, SDL_Point windowSize, SDL_WindowFlags windowFlags, const SDL_Point logicalWindowSize) {
	// Set Logical window sizes:
	LOGICAL_WIDTH = logicalWindowSize.x;
	LOGICAL_HEIGHT = logicalWindowSize.y;
	//Set window size
	width = windowSize.x;
	height = windowSize.y;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "Failed to init SDL2" << SDL_GetError() << std::endl;
		return;
	}

	window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, windowFlags);
	if (!window) {
		std::cerr << "Failed to create a SDL window" << SDL_GetError() << std::endl;
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)	{
		std::cerr << "Faield to create a SDL renderer" << SDL_GetError() << std::endl;
		return;
	}

	//Render controls
	//SDL_RenderSetLogicalSize(renderer, windowWidth, windowHeight);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // no smoothing (pixel perfect)
	SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

	//Init SDL_Image
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		std::cerr << "failed to init required image formats: " << IMG_GetError() << std::endl;
		return;
	}

	// Set vrtScreen prop:
	virtScreen = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		LOGICAL_WIDTH, LOGICAL_HEIGHT);

	appState = true;

	lastTime = SDL_GetTicks();
}

WindowContext::~WindowContext() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void WindowContext::startFrame() {
	// Chnage the render tartext to virtual atrget
	SDL_SetRenderTarget(renderer, virtScreen);
	//Reste flags
	windowResized = false;
	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT) {
			appState = false;
		}

		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				appState = false;
			}
		}

		if (event.type == SDL_WINDOWEVENT) {
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				width = event.window.data1;
				height = event.window.data2;
				windowResized = true;
			}
		}
	}

	//Update dT
	currentTime = SDL_GetTicks();
	deltaTime = (currentTime - lastTime) / 1000.0f; // for heavens sakes, keep this in seconds. Physics relies on it being ins econds!
	lastTime = currentTime;
}

void WindowContext::endFrame() {
	// Rendering should have stoppeed. Change atrgets
	SDL_SetRenderTarget(renderer, nullptr);
	SDL_RenderClear(renderer); //Clear the renderer in case of un-initalised pixels ona  window resize!

	// Calculate scale to fill window completely without letterboxxing (may crop)
	float scaleX = (float)width / LOGICAL_WIDTH;
	float scaleY = (float)height / LOGICAL_HEIGHT;
	float scale = std::max<float>(scaleX, scaleY);

	int destW = (int)(LOGICAL_WIDTH * scale);
	int destH = (int)(LOGICAL_HEIGHT * scale);

	SDL_Rect destRect;
	destRect.x = (width - destW) / 2; // centers horizontally (negative values crop)
	destRect.y = (height - destH) / 2; // centers vertically (negative valeus crop)

	destRect.w = destW;
	destRect.h = destH;

	// Render the scaled texture tot he window
	SDL_RenderCopy(renderer, virtScreen, nullptr, &destRect);
	SDL_RenderPresent(renderer);
}

void VFS::init() {
	#ifdef _WIN32
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	basePath = std::filesystem::path(buffer).parent_path();
	#endif

	#ifdef __LINUX__
	basePath = std::filesystem::path(SDL_GetBasePath());
	basePath = basePath.parent_path(); // Strips the '/'
	#endif

	//Step out of debug directory if debugging
#ifdef _DEBUG
	#ifdef _WIN32
	basePath = basePath.parent_path(); // Debug/
	basePath = basePath.parent_path(); //x64/
	#endif

	#ifdef __linux__
	std::filesystem::path canPath(SDL_GetBasePath());
	//Normalize path to remove trailing slash provided by SDL_GetBasePath().
	canPath = canPath.lexically_normal();
	if (canPath.filename().empty()){
		canPath = canPath.parent_path(); // remvoes empty filename
	}
	basePath = canPath.parent_path();
	#endif
	//Should now be in project root dir
#endif
}

// Resolves relative paths into absolute paths. Returns an empty path on failure.
/*std::filesystem::path VFS::resolve(const std::string& relativePath) {
	std::filesystem::path abs;
	bool state = true;
	try {
		// Attemtps to resolve abs path, if not errors out.
		std::cout << std::filesystem::path(relativePath).is_relative() << std::endl;
		//abs = std::filesystem::canonical(basePath / std::filesystem::path(relativePath).relative_path());
		abs = basePath / std::filesystem::path(relativePath).relative_path();
	}
	catch (const std::filesystem::filesystem_error& e) {
		log(Error, "[VFS::Resolve]: The path '%s' does not exist", abs.c_str());
		state = false;
	}
	if (state) {
		return abs;
	}
	else {
		return std::filesystem::path(relativePath);
	}
}*/

/*ResolvedPath VFS::resolve(const std::string& relativePath) {
	std::filesystem::path abs;
	std::filesystem::path rel(relativePath);

	// Checks for a valid basePath
	if (basePath.empty()) {
		log(Error, "[VFS::Resolve]: Did you forget to init VFS? basePath cannot be empty");
		//return ResolvedPath{ rel, false };
		return ResolvedPath(rel, false);
	}

	// Gets abs path;
	abs = basePath / rel.relative_path(); // <-- .relative_path() allows '/' to be used at the start of path and forces the relPath to be relative.
	if (exists(abs.string(), ABS_PATH)) {
		//return ResolvedPath{ abs, true };
		return ResolvedPath(abs, true);
	}

	//return ResolvedPath{ rel, false };
	return ResolvedPath(rel, false);
}*/

ResolvedPath VFS::resolve(const std::string& relativePath) {
	if (basePath.empty()) {
		log(Error, "[VFS::resolve]: VFS base path not initialized. Did you forget to initialize VFS?");
		return ResolvedPath(std::filesystem::path(relativePath), false);
	}

	std::filesystem::path rel(relativePath);
	std::filesystem::path abs = basePath / rel;

	// Attempt canonical path
	std::filesystem::path finalPath;
	try {
		finalPath = std::filesystem::canonical(abs);
	}
	catch (const std::filesystem::filesystem_error&) {
		finalPath = abs.lexically_normal();
	}

	// Check path existance:
	if (std::filesystem::exists(abs)) {
		return ResolvedPath(finalPath, true);
	}
	else {
		log(WARNING, "[VFS::resolve]: Could not resolve '%s'. Full attempted path: '%s'", relativePath.c_str(), finalPath.string().c_str());
		return ResolvedPath(finalPath, false);
	}
}


bool VFS::exists(const std::string& path, PathType type) {
	switch (type)
	{
	case ABS_PATH:
		return std::filesystem::exists(path);
	case REL_PATH:
		return std::filesystem::exists(basePath / path);
	default:
		break;
	}

	return false;
}


void log(errorType type, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	switch (type) {
	case errorType::INFO:
		std::printf("[INFO]: ");
		break;
	case errorType::WARNING:
		std::printf("[Warning]: ");
		break;
	case errorType::Error:
		std::printf("[Error]: ");
		break;
	case errorType::FATAL_ERROR:
		std::printf("[FATAL ERROR]: ");
		break;
	}

	std::vprintf(fmt, args);
	std::printf("\n");
	va_end(args);

	if (type == FATAL_ERROR) {
		abort();
	}
}