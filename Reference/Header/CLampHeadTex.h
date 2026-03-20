#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CLampHeadTex : public CVIBuffer
{
protected:
	explicit CLampHeadTex();
	explicit CLampHeadTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CLampHeadTex(const CLampHeadTex& rhs);
	virtual ~CLampHeadTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CLampHeadTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END