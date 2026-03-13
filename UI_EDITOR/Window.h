// Window.h
#pragma once
#include <memory>

class Window {
public:
	using Handle = void*;

	Window();
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool Create(int width, int height, const wchar_t* title);
	Handle GetHandle() const;
	bool ProcessMessages();

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

