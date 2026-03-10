#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CPlayerBody.h"

enum BODYPART
{
	PART_HEAD,
	PART_BODY,
	PART_LARM,
	PART_RARM,
	PART_LLEG,
	PART_RLEG,
	PART_END
};

class CPlayer : public CGameObject
{
private:
	explicit CPlayer(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CPlayer(const CGameObject& rhs);
	virtual ~CPlayer();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT			Add_Component();
	void			Key_Input(const _float& fTimeDelta);
	void			Set_OnTerrain();
	_vec3			Picking_OnTerrain();
	void			Render_Part(BODYPART ePart, _float fAngle = 0.f);

private:
	CPlayerBody* m_pBufferCom[PART_END];
	Engine::CTransform* m_pTransformCom;
	Engine::CTexture* m_pTextureCom;
	Engine::CCalculator* m_pCalculatorCom;
	Engine::CCollider* m_pColliderCom;

	_vec3				m_vPartOffset[PART_END];
	_vec3				m_vPartScale[PART_END];

	_float				m_fWalkTime;	// 걷기 누적 시간 (사인파 입력)
	_bool				m_bMoving;		// 이동 중 여부

	static constexpr float m_fGravity = -20.f;
	static constexpr float m_fJumpPower = 8.f;
	static constexpr float m_fMaxFall = -20.f;

	float m_fVelocityY = 0.f;
	bool m_bOnGround = false;
private: //중력 적용과 충돌시 위치값 보정
	void Apply_Gravity(const _float& fTimeDelta);
	void Resolve_BlockCollision();
public:
	static CPlayer* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();

};

// 1. 스카이박스 출력 하기
// 2. 뷰 스페이스 변환 행렬, 원근 투영 행렬을 직접 구현하여 적용해라
