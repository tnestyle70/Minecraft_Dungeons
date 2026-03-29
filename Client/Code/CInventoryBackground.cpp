#include "pch.h"
#include "CInventoryBackground.h"

CInventoryBackground::CInventoryBackground(LPDIRECT3DDEVICE9 pGraphicDev)
	:CUIInterface(pGraphicDev)
{}

CInventoryBackground::~CInventoryBackground()
{}

HRESULT CInventoryBackground::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;
	return S_OK;
}

_int CInventoryBackground::Update_GameObject(const _float& fTimeDelta)
{
	int iExit = CUIInterface::Update_GameObject(fTimeDelta);

	return iExit;
}

void CInventoryBackground::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CUIInterface::LateUpdate_GameObject(fTimeDelta);
}

void CInventoryBackground::Render_GameObject()
{
    BeginUIRender();

    //텍스쳐가 그려질 위치 설정
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
    m_pTexture->Set_Texture(0);
    m_pBufferCom->Render_Buffer();

    EndUIRender();
}

void CInventoryBackground::Hover()
{}

void CInventoryBackground::Clicked()
{}

void CInventoryBackground::Leave()
{}

HRESULT CInventoryBackground::Add_Component()
{
    CComponent* pComponent = nullptr;

    pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    if (!m_pBufferCom)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    pComponent = m_pTexture = dynamic_cast<CTexture*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_InventoryBackgroundTexture"));
    if(!m_pTexture)
        return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_InventoryBG", pComponent });

	return S_OK;
}

CInventoryBackground* CInventoryBackground::Create(LPDIRECT3DDEVICE9 pGraphicDev, float fX, float fY, float fW, float fH)
{
    CInventoryBackground* pBG = new CInventoryBackground(pGraphicDev);

    if (FAILED(pBG->Ready_GameObject()))
    {
        Safe_Release(pBG);
        MSG_BOX("CIventory BG Create Failed");
        return nullptr;
    }

    pBG->Set_Info(fX, fY, fW, fH);
    return pBG;
}

void CInventoryBackground::Free()
{
    CUIInterface::Free();
}
