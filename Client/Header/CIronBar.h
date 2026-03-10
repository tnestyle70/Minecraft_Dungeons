#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CBlockPlacer.h"

class CIronBar : public CGameObject
{
private:
	explicit CIronBar(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CIronBar(const CGameObject& rhs);
	virtual ~CIronBar();
public:
	virtual HRESULT Ready_GameObject(const _vec3& vPos);
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();
private:
	HRESULT Add_Component();
private:
	CCubeTex* m_pBufferCom = nullptr;
	CTransform* m_pTransCom = nullptr;
	CTexture* m_pTextureCom = nullptr;
	CCollider* m_pCollider = nullptr;
	
	_vec3 m_vPos = { 0.f, 0.f, 0.f };
public:
	static CIronBar* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _vec3& vPos);
private:
	virtual void Free();
};