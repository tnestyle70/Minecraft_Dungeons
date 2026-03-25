#include "pch.h"
#include "CNetworkStage.h"
#include "COceanTypes.h"
#include "COcean.h"
#include "CNetworkMgr.h"
#include "CNetworkPlayer.h"
#include "CRemotePlayer.h"
#include "CCollider.h"
#include "CMonster.h"
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
#include "CLightMgr.h"

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

	// ── Day 8: 원격 탑승자 기준 드래곤 위치 동기화 ──────────────────────
	// m_bNetworkControlled는 "원격 점유 중인 드래곤"에만 true로 유지한다.
	bool bRemoteOccupied[4] = { false, false, false, false };
	for (auto& [id, pRemote] : CNetworkMgr::GetInstance()->GetRemoteMap())
	{
		if (!pRemote) continue;

		int idx = pRemote->Get_DragonIdx();
		if (!pRemote->Is_OnDragon() || idx < 0 || idx >= 4 || !m_pDragon[idx])
			continue;

		// 원격 제어 모드 설정 + 위치 강제 이동
		bRemoteOccupied[idx] = true;
		m_pDragon[idx]->Set_NetworkControlled(true);
		m_pDragon[idx]->Force_RootPos(pRemote->Get_DragonPos());
	}

	// 원격 점유되지 않은 드래곤은 로컬 입력 가능 상태로 복원
	for (int i = 0; i < 4; ++i)
	{
		if (m_pDragon[i] && !bRemoteOccupied[i])
			m_pDragon[i]->Set_NetworkControlled(false);
	}

	// ── Day 9: PVP 화살 충돌 검사 → 피해 전송 ──────────────────────────
	if (m_pLocalPlayer)
	{
		bool bDebugLogged = false; // 첫 화살/첫 원격만 진단 로그 1회 남김
		for (auto& pArrow : m_pLocalPlayer->Get_Arrows())
		{
			if (!pArrow || pArrow->Is_Dead() || !pArrow->Get_Collider()) continue;

			for (auto& [id, pRemote] : CNetworkMgr::GetInstance()->GetRemoteMap())
			{
				if (!pRemote) continue;
				auto pRemoteCol = pRemote->Get_Collider();
				if (!pRemoteCol)
				{
					if (!bDebugLogged)
					{
						FILE* fp = nullptr;
						if (_wfopen_s(&fp, L"debug-9b3cff.log", L"a") == 0 && fp)
						{
							fprintf(fp,
								"{\"sessionId\":\"9b3cff\",\"runId\":\"pre-fix\",\"hypothesisId\":\"H7\",\"location\":\"Client/Code/CNetworkStage.cpp:remote_collider_null\",\"message\":\"remote_collider_null\",\"data\":{\"remotePlayerId\":%d},\"timestamp\":%llu}\n",
								pRemote->GetPlayerId(),
								(unsigned long long)GetTickCount64());
							fclose(fp);
						}
						bDebugLogged = true;
					}
					continue;
				}

				if (!bDebugLogged)
				{
					auto aArrowAABB = pArrow->Get_Collider()->Get_AABB();
					auto aRemoteAABB = pRemoteCol->Get_AABB();
					bool bHit = pArrow->Get_Collider()->IsColliding(aRemoteAABB);

					FILE* fp = nullptr;
					if (_wfopen_s(&fp, L"debug-9b3cff.log", L"a") == 0 && fp)
					{
						fprintf(fp,
							"{\"sessionId\":\"9b3cff\",\"runId\":\"pre-fix\",\"hypothesisId\":\"H6\",\"location\":\"Client/Code/CNetworkStage.cpp:debug_pvp_first\",\"message\":\"pvp_aabb_debug_once\",\"data\":{\"remotePlayerId\":%d,\"arrowAABB\":[%.3f,%.3f,%.3f,%.3f,%.3f,%.3f],\"remoteAABB\":[%.3f,%.3f,%.3f,%.3f,%.3f,%.3f],\"isColliding\":%d},\"timestamp\":%llu}\n",
							pRemote->GetPlayerId(),
							aArrowAABB.vMin.x, aArrowAABB.vMin.y, aArrowAABB.vMin.z,
							aArrowAABB.vMax.x, aArrowAABB.vMax.y, aArrowAABB.vMax.z,
							aRemoteAABB.vMin.x, aRemoteAABB.vMin.y, aRemoteAABB.vMin.z,
							aRemoteAABB.vMax.x, aRemoteAABB.vMax.y, aRemoteAABB.vMax.z,
							bHit ? 1 : 0,
							(unsigned long long)GetTickCount64());
						fclose(fp);
					}

					bDebugLogged = true;
				}

				if (pArrow->Get_Collider()->IsColliding(pRemoteCol->Get_AABB()))
				{
					CNetworkMgr::GetInstance()->SendDamage(pRemote->GetPlayerId(), pArrow->Get_Damage());
					// #region agent log
					{
						FILE* fp = nullptr;
						if (_wfopen_s(&fp, L"debug-9b3cff.log", L"a") == 0 && fp)
						{
							fprintf(fp, "{\"sessionId\":\"9b3cff\",\"runId\":\"pre-fix\",\"hypothesisId\":\"H4\",\"location\":\"Client/Code/CNetworkStage.cpp:88\",\"message\":\"arrow_hit_remote_send_damage\",\"data\":{\"targetPlayerId\":%d,\"damage\":%.3f},\"timestamp\":%llu}\n",
								pRemote->GetPlayerId(), pArrow->Get_Damage(), (unsigned long long)GetTickCount64());
							fclose(fp);
						}
					}
					// #endregion
					pArrow->Set_Dead();
					break;  // 화살 하나는 한 대상에만 피해
				}
			}
		}
	}

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
				bool  bOnDragon = m_pLocalPlayer->Is_Riding();
				int   iDragonIdx = -1;
				float fDragonX = 0.f, fDragonY = 0.f, fDragonZ = 0.f;
				if (bOnDragon)
				{
					fDirX = 0.f; fDirZ = 0.f; bMoving = false;
					for (int i = 0; i < 4; ++i)
					{
						if (m_pDragon[i] && m_pDragon[i]->Is_Ridden())
						{
							iDragonIdx = i;
							// Day 8: 용 루트 위치 전송
							_vec3 vDP = m_pDragon[i]->Get_SpineRoot();
							fDragonX = vDP.x; fDragonY = vDP.y; fDragonZ = vDP.z;
							break;
						}
					}
				}

				CNetworkMgr::GetInstance()->SendInput(fDirX, fDirZ, fRotY, bMoving,
					vCurPos.x, vCurPos.y, vCurPos.z, bOnDragon, iDragonIdx,
					fDragonX, fDragonY, fDragonZ);

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

	//파도 조작
	if (m_pOcean)
	{
		if (GetAsyncKeyState('8') & 0x8000)
			m_pOcean->Set_WaveType(0);
		if (GetAsyncKeyState('9') & 0x8000)
			m_pOcean->Set_WaveType(1);
		if (GetAsyncKeyState('0') & 0x8000)
			m_pOcean->Set_WaveType(2);
	}
}

void CNetworkStage::Render_Scene()
{
	CBlockMgr::GetInstance()->Render();

	// ── 원격 플레이어 렌더 ────────────────────────────────────────────
	CNetworkMgr::GetInstance()->Render();

	// 디버그: 원격 플레이어 콜라이더 박스 렌더 (BeginScene 안에서 호출해야 보임)
	for (auto& [id, pRemote] : CNetworkMgr::GetInstance()->GetRemoteMap())
	{
		if (pRemote && pRemote->Get_Collider())
			pRemote->Get_Collider()->Render_Collider();
	}
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

	// Ocean — 격자/타입만 지정, 나머지는 OCEAN_DESC 기본 멤버 초기화값 사용
	OCEAN_DESC oceanDesc{};
	oceanDesc.dwVtxCntX = 128;
	oceanDesc.dwVtxCntZ = 128;
	oceanDesc.eType = static_cast<WAVE_TYPE>(2); // Gerstner

	m_pOcean = COcean::Create(m_pGraphicDev, oceanDesc);
	pGameObject = m_pOcean;
	
	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Ocean", pGameObject)))
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
		{ -10.f, 4.f,  10.f },
		{  10.f, 4.f,  10.f },
		{ -10.f, 4.f, -10.f },
		{  10.f, 4.f, -10.f },
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
		m_pDragon[i]->Set_RootPos(vDragonSpawn[i]);
		m_pDragon[i]->Set_MoveTarget(vDragonSpawn[i]);

		if (FAILED(pLayer->Add_GameObject(szDragonTag[i], m_pDragon[i])))
			return E_FAIL;
	}

	//Player
	pGameObject = CNetworkPlayer::Create(m_pGraphicDev);

	if (!pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"Player", pGameObject)))
		return E_FAIL;

	CNetworkPlayer* pPlayer = dynamic_cast<CNetworkPlayer*>(pGameObject);

	//HUD
	pGameObject = CHUD::Create(m_pGraphicDev);

	if (nullptr == pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"HUD", pGameObject)))
		return E_FAIL;

	CHUD* pHUD = dynamic_cast<CHUD*>(pGameObject);
	pHUD->Set_NetworkPlayer(pPlayer);

	//TriggerBoxMgr
	//CNetworkPlayer* pPlayer = dynamic_cast<CNetworkPlayer*>(pGameObject);
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

		// Day 9: 피격 HP 갱신을 위해 네트워크 매니저에 로컬 플레이어 등록
		CNetworkMgr::GetInstance()->SetLocalPlayer(m_pLocalPlayer);
	}

	// ── 서버 접속 — 프로세스 ID 기반 고유 닉네임 (임시, 추후 로비 UI 연동) ─
	char szNick[32];
	sprintf_s(szNick, sizeof(szNick), "Player%u", GetCurrentProcessId() % 10000);
	CNetworkMgr::GetInstance()->Connect(m_pGraphicDev, "192.168.0.7", 9000, szNick);

	m_mapLayer.insert({ pLayerTag, pLayer });

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
	D3DLIGHT9   tLightInfo;
	ZeroMemory(&tLightInfo, sizeof(D3DLIGHT9));
	tLightInfo.Type = D3DLIGHT_DIRECTIONAL;
	tLightInfo.Diffuse = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	tLightInfo.Specular = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	tLightInfo.Ambient = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	tLightInfo.Direction = { 1.f, -1.f, 1.f };
	if (FAILED(CLightMgr::GetInstance()->Ready_Light(m_pGraphicDev, &tLightInfo, 0)))
		return E_FAIL;
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
	m_pDynamicCamera = nullptr;
	m_pOcean = nullptr;
	m_pLocalPlayer = nullptr;
	CScene::Free();
}
