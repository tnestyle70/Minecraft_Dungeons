#include "pch.h"
#include "CTriggerBox.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CIronBarMgr.h"
#include "CMonsterMgr.h"

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

HRESULT CTriggerBox::Ready_GameObject(const _vec3 & vPos, _int iTriggerID
	,eTriggerBoxType& triggerType)
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	//Setting Collider, Transform
	m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	m_pColliderCom->Update_AABB(vPos);
	m_iTriggerID = iTriggerID;
	m_eTrigger = triggerType;

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

void CTriggerBox::CheckCollide(CCollider* pCollider)
{
	bool bColliding = m_pColliderCom->IsColliding(pCollider->Get_AABB());

	if (bColliding && !m_bTriggered)
	{
		m_bTriggered = true;

		switch (m_eTrigger)
		{
		case TRIGGER_IRONBAR:
			Trigger_Ironbar();
			break;
		case TRIGGER_MONSTER:
			Trigger_Monster();
			break;
		case TRIGGER_SCENECHANGE:
			Trigger_SceneChange();
			break;
		case TRIGGER_END:
			break;
		default:
			break;
		}
	}
	else if (!bColliding && m_bTriggered)
	{
		m_bTriggered = false;
	}
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

void CTriggerBox::Trigger_Ironbar()
{
	//IronbarMgr에게 이벤트 발생만 전달
	if (CIronBarMgr::GetInstance()->IsClosed())
	{
		CIronBarMgr::GetInstance()->Open();
	}
	else
	{
		CIronBarMgr::GetInstance()->Close();
	}
}

void CTriggerBox::Trigger_Monster()
{
	//MonsterMgr의 TriggerID 설정
	CMonsterMgr::GetInstance()->SetTriggerID(m_iTriggerID);
}

void CTriggerBox::Trigger_SceneChange()
{
	//창살 닿은 이후에, 다음 스테이지로 넘어갈 수 있도록 진행
	
}

CTriggerBox* CTriggerBox::Create(LPDIRECT3DDEVICE9 pGraphicDev, 
	const _vec3& vPos, _int iTriggerID, eTriggerBoxType triggerType)
{
	CTriggerBox* pTriggerBox = new CTriggerBox(pGraphicDev);

	if (FAILED(pTriggerBox->Ready_GameObject(vPos, iTriggerID ,triggerType)))
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
