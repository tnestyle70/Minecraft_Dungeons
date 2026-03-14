#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CBlockPlacer.h"

enum eIronBarState
{
	IDLE,
	MOVE_UP, 
	MOVE_DOWN
};

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
public:
	void Update_Animation(const _float& fTimeDelta);
	void MoveUp(const _float& fTimeDelta);
	void MoveDown(const _float& fTimeDelta);
public:
	void SetIronBarState(eIronBarState eState) { m_eState = eState; }
	bool IsClosed() { return m_bClosed; }
private:
	CCubeTex* m_pBufferCom = nullptr;
	CTransform* m_pTransformCom = nullptr;
	CTexture* m_pTextureCom = nullptr;
	CCollider* m_pColliderCom = nullptr;

	eIronBarState m_eState = IDLE;
	//최대값과 최소값 설정
	_float m_fMaxHeight = 1000.f;
	_float m_fMinHeight = -1000.f;
	_float m_fMoveSpeed = 5.f;

	_vec3 m_vPos = { 0.f, 0.f, 0.f };

	bool m_bClosed = false;
public:
	static CIronBar* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _vec3& vPos);
private:
	virtual void Free();
};