#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CRedStoneGolemHipTex : public CVIBuffer
{
protected:
	explicit CRedStoneGolemHipTex();
	explicit CRedStoneGolemHipTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolemHipTex(const CRedStoneGolemHipTex& rhs);
	virtual ~CRedStoneGolemHipTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CRedStoneGolemHipTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END