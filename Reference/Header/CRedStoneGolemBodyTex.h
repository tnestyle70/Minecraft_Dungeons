#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CRedStoneGolemBodyTex : public CVIBuffer
{
protected:
	explicit CRedStoneGolemBodyTex();
	explicit CRedStoneGolemBodyTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolemBodyTex(const CRedStoneGolemBodyTex& rhs);
	virtual ~CRedStoneGolemBodyTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CRedStoneGolemBodyTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END