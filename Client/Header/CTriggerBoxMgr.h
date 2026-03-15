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
	_int Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
public://스테이지에서 생성시에 매니저 호출해서 TriggerBox 컨테이너 채워주기
	void AddTriggerBox(CGameObject* pGameObject);
	void Clear();
	bool IsSceneChanged() { return m_bSceneChanged; }
	void SetSceneChanged(bool changed) { m_bSceneChanged = changed; }
private:
	//스테이지 넘어갈 때 TriggerBox Container 비워주기
	vector<CTriggerBox*> m_vecTriggerBox;
	bool m_bSceneChanged = false;
public:
	void SetPlayerCollider(CCollider* pCollider) { m_pPlayerCollider = pCollider; }
private:
	//플레이어 콜라이더를 Stage 시작 시에 한 번만 캐싱해두고,
	//개별 TriggerBox에 Update때마다 주입시킨다.
	CCollider* m_pPlayerCollider = nullptr;
private:
	virtual void Free();
};