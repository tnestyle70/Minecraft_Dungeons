#pragma once
#include "CBase.h"
#include "Engine_Define.h"

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
	virtual _int Update(const _float& fTimeDelta);
	virtual void LateUpdate(const _float& fTimeDelta);
	virtual void Render();
public:
	void AddDamage(_vec3 vWorldPos, _int iDamage);
private:
	_vec2 WorldToScreen(const _vec3& vWorldPos);
private:
	LPDIRECT3DDEVICE9 m_pGraphicDev;
	list<DAMAGE_TEXT> m_listDamage;
	static constexpr _float m_fLifeTime = 1.5f; //수명 시간
	static constexpr _float m_fRisingSpeed = 40.f;
private:
	virtual void Free();
};