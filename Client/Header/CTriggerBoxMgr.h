#pragma once
#include "CTriggerBox.h"

//트리거 박스 밟았을 경우, IronBarMgr, MonsterMgr에 정보 전달

class CTriggerBoxMgr : public CBase
{
	DECLARE_SINGLETON(CTriggerBoxMgr)
private:
	explicit CTriggerBoxMgr();
	virtual ~CTriggerBoxMgr();
public:
	HRESULT Ready_TriggerBox();
	void Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
public://스테이지에서 생성시에 매니저 호출해서 TriggerBox 컨테이너 채워주기
	void AddTriggerBox(CTriggerBox* pTriggerBox);
	void Clear();
	//스테이지 넘어갈 때 TriggerBox Container 비워주기
	vector<CTriggerBox*> m_vecTriggerBox;
private:
	virtual void Free();
};