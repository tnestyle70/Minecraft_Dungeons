#include "pch.h"
#include "CInventorySlot.h"
#include "CRenderer.h"

CInventorySlot::CInventorySlot(LPDIRECT3DDEVICE9 pGraphicDev)
	:CUIInterface(pGraphicDev)
{}

CInventorySlot::~CInventorySlot()
{}

HRESULT CInventorySlot::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	return S_OK;
}

_int CInventorySlot::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CUIInterface::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_UI, this);

	return iExit;
}

void CInventorySlot::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CUIInterface::LateUpdate_GameObject(fTimeDelta);
}

void CInventorySlot::Render_GameObject()
{
	BeginUIRender();

	//UI는 Normalized Device Coordinate 기준이라서 스크린 좌표를 반환해야 함(-1 ~ 1)
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
	
	m_pNormalTexture->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	EndUIRender();
}

void CInventorySlot::Hover()
{}

void CInventorySlot::Clicked()
{}

void CInventorySlot::Leave()
{}

HRESULT CInventorySlot::Add_Component()
{
	CComponent* pComponent = nullptr;

	// RcTex
	pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	//Normal Texture
	pComponent = m_pNormalTexture = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_FrameTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_NormalTexture", pComponent });

	//Hover Texture
	pComponent = m_pHoverTexture = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_HoverFrameTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_HoverTexture", pComponent });

	//Clicked Texture
	pComponent = m_pClickedTexture = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ClickedFrameTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_ClickedTexture", pComponent });

	return S_OK;
}

CInventorySlot* CInventorySlot::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CInventorySlot* pInventorySlot = new CInventorySlot(pGraphicDev);

	if (FAILED(pInventorySlot->Ready_GameObject()))
	{
		Safe_Release(pInventorySlot);
		MSG_BOX("pInventorySlot Create Failed");
		return nullptr;
	}

	return pInventorySlot;
}

void CInventorySlot::Free()
{
	CUIInterface::Free();
}
