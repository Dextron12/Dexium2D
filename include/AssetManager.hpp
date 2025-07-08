#ifndef ASSET_MAN_HPP
#define ASSET_MAN_HPP

#include "Textures.hpp"
#include "SpriteAnimations.hpp"


// Add types here to include them in the offical assetManager. (! Should mostly handle the new types witout any code changes)
enum AssetType {
	TEXTURE,
	SPRITESHEET,
	Animated_Sprite,
	Animated_Player,
	Unknown_Asset
};

struct AssetEntry {
	std::shared_ptr<void> ptr; // A void ptr (holds any type) eb sure to safetlys cast
	AssetType type = Unknown_Asset;
	std::string registryPath;
};


class AssetManager {
public:
	static AssetManager& get();

	static void init(SDL_Renderer* renderer);
	static void destroyAsset(const std::string& assetID);

	static void listLoadedAssets();

	template<typename T>
	void registerAsset(const std::string& assetID, const std::string& path, bool allowOverwrite = false);
	static void registerAssetsFromManifest(const std::string& manifestPath, bool allowOverwrite = false);

	template<typename T>
	static std::shared_ptr<T> load(const std::string& id, const std::string& path);

	template<typename T>
	static std::shared_ptr<T> use(const std::string& id);

	template<typename T>
	static bool queryAsset(const std::string& assetID);
	AssetEntry* getAssetEntry(const std::string& assetID);

private:

	static std::unordered_map<std::string, AssetEntry> assets;
	static SDL_Renderer* renderer;
	
	AssetManager() = default;

	template<typename T>
	constexpr static AssetType deduceAssetType();

};

#include "AssetManager.inl"




#endif