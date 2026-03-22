#pragma once
#include "CBase.h"
#include "CProtoMgr.h"
#include "CInventorySlot.h"
#include "CTabButton.h"
#include "CEquipSlot.h"

struct TabButton
{
	eInventoryTab eTab;
	float fX, fY, fW, fH;
};

struct InventorySlotUI
{
	ItemData eData;
	eSlotState eState = eSlotState::DEFAULT;
	float fX, fY;
	float fW, fH;
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
	void Init_Items();

	void Update_SlotHover();
	void Update_SlotClick();
	void Update_TabClick();
	void Update_SlotSelection();
	void Update_EquipSlot(); //장착 슬롯 <-> 인벤토리 슬롯 연결

	void Clear_ClickedSlot();
	//장착 슬롯
	CEquipSlot* m_arrEquipSlot[(int)eEquipType::EQUIP_END] = {};
	//탭 슬롯
	CTabButton* m_arrTabButton[(int)eInventoryTab::INVENTORY_END];
	//중앙 인벤토리 12개
	vector<CInventorySlot*> m_vecSlots[(int)eInventoryTab::INVENTORY_END];
	//현재 선택된 슬롯
	CInventorySlot* m_pClickedSlot = nullptr;

private:
	LPDIRECT3DDEVICE9 m_pGraphicDev;

	bool m_bI_Prev = false;
	bool m_bActive = false; //활성 여부
	//현재 인벤토리 탭
	eInventoryTab m_eTab = eInventoryTab::SWORD;
	
private:
	virtual void Free();
};