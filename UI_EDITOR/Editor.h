// Editor.h
#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <map>
#include <memory>

#include "External/imgui/imgui.h"

struct Texture2D {
	void* texture = nullptr;
	std::string name;

	int importedSizeX = 0;
	int importedSizeY = 0;

	int sizeX = 0;
	int sizeY = 0;

	std::string pixelFormat;
};

struct TextureFolder {
	std::string name;
	std::vector<Texture2D> textures;
	std::map<std::string, std::unique_ptr<TextureFolder>> subFolders;
};

class Editor {
public:
	Editor();
	~Editor();

	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;

	void Update();
	void Render();

	void LoadUnrealTexture2D(void* device, const std::string& fullPath, const std::string& relativePath);
	void LoadUI(const std::string& path);
	void SaveUI(const std::string& path);
	void LoadFolder(void* device, const std::string& path);

	void DuplicateSelected();
	void DeleteSelected();

	ImVec2 GetGlobalPos(int index);
	int FindEntityIndexById(int id);
	void SetParent(int childIdx, int parentId);

private:
	void HandleInputs();
	void RefreshUIFileList();
	void RenderTextureFolder(TextureFolder& folder);
	void ReleaseFolder(TextureFolder& folder);
	void RenderHierarchyRecursive(int parentId);

	void* d3dDevice = nullptr;
	std::string currentPath = "../Client/Bin/Resource/Texture/UI/Materials";
	std::string lastLoadedPath = "";

	TextureFolder rootLibrary;
	std::mutex libraryMutex;

	int nextId = 0;
	std::atomic<int> totalFilesToLoad = 0;
	std::atomic<int> currentFilesLoaded = 0;
	std::atomic<bool> isLoading = false;
	std::thread loaderThread;

	std::vector<std::string> uiFiles;
	std::vector<std::string> uiFolders;
	std::vector<std::string> assetFiles;
};

