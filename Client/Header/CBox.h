#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CBoxPart.h"

class CBox : public CGameObject
{
private:
	explicit CBox(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CBox(const CBox& rhs);
	virtual ~CBox();

public:
	virtual	HRESULT Ready_GameObject();
	virtual	_int Update_GameObject(const _float& fTimeDelta);
	virtual	void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual	void Render_GameObject();

public:
	void Open_Box();
private:
	HRESULT	Add_Component();
	void Set_PartsOffset();
	void Set_WorldScale();
	void Set_PartsParent();

private:
	void Box_Animation();

private:
	static constexpr _float m_fWorldScale = 3.f;

	CBoxPart* m_pParts[BOX_END];

	CTexture* m_pTextureCom;
	CTransform* m_pTransformCom;
	CCollider* m_pColliderCom;

	_bool m_bIsOpen;
	_bool m_bIsOpening;
	_float m_fAnimTime;

public:
	static CBox* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

