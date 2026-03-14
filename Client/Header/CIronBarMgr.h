#pragma once
#include "CIronBar.h"

class CIronBarMgr : public CBase
{
	DECLARE_SINGLETON(CIronBarMgr)
private:
	explicit CIronBarMgr();
	virtual ~CIronBarMgr();
public:
	HRESULT Ready_IronBarMgr();
	void Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
public:
	void Open();
	void Close();
public:
	void AddIronBar(CGameObject* pGameObject);
	void Clear();
public:
	bool IsClosed() { return m_bClosed; };
private:
	void UpdateIronBarAnim();
private://IronBar을 컨테이너로 관리
	vector<CIronBar*> m_vecIronBars;
	bool m_bClosed = false;
private:
	virtual void Free();
};