#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CDialogueBox : public CGameObject
{
private:
    explicit CDialogueBox(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CDialogueBox();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

public:
    void Show(const wstring& strName, const wstring& strText);
    void Hide();
    bool Is_Visible() const { return m_bVisible; }

private:
    HRESULT Add_Component();
    void    Render_BeginUI();
    void    Render_EndUI();

public:
    static CDialogueBox* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free();

private:
    Engine::CRcTex* m_pBufferCom = nullptr;
    Engine::CTexture* m_pTextureCom = nullptr;

    _matrix m_matOriginView;
    _matrix m_matOriginProj;

    bool    m_bVisible = false;
    wstring m_strName = L"";
    wstring m_strText = L"";

    float m_fX = 200.f;
    float m_fY = 450.f;
    float m_fW = 900.f;
    float m_fH = 250.f;
};