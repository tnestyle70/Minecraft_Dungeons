#pragma once
#include "CUIInterface.h"
#include "CInventorySlot.h"

enum class eEquipType
{
	MELEE = 0, //근접
	ARMOR, //방어구
	RANGED, //원거리
	EQUIP_END
};

class CEquipSlot : public CUIInterface
{
private:
	explicit CEquipSlot(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CEquipSlot();
public:
	virtual HRESULT Ready_GameObject()                          override;
	virtual _int    Update_GameObject(const _float& fTimeDelta) override;
	virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
	virtual void    Render_GameObject()                         override;

public:
	eSlotState Get_State() { return m_eState; }
	void Set_State(eSlotState eState) { m_eState = eState; }

	void Set_Equipped(bool bEquipped) { m_bEquipped = bEquipped; }
	bool Is_Equipped() { return m_bEquipped; }

private:
	HRESULT         Add_Component();

protected:
	virtual void    Hover()     override;
	virtual void    Clicked()   override;
	virtual void    Leave()     override;

	virtual void BeginUIRender();
	virtual void EndUIRender();

public:
	static CEquipSlot* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		eEquipType eType,
		float fX, float fY,
		float fW, float fH);

private:
	eEquipType m_eEquipType = eEquipType::EQUIP_END;
	eSlotState m_eState = eSlotState::DEFAULT;

	bool m_bEquipped = false; //장착 여부 판단

	CTexture* m_pFrameTexture = nullptr;
	CTexture* m_pClickedTexture = nullptr;

	ItemData m_tItemData;
	bool m_bEmpty = true;

protected:
	virtual void    Free() override;
};