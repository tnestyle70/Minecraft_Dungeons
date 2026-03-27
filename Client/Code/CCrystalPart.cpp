#include "pch.h"
#include "CCrystalPart.h"

CCrystalPart::CCrystalPart(LPDIRECT3DDEVICE9 pGraphicDev, CRYSTAL_PART ePart)
	: CGameObject(pGraphicDev)
	, m_ePart(ePart)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
{
}

CCrystalPart::~CCrystalPart()
{
}

HRESULT CCrystalPart::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	return S_OK;
}

_int CCrystalPart::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	return iExit;
}

void CCrystalPart::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CCrystalPart::Render_GameObject()
{

}

HRESULT CCrystalPart::Add_Component()
{
	return S_OK;
}

CCrystalPart* CCrystalPart::Create(LPDIRECT3DDEVICE9 pGraphicDev, CRYSTAL_PART ePart)
{
	CCrystalPart* pCrystalPart = new CCrystalPart(pGraphicDev, ePart);

	if (FAILED(pCrystalPart->Ready_GameObject()))
	{
		Safe_Release(pCrystalPart);
		MSG_BOX("CrystalPart Create Failed");
		return nullptr;
	}

	return pCrystalPart;
}

void CCrystalPart::Free()
{
	CGameObject::Free();
}