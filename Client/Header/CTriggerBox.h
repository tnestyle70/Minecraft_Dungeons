#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CTriggerBox : public CGameObject
{
private:
	explicit CTriggerBox(LPDIRECT3DDEVICE9 pGarphicDev);
	explicit CTriggerBox(const CGameObject& rhs);
	virtual ~CTriggerBox();
public: //Initialize pos
	virtual HRESULT Ready_GameObject(const _vec3& vPos);
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();
private:
	HRESULT Add_Component();
private:
	CTransform* m_pTransformCom = nullptr;
	CCollider* m_pColliderCom = nullptr;

	_vec3 m_vPos = { 0.f, 0.f, 0.f };
public:
	static CTriggerBox* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _vec3& vPos);
private:
	virtual void Free();
};