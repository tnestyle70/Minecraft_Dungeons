#include "pch.h"
#include "CTriggerBox.h"
#include "CRenderer.h"

CTriggerBox::CTriggerBox(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{
}

CTriggerBox::CTriggerBox(const CGameObject& rhs)
	: CGameObject(rhs)
{
}

CTriggerBox::~CTriggerBox()
{
}

HRESULT CTriggerBox::Ready_GameObject(const _vec3 & vPos)
{
	if (FAILED(Add_Component()))
		return E_FAIL;
	//Setting Collider, Transform
	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	m_pColliderCom->Update_AABB(vPos);

	return S_OK;
}

_int CTriggerBox::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return iExit;
}

void CTriggerBox::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CTriggerBox::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pColliderCom->Render_Collider();
}

HRESULT CTriggerBox::Add_Component()
{
	CComponent* pComponent = nullptr;

	//Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
	if (!pComponent)
		return E_FAIL;
	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	//Collider
	_vec3 vSize, vOffset;
	vSize = { 1.f, 1.f, 1.f };
	vOffset = { 0.f, 0.f, 0.f };
	pComponent = m_pColliderCom = CCollider::Create(m_pGraphicDev,
		vSize, vOffset);
	if (!pComponent)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", pComponent });

	return S_OK;
}

CTriggerBox* CTriggerBox::Create(LPDIRECT3DDEVICE9 pGraphicDev, const _vec3& vPos)
{
	CTriggerBox* pTriggerBox = new CTriggerBox(pGraphicDev);

	if (FAILED(pTriggerBox->Ready_GameObject(vPos)))
	{
		Safe_Release(pTriggerBox);
		MSG_BOX("pTriggerBox Create Failed");
		return nullptr;
	}
	return pTriggerBox;
}

void CTriggerBox::Free()
{
	CGameObject::Free();
}
