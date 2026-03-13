#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CRedStoneGolemHeadTex : public CVIBuffer
{
protected:
	explicit CRedStoneGolemHeadTex();
	explicit CRedStoneGolemHeadTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolemHeadTex(const CRedStoneGolemHeadTex& rhs);
	virtual ~CRedStoneGolemHeadTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CRedStoneGolemHeadTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END