#include "pch.h"
#include "CEquipSlot.h"

CEquipSlot::CEquipSlot(LPDIRECT3DDEVICE9 pGraphicDev)
	:CUIInterface(pGraphicDev)
{
}

CEquipSlot::~CEquipSlot()
{
}

HRESULT CEquipSlot::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	return S_OK;
}

_int CEquipSlot::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CUIInterface::Update_GameObject(fTimeDelta);

	return iExit;
}

void CEquipSlot::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CUIInterface::LateUpdate_GameObject(fTimeDelta);
}

void CEquipSlot::Render_GameObject()
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

	switch (m_eState)
	{
	case eSlotState::DEFAULT:
		m_pFrameTexture->Set_Texture(0);
		break;
	case eSlotState::HOVER:
		m_pFrameTexture->Set_Texture(0);
		break;
	case eSlotState::CLICK:
		m_pClickedTexture->Set_Texture(0);
		break;
	default:
		break;
	}

	m_pBufferCom->Render_Buffer();

	EndUIRender();
}

void CEquipSlot::BeginUIRender()
{
	//알파블렌딩 X
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

void CEquipSlot::EndUIRender()
{
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//투영 다시 적용시켜주기
	m_pGraphicDev->SetTransform(D3DTS_VIEW, &m_matOriginView);
	m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &m_matOriginProj);
}

HRESULT CEquipSlot::Add_Component()
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

	//Clicked Texture
	pComponent = m_pClickedTexture = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_ClickedFrameTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_ClickedTexture", pComponent });

	return S_OK;
}

void CEquipSlot::Hover()
{
	m_eState = eSlotState::HOVER;
}

void CEquipSlot::Clicked()
{
	m_eState = eSlotState::CLICK;
}

void CEquipSlot::Leave()
{
	if (m_eState == eSlotState::HOVER)
		m_eState = eSlotState::DEFAULT;
}

CEquipSlot* CEquipSlot::Create(LPDIRECT3DDEVICE9 pGraphicDev, 
	eEquipType eType, float fX, float fY, float fW, float fH)
{
	CEquipSlot* pSlot = new CEquipSlot(pGraphicDev);
	pSlot->m_eEquipType = eType;

	if (FAILED(pSlot->Ready_GameObject()))
	{
		Safe_Release(pSlot);
		MSG_BOX("CEquipSlot Create Failed");
		return nullptr;
	}
	//크기 설정
	pSlot->Set_Info(fX, fY, fW, fH);
	return pSlot;
}

void CEquipSlot::Free()
{}
