#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CRedStoneGolemArmTex : public CVIBuffer
{
protected:
	explicit CRedStoneGolemArmTex();
	explicit CRedStoneGolemArmTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolemArmTex(const CRedStoneGolemArmTex& rhs);
	virtual ~CRedStoneGolemArmTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CRedStoneGolemArmTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END