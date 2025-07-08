template <typename T>
std::shared_ptr<T> AssetManager::load(const std::string& id, const std::string& path) {
	//Check if file exists:
	if (VFS::exists(path, PathType::ABS_PATH) == false) {
		log(Error, "[Asset Loader]: The asset path '%s' is invalid", path.c_str());
		return nullptr;
	}

	AssetEntry entry;

	entry.type = deduceAssetType<T>();

	//Check for an Unkown type case
	if (entry.type == AssetType::Unknown_Asset) {
		log(Error, "[Asset Loader]: Cannot load an unkown asset type for: '%s'", id.c_str());
		return nullptr;
	}

	//Check for a valid renderer:
	if (renderer == nullptr) {
		log(Error, "[Asset Manager}: No current renderering context");
		log(INFO, "Did you forget to call AssetManager::get().init(renderer)?\n[INFO]: A valid renderer is required for asset loading");
		return nullptr;
	}

	// asset cache has been validated, load it:

	// Individual loaders:
	switch (entry.type) {
	case AssetType::TEXTURE:
		entry.ptr = loadTexture(renderer, path);
		entry.registryPath = path;
		break;
	case AssetType::SPRITESHEET:
		// Load Spritesheet
		entry.ptr = std::make_shared<Spritesheet>();
		load<Texture>(id + "_spritesheet", path); // Load the internal sprite
		entry.registryPath = path; // Spritesheets interally request AssetManager to ahndle its textures
		break;
	case AssetType::Unknown_Asset:
		// Assume we have already caught this error
		break;
	}

	log(INFO, "[Asset Loader]: Successfully loaded: '%s'", id.c_str());

	assets.insert_or_assign(id, entry);
	return std::static_pointer_cast<T>(entry.ptr);
}

template<typename T>
bool AssetManager::queryAsset(const std::string& assetID) {
	auto it = assets.find(assetID);
	if (it == assets.end()) {
		log(INFO, "[Asset Manager}: The asset: '%s' could not be found", assetID.c_str());
		return false;
	}

	if (it->second.type == deduceAssetType<T>()) {
		return true;
	}
	else {
		log(INFO, "[Asset Manager}: the asset: '%s' exists, but theres a type mismatch or another error has occured", assetID.c_str());
		return false;
	}
	return false;

}

template<typename T>
std::shared_ptr<T> AssetManager::use(const std::string& id) {
	// Check if asset is loaded and exists:

	auto entry = assets.find(id);
	

	// Checks if asset-cache exists
	if (entry == assets.end()) {
		log(Error, "[Asset Lookup]: The requested asset: '%s' does not exist!", id.c_str());
		return nullptr;
	}

	// Checks if internal asset is loaded.
	if (entry->second.ptr == nullptr) {
		if (entry->second.registryPath.empty()) {
			log(Error, "[Asset Manager}: Cannot load '%s' from an empty path", entry->first);
			log(INFO, "[Asset Manager}: Did you forget to register or load the asset?");
			return nullptr;
		}
		else {
			// The asste is correctly registered, but has been set to lazy-load
			load<T>(entry->first, entry->second.registryPath);
		}
	}

	// chekcs for type mismatches between the template type and hte type assigned to the asset.
	if (entry->second.type != deduceAssetType<T>()) {
		log(WARNING, "[Asset Lookup]: Asset type mismatch for: '%s'", id.c_str());
		return nullptr;
	}

	// return asset.
	return std::static_pointer_cast<T>(entry->second.ptr);
}

template<typename T>
void AssetManager::registerAsset(const std::string& assetID, const std::string& path, bool allowOverwrite) {
	// Check if the path exists:
	auto& res = VFS::resolve(path);
	if (!res.success) {
		std::cout << res.path.string() << " nbool: " << res.success << std::endl;
		// Failed to resolve rel into abs path
		log(Error, "[Asset Manager}: The path: '%s' for assetID: '%s' is invalid", res.path.string().c_str(), assetID.c_str());
		return;
	}

	AssetEntry entry;
	entry.registryPath = res.path.string();
	entry.type = deduceAssetType<T>();
	entry.ptr = nullptr; // Lazy load when needed.

	// Path resolved, Check if a asset of the same name already exists
	auto it = assets.find(assetID);
	if (it != assets.end()) {
		// An asset already exists
		if (allowOverwrite) {
			// Overwrite the existing asset
			log(WARNING, "[Asset Manager}: Overwriting '%s' with asset '%s'", assetID.c_str(), res.path.c_str());
			assets[assetID] = entry;
		}
		else {
			log(Error, "[Asset Manager}: Cannot overwrite '%s'", assetID.c_str());
			return;
		}
	}
	else {
		// Asset does not exist, create a new one.
		assets.insert({ assetID, entry });
	}
}

template<>
constexpr AssetType AssetManager::deduceAssetType<Texture>() {
	return AssetType::TEXTURE;
}

template <>
constexpr AssetType AssetManager::deduceAssetType<Spritesheet>() {
	return AssetType::SPRITESHEET;
}

