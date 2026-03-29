#pragma once
#include "CUIInterface.h"

class CInventoryBackground : public CUIInterface
{
private:
    explicit CInventoryBackground(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CInventoryBackground();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

public:
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
    static CInventoryBackground* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        float fX, float fY, float fW, float fH);
private:
    CRcTex* m_pBufferCom = nullptr;
    CTexture* m_pTexture = nullptr;

private:
    virtual void Free();
};