#pragma once
#include "CBase.h"
#include "CProtoMgr.h"

class CAncientGuardian;
class CRedStoneGolem;

struct DAMAGE_TEXT
{
	_vec3 vWorldPos; //데미지 발생 위치
	_int iDamage; //받은 데미지
	_float fLifeTime; //수명
	_float fOffsetY; //위로 올라가는 오프셋
};

//플레이어의 데미지를 몬스터 쪽에 띄우기
class CDamageMgr : public CBase
{
	DECLARE_SINGLETON(CDamageMgr)
private:
	explicit CDamageMgr();
	virtual ~CDamageMgr();
public:
	HRESULT Ready_DamageMgr(LPDIRECT3DDEVICE9 pGraphicDev);
	HRESULT Ready_Component();
	virtual _int Update(const _float& fTimeDelta);
	virtual void LateUpdate(const _float& fTimeDelta);
	virtual void Render();
private:
	void Render_BossHP();

	void Render_SingleBossHP(float fHP, float fMaxHP, bool bIdle);
public:
	void AddDamage(_vec3 vWorldPos, _int iDamage);

	void Set_Guardian(CAncientGuardian* pGuardian) { m_pGuardian = pGuardian; }
	void Set_RedStone(CRedStoneGolem* pGolem) { m_pRedStoneGolem = pGolem; }

	void Clear_Boss();
	void Clear_Guardian();
	void Clear_RedStone();
private:
	HRESULT Add_Component();

	_vec2 WorldToScreen(const _vec3& vWorldPos);
private:
	LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;

	CAncientGuardian* m_pGuardian = nullptr;
	CRedStoneGolem* m_pRedStoneGolem = nullptr;

	CTexture* m_pHealthBar = nullptr;
	CTexture* m_pEmptyHealthBar = nullptr;
	CRcTex* m_pBufferCom = nullptr;

	list<DAMAGE_TEXT> m_listDamage;
	static constexpr _float m_fLifeTime = 1.5f; //수명 시간
	static constexpr _float m_fRisingSpeed = 40.f;

	float m_fBarX = 300.f;
	float m_fBarY = 50.f; 
	float m_fBarW = 700.f;;
	float m_fBarH = 30.f;
private:
	virtual void Free();
};