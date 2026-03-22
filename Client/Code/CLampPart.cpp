#include "pch.h"
#include "CLampPart.h"

CLampPart::CLampPart(LPDIRECT3DDEVICE9 pGraphicDev, LAMP_PART ePart)
	: CGameObject(pGraphicDev)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_ePart(ePart)
{
}

CLampPart::~CLampPart()
{
}

HRESULT CLampPart::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	return S_OK;
}

_int CLampPart::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	return iExit;
}

void CLampPart::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CLampPart::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pBufferCom->Render_Buffer();
}

HRESULT CLampPart::Add_Component()
{
	CComponent* pComponent = nullptr;

	// Buffer
	switch (m_ePart)
	{
	case LAMP_BODY:
		pComponent = m_pBufferCom = dynamic_cast<CLampBodyTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_LampBodyTex"));
		break;

	case LAMP_HEAD:
		pComponent = m_pBufferCom = dynamic_cast<CLampHeadTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_LampHeadTex"));
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

CLampPart* CLampPart::Create(LPDIRECT3DDEVICE9 pGraphicDev, LAMP_PART ePart)
{
	CLampPart* pLampPart = new CLampPart(pGraphicDev, ePart);

	if (FAILED(pLampPart->Add_Component()))
	{
		Safe_Release(pLampPart);
		MSG_BOX("LampPart Create Failed");
		return nullptr;
	}

	return pLampPart;
}

void CLampPart::Free()
{
	CGameObject::Free();
}