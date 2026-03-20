#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CLampPart.h"

class CLamp : public CGameObject
{
private:
	explicit CLamp(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CLamp();

public:
	virtual	HRESULT Ready_GameObject();
	virtual	_int Update_GameObject(const _float& fTimeDelta);
	virtual	void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual	void Render_GameObject();

private:
	HRESULT	Add_Component();
	void Set_PartsOffset();
	void Set_WorldScale();
	void Set_PartsParent();

private:
	static constexpr _float m_fWorldScale = 1.f;

	CLampPart* m_pParts[LAMP_END];

	CTexture* m_pTextureCom;
	CTransform* m_pTransformCom;
	CCollider* m_pColliderCom;

	_float m_fAnimTime;

public:
	static CLamp* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

