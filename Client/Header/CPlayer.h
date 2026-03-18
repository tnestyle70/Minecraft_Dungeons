#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CPlayerBody.h"
#include "CPlayerArrow.h"

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
	//_vec3			Picking_OnTerrain();
	_vec3			Picking_OnBlock();
	void			Render_Part(BODYPART ePart, _float fAngleX, _float fAngleY, _float fAngleZ, const _matrix& matRootWorld);

	void			Render_Sword(float fAtkX, float fAtkY, float fSwing);
	void			Render_Bow();
private:
	//플레이어 정보

	float m_fHp = 100.f;
	float m_fMaxHp = 100.f;

	//공격모션
	int   m_iComboStep = 0;      // 0=대기, 1=우→좌, 2=좌→우, 3=찌르기
	float m_fAtkTime = 0.f;    // 현재 공격 경과 시간
	float m_fAtkDuration = 0.3f;   // 공격 지속 시간
	float m_fComboWindow = 0.6f;   // 다음 공격 입력 대기 시간
	float m_fComboTimer = 0.f;    // 콤보 타이머
	bool  m_bAtkInput = false;  // 공격 입력 감지
	bool  m_bAtkKeyPrev = false;

	//화살 발사 변수
	float m_fCharge = 0.f;
	float m_fMaxCharge = 2.f;
	bool  m_bCharging = false;
	vector<CPlayerArrow*> m_vecArrows;
	_matrix m_matLArmWorld;		//왼손위치

	//활 발사 모션
	Engine::CTexture* m_pBowTexture[4] = {};  // 0=standby, 1~3=pulling
	Engine::CRcTex* m_pBowBufferCom = nullptr;



	Engine::CCollider* m_pAtkColliderCom = nullptr; //공격 콜라이더
	bool m_bAtkColliderActive = false;

private:
	CPlayerBody* m_pBufferCom[PART_END];
	Engine::CTransform* m_pTransformCom;
	Engine::CTexture* m_pTextureCom;
	Engine::CCalculator* m_pCalculatorCom;
	Engine::CCollider* m_pColliderCom;

	//칼
	Engine::CRcTex* m_pSwordBufferCom = nullptr;
	Engine::CTexture* m_pSwordTextureCom = nullptr;

	_matrix m_matRArmWorld;  // 오른팔 정보저장, 칼 위치 등 활용하기 위함

	_vec3				m_vPartOffset[PART_END];
	_vec3				m_vPartScale[PART_END];

	_float				m_fWalkTime;	// 걷기 누적 시간 (사인파 입력)
	_bool				m_bMoving;		// 이동 중 여부

	_vec3 m_vTargetPos;
	bool  m_bHasTarget = false;

	static constexpr float m_fGravity = -20.f;
	static constexpr float m_fJumpPower = 8.f;
	static constexpr float m_fMaxFall = -20.f;

	float m_fVelocityY = 0.f;
	bool m_bOnGround = false;


	//구르기
	bool   m_bRolling = false;
	float  m_fRollTime = 0.f;
	float  m_fRollCooldown = 0.f;

	static constexpr float m_fRollDuration = 0.5f;   
	static constexpr float m_fRollSpeed = 22.f;   
	static constexpr float m_fRollCoolMax = 3.f;    
	_vec3  m_vRollDir; 

	//=======FootPrint Effect Variable=======
	Engine::CParticleEmitter* m_pFootStepEmitter = nullptr;
	Engine::CParticleEmitter* m_pAttackEmitter = nullptr;

private:
	//피격
	bool  m_bHit = false;
	float m_fHitTime = 0.f;
	static constexpr float m_fHitDuration = 0.5f;



private: //중력 적용과 충돌시 위치값 보정
	void Apply_Gravity(const _float& fTimeDelta);
	void Resolve_BlockCollision();

	//구르기
	void Roll_Update(const _float& fTimeDelta);

	void Attack_Collision();

	//공격 모션 계산함수
	void Calc_AttackMotion(float& fAtkX, float& fAtkY, float& fTorsoY);


public:
	static CPlayer* Create(LPDIRECT3DDEVICE9 pGraphicDev);

	void Hit();

private:
	virtual void Free();

};