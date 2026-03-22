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
	const _tchar* pTexTag = nullptr; //Proto_TextureName
	const _tchar* pName = nullptr; //Item Name
	const _tchar* pDesc = nullptr;
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
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

public:
	void Set_SlotState(eSlotState eState) { m_eState = eState; }
	eSlotState Get_SlotState() { return m_eState; }

	void Set_SlotInfo(float fX, float fY, float fW, float fH)
	{
		m_fX = fX; m_fY = fY; m_fW = fW; m_fH = fH;
	}

protected:
	virtual void Hover() override; //호버 
	virtual void Clicked() override; //클릭
	virtual void Leave() override;

	virtual void BeginUIRender();
	virtual void EndUIRender();

private:
	HRESULT Add_Component();
public:
	static CInventorySlot* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	CRcTex* m_pBufferCom = nullptr;
	CTexture* m_pFrameTexture = nullptr;
	CTexture* m_pHoverTexture = nullptr;
	CTexture* m_pClickedTexture = nullptr;

	//Slot State
	eSlotState m_eState = eSlotState::DEFAULT;

	//아이템 데이터 - 아이템의 종류와 탭
	ItemData m_tItemData;
	bool m_bEmpty = true;

private:
	virtual void Free();
};