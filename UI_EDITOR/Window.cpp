// Window.cpp
#include "Window.h"
#include <windows.h>
#include <string>

// Forward declare ImGui Win32 handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct Window::Impl {
	HWND hWnd = nullptr;
	std::wstring className;
};

Window::Window() : impl_(std::make_unique<Impl>()) {}
Window::~Window() = default;

bool Window::Create(int width, int height, const wchar_t* title) {

	HINSTANCE hInst = GetModuleHandle(nullptr);
	impl_->className = L"UIEditor_Window_Class";

	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
		// Pass messages to ImGui
		if ( ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) )
			return true;

		if ( msg == WM_NCCREATE ) {
			CREATESTRUCTW* cs = (CREATESTRUCT*)lParam;
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
		}

		auto* pImpl = (Window::Impl*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		if ( msg == WM_DESTROY ) {
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProcW(hWnd, msg, wParam, lParam);
		};
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
	wcex.lpszClassName = impl_->className.c_str();

	if ( !RegisterClassExW(&wcex) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS ) {
		return false;
	}

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	impl_->hWnd = CreateWindowExW(0,
								  impl_->className.c_str(), title,
								  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
								  rc.right - rc.left, rc.bottom - rc.top,
								  nullptr, nullptr, hInst, this);

	if ( impl_->hWnd == nullptr ) {
		return false;
	}

	ShowWindow(impl_->hWnd, SW_SHOWDEFAULT);
	UpdateWindow(impl_->hWnd);

	return true;
}

Window::Handle Window::GetHandle() const {
	return reinterpret_cast<Handle>( impl_->hWnd );
}

bool Window::ProcessMessages() {

	MSG msg{};
	while ( PeekMessageW(&msg, nullptr, NULL, NULL, PM_REMOVE) ) {
		if ( msg.message == WM_QUIT )
			return false;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return true;
}