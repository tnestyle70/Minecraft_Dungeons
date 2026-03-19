#pragma once
#include "CUIInterface.h"

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
	void SetSlotInfo(float fX, float fY, float fW, float fH)
	{
		m_fX = fX; m_fY = fY; m_fW = fW; m_fH = fH;
	}

protected:
	virtual void Hover() override; //호버 
	virtual void Clicked() override; //클릭
	virtual void Leave() override;

private:
	HRESULT Add_Component();
public:
	static CInventorySlot* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	CRcTex* m_pBufferCom = nullptr;
	CTexture* m_pNormalTexture = nullptr;
	CTexture* m_pHoverTexture = nullptr;
	CTexture* m_pClickedTexture = nullptr;

private:
	virtual void Free();
};