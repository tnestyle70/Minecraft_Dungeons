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
	void Init_Emerald();
	void Pop_Emerald(const _float fTimeDelta);
	void Chase_Player(const _float fTimeDelta);

private:
	HRESULT	Add_Component();

private:
	CRcTex* m_pBufferCom;
	CTransform* m_pTransformCom;
	CTexture* m_pTextureCom;
	CCollider* m_pColliderCom;

	_vec3 m_vVelocity;
	_bool m_bDrop = false;
	_bool m_bChase = false;
	_bool m_bDead = false;
	_bool m_bFirstPop = true;

public:
	static CEmerald* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

