#pragma once
#include "CUIInterface.h"
#include "CInventorySlot.h" // eInventoryTab, eSlotState

class CTabButton : public CUIInterface
{
private:
    explicit CTabButton(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CTabButton();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

public:
    eInventoryTab   Get_Tab()   const { return m_eTab; }
    eSlotState      Get_State() const { return m_eState; }
    void            Set_State(eSlotState eState) { m_eState = eState; }
    void            Set_Info(float fX, float fY, float fW, float fH)
    {
        m_fX = fX; m_fY = fY; m_fW = fW; m_fH = fH;
    }

protected:
    virtual void Hover()   override;
    virtual void Clicked() override;
    virtual void Leave()   override;

private:
    HRESULT Add_Component();

public:
    static CTabButton* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        eInventoryTab eTab,
        float fX, float fY, float fW, float fH);
private:
    CRcTex* m_pBufferCom = nullptr;
    CTexture* m_pNormalTexture = nullptr;
    CTexture* m_pClickedTexture = nullptr;

    eInventoryTab m_eTab = eInventoryTab::SWORD;
    eSlotState    m_eState = eSlotState::DEFAULT;

private:
    virtual void Free();
};