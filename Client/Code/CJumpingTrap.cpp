#include "pch.h"
#include "CJumpingTrap.h"
#include "CRenderer.h"
#include "CManagement.h"

CJumpingTrap::CJumpingTrap(LPDIRECT3DDEVICE9 pGraphicDev)
	:CGameObject(pGraphicDev)
{}

CJumpingTrap::CJumpingTrap(const CGameObject& rhs)
	:CGameObject(rhs)
{}

CJumpingTrap::~CJumpingTrap()
{}

HRESULT CJumpingTrap::Ready_GameObject(const _vec3 & vPos)
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	m_pTransformCom->m_vScale = { 5.f, 0.5f, 5.f };
	m_pColliderCom->Update_AABB(vPos);

	m_vOriginPos = vPos;

	return S_OK;
}

_int CJumpingTrap::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	Update_Active(fTimeDelta);

	return iExit;
}

void CJumpingTrap::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CJumpingTrap::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	m_pTextureCom->Set_Texture(0);

	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	//디버깅용으로 일단 렌더링
	m_pColliderCom->Render_Collider();
}

void CJumpingTrap::Update_Active(const _float& fTimeDelta)
{
	//Active 상태가 아니면 return
	if (!m_bActive)
		return;

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	if (m_bRising)
	{
		vPos.y += m_fActiveSpeed * fTimeDelta;

		if (vPos.y >= m_vOriginPos.y + m_fMaxHeight)  // ← m_vPos → m_vOriginPos
		{
			vPos.y = m_vOriginPos.y + m_fMaxHeight;
			m_bRising = false;
		}
	}
	else
	{
		vPos.y -= m_fDeactiveSpeed * fTimeDelta;

		if (vPos.y <= m_vOriginPos.y)  // ← m_vPos → m_vOriginPos
		{
			vPos.y = m_vOriginPos.y;
			m_bActive = false;
		}
	}

	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	m_pColliderCom->Update_AABB(vPos);
}

void CJumpingTrap::Set_Active(bool bActive)
{
	m_bActive = bActive;
	if (bActive)
		m_bRising = true;
}

HRESULT CJumpingTrap::Add_Component()
{
	CComponent* pComponent = nullptr;

	pComponent = m_pBufferCom = dynamic_cast<CCubeTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CubeTex"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	pComponent = m_pTextureCom = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_SandTexture"));

	if (nullptr == pComponent)
	{
		MSG_BOX("PlayerTexture Failed");
		return E_FAIL;
	}

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	//Collider
	_vec3 vSize, vOffset;
	vSize = { 5.f, 0.5f, 1.f };
	vOffset = { 0.f, 0.f, 0.f };

	pComponent = m_pColliderCom = CCollider::Create(m_pGraphicDev,
		vSize, vOffset);
	if (!pComponent)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", pComponent });

	return S_OK;
}

CJumpingTrap* CJumpingTrap::Create(LPDIRECT3DDEVICE9 pGraphicDev, const _vec3 & vPos)
{
	CJumpingTrap* pJumpingTrap = new CJumpingTrap(pGraphicDev);

	if (FAILED(pJumpingTrap->Ready_GameObject(vPos)))
	{
		Safe_Release(pJumpingTrap);
		MSG_BOX("pJumpingTrap Create Failed");
		return nullptr;
	}
	return pJumpingTrap;
}

void CJumpingTrap::Free()
{
	CGameObject::Free();
}
