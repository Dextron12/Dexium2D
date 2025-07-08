#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <algorithm>

#include "window.hpp"

struct Texture {
	float w=0, h=0; // The size of the texture
	//std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)> tex;
	SDL_Texture* tex;

	~Texture();
};

struct subTexture {
	std::string textureID;
	SDL_Rect region;

	subTexture(const std::string& texID, SDL_Rect region) : textureID(texID), region(region) {}
};

// Loads a texture from a file.	
std::shared_ptr<Texture> loadTexture(SDL_Renderer* renderer, const std::string& texturePath);

// Renders a texture either using an assetID(for a texture asset) or directly providing a shared_ptr of a texture object.
void renderTexture(SDL_Renderer* renderer, const std::shared_ptr<Texture>& texturePtr, SDL_Point pos, const std::string& name = "");
void renderTexture(SDL_Renderer* renderer, SDL_Point pos, const std::string& textureID);

// Generated a pink & white checkered fall-back texture
std::shared_ptr<Texture> createFallbackTexture(SDL_Renderer* renderer, int width = 64, int height = 64, int cellSize = 8);


class Spritesheet {
public:
	friend class AnimatedSprite;
	~Spritesheet() = default;

	void addSubTexture(const std::string& subID, const std::string& textureID, SDL_Rect region);
	
	// If unloadTexture = true; the program will request for the AssetManager to unload the texture entirely
	void remSubTexture(const std::string& subID, bool unloadTexture = false);

	virtual void render(SDL_Renderer* renderer, const std::string& subID, SDL_Point pos,
		SDL_RendererFlip flip = SDL_FLIP_NONE, double angle = 0.0f,
		SDL_Point* center = nullptr, SDL_Point scale = { 1,1 });

private:
	std::unordered_map<std::string, subTexture> subTextures;


};

#endif