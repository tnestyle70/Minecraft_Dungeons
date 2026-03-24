#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CEmerald : public CGameObject
{
private:
	explicit CEmerald(LPDIRECT3DDEVICE9 pGraphiDev);
	virtual ~CEmerald();

public:
	virtual	HRESULT Ready_GameObject();
	virtual	_int Update_GameObject(const _float& fTimeDelta);
	virtual	void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual	void Render_GameObject();

private:
	HRESULT	Add_Component();

private:
	CRcTex* m_pBufferCom;
	CTransform* m_pTransformCom;
	CTexture* m_pTextureCom;
	CCollider* m_pColliderCom;

public:
	static CEmerald* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

