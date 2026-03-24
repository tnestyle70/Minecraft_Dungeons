#include "pch.h"
#include "CBox.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CEnvironmentMgr.h"

CBox::CBox(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pTransformCom(nullptr)
	, m_pTextureCom(nullptr)
	, m_pColliderCom(nullptr)
	, m_bIsOpen(false)
	, m_bIsOpening(false)
	, m_pEmerald(nullptr)
	, m_fAnimTime(0.f)
{
	ZeroMemory(m_pParts, sizeof(m_pParts));
}

CBox::CBox(const CBox& rhs)
	: CGameObject(rhs)
	, m_pTransformCom(nullptr)
	, m_pTextureCom(nullptr)
	, m_pColliderCom(nullptr)
	, m_bIsOpen(false)
	, m_bIsOpening(false)
	, m_pEmerald(nullptr)
	, m_fAnimTime(0.f)
{
	ZeroMemory(m_pParts, sizeof(m_pParts));
}

CBox::~CBox()
{
}

HRESULT CBox::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(0.f, 0.f, 0.f);

	Set_WorldScale();
	Set_PartsOffset();
	Set_PartsParent();

	//CEnvironmentMgr::GetInstance()->Add_Box(this);

	return S_OK;
}

_int CBox::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	if (m_bIsOpening)
	{
		m_fAnimTime += fTimeDelta;
		Box_Animation();
	}

	for (_int i = 0; i < BOX_END; ++i)
	{
		m_pParts[i]->Update_GameObject(fTimeDelta);
	}

	if (m_pEmerald)
	{
		m_pEmerald->Update_GameObject(fTimeDelta);
	}

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	m_pColliderCom->Update_AABB(vPos);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return iExit;
}

void CBox::LateUpdate_GameObject(const _float& fTimeDelta)
{
	for (_int i = 0; i < BOX_END; ++i)
	{
		m_pParts[i]->LateUpdate_GameObject(fTimeDelta);
	}

	if (m_pEmerald)
		m_pEmerald->LateUpdate_GameObject(fTimeDelta);
}

void CBox::Render_GameObject()
{
	m_pTextureCom->Set_Texture(0);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	for (_int i = 0; i < BOX_END; ++i)
	{
		m_pParts[i]->Render_GameObject();
	}

	if (m_pEmerald)
		m_pEmerald->Render_GameObject();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void CBox::Open_Box()
{
	if (m_bIsOpen || m_bIsOpening)
		return;

	m_bIsOpening = true;
	m_fAnimTime = 0.f;
}

HRESULT CBox::Add_Component()
{
	CComponent* pComponent = nullptr;

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ { L"Com_Transform", pComponent } });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BoxTexture"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ { L"Com_Texture", pComponent } });

	// Collider
	m_pColliderCom = CCollider::Create(m_pGraphicDev, _vec3(3.5f, 2.5f, 2.5f), _vec3(0.f, 1.85f, 0.f));

	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

	// Create Child
	for (int i = 0; i < BOX_END; i++)
	{
		m_pParts[i] = CBoxPart::Create(m_pGraphicDev, (BOX_PART)i);
	}

	return S_OK;
}

void CBox::Set_PartsOffset()
{
	m_pParts[BOX_BOTTOM]->Set_LocalOffset({ 0.f, 0.37f * m_fWorldScale, 0.f });
	m_pParts[BOX_TOP]->Set_LocalOffset({ 0.f, 0.62f * m_fWorldScale, 0.f });
}

void CBox::Set_WorldScale()
{
	for (_int i = 0; i < BOX_END; ++i)
	{
		m_pParts[i]->Get_Transform()->Set_Scale(m_fWorldScale);
	}
}

void CBox::Set_PartsParent()
{
	m_pParts[BOX_BOTTOM]->Get_Transform()->Set_Parent(m_pTransformCom);
	m_pParts[BOX_TOP]->Get_Transform()->Set_Parent(m_pTransformCom);
}

void CBox::Box_Animation()
{
	const _float fCycleSpeed = 200.f;
	const _float fAngle = m_fAnimTime * fCycleSpeed;

	if (fAngle >= 70.f)
	{
		m_bIsOpen = true;
		m_bIsOpening = false;

		m_pEmerald = CEmerald::Create(m_pGraphicDev);
		CTransform* pEmeraldTransCom = dynamic_cast<CTransform*>(m_pEmerald->Get_Component(ID_DYNAMIC, L"Com_Transform"));
		
		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);

		pEmeraldTransCom->Set_Pos(vPos.x, vPos.y + 2.f, vPos.z);

		return;
	}

	CTransform* pTopTransform = m_pParts[BOX_TOP]->Get_Transform();

	_float hingeZ = 0.62f * m_fWorldScale;

	pTopTransform->Set_Rotation(ROT_X, -fAngle);

	_float rad = D3DXToRadian(-fAngle);
	_float dz = (hingeZ * cosf(rad) - hingeZ) * 0.5f;

	pTopTransform->Set_Pos(0.f, 0.62f * m_fWorldScale, dz);
}

CBox* CBox::Create(LPDIRECT3DDEVICE9 pGraphiDev)
{
	CBox* pBox = new CBox(pGraphiDev);

	if (FAILED(pBox->Ready_GameObject()))
	{
		Safe_Release(pBox);
		MSG_BOX("Box Create Failed");
		return nullptr;
	}

	return pBox;
}

void CBox::Free()
{
	for (_int i = 0; i < BOX_END; ++i)
	{
		Safe_Release(m_pParts[i]);
	}

	CGameObject::Free();
}