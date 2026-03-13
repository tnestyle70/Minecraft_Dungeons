#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CRedStoneGolemCoreTex : public CVIBuffer
{
protected:
	explicit CRedStoneGolemCoreTex();
	explicit CRedStoneGolemCoreTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolemCoreTex(const CRedStoneGolemCoreTex& rhs);
	virtual ~CRedStoneGolemCoreTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

public:
	static CRedStoneGolemCoreTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END