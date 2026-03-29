#pragma once
#include "CBase.h"
#include "CProtoMgr.h"
#include "CInventorySlot.h"
#include "CTabButton.h"
#include "CEquipSlot.h"
#include "CItemPanel.h"

class CPlayer;
class CInventoryBackground;

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

	void Set_Player(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void Clear_Player() { m_pPlayer = nullptr; }
private:
	void Init_Items();

	void Update_SlotHover();
	void Update_SlotClick();
	void Update_TabClick();
	void Update_SlotSelection();
	void Update_DoubleClickEquip();
	void Update_EquipSlot(); //장착 슬롯 <-> 인벤토리 슬롯 연결

	void Render_PlayerPreview();

	void Clear_ClickedSlot();
	//장착 슬롯
	CEquipSlot* m_arrEquipSlot[(int)eEquipType::EQUIP_END] = {};
	//탭 슬롯
	CTabButton* m_arrTabButton[(int)eInventoryTab::INVENTORY_END] = {};
	//중앙 인벤토리 12개
	vector<CInventorySlot*> m_vecSlots[(int)eInventoryTab::INVENTORY_END] = {};
	//아이템 패널 설명창
	CItemPanel* m_arrItemPanel[(int)eEquipType::EQUIP_END] = {};
	//현재 선택된 슬롯, 장비창
	CInventorySlot* m_pClickedSlot = nullptr;
	CEquipSlot* m_pEquipSlot = nullptr;
	//플레이어 
	CPlayer* m_pPlayer = nullptr;
	//인벤토리 배경
	CInventoryBackground* m_pInventoryBG = nullptr;
	
private:
	LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;

	bool m_bI_Prev = false;
	bool m_bActive = false; //활성 여부
	//현재 인벤토리 탭
	eInventoryTab m_eTab = eInventoryTab::SWORD;
	
private:
	virtual void Free();
};