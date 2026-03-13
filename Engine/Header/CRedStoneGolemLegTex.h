#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CRedStoneGolemLegTex : public CVIBuffer
{
protected:
	explicit CRedStoneGolemLegTex();
	explicit CRedStoneGolemLegTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolemLegTex(const CRedStoneGolemLegTex& rhs);
	virtual ~CRedStoneGolemLegTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CRedStoneGolemLegTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END