#pragma once
#include "CIronBar.h"

class CIronBarMgr : public CBase
{
private:
	explicit CIronBarMgr();
	virtual ~CIronBarMgr();
public:
	HRESULT Ready_IronBarMgr();
	void Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
public:
	void AddIronBar(CIronBar* pIronBar);
	void Clear();
public:
	bool IsOpen() { return m_bIsOpened; };
	bool SetOpen(bool bIsOpen) { m_bIsOpened = bIsOpen; }
	bool SetTrigger(bool bTrigger) { m_bTriggered = bTrigger; }
private:
	void UpdateIronBarAnim();
private://IronBar을 컨테이너로 관리
	vector<CIronBar*> m_vecIronBars;
	bool m_bIsOpened = false;
	bool m_bTriggered = false;
private:
	virtual void Free();
};