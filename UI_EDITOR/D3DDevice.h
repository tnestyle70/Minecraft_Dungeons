// D3DDevice.h
#pragma once
#include <memory>

class D3DDevice {

public:
	D3DDevice();
	~D3DDevice();

	D3DDevice(const D3DDevice&) = delete;
	D3DDevice& operator=(const D3DDevice&) = delete;

	bool Initialize(void* handle);
	void BeginFrame();
	long EndFrame();
	bool Reset();

	void* GetDevice() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};