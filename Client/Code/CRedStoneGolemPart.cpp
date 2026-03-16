#include "pch.h"
#include "CRedStoneGolemPart.h"
#include "CRenderer.h"

CRedStoneGolemPart::CRedStoneGolemPart(LPDIRECT3DDEVICE9 pGrpahicDev, REDSTONEGOLEM_PART ePart)
	: CGameObject(pGrpahicDev)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_ePart(ePart)
{
}

CRedStoneGolemPart::CRedStoneGolemPart(const CRedStoneGolemPart& rhs)
	: CGameObject(rhs)
	, m_pBufferCom(nullptr)
	, m_pTransformCom(nullptr)
	, m_ePart(rhs.m_ePart)
{
}

CRedStoneGolemPart::~CRedStoneGolemPart()
{
}

HRESULT CRedStoneGolemPart::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

	return S_OK;
}

_int CRedStoneGolemPart::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);

    return iExit;
}

void CRedStoneGolemPart::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CRedStoneGolemPart::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransformCom->Get_World());

	m_pBufferCom->Render_Buffer();
}

HRESULT CRedStoneGolemPart::Add_Component() 
{
    Engine::CComponent* pComponent = nullptr;

    // Buffer

    switch (m_ePart)
    {
    case GOLEM_HEAD:
        pComponent = m_pBufferCom = dynamic_cast<CRedStoneGolemHeadTex*>(
            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemHeadTex"));
        break;

    case GOLEM_BODY:
        pComponent = m_pBufferCom = dynamic_cast<CRedStoneGolemBodyTex*>(
            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemBodyTex"));
        break;

    case GOLEM_LARM:
    case GOLEM_RARM:
        pComponent = m_pBufferCom = dynamic_cast<CRedStoneGolemArmTex*>(
            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemArmTex"));
        break;

    case GOLEM_LLEG:
    case GOLEM_RLEG:
        pComponent = m_pBufferCom = dynamic_cast<CRedStoneGolemLegTex*>(
            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemLegTex"));
        break;

    case GOLEM_CORE:
        pComponent = m_pBufferCom = dynamic_cast<CRedStoneGolemCoreTex*>(
            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemCoreTex"));
        break;

    case GOLEM_LSHOULDER:
    case GOLEM_RSHOULDER:
        pComponent = m_pBufferCom = dynamic_cast<CRedStoneGolemShoulderTex*>(
            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemShoulderTex"));
        break;

    case GOLEM_HIP:
        pComponent = m_pBufferCom = dynamic_cast<CRedStoneGolemHipTex*>(
            CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RedStoneGolemHipTex"));
        break;

    default:
        return E_FAIL;
    }

    if (!pComponent)
        return E_FAIL;

    m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

    // Transform
    pComponent = m_pTransformCom = dynamic_cast<CTransform*>(
        CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

    if (pComponent == nullptr)
        return E_FAIL;

    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    return S_OK;
}

CRedStoneGolemPart* CRedStoneGolemPart::Create(LPDIRECT3DDEVICE9 pGraphicDev, REDSTONEGOLEM_PART ePart)
{
	CRedStoneGolemPart* pPart = new CRedStoneGolemPart(pGraphicDev, ePart);

	if (FAILED(pPart->Ready_GameObject()))
	{
		Safe_Release(pPart);
		MSG_BOX("RedStoneGolemPart Create Failed");
		return nullptr;
	}

	return pPart;
}

void CRedStoneGolemPart::Free()
{
    CGameObject::Free();
}