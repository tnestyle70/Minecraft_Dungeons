#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CRedStoneGolemPart.h"

enum GOLEM_STATE
{
	GOLEM_STATE_IDLE,
	GOLEM_STATE_WALK,
	GOLEM_STATE_ATTACK,
	GOLEM_STATE_SKILL,
	GOLEM_STATE_DEAD,

	GOLEM_STATE_END
};

class CRedStoneGolem : public CGameObject
{
private:
	explicit CRedStoneGolem(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolem(const CRedStoneGolem& rhs);
	virtual ~CRedStoneGolem();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT			Add_Component();
	void Set_DefaultScale();
	void Set_WorldScale();
	void Set_PartsOffset();
	void Set_PartsParent();

private:
	void Debug_Input();
	void Golem_Animation(const _float& fTimeDelta);
	void Reset_Pose();
	void Idle_Animation();
	void Walk_Animation();
	void NormalAttack_Animation();
	void Skill_Animation();
	void Dead_Animation();
	
private:
	static constexpr _float m_fWorldScale = 2.f;

	CRedStoneGolemPart* m_pParts[GOLEM_END];

	Engine::CTransform* m_pTransformCom;
	Engine::CTexture* m_pTextureCom;
	Engine::CCollider* m_pColliderCom;

	GOLEM_STATE m_eState;

	_float m_fAnimTime;

public:
	static CRedStoneGolem* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

