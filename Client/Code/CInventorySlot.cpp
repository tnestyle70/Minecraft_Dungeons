#include "pch.h"
#include "CInventorySlot.h"
#include "CRenderer.h"
#include "CCursorMgr.h"

CInventorySlot::CInventorySlot(LPDIRECT3DDEVICE9 pGraphicDev)
	:CUIInterface(pGraphicDev)
{
}

CInventorySlot::~CInventorySlot()
{
}

HRESULT CInventorySlot::Ready_GameObject(eInventoryTab eTab)
{
	//어떤 인벤토리 탭인지 설정
	m_eInventoryTab = eTab;

	if (FAILED(Add_Component()))
		return E_FAIL;

	return S_OK;
}

_int CInventorySlot::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CUIInterface::Update_GameObject(fTimeDelta);

	return iExit;
}

void CInventorySlot::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CUIInterface::LateUpdate_GameObject(fTimeDelta);
}

void CInventorySlot::Render_GameObject()
{
	BeginUIRender();

	//1. 슬롯 배경 렌더링
	_matrix matWorld = Calc_WorldMatrix(m_fX, m_fY, m_fW, m_fH);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	
	switch (m_eState)
	{
	case eSlotState::DEFAULT:
		m_pFrameTexture->Set_Texture(0);
		break;
	case eSlotState::HOVER:
		m_pHoverTexture->Set_Texture(0);
		break;
	case eSlotState::CLICK:
		m_pClickedTexture->Set_Texture(0);
		break;
	default:
		break;
	}
	
	m_pBufferCom->Render_Buffer();
	
	//아이템 렌더링
	if (!m_bEmpty && m_pItemTexture)
	{
		BeginItemRender();
		m_pItemTexture->Set_Texture(0);
		m_pBufferCom->Render_Buffer();
		EndItemRender();
	}
	
	EndUIRender();
}

void CInventorySlot::Set_SlotInfo(float fX, float fY, float fW, float fH,
	eInventoryTab eType)
{
	m_fX = fX; m_fY = fY; m_fW = fW; m_fH = fH;
}

void CInventorySlot::Set_ItemInfo(float fX, float fY, float fW, float fH)
{
	//아이템 위치 크기 설정
	m_fItemX = fX; m_fItemY = fY; m_fItemW = fW; m_fItemH = fH;

	m_bEmpty = false;
}

void CInventorySlot::Hover()
{
	m_eState = eSlotState::HOVER;
}

void CInventorySlot::Clicked()
{
	//더블 클릭 감지
	DWORD dwNow = GetTickCount();

	if (dwNow - m_dwLastClickTime < DOUBLE_CLICK_MS)
		m_bDoubleClicked = true;

	m_dwLastClickTime = dwNow;
	m_eState = eSlotState::CLICK;
}

void CInventorySlot::Leave()
{
	//Hover일 때만, Click 상태면 유지
	if (m_eState == eSlotState::HOVER)
	{
		m_eState = eSlotState::DEFAULT;
	}
}

void CInventorySlot::BeginUIRender()
{
	//Alpha Blending Off
	//월드 행렬 항등 행렬로 설정
	_matrix matWorld;
	D3DXMatrixIdentity(&matWorld);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	//원본 뷰, 투영 저장
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &m_matOriginProj);
	//뷰, 투영 풀어주기
	_matrix matView, matProj;
	D3DXMatrixIdentity(&matView);
	D3DXMatrixIdentity(&matProj);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);
	//CullMode 설정
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
}

void CInventorySlot::EndUIRender()
{
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//투영 다시 적용시켜주기
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &m_matOriginProj);
}

void CInventorySlot::BeginItemRender()
{
	//Alpha Blending On
	//월드 행렬 항등 행렬로 설정
	_matrix matWorld;
	D3DXMatrixIdentity(&matWorld);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	//원본 뷰, 투영 저장
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &m_matOriginProj);
	//뷰, 투영 풀어주기
	_matrix matView, matProj;
	D3DXMatrixIdentity(&matView);
	D3DXMatrixIdentity(&matProj);
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);
	//CullMode 설정
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//알파 블렌딩 활성화
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHAREF, 0xc0);
}

void CInventorySlot::EndItemRender()
{
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	//투영 다시 적용시켜주기
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &m_matOriginProj);
	//알파블렌딩 - 옵션 다시 꺼주기!!
	m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

HRESULT CInventorySlot::Add_Component()
{
	CComponent* pComponent = nullptr;

	// RcTex
	pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	//Frame Texture
	pComponent = m_pFrameTexture = dynamic_cast<CTexture*>
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

	//Item Texture
	const _tchar* pTexture = nullptr;
	
	switch (m_eInventoryTab)
	{
	case eInventoryTab::SWORD:
		pTexture = L"Proto_IronSwordTexture";
		break;
	case eInventoryTab::ARMOR:
		pTexture = L"Proto_RobeTexture";
		break;
	case eInventoryTab::BOW:
		pTexture = L"Proto_ArrowLoadedBowTexture";
		break;
	default:
		break;
	}

	pComponent = m_pItemTexture = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(pTexture));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_ItemTexture", pComponent });

	return S_OK;
}

_matrix CInventorySlot::Calc_WorldMatrix(float fX, float fY, float fW, float fH)
{
	_matrix matWorld;
	float fNDCX = (fX + fW * 0.5f) / (WINCX * 0.5f) - 1.f;
	float fNDCY = 1.f - (fY + fH * 0.5f) / (WINCY * 0.5f);
	float fScaleX = fW / WINCX;
	float fScaleY = fH / WINCY;

	D3DXMatrixTransformation2D(&matWorld,
		nullptr, 0.f,
		&_vec2(fScaleX, fScaleY),
		nullptr, 0.f,
		&_vec2(fNDCX, fNDCY));

	return matWorld;
}

CInventorySlot* CInventorySlot::Create(LPDIRECT3DDEVICE9 pGraphicDev,
	eInventoryTab eTab)
{
	CInventorySlot* pInventorySlot = new CInventorySlot(pGraphicDev);
	

	if (FAILED(pInventorySlot->Ready_GameObject(eTab)))
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
