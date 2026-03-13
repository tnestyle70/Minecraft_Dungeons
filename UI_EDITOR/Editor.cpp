// Editor.cpp
#include "Editor.h"
#include "Entity.h"
#include "windows.h"
#include "d3d9.h"
#include "d3dx9.h"
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <map>
#include <algorithm>
#include "imgui.h"
#include "imgui_internal.h"
#include "json.hpp"

#pragma comment(lib, "d3dx9.lib")

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {
	struct EditorState {
		std::vector<Entity> entities;
		std::vector<int> selectedIndices;
		bool dragging = false;
		std::vector<ImVec2> selectedOffsets; 

		// Stage 3
		bool snapToGrid = false;
		int gridSize = 16;

		// Stage 4
		char searchFilter[128] = "";
		std::vector<Entity> clipboard;
	};

	EditorState state;

	struct Command {
		virtual ~Command() = default;
		virtual void Undo() = 0;
		virtual void Redo() = 0;
	};

	std::vector<std::unique_ptr<Command>> undoStack;
	std::vector<std::unique_ptr<Command>> redoStack;

	void ExecuteCommand(std::unique_ptr<Command> cmd) {
		cmd->Redo();
		undoStack.push_back(std::move(cmd));
		redoStack.clear();
		if (undoStack.size() > 50) undoStack.erase(undoStack.begin());
	}

	bool IsSelected(int index) {
		return std::find(state.selectedIndices.begin(), state.selectedIndices.end(), index) != state.selectedIndices.end();
	}

	void ToggleSelection(int index, bool multi) {
		auto it = std::find(state.selectedIndices.begin(), state.selectedIndices.end(), index);
		if (multi) {
			if (it != state.selectedIndices.end()) state.selectedIndices.erase(it);
			else state.selectedIndices.push_back(index);
		} else {
			state.selectedIndices.clear();
			state.selectedIndices.push_back(index);
		}
	}
}

Editor::Editor() { RefreshUIFileList(); }
Editor::~Editor() { if (loaderThread.joinable()) loaderThread.join(); ReleaseFolder(rootLibrary); }

ImVec2 Editor::GetGlobalPos(int index) {
	if (index < 0 || index >= (int)state.entities.size()) return ImVec2(0, 0);
	const Entity& e = state.entities[index];
	if (e.parentId == -1) return ImVec2(e.x, e.y);
	int pIdx = FindEntityIndexById(e.parentId);
	if (pIdx == -1 || pIdx == index) return ImVec2(e.x, e.y);
	ImVec2 pPos = GetGlobalPos(pIdx);
	return ImVec2(pPos.x + e.x, pPos.y + e.y);
}

int Editor::FindEntityIndexById(int id) {
	for (int i = 0; i < (int)state.entities.size(); ++i) if (state.entities[i].id == id) return i;
	return -1;
}

void Editor::SetParent(int childIdx, int parentId) {
	if (childIdx < 0 || childIdx >= (int)state.entities.size()) return;
	int cur = parentId;
	while (cur != -1) {
		if (cur == state.entities[childIdx].id) return;
		int pIdx = FindEntityIndexById(cur);
		if (pIdx == -1) break;
		cur = state.entities[pIdx].parentId;
	}
	ImVec2 gPos = GetGlobalPos(childIdx);
	state.entities[childIdx].parentId = parentId;
	if (parentId == -1) {
		state.entities[childIdx].x = gPos.x; state.entities[childIdx].y = gPos.y;
	} else {
		int pIdx = FindEntityIndexById(parentId);
		if (pIdx != -1) {
			ImVec2 pgPos = GetGlobalPos(pIdx);
			state.entities[childIdx].x = gPos.x - pgPos.x; state.entities[childIdx].y = gPos.y - pgPos.y;
		}
	}
}

void Editor::DuplicateSelected() {
	if (state.selectedIndices.empty()) return;
	std::vector<int> newSels;
	for (int idx : state.selectedIndices) {
		Entity copy = state.entities[idx];
		copy.id = nextId++; copy.x += 20; copy.y += 20;
		state.entities.push_back(copy);
		newSels.push_back((int)state.entities.size() - 1);
	}
	state.selectedIndices = newSels;
}

void Editor::DeleteSelected() {
	if (state.selectedIndices.empty()) return;
	std::sort(state.selectedIndices.begin(), state.selectedIndices.end(), std::greater<int>());
	for (int idx : state.selectedIndices) {
		int id = state.entities[idx].id;
		for (auto& e : state.entities) if (e.parentId == id) e.parentId = -1;
		state.entities.erase(state.entities.begin() + idx);
	}
	state.selectedIndices.clear();
}

void Editor::HandleInputs() {
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantTextInput) return;
	if (io.KeyCtrl) {
		if (ImGui::IsKeyPressed(ImGuiKey_D)) DuplicateSelected();
		if (ImGui::IsKeyPressed(ImGuiKey_S)) { if (!lastLoadedPath.empty()) SaveUI(lastLoadedPath); }
		if (ImGui::IsKeyPressed(ImGuiKey_C)) {
			state.clipboard.clear();
			for (int idx : state.selectedIndices) state.clipboard.push_back(state.entities[idx]);
		}
		if (ImGui::IsKeyPressed(ImGuiKey_V)) {
			state.selectedIndices.clear();
			for (auto& e : state.clipboard) {
				e.id = nextId++; e.x += 10; e.y += 10;
				state.entities.push_back(e);
				state.selectedIndices.push_back((int)state.entities.size() - 1);
			}
		}
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Delete)) DeleteSelected();
}

void Editor::RenderHierarchyRecursive(int parentId) {
	for (int i = 0; i < (int)state.entities.size(); ++i) {
		if (state.entities[i].parentId != parentId) continue;
		if (state.searchFilter[0] != '\0') {
			std::string n = state.entities[i].name; std::string f = state.searchFilter;
			std::transform(n.begin(), n.end(), n.begin(), ::tolower);
			std::transform(f.begin(), f.end(), f.begin(), ::tolower);
			if (n.find(f) == std::string::npos) continue;
		}
		ImGuiTreeNodeFlags f = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (IsSelected(i)) f |= ImGuiTreeNodeFlags_Selected;
		bool hasChild = false;
		for (const auto& e : state.entities) if (e.parentId == state.entities[i].id) { hasChild = true; break; }
		if (!hasChild) f |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		bool open = ImGui::TreeNodeEx((void*)(intptr_t)state.entities[i].id, f, "%s%s", state.entities[i].name.c_str(), state.entities[i].visible ? "" : " (Hidden)");
		if (ImGui::IsItemClicked()) ToggleSelection(i, ImGui::GetIO().KeyCtrl);
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("DND_ENTITY", &i, sizeof(int));
			ImGui::Text("Move %s", state.entities[i].name.c_str());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("DND_ENTITY")) SetParent(*(int*)p->Data, state.entities[i].id);
			ImGui::EndDragDropTarget();
		}
		if (open && hasChild) { RenderHierarchyRecursive(state.entities[i].id); ImGui::TreePop(); }
	}
}

void Editor::ReleaseFolder(TextureFolder& f) {
	for (auto& t : f.textures) if (t.texture) ((IDirect3DTexture9*)t.texture)->Release();
	for (auto& p : f.subFolders) ReleaseFolder(*p.second);
	f.textures.clear(); f.subFolders.clear();
}

void Editor::RenderTextureFolder(TextureFolder& f) {
	if (f.name.empty()) { for (auto& p : f.subFolders) RenderTextureFolder(*p.second); }
	else if (ImGui::TreeNodeEx(f.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		float winX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		for (size_t i = 0; i < f.textures.size(); ++i) {
			ImGui::PushID((int)i); ImGui::BeginGroup();
			ImVec2 size(64, 64), uv0(0, 0), uv1(1, 1);
			if (f.textures[i].sizeX > 0) { uv1.x = (float)f.textures[i].importedSizeX / f.textures[i].sizeX; uv1.y = (float)f.textures[i].importedSizeY / f.textures[i].sizeY; }
			if (ImGui::ImageButton("##t", (ImTextureID)f.textures[i].texture, size, uv0, uv1)) {
				Entity e{}; e.id = nextId++; e.name = f.textures[i].name; e.width = (float)f.textures[i].importedSizeX; e.height = (float)f.textures[i].importedSizeY;
				e.texture = f.textures[i].texture; e.uv_u = uv1.x; e.uv_v = uv1.y;
				state.entities.push_back(e);
			}
			ImGui::Text("%s", f.textures[i].name.c_str()); ImGui::EndGroup();
			if (i + 1 < f.textures.size() && ImGui::GetItemRectMax().x + 70 < winX2) ImGui::SameLine();
			ImGui::PopID();
		}
		for (auto& p : f.subFolders) RenderTextureFolder(*p.second);
		ImGui::TreePop();
	}
}

void Editor::Update() { HandleInputs(); }

void Editor::Render() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) { state.entities.clear(); state.selectedIndices.clear(); lastLoadedPath = ""; }
			if (ImGui::MenuItem("Save", "Ctrl+S", false, !lastLoadedPath.empty())) SaveUI(lastLoadedPath);
			if (ImGui::MenuItem("Exit")) PostQuitMessage(0);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->WorkPos); ImGui::SetNextWindowSize(vp->WorkSize);
	ImGui::Begin("MainDock", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoNavFocus);
	ImGui::DockSpace(ImGui::GetID("Dock"), ImVec2(0,0)); ImGui::End();

	ImGui::Begin("Toolbar");
	ImGui::Checkbox("Snap", &state.snapToGrid); ImGui::SameLine(); ImGui::SetNextItemWidth(60); ImGui::DragInt("##G", &state.gridSize, 1, 4, 128);
	ImGui::SameLine(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical); ImGui::SameLine();
	if (ImGui::Button("L")) for (int idx : state.selectedIndices) state.entities[idx].x = 0; ImGui::SameLine();
	if (ImGui::Button("C")) for (int idx : state.selectedIndices) state.entities[idx].x = (1280 - state.entities[idx].width) * 0.5f; ImGui::SameLine();
	if (ImGui::Button("R")) for (int idx : state.selectedIndices) state.entities[idx].x = 1280 - state.entities[idx].width; ImGui::SameLine();
	if (ImGui::Button("T")) for (int idx : state.selectedIndices) state.entities[idx].y = 0; ImGui::SameLine();
	if (ImGui::Button("M")) for (int idx : state.selectedIndices) state.entities[idx].y = (720 - state.entities[idx].height) * 0.5f; ImGui::SameLine();
	if (ImGui::Button("B")) for (int idx : state.selectedIndices) state.entities[idx].y = 720 - state.entities[idx].height;
	ImGui::End();

	ImGui::Begin("Hierarchy");
	ImGui::InputTextWithHint("##F", "Search...", state.searchFilter, 128);
	RenderHierarchyRecursive(-1);
	ImGui::Dummy(ImGui::GetContentRegionAvail());
	if (ImGui::BeginDragDropTarget()) { if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("DND_ENTITY")) SetParent(*(int*)p->Data, -1); ImGui::EndDragDropTarget(); }
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
	ImGui::Begin("Canvas");
	ImVec2 cAvail = ImGui::GetContentRegionAvail(), cP0 = ImGui::GetCursorScreenPos();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(cP0, ImVec2(cP0.x+cAvail.x, cP0.y+cAvail.y), IM_COL32(30,30,30,255));
	ImVec2 wP0 = ImVec2(cP0.x + (cAvail.x-1280)*0.5f, cP0.y + (cAvail.y-720)*0.5f);
	dl->AddRectFilled(wP0, ImVec2(wP0.x+1280, wP0.y+720), IM_COL32(45,45,45,255));
	for (size_t i = 0; i < state.entities.size(); ++i) {
		Entity& e = state.entities[i]; ImVec2 g = GetGlobalPos((int)i);
		ImVec2 eMin(wP0.x+g.x, wP0.y+g.y), eMax(eMin.x+e.width, eMin.y+e.height);
		ImGui::SetCursorScreenPos(eMin); ImGui::PushID(e.id);
		ImGui::InvisibleButton("##e", ImVec2(e.width, e.height));
		if (ImGui::IsItemActivated()) {
			if (!ImGui::GetIO().KeyCtrl && !IsSelected((int)i)) ToggleSelection((int)i, false);
			else if (ImGui::GetIO().KeyCtrl) ToggleSelection((int)i, true);
			state.dragging = true; state.selectedOffsets.clear();
			for (int idx : state.selectedIndices) state.selectedOffsets.push_back(ImVec2(state.entities[idx].x, state.entities[idx].y));
		}
		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
			ImVec2 d = ImGui::GetMouseDragDelta(0);
			for (size_t s = 0; s < state.selectedIndices.size(); ++s) {
				int idx = state.selectedIndices[s]; if (state.entities[idx].locked) continue;
				float nx = state.selectedOffsets[s].x + d.x, ny = state.selectedOffsets[s].y + d.y;
				if (state.snapToGrid) { nx = roundf(nx/state.gridSize)*state.gridSize; ny = roundf(ny/state.gridSize)*state.gridSize; }
				state.entities[idx].x = nx; state.entities[idx].y = ny;
			}
		}
		ImGui::PopID();
		if (!e.visible) continue;
		ImU32 col = IM_COL32((e.color>>16)&0xFF, (e.color>>8)&0xFF, e.color&0xFF, (e.color>>24)&0xFF);
		if (e.texture) dl->AddImage((ImTextureID)e.texture, eMin, eMax, ImVec2(0,0), ImVec2(e.uv_u, e.uv_v), col);
		else dl->AddRectFilled(eMin, eMax, col);
		if (IsSelected((int)i)) dl->AddRect(eMin, eMax, IM_COL32(255,255,0,255), 0, 0, 2);
	}
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) state.selectedIndices.clear();
	ImGui::End(); ImGui::PopStyleVar();

	ImGui::Begin("Properties");
	if (!state.selectedIndices.empty()) {
		if (state.selectedIndices.size() == 1) {
			Entity& s = state.entities[state.selectedIndices[0]];
			char b[128]; strcpy_s(b, s.name.c_str()); if (ImGui::InputText("Name", b, 128)) s.name = b;
			ImGui::Checkbox("Visible", &s.visible); ImGui::SameLine(); ImGui::Checkbox("Locked", &s.locked);
			ImGui::DragFloat2("Pos", &s.x); ImGui::DragFloat2("Size", &s.width);
			ImGui::SeparatorText("Layout");
			ImGui::SliderFloat2("Pivot", &s.pivotX, 0, 1);
			ImGui::SliderFloat2("Anchor Min", &s.anchorMinX, 0, 1);
			ImGui::SliderFloat2("Anchor Max", &s.anchorMaxX, 0, 1);
		} else ImGui::Text("%d Selected", (int)state.selectedIndices.size());
		if (ImGui::Button("Delete", ImVec2(-1, 0))) DeleteSelected();
	} else ImGui::TextDisabled("No Selection");
	ImGui::End();

	ImGui::Begin("Library");
	std::lock_guard<std::mutex> lock(libraryMutex); RenderTextureFolder(rootLibrary);
	ImGui::End();
}

void Editor::LoadUnrealTexture2D(void* device, const std::string& fullPath, const std::string& relativePath) {
	auto* d3d = (IDirect3DDevice9*)device; if (!d3d) return;
	Texture2D tex{}; std::string bName = fs::path(fullPath).stem().string();
	std::ifstream f(fullPath + ".json");
	if (f.is_open()) { try { json j = json::parse(f); if (j.is_array() && !j.empty()) {
		tex.name = j[0].value("Name", bName);
		if (j[0].contains("Properties") && j[0]["Properties"].contains("ImportedSize")) {
			tex.importedSizeX = j[0]["Properties"]["ImportedSize"].value("X", 0); tex.importedSizeY = j[0]["Properties"]["ImportedSize"].value("Y", 0);
		}
		tex.sizeX = j[0].value("SizeX", 0); tex.sizeY = j[0].value("SizeY", 0);
	}} catch(...) {} }
	D3DXIMAGE_INFO info; IDirect3DTexture9* dt = nullptr;
	if (SUCCEEDED(D3DXCreateTextureFromFileExA(d3d, (fullPath+".png").c_str(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, &info, nullptr, &dt))) {
		tex.texture = dt; if (tex.sizeX == 0) { tex.sizeX = info.Width; tex.sizeY = info.Height; tex.importedSizeX = info.Width; tex.importedSizeY = info.Height; }
		std::lock_guard<std::mutex> l(libraryMutex); TextureFolder* cur = &rootLibrary;
		for (const auto& p : fs::path(relativePath).parent_path()) {
			std::string fn = p.string(); if (cur->subFolders.find(fn) == cur->subFolders.end()) { cur->subFolders[fn] = std::make_unique<TextureFolder>(); cur->subFolders[fn]->name = fn; }
			cur = cur->subFolders[fn].get();
		}
		cur->textures.push_back(tex);
	}
}

void Editor::LoadFolder(void* device, const std::string& path) {
	d3dDevice = device; if (isLoading) return; if (loaderThread.joinable()) loaderThread.join();
	ReleaseFolder(rootLibrary); if (!fs::exists(path)) return;
	isLoading = true; currentFilesLoaded = 0; totalFilesToLoad = 0;
	loaderThread = std::thread([this, device, path]() {
		std::vector<std::pair<std::string, std::string>> ps;
		for (const auto& e : fs::recursive_directory_iterator(path)) if (e.is_regular_file() && e.path().extension() == ".png") ps.push_back({e.path().string().substr(0, e.path().string().find_last_of('.')), fs::relative(e.path(), path).string()});
		totalFilesToLoad = (int)ps.size();
		for (const auto& p : ps) { LoadUnrealTexture2D(device, p.first, p.second); currentFilesLoaded++; }
		isLoading = false;
	});
}

void Editor::LoadUI(const std::string& path) {
	std::ifstream f(path); if (!f.is_open()) return;
	try { json j = json::parse(f); if (!j.is_array()) return; state.entities.clear(); state.selectedIndices.clear(); lastLoadedPath = path;
		std::map<std::string, const json*> m; for (const auto& o : j) if (o.contains("Name")) m[o["Name"].get<std::string>()] = &o;
		for (const auto& o : j) if (o.value("Type", "") == "CanvasPanelSlot") {
			const auto& p = o["Properties"]; if (p.contains("Content")) {
				std::string cn = p["Content"].value("ObjectName", "");
				size_t dot = cn.find_last_of('.'); if (dot != std::string::npos) cn = cn.substr(dot+1);
				size_t q = cn.find_last_of('\''); if (q != std::string::npos) cn = cn.substr(0, q);
				if (m.count(cn)) { const json& w = *m.at(cn); Entity e{}; e.id = nextId++; e.name = w.value("Name", "Unnamed");
					if (p.contains("LayoutData")) { const auto& off = p["LayoutData"]["Offsets"]; e.x = off.value("Left", 0.0f); e.y = off.value("Top", 0.0f); e.width = off.value("Right", 100.0f); e.height = off.value("Bottom", 40.0f); }
					state.entities.push_back(e);
				}
			}
		}
	} catch(...) {}
}

void Editor::SaveUI(const std::string& path) {
	try { json data = json::array();
		for (const auto& e : state.entities) {
			json w = {{"Name", e.name}, {"Type", "Image"}};
			json s = {{"Type", "CanvasPanelSlot"}, {"Properties", {{"Content", {{"ObjectName", "Widget'"+e.name+"'"}}}, {"LayoutData", {{"Offsets", {{"Left", e.x}, {"Top", e.y}, {"Right", e.width}, {"Bottom", e.height}}}}}}}};
			data.push_back(w); data.push_back(s);
		}
		std::ofstream f(path); if (f.is_open()) f << data.dump(4);
	} catch(...) {}
}

void Editor::RefreshUIFileList() {
	uiFiles.clear(); uiFolders.clear(); assetFiles.clear(); if (!fs::exists(currentPath)) return;
	for (const auto& e : fs::directory_iterator(currentPath)) {
		if (e.is_directory()) uiFolders.push_back(e.path().filename().string());
		else if (e.is_regular_file()) { std::string ext = e.path().extension().string(); if (ext == ".json") uiFiles.push_back(e.path().filename().string()); else if (ext == ".png") assetFiles.push_back(e.path().filename().string()); }
	}
}
