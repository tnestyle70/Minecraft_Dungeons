#include "pch.h"
#include "CItemPanel.h"
#include "CFontMgr.h"

CItemPanel::CItemPanel(LPDIRECT3DDEVICE9 pGraphicDev)
	:CUIInterface(pGraphicDev)
{
}

CItemPanel::~CItemPanel()
{
}

HRESULT CItemPanel::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	return S_OK;
}

_int CItemPanel::Update_GameObject(const _float& fTimeDelta)
{
	if (!m_bSelected)
		return 0;
	
	_int iExit = CUIInterface::Update_GameObject(fTimeDelta);

	return iExit;
}

void CItemPanel::LateUpdate_GameObject(const _float& fTimeDelta)
{
	if (!m_bSelected)
		return;
	
	CUIInterface::LateUpdate_GameObject(fTimeDelta);
}

void CItemPanel::Render_GameObject()
{
	if (!m_bSelected)
		return;

	//1. 아이템 이미지
	BeginUIRender();

	_matrix mat = Calc_WorldMatrix(m_fItemX, m_fItemY, m_fItemW, m_fItemH);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &mat);
	m_pItemTexture->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	//2. 버튼 이미지 
	mat = Calc_WorldMatrix(m_fBtn1X, m_fBtn1Y, m_fBtnW, m_fBtnH);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &mat);
	m_pUpgradeBtn1->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	mat = Calc_WorldMatrix(m_fBtn2X, m_fBtn2Y, m_fBtnW, m_fBtnH);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &mat);
	m_pUpgradeBtn2->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	mat = Calc_WorldMatrix(m_fBtn3X, m_fBtn3Y, m_fBtnW, m_fBtnH);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &mat);
	m_pUpgradeBtn3->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	EndUIRender();

	_vec2 vNamePos = {800.f, 100.f};
	_vec2 vDescPos = {800.f, 150.f};

	const _tchar* szName = nullptr;
	const _tchar* szDesc = nullptr;
	
	switch (m_eEquipType)
	{
	case eEquipType::MELEE:
		szName = L"Iron Sword";
		szDesc = L"Damage : 1";
		break;
	case eEquipType::ARMOR:
		szName = L"Robe";
		szDesc = L"Defense : 1";
		break;
	case eEquipType::RANGED:
		szName = L"Bow";
		szDesc = L"Damage : 1";
		break;
	default:
		break;
	}

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", szName,
		&vNamePos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));

	CFontMgr::GetInstance()->Render_Font(
		L"Font_Minecraft", szDesc,
		&vDescPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

HRESULT CItemPanel::Add_Component()
{
	CComponent* pComponent = nullptr;

	// RcTex 버퍼
	pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
	if (!pComponent) 
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// 아이템 텍스처 (타입별)
	const _tchar* pItemTex = nullptr;
	switch (m_eEquipType)
	{
	case eEquipType::MELEE:  pItemTex = L"Proto_IronSwordTexture";      break;
	case eEquipType::ARMOR:  pItemTex = L"Proto_RobeTexture";           break;
	case eEquipType::RANGED: pItemTex = L"Proto_ArrowLoadedBowTexture"; break;
	}
	pComponent = m_pItemTexture = dynamic_cast<CTexture*>(
		CProtoMgr::GetInstance()->Clone_Prototype(pItemTex));
	if (!pComponent) return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_ItemTexture", pComponent });

	// 버튼 텍스처 3종
	pComponent = m_pUpgradeBtn1 = dynamic_cast<CTexture*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_UpgradeTexture"));
	if (!pComponent) return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_UpgradeBtn1", pComponent });

	pComponent = m_pUpgradeBtn2 = dynamic_cast<CTexture*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_UpgradeTexture"));
	if (!pComponent) return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_UpgradeBtn1", pComponent });

	pComponent = m_pUpgradeBtn3 = dynamic_cast<CTexture*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_UpgradeTexture"));
	if (!pComponent) return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_UpgradeBtn1", pComponent });

	return S_OK;
}

_matrix CItemPanel::Calc_WorldMatrix(float fX, float fY, float fW, float fH)
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

void CItemPanel::Hover()
{
}

void CItemPanel::Clicked()
{
}

void CItemPanel::Leave()
{
}

void CItemPanel::BeginUIRender()
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

void CItemPanel::EndUIRender()
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

CItemPanel* CItemPanel::Create(LPDIRECT3DDEVICE9 pGraphicDev, eEquipType eType)
{
	CItemPanel* pPanel = new CItemPanel(pGraphicDev);

	pPanel->Set_Type(eType);
	
	if (FAILED(pPanel->Ready_GameObject()))
	{
		Safe_Release(pPanel);
		MSG_BOX("pPanel Create Failed");
		return nullptr;
	}
	
	return pPanel;
}

void CItemPanel::Free()
{
	CUIInterface::Free();
}
