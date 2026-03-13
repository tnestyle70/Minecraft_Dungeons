#include "pch.h"
#include "CRedStoneGolem.h"
#include "CRenderer.h"

CRedStoneGolem::CRedStoneGolem(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
	, m_pHeadBufferCom(nullptr)
	, m_pBodyBufferCom(nullptr)
	, m_pLeftShoulderBufferCom(nullptr)
	, m_pRightShoulderBufferCom(nullptr)
	, m_pHipBufferCom(nullptr)
	, m_pCoreBufferCom(nullptr)
	, m_pLeftArmBufferCom(nullptr)
	, m_pRightArmBufferCom(nullptr)
	, m_pLeftLegBufferCom(nullptr)
	, m_pRightLegBufferCom(nullptr)
	, m_pTextureCom(nullptr)
	, m_pHeadTransformCom(nullptr)
	, m_pBodyTransformCom(nullptr)
	, m_pLeftShoulderTransformCom(nullptr)
	, m_pRightShoulderTransformCom(nullptr)
	, m_pHipTransformCom(nullptr)
	, m_pCoreTransformCom(nullptr)
	, m_pLeftArmTransformCom(nullptr)
	, m_pRightArmTransformCom(nullptr)
	, m_pLeftLegTransformCom(nullptr)
	, m_pRightLegTransformCom(nullptr)
{
}

CRedStoneGolem::CRedStoneGolem(const CRedStoneGolem& rhs)
	: CGameObject(rhs)
	, m_pHeadBufferCom(nullptr)
	, m_pBodyBufferCom(nullptr)
	, m_pLeftShoulderBufferCom(nullptr)
	, m_pRightShoulderBufferCom(nullptr)
	, m_pHipBufferCom(nullptr)
	, m_pCoreBufferCom(nullptr)
	, m_pLeftArmBufferCom(nullptr)
	, m_pRightArmBufferCom(nullptr)
	, m_pLeftLegBufferCom(nullptr)
	, m_pRightLegBufferCom(nullptr)
	, m_pTextureCom(nullptr)
	, m_pHeadTransformCom(nullptr)
	, m_pBodyTransformCom(nullptr)
	, m_pLeftShoulderTransformCom(nullptr)
	, m_pRightShoulderTransformCom(nullptr)
	, m_pHipTransformCom(nullptr)
	, m_pCoreTransformCom(nullptr)
	, m_pLeftArmTransformCom(nullptr)
	, m_pRightArmTransformCom(nullptr)
	, m_pLeftLegTransformCom(nullptr)
	, m_pRightLegTransformCom(nullptr)
{
}

CRedStoneGolem::~CRedStoneGolem()
{
}

HRESULT CRedStoneGolem::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	Set_DefaultScale();

	m_pBodyTransformCom->Set_Pos(0.f, 10.f, 0.f);

	return S_OK;
}

_int CRedStoneGolem::Update_GameObject(const _float& fTimeDelta)
{
	Set_PartsPos();

	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return iExit;
}

void CRedStoneGolem::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CRedStoneGolem::Render_GameObject()
{
	// Body
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pBodyTransformCom->Get_World());
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pTextureCom->Set_Texture(0);
	m_pBodyBufferCom->Render_Buffer();

	// Head
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pHeadTransformCom->Get_World());
	m_pHeadBufferCom->Render_Buffer();

	// Left Shoulder
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pLeftShoulderTransformCom->Get_World());
	m_pLeftShoulderBufferCom->Render_Buffer();

	// Right Shoulder
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pRightShoulderTransformCom->Get_World());
	m_pRightShoulderBufferCom->Render_Buffer();

	// Hip
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pHipTransformCom->Get_World());
	m_pHipBufferCom->Render_Buffer();

	// Core
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pCoreTransformCom->Get_World());
	m_pCoreBufferCom->Render_Buffer();

	// Left Arm
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pLeftArmTransformCom->Get_World());
	m_pLeftArmBufferCom->Render_Buffer();

	// Right Arm
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pRightArmTransformCom->Get_World());
	m_pRightArmBufferCom->Render_Buffer();

	// Left Leg
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pLeftLegTransformCom->Get_World());
	m_pLeftLegBufferCom->Render_Buffer();

	// Right Leg
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pRightLegTransformCom->Get_World());
	m_pRightLegBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

HRESULT CRedStoneGolem::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

	// Buffer

	pComponent = m_pHeadBufferCom = dynamic_cast<CRedStoneGolemHeadTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemHeadTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_HeadBuffer", pComponent });

	pComponent = m_pBodyBufferCom = dynamic_cast<CRedStoneGolemBodyTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemBodyTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_BodyBuffer", pComponent });

	pComponent = m_pLeftShoulderBufferCom = dynamic_cast<CRedStoneGolemShoulderTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemShoulderTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_LeftShoulderBuffer", pComponent });

	pComponent = m_pRightShoulderBufferCom = dynamic_cast<CRedStoneGolemShoulderTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemShoulderTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_RightShoulderBuffer", pComponent });

	pComponent = m_pHipBufferCom = dynamic_cast<CRedStoneGolemHipTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemHipTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_HipBuffer", pComponent });

	pComponent = m_pCoreBufferCom = dynamic_cast<CRedStoneGolemCoreTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemCoreTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_CoreBuffer", pComponent });

	pComponent = m_pLeftArmBufferCom = dynamic_cast<CRedStoneGolemArmTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemArmTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_LeftArmBuffer", pComponent });

	pComponent = m_pRightArmBufferCom = dynamic_cast<CRedStoneGolemArmTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemArmTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_RightArmBuffer", pComponent });

	pComponent = m_pLeftLegBufferCom = dynamic_cast<CRedStoneGolemLegTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemLegTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_LeftLegBuffer", pComponent });

	pComponent = m_pRightLegBufferCom = dynamic_cast<CRedStoneGolemLegTex*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemLegTex"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_RightLegBuffer", pComponent });

	// Texture

	pComponent = m_pTextureCom = dynamic_cast<CTexture*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemTexture"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });


	// Transform

	pComponent = m_pBodyTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_BodyTransform", pComponent });

	pComponent = m_pHeadTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_HeadTransform", pComponent });

	pComponent = m_pLeftShoulderTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_LeftShoulderTransform", pComponent });

	pComponent = m_pRightShoulderTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_RightShoulderTransform", pComponent });

	pComponent = m_pHipTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_HipTransform", pComponent });

	pComponent = m_pCoreTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_CoreTransform", pComponent });

	pComponent = m_pLeftArmTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_LeftArmTransform", pComponent });

	pComponent = m_pRightArmTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_RightArmTransform", pComponent });

	pComponent = m_pLeftLegTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_LeftLegTransform", pComponent });

	pComponent = m_pRightLegTransformCom = dynamic_cast<CTransform*>(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (pComponent == nullptr)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_RightLegTransform", pComponent });

	return S_OK;
}

void CRedStoneGolem::Set_PartsPos()
{
	// Root
	_vec3 vBodyPos;
	m_pBodyTransformCom->Get_Info(INFO_POS, &vBodyPos);

	// Head
	m_pHeadTransformCom->Set_Pos(vBodyPos.x, vBodyPos.y + 0.15f, vBodyPos.z - 0.42f);

	// LeftShoulder
	m_pLeftShoulderTransformCom->Set_Pos(vBodyPos.x + 0.7f, vBodyPos.y, vBodyPos.z);

	// RightShoulder
	m_pRightShoulderTransformCom->Set_Pos(vBodyPos.x - 0.7f, vBodyPos.y, vBodyPos.z);

	// Hip
	m_pHipTransformCom->Set_Pos(vBodyPos.x, vBodyPos.y - 0.5f, vBodyPos.z);

	// Core
	m_pCoreTransformCom->Set_Pos(vBodyPos.x, vBodyPos.y - 0.1f, vBodyPos.z + 0.15f);

	// LeftArm
	m_pLeftArmTransformCom->Set_Pos(vBodyPos.x + 0.8f, vBodyPos.y - 0.6f, vBodyPos.z);

	// RightArm
	m_pRightArmTransformCom->Set_Pos(vBodyPos.x - 0.8f, vBodyPos.y - 0.6f, vBodyPos.z);

	// LeftLeg
	m_pLeftLegTransformCom->Set_Pos(vBodyPos.x + 0.35f, vBodyPos.y - 0.9f, vBodyPos.z);

	// RightLeg
	m_pRightLegTransformCom->Set_Pos(vBodyPos.x - 0.35f, vBodyPos.y - 0.9f, vBodyPos.z);
}

void CRedStoneGolem::Set_DefaultScale()
{
	m_pHeadTransformCom->Set_Scale(0.45f);
	m_pLeftShoulderTransformCom->Set_Scale(0.6f);
	m_pRightShoulderTransformCom->Set_Scale(0.6f);
	m_pHipTransformCom->Set_Scale(0.6f);
	m_pCoreTransformCom->Set_Scale(0.45f);
	m_pLeftArmTransformCom->Set_Scale(0.6f);
	m_pRightArmTransformCom->Set_Scale(0.6f);
	m_pLeftLegTransformCom->Set_Scale(0.55f);
	m_pRightLegTransformCom->Set_Scale(0.55f);
}

CRedStoneGolem* CRedStoneGolem::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CRedStoneGolem* pRedStoneGolem = new CRedStoneGolem(pGraphicDev);

	if (FAILED(pRedStoneGolem->Ready_GameObject()))
	{
		Safe_Release(pRedStoneGolem);
		MSG_BOX("RedStoneGolem Create Failed");
		return nullptr;
	}

	return pRedStoneGolem;
}

void CRedStoneGolem::Free()
{
	CGameObject::Free();
}