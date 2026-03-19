#include "pch.h"
#include "CSquidCoast.h"
#include "CMonster.h"
#include "CPlayer.h"
#include "CIronBar.h"
#include "CTriggerBox.h"
#include "CMonsterAnim.h"
#include "CEditor.h"
#include "CBlockMgr.h"
#include "CTriggerBoxMgr.h"
#include "CIronBarMgr.h"
#include "CMonsterMgr.h"
#include "CDynamicCamera.h"
#include "CSceneChanger.h"
#include "CRenderer.h"
#include "StageData.h"
#include "CRedStoneGolem.h"
#include "CParticleMgr.h"
#include "CHotbar.h"
#include "CLayer.h"
#include "CAncientGuardian.h"
#include "CHUD.h"
#include "CDragon.h"
#include "CBox.h"
#include "CInventoryMgr.h"
#include "CInventorySlot.h"

CSquidCoast::CSquidCoast(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{
}

CSquidCoast::~CSquidCoast()
{
}

HRESULT CSquidCoast::Ready_Scene()
{
	if (FAILED(Ready_Light()))
		return E_FAIL;

	if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
		return E_FAIL;

	if (FAILED(Ready_GameLogic_Layer(L"GameLogic_Layer")))
		return E_FAIL;

	if (FAILED(Ready_UI_Layer(L"UI_Layer")))
		return E_FAIL;

	Ready_StageData(L"../Bin/Data/Stage1.dat");

	return S_OK;
}

_int CSquidCoast::Update_Scene(const _float& fTimeDelta)
{
	//Inventory 활성화되었을 경우 Inventory만 Update
	if (CInventoryMgr::GetInstance()->IsActive())
	{
		CInventoryMgr::GetInstance()->Update(fTimeDelta);
		return 0;
	}

	_int iExit = CScene::Update_Scene(fTimeDelta);

	CBlockMgr::GetInstance()->Update(fTimeDelta);

	CTriggerBoxMgr::GetInstance()->Update(fTimeDelta);

	CIronBarMgr::GetInstance()->Update(fTimeDelta);

	CMonsterMgr::GetInstance()->Update(fTimeDelta);

	CParticleMgr::GetInstance()->Update(fTimeDelta);

	//이펙트 테스트
	if (GetAsyncKeyState('1') & 0x8000)
	{
		CParticleMgr::GetInstance()->Add_Emitter(
			CParticleEmitter::Create(m_pGraphicDev,
				PARTICLE_FIREWORK, _vec3(0.f, 2.f, 0.f), nullptr)
		);

		CParticleMgr::GetInstance()->Add_Emitter(
			CParticleEmitter::Create(m_pGraphicDev,
				PARTICLE_HIT, _vec3(5.f, 2.f, 0.f), nullptr)
		);
	}

	//Inventory Toggle
	if (GetAsyncKeyState('I') & 0x8000)
	{
		CInventoryMgr::GetInstance()->SetInventory(true);
	}
	
	//Scene Change
	if (GetAsyncKeyState(VK_RETURN) || CTriggerBoxMgr::GetInstance()->IsSceneChanged())
	{
		//Render Group Clear Before Change Scene!!!!
		//TriggerBoxMgr 다시 설정
		CTriggerBoxMgr::GetInstance()->SetSceneChanged(false);
		CRenderer::GetInstance()->Clear_RenderGroup();
		CTriggerBoxMgr::GetInstance()->Clear();
		CIronBarMgr::GetInstance()->Clear();
		CMonsterMgr::GetInstance()->Clear();
		CParticleMgr::GetInstance()->Clear_Emitters();
		if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_CAMP)))
		{
			MSG_BOX("Camp Create Failed");
			return -1;
		}

		return iExit;
	}

	auto iter = m_mapLayer.find(L"GameLogic_Layer");

	if (iter != m_mapLayer.end())
		iter->second->Delete_GameObject(fTimeDelta);

	return iExit;
}

void CSquidCoast::LateUpdate_Scene(const _float& fTimeDelta)
{
	if (CInventoryMgr::GetInstance()->IsActive())
	{
		CInventoryMgr::GetInstance()->LateUpdate(fTimeDelta);
		return;
	}

	CScene::LateUpdate_Scene(fTimeDelta);

	CTriggerBoxMgr::GetInstance()->LateUpdate(fTimeDelta);

	CIronBarMgr::GetInstance()->LateUpdate(fTimeDelta);

	CMonsterMgr::GetInstance()->LateUpdate(fTimeDelta);
}

void CSquidCoast::Render_Scene()
{
	if (CInventoryMgr::GetInstance()->IsActive())
	{
		CInventoryMgr::GetInstance()->Render();
		return;
	}

	CBlockMgr::GetInstance()->Render();

	CParticleMgr::GetInstance()->Render();
}

void CSquidCoast::Render_UI()
{
	if (CInventoryMgr::GetInstance()->IsActive())
		return;
	//데미지 폰트 렌더링
	//CDamageMgr
}

HRESULT CSquidCoast::Ready_Environment_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//dynamic camera
	_vec3 vEye{ 0.f, 10.f, -10.f };
	_vec3 vAt{ 0.f, 0.f, 1.f };
	_vec3 vUp{ 0.f, 1.f, 0.f };

	m_pDynamicCamera = CDynamicCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);
	pGameObject = m_pDynamicCamera;

	CDynamicCamera* pDynamicCam = dynamic_cast<CDynamicCamera*>(pGameObject);
	if (!pDynamicCam)
		return E_FAIL;

	//pDynamicCam->SetActionCam();

	if (!pGameObject)
		return E_FAIL;
	
	if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
		return E_FAIL;

	//Effect
	CParticleMgr::GetInstance()->Add_Emitter(
		CParticleEmitter::Create(m_pGraphicDev,
			PARTICLE_FIREWORK, _vec3(0.f, 2.f, 0.f), nullptr)
	);

	CParticleMgr::GetInstance()->Add_Emitter(
		CParticleEmitter::Create(m_pGraphicDev,
			PARTICLE_HIT, _vec3(5.f, 2.f, 0.f), nullptr)
	);

	//SkyBox 추가

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CSquidCoast::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	//Player
	pGameObject = CPlayer::Create(m_pGraphicDev);
	
	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Player", pGameObject)))
		return E_FAIL;

	CPlayer* pPlayer = dynamic_cast<CPlayer*>(pGameObject);

	//TriggerBoxMgr
	CCollider* pCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
	if (!pCollider)
	{
		MSG_BOX("Player Collider Set Failed");
	}
	CTriggerBoxMgr::GetInstance()->SetPlayerCollider(pCollider);
	CMonsterMgr::GetInstance()->SetPlayer(pPlayer);

	//고정 카메라 추가
	if (m_pDynamicCamera)
		m_pDynamicCamera->SetFollowTarget(
			dynamic_cast<Engine::CTransform*>(pPlayer->Get_Component(ID_DYNAMIC, L"Com_Transform")));
	
	//Dragon
	//pGameObject = CDragon::Create(m_pGraphicDev);
	//if (!pGameObject)
	//{
	//	MSG_BOX("Dragon Create Failed");
	//	return E_FAIL;
	//}
	//pLayer->Add_GameObject(L"Dragon", pGameObject);

	//m_mapLayer.insert({ pLayerTag, pLayer });

	////Boss
	//pGameObject = CRedStoneGolem::Create(m_pGraphicDev);

	//if (!pGameObject)
	//	return E_FAIL;

	//if (FAILED(pLayer->Add_GameObject(L"RedStoneGolem", pGameObject)))
	//	return E_FAIL; 

	//pGameObject = CAncientGuardian::Create(m_pGraphicDev, _vec3(5.f, 5.f, 5.f));
	//if (!pGameObject) 
	//	return E_FAIL;

	//if (FAILED(pLayer->Add_GameObject(L"AncientGuardian", pGameObject)))
	//	return E_FAIL;

	//Object
	pGameObject = CBox::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Box", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CSquidCoast::Ready_UI_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;
	//HUD
	pGameObject = CHUD::Create(m_pGraphicDev);

	if (nullptr == pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"HUD", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	//InventorySlot
	//pGameObject = CInventorySlot::Create(m_pGraphicDev);
	//if (!pGameObject)
	//	return E_FAIL;
	//if (FAILED(pLayer->Add_GameObject(L"InventorySlot", pGameObject)))
	//	return E_FAIL;
	////Set Position, Scale
	//CInventorySlot* pSlot = dynamic_cast<CInventorySlot*>(pGameObject);
	//pSlot->SetSlotInfo(300.f, 300.f, 150.f, 150.f);

	//m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CSquidCoast::Ready_Light()
{
	return S_OK;
}

HRESULT CSquidCoast::Ready_StageData(const _tchar* szPath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, szPath, L"rb");
	if (!pFile)
		return E_FAIL;

	// 1. 블럭 (LoadBlocks 내부에서 RebuildBatchMesh까지)
	
	//BlockMgr
	if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
	{
		MSG_BOX("block mgr create failed");
		return E_FAIL;
	}

	CBlockMgr::GetInstance()->SetRenderMode(eRenderMode::RENDER_BATCH); // 먼저 모드 설정

	CBlockMgr::GetInstance()->LoadBlocks(pFile);

	// 2. 몬스터 - map에 안 담고 레이어에 바로 추가
	int iCount = 0;
	fread(&iCount, sizeof(int), 1, pFile);

	for (int i = 0; i < iCount; ++i)
	{
		MonsterData tData;
		fread(&tData, sizeof(MonsterData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };

		CGameObject* pMonster = CMonster::Create(
			m_pGraphicDev, (EMonsterType)tData.iMonsterType, vPos);

		//MonsterMgr 쪽에 추가
		if(pMonster)
			CMonsterMgr::GetInstance()->AddMonster(pMonster, tData.iTriggerID);
	}

	// 3. 창살
	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		IronBarData tData;
		fread(&tData, sizeof(IronBarData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };

		CGameObject* pIronBar = CIronBar::Create(m_pGraphicDev, vPos);
		if (pIronBar)
			CIronBarMgr::GetInstance()->AddIronBar(pIronBar, tData.iTriggerID);
			//m_mapLayer[L"GameLogic_Layer"]->Add_GameObject(L"IronBar", pIronBar);
	}
	
	// 4. 트리거박스
	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		TriggerBoxData tData;
		fread(&tData, sizeof(TriggerBoxData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };

		CGameObject* pTriggerBox = CTriggerBox::Create(m_pGraphicDev, vPos, tData.iTriggerID ,(eTriggerBoxType)tData.iTriggerBoxType);
		if (pTriggerBox)
			CTriggerBoxMgr::GetInstance()->AddTriggerBox(pTriggerBox);
			//m_mapLayer[L"GameLogic_Layer"]->Add_GameObject(L"TriggerBox", pTriggerBox);
	}

	fclose(pFile);
	return S_OK;
}

CSquidCoast* CSquidCoast::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CSquidCoast* pSquidCoast = new CSquidCoast(pGraphicDev);

	if (FAILED(pSquidCoast->Ready_Scene()))
	{
		Safe_Release(pSquidCoast);
		MSG_BOX("pSquidCoast Create Failed");
		return nullptr;
	}

	return pSquidCoast;
}

void CSquidCoast::Free()
{
	CRenderer::GetInstance()->Clear_RenderGroup();
	CTriggerBoxMgr::GetInstance()->Clear();
	CIronBarMgr::GetInstance()->Clear();
	CMonsterMgr::GetInstance()->Clear();
	CParticleMgr::GetInstance()->Clear_Emitters();
	CBlockMgr::GetInstance()->ClearBlocks();

	CMonsterMgr::GetInstance()->Clear();
	CTriggerBoxMgr::GetInstance()->Clear();
	CIronBarMgr::GetInstance()->Clear();

	CScene::Free();
}
