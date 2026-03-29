#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CBlockPlacer.h"

enum eJumpingTrapState
{
	TRAP_IDLE,
	TRA_ACTIVE
};

class CJumpingTrap : public CGameObject
{
private:
	explicit CJumpingTrap(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CJumpingTrap(const CGameObject& rhs);
	virtual ~CJumpingTrap();
public:
	virtual HRESULT Ready_GameObject(const _vec3& vPos);
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();
private:
	HRESULT Add_Component();
public:
	void Update_Active(const _float& fTimeDelta);
public:
	void Set_JumpingTrapState(eJumpingTrapState eState) { m_eState = eState; }

	void Set_Active(bool bActive);
	bool Is_Active() { return m_bActive; }

private:
	CCubeTex* m_pBufferCom = nullptr;
	CTransform* m_pTransformCom = nullptr;
	CTexture* m_pTextureCom = nullptr;
	CCollider* m_pColliderCom = nullptr;

	eJumpingTrapState m_eState = TRAP_IDLE;
	//높이, 속도 설정
	_float m_fMaxHeight = 3.f;
	_float m_fActiveSpeed = 20.f;
	_float m_fDeactiveSpeed = 10.f;

	_vec3 m_vOriginPos = { 0.f, 0.f, 0.f };

	bool m_bActive = false;
	bool m_bRising = false;
	
public:
	static CJumpingTrap* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _vec3& vPos);

private:
	virtual void Free();
};
