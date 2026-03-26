#include "pch.h"
#include "CEmerald.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CSoundMgr.h"

CEmerald::CEmerald(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_pTextureCom(nullptr)
	, m_pColliderCom(nullptr)
{
}

CEmerald::~CEmerald()
{
}

HRESULT CEmerald::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	Init_Emerald();

	CSoundMgr::GetInstance()->PlayEffect(L"Emerald/sfx_item_emerald-001_soundWave.wav", 0.6f);

	return S_OK;
}

_int CEmerald::Update_GameObject(const _float& fTimeDelta)
{
	if (m_bDead)
		return -1;

	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	Pop_Emerald(fTimeDelta);
	Chase_Player(fTimeDelta);

	_vec3 vPos;
	m_pTransformCom->Get_Info(INFO_POS, &vPos);

	m_pColliderCom->Update_AABB(vPos);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

	return iExit;
}

void CEmerald::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CEmerald::Render_GameObject()
{
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// 월드 행렬 복사
	_matrix matWorld = *m_pTransformCom->Get_World();

	// 카메라 위치 가져오기
	_matrix matView;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);

	// 뷰 행렬에서 카메라 방향만 추출
	matView._41 = matView._42 = matView._43 = 0.f; // 위치 제거
	D3DXMatrixInverse(&matView, NULL, &matView);  // 뷰 -> 월드 방향

	// 빌보드 회전 적용
	matWorld._11 = matView._11; matWorld._12 = matView._12; matWorld._13 = matView._13;
	matWorld._21 = matView._21; matWorld._22 = matView._22; matWorld._23 = matView._23;
	matWorld._31 = matView._31; matWorld._32 = matView._32; matWorld._33 = matView._33;

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	m_pTextureCom->Set_Texture(0);
	m_pBufferCom->Render_Buffer();

	m_pColliderCom->Render_Collider();

	m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void CEmerald::Init_Emerald()
{
	m_bDrop = true;

	_float fAngle = ((float)(rand() % 360)) * D3DX_PI / 180.f;

	_float fSpeed = 3.f + (rand() % 3);

	m_vVelocity.x = cosf(fAngle) * fSpeed;
	m_vVelocity.z = sinf(fAngle) * fSpeed;
	m_vVelocity.y = 10.f;
}

void CEmerald::Pop_Emerald(const _float fTimeDelta)
{
	if (m_bDrop)
	{
		if (m_bFirstPop)
		{
			CSoundMgr::GetInstance()->PlayEffect(L"Emerald/sfx_item_emeraldBurstOut-001_soundWave.wav", 0.6f);

			m_bFirstPop = false;
		}

		_vec3 vPos;
		m_pTransformCom->Get_Info(INFO_POS, &vPos);

		m_vVelocity.y -= 40.f * fTimeDelta;

		vPos += m_vVelocity * fTimeDelta;

		if (vPos.y <= 1.5f)
		{
			vPos.y = 1.5f;

			// 방법 1: 그냥 멈춤
			m_vVelocity = _vec3(0.f, 0.f, 0.f);
			m_bDrop = false;
			m_bChase = true;

			CSoundMgr::GetInstance()->PlayEffect(L"Emerald/sfx_item_emeraldBurstOutPing-001_soundWave.wav", 0.6f);

			// 방법 2: 살짝 바운스 (원하면)
			// m_vVelocity.y *= -0.3f;
		}

		m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	}
}

void CEmerald::Chase_Player(const _float fTimeDelta)
{
	if (!m_bChase)
		return;

	CTransform* pPlayerTrans = dynamic_cast<CTransform*>(CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));

	if (!pPlayerTrans)
		return;

	_vec3 vPlayerPos, vMyPos;
	pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);
	m_pTransformCom->Get_Info(INFO_POS, &vMyPos);

	_vec3 vDir;
	vDir = vPlayerPos - vMyPos;

	_float fDiff = D3DXVec3Length(&vDir);

	if (fDiff <= 0.5f)
	{
		m_bDead = true;

		CSoundMgr::GetInstance()->PlayEffect(L"Emerald/sfx_item_emeraldCollect-001_soundWave.wav", 0.6f);
	}
	else
	{
		m_pTransformCom->Chase_Target(&vPlayerPos, 30.f, fTimeDelta);
	}
}

HRESULT CEmerald::Add_Component()
{
	CComponent* pComponent = nullptr;

	pComponent = m_pBufferCom = dynamic_cast<CRcTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ { L"Com_Buffer", pComponent } });

	// Transform
	pComponent = m_pTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ { L"Com_Transform", pComponent } });

	// Texture
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_EmeraldTexture"));

	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ { L"Com_Texture", pComponent } });

	// Collider
	m_pColliderCom = CCollider::Create(m_pGraphicDev, _vec3(1.5f, 1.5f, 1.5f), _vec3(0.f, 0.f, 0.f));

	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });


	return S_OK;
}

CEmerald* CEmerald::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CEmerald* pEmerald = new CEmerald(pGraphicDev);

	if (FAILED(pEmerald->Ready_GameObject()))
	{
		Safe_Release(pEmerald);
		MSG_BOX("Emerald Create Failed");
		return nullptr;
	}

	return pEmerald;
}

void CEmerald::Free()
{
	CGameObject::Free();
}