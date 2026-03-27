#pragma once
#include "CGameObject.h"

class CCrystal : public CGameObject
{
private:
	explicit CCrystal(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CCrystal();

public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

private:
	HRESULT Add_Component();

public:
	static CCrystal* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

