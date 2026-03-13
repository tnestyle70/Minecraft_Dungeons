#pragma once
#include "CMonster.h"

//스테이지별로 트리거 박스 밟으면 생성될 몬스터 목록들 저장해두고, 
//트리거 박스 밟을 경우 몬스터들 지정된 위치로 쭉 스폰되도록 설정
class CMonsterMgr : public CBase
{
	DECLARE_SINGLETON(CMonsterMgr)
private:
	explicit CMonsterMgr();
	virtual ~CMonsterMgr();
public:
	HRESULT Ready_MonsterMgr();
	
};