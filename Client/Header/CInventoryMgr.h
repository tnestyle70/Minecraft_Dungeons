#pragma once
#include "CBase.h"
#include "CProtoMgr.h"

enum class eInventoryTab
{
	SWORD = 0,
	ARMOR,
	BOW,
	INVENTORY_END
};

class CInventoryMgr : public CBase
{
	DECLARE_SINGLETON(CInventoryMgr)
private:
	explicit CInventoryMgr();
	virtual ~CInventoryMgr();
public:
	HRESULT Ready_InventoryMgr(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual _int Update(const _float& fTimeDelta);
	virtual void LateUpdate(const _float& fTimeDelta);
	virtual void Render();
public:
	bool IsActive() { return m_bActive; }
	void SetInventory(bool bActive) { m_bActive = bActive; }
private:
	LPDIRECT3DDEVICE9 m_pGraphicDev;
	bool m_bActive = false; //활성 여부

	eInventoryTab m_eTab = eInventoryTab::SWORD;

	map<eInventoryTab, _int> m_mapInventoryTab;
	
private:
	virtual void Free();
};