// Application.h
#pragma once
#include <memory>

class Window;
class D3DDevice;
class ImGuiLayer;
class Editor;

class Application {
public:
	Application();
	~Application();

	int Run();

private:
	std::unique_ptr<Window>     window_;
	std::unique_ptr<D3DDevice>  device_;
	std::unique_ptr<ImGuiLayer> imgui_;
	std::unique_ptr<Editor>     editor_;

	bool initialized_ = false;
};