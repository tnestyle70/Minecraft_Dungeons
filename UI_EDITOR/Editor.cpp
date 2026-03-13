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
#include "json.hpp"

#pragma comment(lib, "d3dx9.lib")

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

	struct EditorState {
		std::vector<Entity> entities;
		int selected = -1;
		bool dragging = false;
		ImVec2 dragOffset = ImVec2(0, 0);
	};

	EditorState state;
}

Editor::Editor() {
	RefreshUIFileList();

	Entity a{};
	a.name = "Button A";
	a.x = 50;
	a.y = 50;
	a.width = 120;
	a.height = 40;
	a.color = 0xFF64C8FFU;

	Entity b{};
	b.name = "Panel 1";
	b.x = 220;
	b.y = 30;
	b.width = 240;
	b.height = 160;
	b.color = 0xFFC8C8C8U;

	state.entities.push_back(a);
	state.entities.push_back(b);
}
Editor::~Editor() {
	if ( loaderThread.joinable() ) loaderThread.join();
	ReleaseFolder(rootLibrary);
}

void Editor::ReleaseFolder(TextureFolder& folder) {
	for ( auto& tex : folder.textures ) {
		if ( tex.texture ) ( (IDirect3DTexture9*)tex.texture )->Release();
	}
	for ( auto& pair : folder.subFolders ) {
		ReleaseFolder(*pair.second);
	}
	folder.textures.clear();
	folder.subFolders.clear();
}

void Editor::RenderTextureFolder(TextureFolder& folder) {
	if ( folder.name.empty() ) { // Root
		for ( auto& tex : folder.textures ) {
			// Render textures (this part is duplicated for now, will refactor)
		}
		for ( auto& pair : folder.subFolders ) {
			RenderTextureFolder(*pair.second);
		}
	}
	else {
		if ( ImGui::TreeNodeEx(folder.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen) ) {
			auto& style = ImGui::GetStyle();
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

			for ( size_t i = 0; i < folder.textures.size(); ++i ) {
				ImGui::PushID((int)i);
				ImGui::BeginGroup();

				ImVec2 size(64, 64);
				ImVec2 uv0(0, 0);
				ImVec2 uv1(1, 1);
				if ( folder.textures[i].sizeX > 0 && folder.textures[i].sizeY > 0 ) {
					uv1.x = (float)folder.textures[i].importedSizeX / (float)folder.textures[i].sizeX;
					uv1.y = (float)folder.textures[i].importedSizeY / (float)folder.textures[i].sizeY;
				}

				std::string btn_id = "##tex_btn_" + std::to_string(i);
				if ( ImGui::ImageButton(btn_id.c_str(), (ImTextureID)folder.textures[i].texture, size, uv0, uv1) ) {
					Entity e{};
					e.name = folder.textures[i].name;
					e.width = (float)folder.textures[i].importedSizeX;
					e.height = (float)folder.textures[i].importedSizeY;
					e.color = 0xFFFFFFFFU;
					e.texture = folder.textures[i].texture;
					e.uv_u = uv1.x;
					e.uv_v = uv1.y;
					state.entities.push_back(e);
				}
				if ( ImGui::IsItemHovered() ) {
					ImGui::SetTooltip("%s\n%dx%d", folder.textures[i].name.c_str(),
									  folder.textures[i].importedSizeX, folder.textures[i].importedSizeY);
				}
				ImGui::Text("%s", folder.textures[i].name.c_str());
				ImGui::EndGroup();

				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + size.x;
				if ( i + 1 < folder.textures.size() && next_button_x2 < window_visible_x2 )
					ImGui::SameLine();

				ImGui::PopID();
			}

			for ( auto& pair : folder.subFolders ) {
				RenderTextureFolder(*pair.second);
			}
			ImGui::TreePop();
		}
	}
}

void Editor::Update() {
	// 마우스 키보드 입력 처리
}

void Editor::Render() {
	auto& style = ImGui::GetStyle();

	// --------------------------------------------------------
	// 1. 메인 메뉴바
	// --------------------------------------------------------
	if ( ImGui::BeginMainMenuBar() ) {
		if ( ImGui::BeginMenu("File") ) {
			if ( ImGui::MenuItem("New") ) {
				state.entities.clear();
				state.selected = -1;
			}
			if ( ImGui::BeginMenu("Browse Folders") ) {
				if ( ImGui::MenuItem("..") ) {
					currentPath = fs::path(currentPath).parent_path().string();
					RefreshUIFileList();
				}
				for ( const auto& folder : uiFolders ) {
					if ( ImGui::MenuItem(folder.c_str()) ) {
						currentPath = ( fs::path(currentPath) / folder ).string();
						RefreshUIFileList();
					}
				}
				ImGui::EndMenu();
			}
			if ( ImGui::BeginMenu("Load Asset (PNG)") ) {
				for ( const auto& file : assetFiles ) {
					if ( ImGui::MenuItem(file.c_str()) ) {
						std::string fullPath = ( fs::path(currentPath) / file ).string();
						std::string basePath = fullPath.substr(0, fullPath.find_last_of('.'));
						LoadUnrealTexture2D(d3dDevice, basePath, file);
					}
				}
				if ( assetFiles.empty() ) ImGui::TextDisabled("(No PNGs)");
				ImGui::EndMenu();
			}
			if ( ImGui::BeginMenu("Load UI Layout (JSON)") ) {
				for ( const auto& file : uiFiles ) {
					if ( ImGui::MenuItem(file.c_str()) ) {
						LoadUI(( fs::path(currentPath) / file ).string());
					}
				}
				if ( uiFiles.empty() ) ImGui::TextDisabled("(No JSONs)");
				ImGui::EndMenu();
			}
			if ( ImGui::MenuItem("Exit") ) PostQuitMessage(0);
			ImGui::EndMenu();
		}

		if ( isLoading ) {
			float progress = (float)currentFilesLoaded / (float)totalFilesToLoad;
			char buf[32];
			sprintf_s(buf, "%d / %d", currentFilesLoaded.load(), totalFilesToLoad.load());
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 210);
			ImGui::ProgressBar(progress, ImVec2(200, 0), buf);
		}

		ImGui::EndMainMenuBar();
	}

	// --------------------------------------------------------
	// 2. 전체 화면 DockSpace
	// --------------------------------------------------------
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags host_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace Window", nullptr, host_window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
	ImGui::End();

	// --------------------------------------------------------
	// Toolbar (Top)
	// --------------------------------------------------------
	ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	if ( ImGui::Button("Add Box") ) {
		Entity e{};
		e.name = "New Box";
		e.width = 100; e.height = 100;
		e.color = 0xFFFFFFFFU;
		state.entities.push_back(e);
		state.selected = (int)state.entities.size() - 1;
	}
	ImGui::SameLine();
	if ( ImGui::Button("Add Text") ) {
		Entity e{};
		e.name = "New Text";
		e.width = 150; e.height = 30;
		e.color = 0xFFFFFF00U;
		state.entities.push_back(e);
		state.selected = (int)state.entities.size() - 1;
	}
	ImGui::SameLine();
	if ( ImGui::Button("Add Image") ) {
		Entity e{};
		e.name = "New Image";
		e.width = 200; e.height = 200;
		e.color = 0xFF64C8FFU;
		state.entities.push_back(e);
		state.selected = (int)state.entities.size() - 1;
	}
	ImGui::End();

	// --------------------------------------------------------
	// 3. Hierarchy (좌측)
	// --------------------------------------------------------
	ImGui::Begin("Hierarchy");
	for (int i = 0; i < (int)state.entities.size(); ++i ) {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (i == state.selected )
			flags |= ImGuiTreeNodeFlags_Selected;
		ImGui::TreeNodeEx((void*)i, flags, "%s", state.entities[i].name.c_str());
		if (ImGui::IsItemClicked())
			state.selected = i;
	}
	ImGui::End();

	// --------------------------------------------------------
	// 4. Canvas (중앙: 1280x720 고정 작업 공간)
	// --------------------------------------------------------
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

	ImVec2 canvas_avail = ImGui::GetContentRegionAvail();
	ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// 전체 창 배경 (어두운 회색)
	draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_avail.x, canvas_p0.y + canvas_avail.y), IM_COL32(35, 35, 35, 255));

	// 1280x720 작업 영역 중앙 정렬
	const float WORK_W = 1280.0f;
	const float WORK_H = 720.0f;

	float offset_x = ( canvas_avail.x > WORK_W ) ? ( canvas_avail.x - WORK_W ) * 0.5f : 0.0f;
	float offset_y = ( canvas_avail.y > WORK_H ) ? ( canvas_avail.y - WORK_H ) * 0.5f : 0.0f;

	ImVec2 work_p0 = ImVec2(canvas_p0.x + offset_x, canvas_p0.y + offset_y);
	ImVec2 work_p1 = ImVec2(work_p0.x + WORK_W, work_p0.y + WORK_H);

	// 작업 영역 배경 (살짝 밝은 회색)
	draw_list->AddRectFilled(work_p0, work_p1, IM_COL32(50, 50, 50, 255));

	// Draw Grid
	const float grid_step = 64.0f;
	for ( float x = 0; x <= WORK_W; x += grid_step )
		draw_list->AddLine(ImVec2(work_p0.x + x, work_p0.y), ImVec2(work_p0.x + x, work_p1.y), IM_COL32(200, 200, 200, 40));
	for ( float y = 0; y <= WORK_H; y += grid_step )
		draw_list->AddLine(ImVec2(work_p0.x, work_p0.y + y), ImVec2(work_p1.x, work_p0.y + y), IM_COL32(200, 200, 200, 40));

	draw_list->AddRect(work_p0, work_p1, IM_COL32(255, 255, 255, 100), 0.0f, 0, 2.0f);

	// ✨ [핵심] 엔티티 렌더링 & 완벽한 드래그 로직 (ImGui 네이티브 방식)
	for ( size_t i = 0; i < state.entities.size(); ++i ) {
		Entity& e = state.entities[i];
		ImVec2 e_min(work_p0.x + e.x, work_p0.y + e.y);
		ImVec2 e_max(e_min.x + e.width, e_min.y + e.height);

		// 1. ImGui에게 이 영역이 상호작용 가능한 '버튼'임을 알림
		ImGui::SetCursorScreenPos(e_min);
		ImGui::PushID((int)i); // 고유 ID 부여 (필수)
		ImGui::InvisibleButton("##entity", ImVec2(e.width, e.height));

		// 2. 상태 체크
		if ( ImGui::IsItemActivated() ) {
			state.selected = (int)i;
			state.dragOffset = ImVec2(e.x, e.y);
		}

		if ( ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) ) {
			ImVec2 mouse_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
			e.x = std::clamp(state.dragOffset.x + mouse_delta.x, 0.0f, WORK_W - e.width);
			e.y = std::clamp(state.dragOffset.y + mouse_delta.y, 0.0f, WORK_H - e.height);
		}
		ImGui::PopID();

		// 3. 눈에 보이는 도형 렌더링
		ImU32 col = IM_COL32(( e.color >> 16 ) & 0xFF, ( e.color >> 8 ) & 0xFF, ( e.color ) & 0xFF, ( e.color >> 24 ) & 0xFF);
		if ( e.texture ) {
			draw_list->AddImage((ImTextureID)e.texture, e_min, e_max, ImVec2(0, 0), ImVec2(e.uv_u, e.uv_v), col);
		}
		else {
			draw_list->AddRectFilled(e_min, e_max, col);
		}
		draw_list->AddRect(e_min, e_max, IM_COL32(0, 0, 0, 200));

		// 선택되었을 때 노란 테두리
		if ( (int)i == state.selected ) {
			draw_list->AddRect(e_min, e_max, IM_COL32(255, 255, 0, 255), 0.0f, 0, 3.0f);
		}
	}

	if ( ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() ) {
		state.selected = -1;
	}

	ImGui::SetCursorPos(ImVec2(offset_x, offset_y));
	ImGui::Dummy(ImVec2(WORK_W, WORK_H));

	ImGui::End();
	ImGui::PopStyleVar();

	// --------------------------------------------------------
	// 5. Properties (우측)
	// --------------------------------------------------------
	ImGui::Begin("Properties");
	if ( state.selected >= 0 && state.selected < (int)state.entities.size() ) {
		Entity& s = state.entities[state.selected];
		char buf[256];
		strncpy_s(buf, s.name.c_str(), _TRUNCATE);
		if ( ImGui::InputText("Name", buf, sizeof(buf)) ) s.name = buf;
		ImGui::DragFloat2("Position", &s.x, 1.0f);
		ImGui::DragFloat2("Size", &s.width, 1.0f, 1.0f, 2000.0f);

		ImVec4 colf = ImGui::ColorConvertU32ToFloat4(s.color);
		if ( ImGui::ColorEdit4("Color", (float*)&colf) ) {
			s.color = ImGui::ColorConvertFloat4ToU32(colf);
		}
		if ( ImGui::Button("Delete") ) {
			state.entities.erase(state.entities.begin() + state.selected);
			state.selected = -1;
		}
	}
	else {
		ImGui::TextDisabled("No selection");
	}
	ImGui::End();

	// --------------------------------------------------------
	// 6. Texture Library (하단/우측)
	// --------------------------------------------------------
	ImGui::Begin("Texture Library");

	std::lock_guard<std::mutex> lock(libraryMutex);

	if ( rootLibrary.textures.empty() && rootLibrary.subFolders.empty() ) {
		if ( isLoading ) ImGui::Text("Loading textures... (%d / %d)", currentFilesLoaded.load(), totalFilesToLoad.load());
		else ImGui::TextDisabled("No textures loaded. Use 'Load Assets (Folder)' from menu.");
	}
	else {
		RenderTextureFolder(rootLibrary);
	}
	ImGui::End();
}

void Editor::LoadUnrealTexture2D(void* device, const std::string& fullPath, const std::string& relativePath) {
	auto* d3dDevicePtr = static_cast<IDirect3DDevice9*>( device );
	if ( d3dDevicePtr == nullptr ) return;

	Texture2D tex{};
	std::string baseName = fs::path(fullPath).stem().string();

	std::ifstream file(fullPath + ".json");
	if ( file.is_open() ) {
		try {
			json data = json::parse(file);
			if ( data.is_array() && !data.empty() ) {
				const auto& jsonData = data[0];
				tex.name = jsonData.value("Name", baseName);
				tex.pixelFormat = jsonData.value("PixelFormat", "Unknown");
				if ( jsonData.contains("Properties") && jsonData["Properties"].contains("ImportedSize") ) {
					tex.importedSizeX = jsonData["Properties"]["ImportedSize"].value("X", 0);
					tex.importedSizeY = jsonData["Properties"]["ImportedSize"].value("Y", 0);
				}
				tex.sizeX = jsonData.value("SizeX", 0);
				tex.sizeY = jsonData.value("SizeY", 0);
			}
		}
		catch ( ... ) {}
	}

	D3DXIMAGE_INFO info;
	IDirect3DTexture9* d3dTex = nullptr;
	if ( SUCCEEDED(D3DXCreateTextureFromFileExA(d3dDevicePtr, ( fullPath + ".png" ).c_str(),
												D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
												D3DX_DEFAULT, D3DX_DEFAULT, 0, &info, nullptr, &d3dTex)) ) {
		tex.texture = d3dTex;
		if ( tex.sizeX == 0 ) tex.sizeX = info.Width;
		if ( tex.sizeY == 0 ) tex.sizeY = info.Height;
		if ( tex.importedSizeX == 0 ) tex.importedSizeX = info.Width;
		if ( tex.importedSizeY == 0 ) tex.importedSizeY = info.Height;

		std::lock_guard<std::mutex> lock(libraryMutex);

		TextureFolder* current = &rootLibrary;
		fs::path rel(relativePath);
		fs::path parent = rel.parent_path();

		for ( const auto& part : parent ) {
			std::string folderName = part.string();
			if ( current->subFolders.find(folderName) == current->subFolders.end() ) {
				current->subFolders[folderName] = std::make_unique<TextureFolder>();
				current->subFolders[folderName]->name = folderName;
			}
			current = current->subFolders[folderName].get();
		}
		current->textures.push_back(tex);
	}
}

void Editor::LoadFolder(void* device, const std::string& path) {
	if ( device == nullptr ) return;
	d3dDevice = device;

	if ( isLoading ) return;

	if ( loaderThread.joinable() ) loaderThread.join();

	// Clear old library
	ReleaseFolder(rootLibrary);

	if ( !fs::exists(path) ) return;

	isLoading = true;
	currentFilesLoaded = 0;
	totalFilesToLoad = 0;

	loaderThread = std::thread([this, device, path]() {
		struct PathInfo { std::string full; std::string rel; };
		std::vector<PathInfo> paths;

		for ( const auto& entry : fs::recursive_directory_iterator(path) ) {
			if ( entry.is_regular_file() && entry.path().extension() == ".png" ) {
				std::string fullPath = entry.path().string();
				std::string relPath = fs::relative(entry.path(), path).string();

				// Strip extension
				size_t lastDot = fullPath.find_last_of('.');
				size_t lastDotRel = relPath.find_last_of('.');

				paths.push_back({
					fullPath.substr(0, lastDot),
					relPath.substr(0, lastDotRel)
								});
			}
		}

		totalFilesToLoad = (int)paths.size();

		for ( const auto& p : paths ) {
			LoadUnrealTexture2D(device, p.full, p.rel);
			currentFilesLoaded++;
		}
		isLoading = false;
							   });
}

void Editor::LoadUI(const std::string& path) {
	std::ifstream file(path);
	if ( !file.is_open() ) return;

	try {
		json data = json::parse(file);
		if ( !data.is_array() ) return;

		state.entities.clear();
		state.selected = -1;

		std::map<std::string, const json*> objectMap;
		for ( const auto& obj : data ) {
			if ( obj.contains("Name") ) {
				objectMap[obj["Name"].get<std::string>()] = &obj;
			}
		}

		for ( const auto& obj : data ) {
			if ( obj.value("Type", "") == "CanvasPanelSlot" ) {
				if ( !obj.contains("Properties") ) continue;
				const auto& props = obj["Properties"];

				if ( props.contains("Content") ) {
					std::string contentName = props["Content"].value("ObjectName", "");
					size_t lastDot = contentName.find_last_of('.');
					if ( lastDot != std::string::npos ) {
						contentName = contentName.substr(lastDot + 1);
					}
					size_t lastQuote = contentName.find_last_of('\'');
					if ( lastQuote != std::string::npos ) {
						contentName = contentName.substr(0, lastQuote);
					}

					if ( objectMap.count(contentName) ) {
						const json& widget = *objectMap.at(contentName);
						Entity e{};
						e.name = widget.value("Name", "Unnamed");

						std::string type = widget.value("Type", "");
						if ( type == "TextBlock" ) e.color = 0xFFFFFFFFU;
						else if ( type == "Image" ) e.color = 0xFF64C8FFU;
						else if ( type == "Button" ) e.color = 0xFFC8C8C8U;
						else e.color = 0xFF888888U;

						if ( props.contains("LayoutData") ) {
							const auto& layout = props["LayoutData"];
							if ( layout.contains("Offsets") ) {
								const auto& offsets = layout["Offsets"];
								e.x = offsets.value("Left", 0.0f);
								e.y = offsets.value("Top", 0.0f);
								e.width = offsets.value("Right", 100.0f);
								e.height = offsets.value("Bottom", 40.0f);
							}
						}
						state.entities.push_back(e);
					}
				}
			}
		}
	}
	catch ( const std::exception& e ) {
		OutputDebugStringA(( "LoadUI Error: " + std::string(e.what()) + "\n" ).c_str());
	}
}

void Editor::RefreshUIFileList() {
	uiFiles.clear();
	uiFolders.clear();
	assetFiles.clear();

	if ( !fs::exists(currentPath) ) return;

	for ( const auto& entry : fs::directory_iterator(currentPath) ) {
		if ( entry.is_directory() ) {
			uiFolders.push_back(entry.path().filename().string());
		}
		else if ( entry.is_regular_file() ) {
			std::string ext = entry.path().extension().string();
			if ( ext == ".json" ) {
				uiFiles.push_back(entry.path().filename().string());
			}
			else if ( ext == ".png" ) {
				assetFiles.push_back(entry.path().filename().string());
			}
		}
	}
}
