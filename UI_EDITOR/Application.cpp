// Application.cpp
#include "Application.h"
#include "Window.h"
#include "D3DDevice.h"
#include "ImGuiLayer.h"
#include "Editor.h"
#include <chrono>
#include <thread>

Application::Application()
	: window_(std::make_unique<Window>())
	, device_(std::make_unique<D3DDevice>())
	, imgui_(std::make_unique<ImGuiLayer>())
	, editor_(std::make_unique<Editor>()) {
}
Application::~Application() = default;

int Application::Run() {

	if ( !window_->Create(1440, 810, L"UI Editor") )
		return -1;

	void* hWnd = (void*)window_->GetHandle();
	if ( !device_->Initialize(hWnd) )
		return -2;

	imgui_->Initialize(hWnd, device_->GetDevice());
	initialized_ = true;

	editor_->LoadFolder(
		device_->GetDevice(),
		"../Client/Bin/Resource/Texture/UI/Materials"
	);

	using clock = std::chrono::high_resolution_clock;
	auto last = clock::now();

	while ( window_->ProcessMessages() ) {

		auto now = clock::now();
		float delta = ( ( std::chrono::duration<float> )( now - last ) ).count();
		last = now;

		device_->BeginFrame();

		imgui_->Begin();
		editor_->Update();
		editor_->Render();
		imgui_->End();

		imgui_->UpdatePlatform();

		device_->EndFrame();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return 0;
}