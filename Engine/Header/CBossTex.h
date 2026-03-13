#pragma once
#include "CVIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CBossTex : public CVIBuffer
{
protected:
	explicit CBossTex();
	explicit CBossTex(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CBossTex(const CBossTex& rhs);
	virtual ~CBossTex();

public:
	HRESULT Ready_Buffer();
	virtual void Render_Buffer();

private:
	void Compute_MaxScale();

public:
	static CBossTex* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void Free();
};

END