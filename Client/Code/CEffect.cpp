#include "pch.h"
#include "CEffect.h"
#include "CRenderer.h"

CEffect::CEffect(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{
}

CEffect::CEffect(const CGameObject& rhs)
	: CGameObject(rhs)
{
}

CEffect::~CEffect()
{
}

HRESULT CEffect::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransCom->Set_Pos(_float(rand() % 20), 0.f, _float(rand() % 20));

	m_fFrame = 0.f;

	return S_OK;
}

_int CEffect::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);
	//이펙트 프레임 이동 시키기
	m_fFrame += 90.f * fTimeDelta;

	if (m_fFrame > 90.f)
	{
		m_fFrame = 0.f;
	}

	_vec3 vPos;
	m_pTransCom->Get_Info(INFO_POS, &vPos);

	//compute view z 빌보드 구현
	CGameObject::Compute_ViewZ(&vPos);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

	return iExit;
}

void CEffect::LateUpdate_GameObject(const _float& fTimeDelta)
{
	_matrix matBill, matWorld, matView;

	matWorld = *m_pTransCom->Get_World();

	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixIdentity(&matBill);

	//Y축 회전만 제거 11 13 31 33이 Y축 회전에 대한 정보를 가지고 있음!
	matBill._11 = matView._11;
	matBill._13 = matView._13;
	matBill._31 = matView._31;
	matBill._33 = matView._33;
	
	D3DXMatrixInverse(&matBill, 0, &matBill);

	matWorld = matBill * matWorld;

	m_pTransCom->Set_World(&matWorld);

	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CEffect::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransCom->Get_World());
	
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pTexture->Set_Texture(_uint(m_fFrame));

	m_pRcTex->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CEffect::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

	// RcTex
	pComponent = m_pRcTex = dynamic_cast<CRcTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Texture
	pComponent = m_pTexture = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_EffectTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	// Transform
	pComponent = m_pTransCom = dynamic_cast<CTransform*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	return S_OK;
}

CEffect* CEffect::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CEffect* pEffect = new CEffect(pGraphicDev);

	if (FAILED(pEffect->Ready_GameObject()))
	{
		Safe_Release(pEffect);
		MSG_BOX("pEffect Create Failed");
		return nullptr;
	}

	return pEffect;
}

void CEffect::Free()
{
	CGameObject::Free();
}
