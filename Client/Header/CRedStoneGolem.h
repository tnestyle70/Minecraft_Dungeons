#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CRedStoneGolemPart.h"
#include "IGolemState.h"
#include "CGolemStates.h"

enum GOLEM_STATE
{
	GOLEM_STATE_IDLE,
	GOLEM_STATE_WALK,
	GOLEM_STATE_ATTACK,
	GOLEM_STATE_SKILL,
	GOLEM_STATE_HIT,
	GOLEM_STATE_DEAD,

	GOLEM_STATE_END
};

class CRedStoneGolem : public CGameObject
{
private:
	explicit CRedStoneGolem(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CRedStoneGolem();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

public:
	void Change_State(GOLEM_STATE eState);
	void Anim_Idle();
	void Anim_Walk();
	void Anim_NormalAttack();
	void Anim_Skill();
	void Anim_Hit();
	void Anim_Dead();

	void LookAt_Player();
	void Check_Distance();
	void Chase_Player(const _float& fTimeDelta);
	void Reset_Pose();

	void Set_AnimTime(_float f) { m_fAnimTime = f; }
	_float Get_AnimTime() const { return m_fAnimTime; }
	_bool Check_AttackHit();
	void Check_Hit();
	void Take_Damage(_float iDamage);

private:
	HRESULT	Add_Component();
	void Set_DefaultScale();
	void Set_WorldScale();
	void Set_PartsOffset();
	void Set_PartsParent();

private:
	void Debug_Input();
	void Golem_Animation(const _float& fTimeDelta);

private:
	void Apply_Gravity(const _float& fTimeDelta);
	void Resolve_BlockCollision();

private:
	static constexpr _float m_fWorldScale = 4.f;
	static constexpr _float m_fGravity = -20.f;
	static constexpr _float m_fMaxFall = -20.f;

	CRedStoneGolemPart* m_pParts[GOLEM_END];
	IGolemState* m_pCurState = nullptr;
	IGolemState* m_pStates[GOLEM_STATE_END] = {};

	Engine::CTransform* m_pTransformCom;
	Engine::CTexture* m_pTextureCom;
	Engine::CCollider* m_pColliderCom;
	Engine::CCollider* m_pAtkColliderCom;

	GOLEM_STATE m_eState;

	_float m_fAnimTime;

	_bool m_bOnGround;
	_float m_fVelocityY;

	_bool m_bHitCool = false;
	_float m_fHitCoolTime = 0.f;

	_float m_fMaxHp;
	_float m_fHp;
	_float m_fAtk;

public:
	static CRedStoneGolem* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

