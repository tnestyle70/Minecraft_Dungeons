#pragma once
#include "CUIInterface.h"

enum class eInventoryTab
{
	SWORD = 0,
	ARMOR,
	BOW,
	INVENTORY_END
};

enum class eItemType
{
	NORMAL_SWORD,
	NORMAL_ARMOR,
	NORMAL_BOW,
	ITEM_END
};

struct ItemData
{
	eItemType eType = eItemType::ITEM_END;
	eInventoryTab eTab = eInventoryTab::INVENTORY_END;
	CRcTex* m_pBufferCom = nullptr;
	CTexture* m_pItemTexture = nullptr;
	const _tchar* pName = nullptr; //Item Name
	const _tchar* pDesc = nullptr; // Item Description
	_int iAtk = 0;
	_int iDefense = 0;
};

enum class eSlotState
{
	DEFAULT,
	HOVER,
	CLICK
};


class CInventorySlot : public CUIInterface
{
private:
	explicit CInventorySlot(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CInventorySlot();
	
public:
	virtual			HRESULT		Ready_GameObject(eInventoryTab eTab);
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

public:
	void Set_SlotState(eSlotState eState) { m_eState = eState; }
	eSlotState Get_SlotState() { return m_eState; }

	void Set_SlotInfo(float fX, float fY, float fW, float fH, eInventoryTab eType);

	void Set_ItemInfo(float fX, float fY, float fW, float fH);
	void Clear_Item() { m_tItemData = {}; m_bEmpty = true; }
	const ItemData& Get_ItemData() const { return m_tItemData; }
	bool Is_Empty()        const { return m_bEmpty; }

	// 더블클릭 감지
	bool Is_DoubleClicked() const { return m_bDoubleClicked; }
	void Consume_DoubleClick() { m_bDoubleClicked = false; }

private:
	DWORD  m_dwLastClickTime = 0;
	bool   m_bDoubleClicked = false;
	static constexpr DWORD DOUBLE_CLICK_MS = 300;

protected:
	virtual void Hover() override; //호버 
	virtual void Clicked() override; //클릭
	virtual void Leave() override;

	virtual void BeginUIRender();
	virtual void EndUIRender();

private:
	HRESULT Add_Component();

	_matrix Calc_WorldMatrix(float fX, float fY, float fW, float fH);

	void BeginItemRender();
	void EndItemRender();
public:
	static CInventorySlot* Create(LPDIRECT3DDEVICE9 pGraphicDev, eInventoryTab eTab);
private:
	CRcTex* m_pBufferCom = nullptr;
	CTexture* m_pFrameTexture = nullptr;
	CTexture* m_pHoverTexture = nullptr;
	CTexture* m_pClickedTexture = nullptr;
	
	CTexture* m_pItemTexture = nullptr;
	eInventoryTab m_eInventoryTab = eInventoryTab::INVENTORY_END;

	//Slot State
	eSlotState m_eState = eSlotState::DEFAULT;

	//아이템 데이터 - 아이템의 종류와 탭
	ItemData m_tItemData;
	bool m_bEmpty = true;

	float m_fItemX, m_fItemY, m_fItemW, m_fItemH;
private:
	virtual void Free();
};