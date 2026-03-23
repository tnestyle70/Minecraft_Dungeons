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
		m_pHoverTexture->Set_Texture(0);
		break;
	case eSlotState::CLICK:
		m_pClickedTexture->Set_Texture(0);
		break;
	default:
		break;
	}
	
	m_pBufferCom->Render_Buffer();

	EndUIRender();

	//2. 아이템 렌더링
	if (!m_bEquipped)
		return;

	BeginItemRender();

	matWorld = Calc_WorldMatrix(m_fItemX, m_fItemY, m_fItemW, m_fItemH);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	m_pItemTexture->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	EndItemRender();
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

void CEquipSlot::Set_ItemInfo(float fX, float fY, float fW, float fH)
{
	m_fItemX = fX; m_fItemY = fY; m_fItemW = fW; m_fItemH = fH;
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

	switch (m_eEquipType)
	{
	case eEquipType::MELEE:
		pTexture = L"Proto_IronSwordTexture";
		break;
	case eEquipType::ARMOR:
		pTexture = L"Proto_RobeTexture";
		break;
	case eEquipType::RANGED:
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

_matrix CEquipSlot::Calc_WorldMatrix(float fX, float fY, float fW, float fH)
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

void CEquipSlot::BeginItemRender()
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

void CEquipSlot::EndItemRender()
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

void CEquipSlot::Hover()
{
	m_eState = eSlotState::HOVER;
}

void CEquipSlot::Clicked()
{
	//더블 클릭 감지
	DWORD dwNow = GetTickCount();

	if (dwNow - m_dwLastClickTime < DOUBLE_CLICK_MS)
		m_bDoubleClicked = true;

	m_dwLastClickTime = dwNow;
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
