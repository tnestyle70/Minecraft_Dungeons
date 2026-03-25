#include "pch.h"
#include "CNetworkStage.h"
#include "CNetworkMgr.h"
#include "CMonster.h"
#include "CNetworkPlayer.h"
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
#include "CHUD.h"
#include "CDragon.h"

CNetworkStage::CNetworkStage(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev)
{}

CNetworkStage::~CNetworkStage()
{}

HRESULT CNetworkStage::Ready_Scene()
{
	if (FAILED(Ready_Light()))
		return E_FAIL;

	if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
		return E_FAIL;

	if (FAILED(Ready_GameLogic_Layer(L"GameLogic_Layer")))
		return E_FAIL;

	if (FAILED(Ready_UI_Layer(L"UI_Layer")))
		return E_FAIL;

	Ready_StageData(L"../Bin/Data/Stage3.dat");

	return S_OK;
}

_int CNetworkStage::Update_Scene(const _float& fTimeDelta)
{
	_int iExit = CScene::Update_Scene(fTimeDelta);

	// ── 네트워크 수신 + 원격 플레이어 업데이트 ──────────────────────────
	CNetworkMgr::GetInstance()->Update(fTimeDelta);

	// ── 로컬 플레이어 이동 방향 → 서버 전송 (20TPS) ─────────────────────
	if (m_pLocalPlayer && CNetworkMgr::GetInstance()->IsLoggedIn())
	{
		m_fInputTimer += fTimeDelta;
		if (m_fInputTimer >= 0.05f)   // 20TPS = 50ms
		{
			m_fInputTimer = 0.f;

			Engine::CTransform* pTr = m_pLocalPlayer->Get_Transform();
			if (pTr)
			{
				_vec3 vCurPos;
				pTr->Get_Info(INFO_POS, &vCurPos);

				// 이전 프레임 대비 이동 벡터로 방향 추출
				_vec3 vDelta = vCurPos - m_vPrevPlayerPos;
				vDelta.y = 0.f;
				float fLen = D3DXVec3Length(&vDelta);
				bool  bMoving = (fLen > 0.01f);

				float fDirX = 0.f, fDirZ = 0.f;
				if (bMoving)
				{
					_vec3 vNorm;
					D3DXVec3Normalize(&vNorm, &vDelta);
					fDirX = vNorm.x;
					fDirZ = vNorm.z;
				}

				float fRotY = pTr->m_vAngle.y;

				// 탑승 중이면 방향/이동 0으로 명시 (서버 적분 스킵과 의미 일치)
				bool bOnDragon = m_pLocalPlayer->Is_Riding();
				int  iDragonIdx = -1;
				if (bOnDragon)
				{
					fDirX = 0.f; fDirZ = 0.f; bMoving = false;
					for (int i = 0; i < 4; ++i)
					{
						if (m_pDragon[i] && m_pDragon[i]->Is_Ridden())
						{
							iDragonIdx = i; break;
						}
					}
				}

				CNetworkMgr::GetInstance()->SendInput(fDirX, fDirZ, fRotY, bMoving,
					vCurPos.x, vCurPos.y, vCurPos.z, bOnDragon, iDragonIdx);

				m_vPrevPlayerPos = vCurPos;
			}
		}
	}

	CBlockMgr::GetInstance()->Update(fTimeDelta);

	CTriggerBoxMgr::GetInstance()->Update(fTimeDelta);

	CIronBarMgr::GetInstance()->Update(fTimeDelta);

	CMonsterMgr::GetInstance()->Update(fTimeDelta);

	//if (GetAsyncKeyState(VK_RETURN) || CTriggerBoxMgr::GetInstance()->IsSceneChanged())
	//{
	//	//Render Group Clear Before Change Scene!!!!
	//	//TriggerBoxMgr 다시 설정
	//	CTriggerBoxMgr::GetInstance()->SetSceneChanged(false);
	//	CRenderer::GetInstance()->Clear_RenderGroup();
	//	CTriggerBoxMgr::GetInstance()->Clear();
	//	CIronBarMgr::GetInstance()->Clear();
	//	CMonsterMgr::GetInstance()->Clear();
	//	if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_OBSIDIAN)))
	//	{
	//		MSG_BOX("Obsidian Create Failed");
	//		return -1;
	//	}
	//	return iExit;
	//}

	return iExit;
}

void CNetworkStage::LateUpdate_Scene(const _float& fTimeDelta)
{
	CScene::LateUpdate_Scene(fTimeDelta);

	// ── 원격 플레이어 LateUpdate ─────────────────────────────────────────
	CNetworkMgr::GetInstance()->LateUpdate(fTimeDelta);

	CTriggerBoxMgr::GetInstance()->LateUpdate(fTimeDelta);

	CIronBarMgr::GetInstance()->LateUpdate(fTimeDelta);

	CMonsterMgr::GetInstance()->LateUpdate(fTimeDelta);
}

void CNetworkStage::Render_Scene()
{
	CBlockMgr::GetInstance()->Render();

	// ── 원격 플레이어 렌더 (Day 3: CPlayerBody 연동 예정) ────────────────
	CNetworkMgr::GetInstance()->Render();
}

void CNetworkStage::Render_UI()
{}

HRESULT CNetworkStage::Ready_Environment_Layer(const _tchar* pLayerTag)
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
	pDynamicCam->SetActionCam();

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
		return E_FAIL;

	//SkyBox 추가

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CNetworkStage::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	// ── 드래곤 4마리 스폰 ──────────────────────────────────────────────────
	const _vec3 vDragonSpawn[4] = {
		{ -5.f, 8.f,  5.f },
		{  5.f, 16.f,  5.f },
		{ -5.f, 24.f, -5.f },
		{  5.f, 32.f, -5.f },
	};

	const wchar_t* szDragonTag[4] = {
		L"Dragon_0", L"Dragon_1", L"Dragon_2", L"Dragon_3"
	};

	for (int i = 0; i < 4; ++i)
	{
		m_pDragon[i] = CDragon::Create(m_pGraphicDev);
		if (!m_pDragon[i])
		{
			MSG_BOX("Dragon Create Failed");
			return E_FAIL;
		}
		m_pDragon[i]->Set_MoveTarget(vDragonSpawn[i]);
		//m_pDragon[i]->Set_PatrolStartIndex(i);  // 0,1,2,3 → 각자 다른 순찰 포인트에서 시작

		if (FAILED(pLayer->Add_GameObject(szDragonTag[i], m_pDragon[i])))
			return E_FAIL;
	}

	m_mapLayer.insert({ pLayerTag, pLayer });

	//Player
	pGameObject = CNetworkPlayer::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Player", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	//TriggerBoxMgr
	CNetworkPlayer* pPlayer = dynamic_cast<CNetworkPlayer*>(pGameObject);
	CCollider* pCollider = pPlayer
		? dynamic_cast<CCollider*>(pPlayer->Get_Component(ID_STATIC, L"Com_Collider"))
		: nullptr;
	if (!pCollider)
	{
		MSG_BOX("Player Collider Set Failed");
	}
	CTriggerBoxMgr::GetInstance()->SetPlayerCollider(pCollider);
	//CMonsterMgr::GetInstance()->SetPlayer(pPlayer);

	//고정카메라 추가
	if (m_pDynamicCamera)
		m_pDynamicCamera->SetFollowTarget(
			dynamic_cast<Engine::CTransform*>(pPlayer->Get_Component(ID_DYNAMIC, L"Com_Transform")));

	// ── 로컬 플레이어 포인터 저장 (입력 추출용) ──────────────────────────
	m_pLocalPlayer = pPlayer;
	if (m_pLocalPlayer)
	{
		Engine::CTransform* pTr = m_pLocalPlayer->Get_Transform();
		if (pTr) pTr->Get_Info(INFO_POS, &m_vPrevPlayerPos);

		// 드래곤 리스트 전달 (탑승 인식용)
		m_pLocalPlayer->Set_DragonList(m_pDragon, 4);
	}

	// ── 서버 접속 — 프로세스 ID 기반 고유 닉네임 (임시, 추후 로비 UI 연동) ─
	char szNick[32];
	sprintf_s(szNick, sizeof(szNick), "Player%u", GetCurrentProcessId() % 10000);
	//와이파이 IP로 접속 가능하도록 변경
	CNetworkMgr::GetInstance()->Connect(m_pGraphicDev, "192.168.0.7", 9000, szNick);
	
	//Boss
	//pGameObject = CRedStoneGolem::Create(m_pGraphicDev);

	//if (!pGameObject)
	//	return E_FAIL;

	//if (FAILED(pLayer->Add_GameObject(L"RedStoneGolem", pGameObject)))
	//	return E_FAIL;

	//m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

HRESULT CNetworkStage::Ready_UI_Layer(const _tchar* pLayerTag)
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

	return S_OK;
}

HRESULT CNetworkStage::Ready_Light()
{
	return S_OK;
}

HRESULT CNetworkStage::Ready_StageData(const _tchar* szPath)
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
		if (pMonster)
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
	}

	// 4. 트리거박스
	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		TriggerBoxData tData;
		fread(&tData, sizeof(TriggerBoxData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };

		CGameObject* pTriggerBox = CTriggerBox::Create(m_pGraphicDev, vPos, tData.iTriggerID, (eTriggerBoxType)tData.iTriggerBoxType);
		if (pTriggerBox)
			CTriggerBoxMgr::GetInstance()->AddTriggerBox(pTriggerBox);
		//m_mapLayer[L"GameLogic_Layer"]->Add_GameObject(L"TriggerBox", pTriggerBox);
	}

	fclose(pFile);
	return S_OK;
}

CNetworkStage* CNetworkStage::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CNetworkStage* pRedStone = new CNetworkStage(pGraphicDev);

	if (FAILED(pRedStone->Ready_Scene()))
	{
		Safe_Release(pRedStone);
		MSG_BOX("pRedStone Create Failed");
		return nullptr;
	}

	return pRedStone;
}

void CNetworkStage::Free()
{
	CScene::Free();
}
