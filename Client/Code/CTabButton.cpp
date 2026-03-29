#include "pch.h"
#include "pch.h"
#include "CTabButton.h"
#include "CRenderer.h"

CTabButton::CTabButton(LPDIRECT3DDEVICE9 pGraphicDev)
    : CUIInterface(pGraphicDev)
{}

CTabButton::~CTabButton() {}

HRESULT CTabButton::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;
    return S_OK;
}

_int CTabButton::Update_GameObject(const _float& fTimeDelta)
{
    int iExit = CUIInterface::Update_GameObject(fTimeDelta);

    return iExit;
}

void CTabButton::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CUIInterface::LateUpdate_GameObject(fTimeDelta);
}

void CTabButton::Render_GameObject()
{
    BeginUIRender();

    _matrix matWorld;
    float fNDCX = (m_fX + m_fW * 0.5f) / (WINCX * 0.5f) - 1.f;
    float fNDCY = 1.f - (m_fY + m_fH * 0.5f) / (WINCY * 0.5f);
    float fScaleX = m_fW / WINCX;
    float fScaleY = m_fH / WINCY;

    D3DXMatrixTransformation2D(&matWorld,
        nullptr, 0.f,
        &_vec2(fScaleX, fScaleY),
        nullptr, 0.f,
        &_vec2(fNDCX, fNDCY));

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    //Set_Texture
    //GPU의 텍스쳐 스테이지에 해당 이미지를 올려둠, 모든 도형에는 해당 텍스쳐로 그려짐
    //Vertex Texture에서 설정한 texture UV 값을 통해서 
    //GPU는 설정된 텍스쳐의 어디서부터 어디까지를 그릴지를 결정해서 렌더링한다

    switch (m_eState)
    {
    case eSlotState::DEFAULT: 
        m_pNormalTexture->Set_Texture(0); 
        break;
    case eSlotState::HOVER:
        m_pClickedTexture->Set_Texture(0);
        break;
    case eSlotState::CLICK:   
        m_pClickedTexture->Set_Texture(0); 
        break;
    }

    m_pBufferCom->Render_Buffer();

    EndUIRender();
}

void CTabButton::Hover() 
{ 
    if (m_eState != eSlotState::CLICK)
        m_eState = eSlotState::HOVER;
}
void CTabButton::Clicked()
{
    m_eState = eSlotState::CLICK; 
}
void CTabButton::Leave()
{
    if (m_eState == eSlotState::HOVER)
        m_eState = eSlotState::DEFAULT;
}

HRESULT CTabButton::Add_Component()
{
    CComponent* pComponent = nullptr;

    pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    // 탭 전용 텍스처 프로토타입 — CMainApp에서 등록 필요
    const wchar_t* pNormalProtoText = nullptr;
    const wchar_t* pClickedProtoText = nullptr;
    switch (m_eTab)
    {
    case eInventoryTab::SWORD:
        pNormalProtoText = L"Proto_SwordTabOff";
        pClickedProtoText = L"Proto_SwordTabOn";
        break;
    case eInventoryTab::ARMOR:
        pNormalProtoText = L"Proto_ArmorTabOff";
        pClickedProtoText = L"Proto_ArmorTabOn";
        break;
    case eInventoryTab::BOW:
        pNormalProtoText = L"Proto_BowTabOff";
        pClickedProtoText = L"Proto_BowTabOn";
        break;
    case eInventoryTab::INVENTORY_END:
        break;
    default:
        break;
    }

    pComponent = m_pNormalTexture = dynamic_cast<CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(pNormalProtoText));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_TabNormal", pComponent });

    pComponent = m_pClickedTexture = dynamic_cast<CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(pClickedProtoText));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_TabClicked", pComponent });

    return S_OK;
}

CTabButton* CTabButton::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    eInventoryTab eTab,
    float fX, float fY, float fW, float fH)
{
    CTabButton* pBtn = new CTabButton(pGraphicDev);
    pBtn->m_eTab = eTab;

    if (FAILED(pBtn->Ready_GameObject()))
    {
        Safe_Release(pBtn);
        MSG_BOX("CTabButton Create Failed");
        return nullptr;
    }

    pBtn->Set_Info(fX, fY, fW, fH);
    return pBtn;
}

void CTabButton::Free()
{
    CUIInterface::Free();
}