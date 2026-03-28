#include "pch.h"
#include "CDialogueBox.h"
#include "CRenderer.h"
#include "CFontMgr.h"

CDialogueBox::CDialogueBox(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
    D3DXMatrixIdentity(&m_matOriginView);
    D3DXMatrixIdentity(&m_matOriginProj);
}

CDialogueBox::~CDialogueBox()
{
}

HRESULT CDialogueBox::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;
    return S_OK;
}

_int CDialogueBox::Update_GameObject(const _float& fTimeDelta)
{
    if (!m_bVisible)
        return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);
    CRenderer::GetInstance()->Add_RenderGroup(RENDER_UI, this);
    return iExit;
}

void CDialogueBox::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CDialogueBox::Render_GameObject()
{
    if (!m_bVisible)
        return;

    Render_BeginUI();

    // 박스 위치/크기 설정
    float fNDCX = (m_fX + m_fW * 0.5f) / (WINCX * 0.5f) - 1.f;
    float fNDCY = 1.f - (m_fY + m_fH * 0.5f) / (WINCY * 0.5f);

    _matrix matWorld;
    D3DXMatrixTransformation2D(&matWorld,
        nullptr, 0.f,
        &_vec2(m_fW / WINCX, m_fH / WINCY),
        nullptr, 0.f,
        &_vec2(fNDCX, fNDCY));

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
    m_pTextureCom->Set_Texture(0);
    m_pBufferCom->Render_Buffer();

    Render_EndUI();

    // 이름
    _vec2 vNamePos = { m_fX + 80.f, m_fY + 30.f };
    CFontMgr::GetInstance()->Render_Font(
        L"Font_Minecraft", m_strName.c_str(), &vNamePos, D3DXCOLOR(1.f, 0.9f, 0.2f, 1.f));

    // 대화 텍스트
    _vec2 vTextPos = { m_fX + 100.f, m_fY + 80.f };
    wstring strRemain = m_strText;
    float fLineHeight = 35.f;
    int iMaxChars = 25; // 한 줄 최대 글자 수

    while (!strRemain.empty())
    {
        wstring strLine = strRemain.substr(0, iMaxChars);
        strRemain = strRemain.size() > iMaxChars ? strRemain.substr(iMaxChars) : L"";
        CFontMgr::GetInstance()->Render_Font(
            L"Font_Minecraft", strLine.c_str(), &vTextPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
        vTextPos.y += fLineHeight;
    }
}

HRESULT CDialogueBox::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pBufferCom = dynamic_cast<Engine::CRcTex*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

    if (!pComponent) 
       return E_FAIL;

    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_DialogueBoxTexture"));

    if (!pComponent)
        return E_FAIL;

    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    return S_OK;
}

void CDialogueBox::Render_BeginUI()
{
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &m_matOriginView);
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &m_matOriginProj);

    _matrix matIdentity;
    D3DXMatrixIdentity(&matIdentity);
    m_pGraphicDev->SetTransform(D3DTS_VIEW, &matIdentity);
    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matIdentity);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
}

void CDialogueBox::Render_EndUI()
{
    m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matOriginView);
    m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &m_matOriginProj);

    m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CDialogueBox::Show(const wstring& strName, const wstring& strText)
{
    m_strName = strName;
    m_strText = strText;
    m_bVisible = true;
}

void CDialogueBox::Hide()
{
    m_bVisible = false;
}

CDialogueBox* CDialogueBox::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CDialogueBox* pInstance = new CDialogueBox(pGraphicDev);

    if (FAILED(pInstance->Ready_GameObject()))
    {
        Safe_Release(pInstance);
        MSG_BOX("CDialogueBox Create Failed");
        return nullptr;
    }

    return pInstance;
}

void CDialogueBox::Free()
{
    CGameObject::Free();
}