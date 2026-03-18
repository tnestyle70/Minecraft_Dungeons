#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CBoxTopTex : public CVIBuffer
{
protected:
	explicit CBoxTopTex();
	explicit CBoxTopTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CBoxTopTex(const CBoxTopTex& rhs);
	virtual ~CBoxTopTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CBoxTopTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();

};

END