#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CRedStoneGolemShoulderTex : public CVIBuffer
{
protected:
	explicit CRedStoneGolemShoulderTex();
	explicit CRedStoneGolemShoulderTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolemShoulderTex(const CRedStoneGolemShoulderTex& rhs);
	virtual ~CRedStoneGolemShoulderTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CRedStoneGolemShoulderTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END