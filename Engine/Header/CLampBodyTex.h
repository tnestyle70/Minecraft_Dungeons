#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CLampBodyTex : public CVIBuffer
{
protected:
	explicit CLampBodyTex();
	explicit CLampBodyTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CLampBodyTex(const CLampBodyTex& rhs);
	virtual ~CLampBodyTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CLampBodyTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END