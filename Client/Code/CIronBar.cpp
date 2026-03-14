#include "pch.h"
#include "CIronBar.h"
#include "CRenderer.h"

CIronBar::CIronBar(LPDIRECT3DDEVICE9 pGraphicDev)
	:CGameObject(pGraphicDev)
{
}

CIronBar::CIronBar(const CGameObject& rhs)
	:CGameObject(rhs)
{
}

CIronBar::~CIronBar()
{
}

HRESULT CIronBar::Ready_GameObject(const _vec3& vPos)
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	m_pTransformCom->m_vScale = { 0.5f, 9.f, 0.5f };
	m_pColliderCom->Update_AABB(vPos);

	//Set Max, Min Height half of iron bar size
	m_fMaxHeight = vPos.y + 1.f;
	m_fMinHeight = vPos.y - 5.f;

	return S_OK;
}

_int CIronBar::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

	if (GetAsyncKeyState('8'))
	{
		m_eState = eIronBarState::MOVE_UP;
	}
	if (GetAsyncKeyState('9'))
	{
		m_eState = eIronBarState::MOVE_DOWN;
	}
	if (GetAsyncKeyState('0'))
	{
		m_eState = eIronBarState::IDLE;
	}

	Update_Animation(fTimeDelta);

	return iExit;
}

void CIronBar::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CIronBar::Render_GameObject()
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

HRESULT CIronBar::Add_Component()
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

	// Texture
	//pComponent = m_pTextureCom = dynamic_cast<CTexture*>
	//	(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RockTexture"));
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_StoneBrickTexture"));

	if (nullptr == pComponent)
	{
		MSG_BOX("PlayerTexture Failed");
		return E_FAIL;
	}
	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	//Collider
	_vec3 vSize, vOffset;
	vSize = { 1.f, 10.f, 1.f };
	vOffset = { 0.f, 0.f, 0.f };

	pComponent = m_pColliderCom = CCollider::Create(m_pGraphicDev,
		vSize, vOffset);
	if (!pComponent)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", pComponent });

	return S_OK;
}

void CIronBar::Update_Animation(const _float& fTimeDelta)
{
	switch (m_eState)
	{
	case IDLE:
		break;
	case MOVE_UP:
		//올라가는 애니메이션
		MoveUp(fTimeDelta);
		break;
	case MOVE_DOWN:
		//내려가는 애니메이션
		MoveDown(fTimeDelta);
		break;
	default:
		break;
	}
}

void CIronBar::MoveUp(const _float& fTimeDelta)
{
	//1프레임 시간만큼 움직인 거리
	_float fDeltaYPos = m_fMoveSpeed * fTimeDelta;
	//Pos 정보를 받아와서 MaxHeight에 도달했는지를 검증
	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);
	
	if (vPos.y + fDeltaYPos < m_fMaxHeight)
	{
		m_pTransformCom->Set_Pos(vPos.x, vPos.y + fDeltaYPos, vPos.z);
	}
}

void CIronBar::MoveDown(const _float & fTimeDelta)
{
	//1프레임 시간만큼 움직인 거리
	_float fDeltaYPos = m_fMoveSpeed * fTimeDelta;
	//Pos 정보를 받아와서 MaxHeight에 도달했는지를 검증
	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	if (vPos.y - fDeltaYPos > m_fMinHeight)
	{
		m_pTransformCom->Set_Pos(vPos.x, vPos.y - fDeltaYPos, vPos.z);
	}
}

CIronBar* CIronBar::Create(LPDIRECT3DDEVICE9 pGraphicDev, const _vec3& vPos)
{
	CIronBar* pIronBar = new CIronBar(pGraphicDev);

	if (FAILED(pIronBar->Ready_GameObject(vPos)))
	{
		Safe_Release(pIronBar);
		MSG_BOX("IronBar Create Failed");
		return nullptr;
	}
	return pIronBar;
}

void CIronBar::Free()
{
	CGameObject::Free();
}
