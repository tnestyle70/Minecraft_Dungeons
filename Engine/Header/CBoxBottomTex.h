#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CBoxBottomTex : public CVIBuffer
{
protected:
	explicit CBoxBottomTex();
	explicit CBoxBottomTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CBoxBottomTex(const CBoxBottomTex& rhs);
	virtual ~CBoxBottomTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CBoxBottomTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();

};

END