#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CPlayerBody.h"
#include "CPlayerArrow.h"
#include "CTNT.h"
#include "CEquipSlot.h"

class CMonster;
class CRedStoneGolem;
class CAncientGuardian;

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

enum ARMOR_TYPE
{
	ARMOR_NONE = 0,
	ARMOR_BARDSGARD,
	ARMOR_END
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
	void			Render_Part(BODYPART ePart, _float fAngleX, _float fAngleY, _float fAngleZ,
								const _matrix& matRootWorld, Engine::CTexture* pTex = nullptr, CPlayerBody* pBuf = nullptr);

	void			Render_Sword(float fAtkX, float fAtkY, float fSwing);
	void			Render_Bow();

public:
	const vector<CPlayerArrow*>& Get_Arrows() const { return m_vecArrows; }

private:
	//플레이어 정보

	_float m_fHp = 100.f;
	_float m_fMaxHp = 100.f;
	_float m_fMeleeDmg = 10.f;
	_float m_fBowDmg = 10.f;
	_float m_fMoveSpeed = 30.f;

	_float m_fBowCooldown = 0.f;
	static constexpr float m_fBowCoolMax = 1.f;

	ARMOR_TYPE m_eArmorType = ARMOR_NONE;
	_matrix m_matPartWorld[PART_END];

	//공격모션
	_int   m_iComboStep = 0;      // 0=대기, 1=우→좌, 2=좌→우, 3=찌르기
	_float m_fAtkTime = 0.f;    // 현재 공격 경과 시간
	_float m_fAtkDuration = 0.55f;   // 공격 지속 시간
	_float m_fComboWindow = 0.6f;   // 다음 공격 입력 대기 시간
	_float m_fComboTimer = 0.f;    // 콤보 타이머
	_bool  m_bAtkInput = false;  // 공격 입력 감지
	_bool  m_bAtkKeyPrev = false;

	//화살 발사 변수
	_float m_fCharge = 0.f;
	_float m_fMaxCharge = 2.f;
	_bool  m_bCharging = false;
	_vec3 m_vBowDir = { 0.f, 0.f, 1.f };
	vector<CPlayerArrow*> m_vecArrows;
	_matrix m_matLArmWorld;		//왼손위치
	//폭죽화살 변수
	_bool m_bFireworkArrow = false;
	_bool m_bRKeyPrev = false;
	_float m_fLastChargeRatio = 0.f;
	
	//TNT
	CTNT* m_pHeldTNT = nullptr;
	vector<CTNT*> m_vecTNTs;
	
	//활 발사 모션
	Engine::CTexture* m_pBowTexture[4] = {};  // 0=standby, 1~3=pulling
	Engine::CRcTex* m_pBowBufferCom = nullptr;



	Engine::CCollider* m_pAtkColliderCom = nullptr; //공격 콜라이더
	_bool m_bAtkColliderActive = false;

	//공격 이펙트
	LPDIRECT3DTEXTURE9 m_pAttackTexture = nullptr;

public:
	//플레이어 정보 Get / Set
	_float Get_Hp() const { return m_fHp; }
	_float Get_MaxHp() const { return m_fMaxHp; }
	void Set_Hp(float fHp) { m_fHp = fHp; }

	_float Get_MeleeDmg() const { return m_fMeleeDmg; }
	_float Get_BowDmg() const { return m_fBowDmg + 15.f * m_fLastChargeRatio; }
	void Set_MeleeDmg(float fDmg) { m_fMeleeDmg = fDmg; }
	void Set_BowDmg(float fDmg) { m_fBowDmg = fDmg; }


	void Set_Armor(ARMOR_TYPE eType) { m_eArmorType = eType; }
	ARMOR_TYPE Get_ArmorType() const { return m_eArmorType; }

	//콜라이더 박스 온오프 확인
	_bool Get_AtkColliderActive() const { return m_bAtkColliderActive; }

	//TNT 추가함수
	void Add_TNT(CTNT* pTNT) { m_vecTNTs.push_back(pTNT); }
	const vector<CTNT*>& Get_TNTs() const { return m_vecTNTs; }
	
	//보스 추가함수
	void Set_Boss(CRedStoneGolem* pBoss) { m_pTargetBoss = pBoss; }

	void Set_Guardian(CAncientGuardian* pGuardian) { m_pTargetGuardian = pGuardian; }

	CTransform* Get_Transform() { return m_pTransformCom; }

	void Equip(eEquipType eType);
	void UnEquip(eEquipType eType);
private:
	CPlayerBody* m_pBufferCom[PART_END];
	CPlayerBody* m_pArmorBufferCom[PART_END] = {};
	Engine::CTransform* m_pTransformCom;
	Engine::CTexture* m_pTextureCom;
	Engine::CCalculator* m_pCalculatorCom;
	Engine::CCollider* m_pColliderCom;

	//칼
	Engine::CRcTex* m_pSwordBufferCom = nullptr;
	Engine::CTexture* m_pSwordTextureCom = nullptr;

	//아머
	Engine::CTexture* m_pArmorTextureCom = nullptr;

	_matrix m_matRArmWorld;  // 오른팔 정보저장, 칼 위치 등 활용하기 위함

	_vec3				m_vPartOffset[PART_END];
	_vec3				m_vPartScale[PART_END];

	_float				m_fWalkTime;	// 걷기 누적 시간 (사인파 입력)
	_bool				m_bMoving;		// 이동 중 여부

	_vec3 m_vTargetPos;
	_bool  m_bHasTarget = false;

	_float m_fGravity = -20.f;
	_float m_fJumpPower = 8.f;
	_float m_fMaxFall = -20.f;

	_float m_fVelocityY = 0.f;
	_bool m_bOnGround = false;


	//구르기
	_bool   m_bRolling = false;
	_float  m_fRollTime = 0.f;
	_float  m_fRollCooldown = 0.f;

	_float m_fRollDuration = 0.5f;   
	_float m_fRollSpeed = 22.f;   
	_float m_fRollCoolMax = 3.f;    
	_vec3  m_vRollDir; 

	//=======FootPrint Effect Variable=======
	Engine::CParticleEmitter* m_pFootStepEmitter = nullptr;
	Engine::CParticleEmitter* m_pAttackEmitter = nullptr;

	_bool m_bSwordEquipped = true;
	_bool m_bBowEquipped = false;

private:
	//피격
	_bool  m_bHit = false;
	_float m_fHitTime = 0.f;

	// 넉백
	_bool  m_bKnockback = false;
	_float m_fKnockbackTime = 0.f;
	_float m_fKnockbackDuration = 0.3f;
	_float m_fKnockbackSpeed = 20.f;
	_float m_fKnockbackThreshold = 12.f;
	_vec3 m_vKnockbackDir = {};

	_float m_fHitDuration = 0.5f;
	//몬스터 공격타겟
	CMonster* m_pTargetMonster = nullptr;
	//레드스톤골렘 타겟
	CRedStoneGolem* m_pTargetBoss = nullptr;
	//가디언 타겟
	CAncientGuardian* m_pTargetGuardian = nullptr;

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

	void Hit(float fDamage);

private:
	virtual void Free();

};