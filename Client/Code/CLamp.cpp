#include "pch.h"
#include "CLamp.h"
#include "CRenderer.h"

CLamp::CLamp(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pTransformCom(nullptr)
	, m_pTextureCom(nullptr)
	, m_pColliderCom(nullptr)
	, m_fAnimTime(0.f)
{
	ZeroMemory(m_pParts, sizeof(m_pParts));
}

CLamp::~CLamp()
{

}

HRESULT CLamp::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(-10.f, 0.f, 0.f);

	Set_WorldScale();
	Set_PartsOffset();
	Set_PartsParent();

	return S_OK;
}

_int CLamp::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	m_fAnimTime += fTimeDelta;

	for (_int i = 0; i < LAMP_END; ++i)
	{
		m_pParts[i]->Update_GameObject(fTimeDelta);
	}

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	m_pColliderCom->Update_AABB(vPos);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return iExit;
}

void CLamp::LateUpdate_GameObject(const _float& fTimeDelta)
{
	for (_int i = 0; i < LAMP_END; ++i)
	{
		m_pParts[i]->LateUpdate_GameObject(fTimeDelta);
	}
}

void CLamp::Render_GameObject()
{
	m_pTextureCom->Set_Texture(0);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	for (_int i = 0; i < LAMP_END; ++i)
	{
		m_pParts[i]->Render_GameObject();
	}

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CLamp::Add_Component()
{
	CComponent* pComponent = nullptr;

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ { L"Com_Transform", pComponent } });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_LampTexture"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ { L"Com_Texture", pComponent } });

	// Collider
	m_pColliderCom = CCollider::Create(m_pGraphicDev, _vec3(3.5f, 2.5f, 2.5f), _vec3(0.f, 1.85f, 0.f));

	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

	// Create Child
	for (int i = 0; i < LAMP_END; i++)
	{
		m_pParts[i] = CLampPart::Create(m_pGraphicDev, (LAMP_PART)i);
	}

	return S_OK;
}

void CLamp::Set_PartsOffset()
{
	m_pParts[LAMP_BODY]->Set_LocalOffset({ 0.f, 2.5f * m_fWorldScale, 0.f });
	m_pParts[LAMP_HEAD]->Set_LocalOffset({ 0.f, 5.f * m_fWorldScale, 0.f });
}

void CLamp::Set_WorldScale()
{
	for (_int i = 0; i < LAMP_END; ++i)
	{
		m_pParts[i]->Get_Transform()->Set_Scale(m_fWorldScale);
	}
}

void CLamp::Set_PartsParent()
{
	m_pParts[LAMP_BODY]->Get_Transform()->Set_Parent(m_pTransformCom);
	m_pParts[LAMP_HEAD]->Get_Transform()->Set_Parent(m_pTransformCom);
}

CLamp* CLamp::Create(LPDIRECT3DDEVICE9 pGraphiDev)
{
	CLamp* pLamp = new CLamp(pGraphiDev);

	if (FAILED(pLamp->Ready_GameObject()))
	{
		Safe_Release(pLamp);
		MSG_BOX("Lamp Create Failed");
		return nullptr;
	}

	return pLamp;
}

void CLamp::Free()
{
	for (_int i = 0; i < LAMP_END; ++i)
	{
		Safe_Release(m_pParts[i]);
	}

	CGameObject::Free();
}