// D3DDevice.cpp
#include "D3DDevice.h"
#include <d3d9.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

struct D3DDevice::Impl {
	ComPtr<IDirect3D9> d3d;
	ComPtr<IDirect3DDevice9> device;
	D3DPRESENT_PARAMETERS d3dpp{};
};

D3DDevice::D3DDevice() : impl_(std::make_unique<Impl>()) {}
D3DDevice::~D3DDevice() = default;

bool D3DDevice::Initialize(void* handle) {
	impl_->d3d.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if ( impl_->d3d == nullptr ) return false;

	ZeroMemory(&impl_->d3dpp, sizeof(impl_->d3dpp));
	impl_->d3dpp.Windowed = TRUE;
	impl_->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	impl_->d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	impl_->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT result = impl_->d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		reinterpret_cast<HWND>( handle ),
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
		&impl_->d3dpp,
		impl_->device.GetAddressOf());

	return SUCCEEDED(result);
}

void D3DDevice::BeginFrame() {

	if ( impl_->device == nullptr ) return;

	impl_->device->Clear(
		0, nullptr,
		D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(114, 144, 154),
		1.0f, 0
	);

	HRESULT result = impl_->device->BeginScene();

	if ( SUCCEEDED(result) ) {
		// nothing
	}
}

HRESULT D3DDevice::EndFrame() {
	if ( impl_->device == nullptr ) return D3DERR_INVALIDCALL;

	impl_->device->EndScene();
	return impl_->device->Present(nullptr, nullptr, nullptr, nullptr);
}

bool D3DDevice::Reset() {
	if ( impl_->device == nullptr ) return false;

	HRESULT result = impl_->device->Reset(&impl_->d3dpp);

	return result == D3D_OK;
}

void* D3DDevice::GetDevice() const {
	return reinterpret_cast<void*>( impl_->device.Get() );
}