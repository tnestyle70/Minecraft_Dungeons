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
#include "CHotbar.h"
#include "CBlockRenderer.h"
#include "CLayer.h"

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
	_int iExit = CScene::Update_Scene(fTimeDelta);

	CBlockMgr::GetInstance()->Update(fTimeDelta);

	CTriggerBoxMgr::GetInstance()->Update(fTimeDelta);

	CIronBarMgr::GetInstance()->Update(fTimeDelta);

	CMonsterMgr::GetInstance()->Update(fTimeDelta);
	
	if (GetAsyncKeyState(VK_RETURN) || CTriggerBoxMgr::GetInstance()->IsSceneChanged())
	{
		//Render Group Clear Before Change Scene!!!!
		//TriggerBoxMgr 다시 설정
		CTriggerBoxMgr::GetInstance()->SetSceneChanged(false);
		CRenderer::GetInstance()->Clear_RenderGroup();
		CTriggerBoxMgr::GetInstance()->Clear();
		CIronBarMgr::GetInstance()->Clear();
		CMonsterMgr::GetInstance()->Clear();
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
	CScene::LateUpdate_Scene(fTimeDelta);

	CTriggerBoxMgr::GetInstance()->LateUpdate(fTimeDelta);

	CIronBarMgr::GetInstance()->LateUpdate(fTimeDelta);

	CMonsterMgr::GetInstance()->LateUpdate(fTimeDelta);
}

void CSquidCoast::Render_Scene()
{
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

	pGameObject = CDynamicCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);

	if (!pGameObject)
		return E_FAIL;
	
	if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
		return E_FAIL;

	// Block Renderer
	pGameObject = CBlockRenderer::Create(m_pGraphicDev);
	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"BlockRenderer", pGameObject)))
		return E_FAIL;

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
	
	//TriggerBoxMgr
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(pGameObject);
	CCollider* pCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
	if (!pCollider)
	{
		MSG_BOX("Player Collider Set Failed");
	}
	CTriggerBoxMgr::GetInstance()->SetPlayerCollider(pCollider);

	//Monster
	pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::ZOMBIE);
	
	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Monster", pGameObject)))
		return E_FAIL;
	//멀티맵이라 이름 같아도 가능, 그냥 맵은 안 됨
	pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::SKELETON);
	
	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Monster", pGameObject)))
		return E_FAIL; 

	pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::CREEPER);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Monster", pGameObject)))
		return E_FAIL; 

	pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::SPIDER);

	if (!pGameObject)
		return E_FAIL;
	if (FAILED(pLayer->Add_GameObject(L"Monster", pGameObject)))
		return E_FAIL;

	//Boss
	pGameObject = CRedStoneGolem::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"RedStoneGolem", pGameObject)))
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

	// Hotbar UI (Composite)
	pGameObject = CHotbar::Create(m_pGraphicDev);
	if (!pGameObject)
		return E_FAIL;

	CHotbar* pHotbar = static_cast<CHotbar*>(pGameObject);
	pHotbar->Set_Pos(0.f, 0.f); // Root at top-left for full-screen test image

	if (FAILED(pLayer->Add_GameObject(L"Hotbar", pGameObject)))

		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

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

	CBlockMgr::GetInstance()->SetEditorMode(false); // 먼저 모드 설정

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
	CScene::Free();
}
