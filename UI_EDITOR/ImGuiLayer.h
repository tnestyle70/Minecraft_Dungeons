// ImGuiLayer.h
#pragma once

class ImGuiLayer {
public:
	ImGuiLayer();
	~ImGuiLayer();

	ImGuiLayer(const ImGuiLayer&) = delete;
	ImGuiLayer& operator=(const ImGuiLayer&) = delete;

	void Initialize(void* handle, void* d3dDevice);
	void Begin();
	void End();
	void Shutdown();

	void UpdatePlatform();
};