#include "pch.h"
#include "CBoxPart.h"

CBoxPart::CBoxPart(LPDIRECT3DDEVICE9 pGraphicDev, BOX_PART ePart)
	: CGameObject(pGraphicDev)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_ePart(ePart)
{
}

CBoxPart::CBoxPart(const CBoxPart& rhs)
	: CGameObject(rhs)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_ePart(rhs.m_ePart)
{
}

CBoxPart::~CBoxPart()
{
}

HRESULT CBoxPart::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	return S_OK;
}

_int CBoxPart::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	return iExit;
}

void CBoxPart::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CBoxPart::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pBufferCom->Render_Buffer();
}

HRESULT CBoxPart::Add_Component()
{
	CComponent* pComponent = nullptr;

	// Buffer
	switch (m_ePart)
	{
	case BOX_BOTTOM:
		pComponent = m_pBufferCom = dynamic_cast<CBoxBottomTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BoxBottomTex"));
		break;

	case BOX_TOP:
		pComponent = m_pBufferCom = dynamic_cast<CBoxTopTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BoxTopTex"));
		break;

	default:
		break;
	}

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });


	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	return S_OK;
}

CBoxPart* CBoxPart::Create(LPDIRECT3DDEVICE9 pGraphicDev, BOX_PART ePart)
{
	CBoxPart* pBoxPart = new CBoxPart(pGraphicDev, ePart);

	if (FAILED(pBoxPart->Add_Component()))
	{
		Safe_Release(pBoxPart);
		MSG_BOX("BoxPart Create Failed");
		return nullptr;
	}

	return pBoxPart;
}

void CBoxPart::Free()
{
	CGameObject::Free();
}