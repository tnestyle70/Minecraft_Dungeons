#pragma once
#include "CUIInterface.h"
#include "CInventorySlot.h"
#include "CEquipSlot.h"

class CItemPanel : public CUIInterface
{
private:
	explicit CItemPanel(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CItemPanel();

public:
	virtual HRESULT Ready_GameObject()                          override;
	virtual _int    Update_GameObject(const _float& fTimeDelta) override;
	virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
	virtual void    Render_GameObject()                         override;

public:
	void Set_Type(eEquipType eState) { m_eEquipType = eState; }
	
private:
	HRESULT         Add_Component();
	_matrix Calc_WorldMatrix(float fX, float fY, float fW, float fH);

protected:
	virtual void    Hover()     override;
	virtual void    Clicked()   override;
	virtual void    Leave()     override;

	virtual void BeginUIRender();
	virtual void EndUIRender();

public:
	// 더블클릭 감지
	bool Is_DoubleClicked() const { return m_bDoubleClicked; }
	void Consume_DoubleClick() { m_bDoubleClicked = false; }

	bool Is_Selected() { return m_bSelected; }
	void Set_Selected(bool bSelected) { m_bSelected = bSelected; }
	
private:
	DWORD  m_dwLastClickTime = 0;
	bool   m_bDoubleClicked = false;
	static constexpr DWORD DOUBLE_CLICK_MS = 300;
	
public:
	static CItemPanel* Create(LPDIRECT3DDEVICE9 pGraphicDev, eEquipType eType);
	
private: 
	//장비 타입에 따른 설명, 및 아이템 텍스쳐 다르게 렌더링하기
	eEquipType m_eEquipType = eEquipType::EQUIP_END;
	
	bool m_bSelected = true; 

	CTexture* m_pItemTexture = nullptr;
	CTexture* m_pUpgradeBtn1 = nullptr;
	CTexture* m_pUpgradeBtn2 = nullptr;
	CTexture* m_pUpgradeBtn3 = nullptr;
	// 아이템 이미지
	float m_fItemX = 1000.f, m_fItemY = 200.f;
	float m_fItemW = 150.f, m_fItemH = 150.f;
	// 버튼 공통 크기
	float m_fBtnW = 120.f, m_fBtnH = 120.f;
	// 버튼 1 
	float m_fBtn1X = 850.f, m_fBtn1Y = 550.f;
	// 버튼 2 
	float m_fBtn2X = 1000.f, m_fBtn2Y = 550.f;
	// 버튼 3 
	float m_fBtn3X = 1150.f, m_fBtn3Y = 550.f;
	
	_matrix m_matOriginView = {};
	_matrix m_matOriginProj = {};

protected:
	virtual void    Free() override;
};