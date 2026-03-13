#include "pch.h"
#include "CEditor.h"
#include "CBlockMgr.h"
#include "CDynamicCamera.h"
#include "CMonsterUV.h"
#include "CSceneChanger.h"
#include "CTriggerBox.h"

CEditor::CEditor(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev), m_bEditorMode(false)
{
	m_pGraphicDev->AddRef();
}

CEditor::~CEditor()
{
}

HRESULT CEditor::Ready_Scene()
{
	if (FAILED(Ready_ProtoType()))
		return E_FAIL;

	if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
		return E_FAIL;

	//blockmgr 초기화
	if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
	{
		MSG_BOX("block mgr create failed");
		return E_FAIL;
	}

	if (FAILED(CBlockMgr::GetInstance()->Ready_Textures()))
	{
		MSG_BOX("block mgr create failed");
		return E_FAIL;
	}

	//block placer 연결
	m_pBlockPlacer = new CBlockPlacer(m_pGraphicDev);

	if (!m_pBlockPlacer)
	{
		MSG_BOX("block placer create failed");
		return E_FAIL;
	}

	return S_OK;
}

_int CEditor::Update_Scene(const _float& fTimeDelta)
{
	if (!m_bEditorMode)
	{
		return 0;
	}

	_int iExit = CScene::Update_Scene(fTimeDelta);

	//TriggerBox, Monster Rendering
	for (auto& pair : m_mapTriggerBoxes)
	{
		pair.second->Update_GameObject(fTimeDelta);
	}
	for (auto& pair : m_mapMonsters)
	{
		pair.second->Update_GameObject(fTimeDelta);
	}
	for (auto& pair : m_mapIronBars)
	{
		pair.second->Update_GameObject(fTimeDelta);
	}

	switch (m_eEditMode)
	{
	case MODE_BLOCK:
		//update block by selected pannel of the palette
		m_pBlockPlacer->Update_Placer(m_eSelectedBlock);
		CBlockMgr::GetInstance()->Update(fTimeDelta);
		break;
	case MODE_MONSTER:
		UpdateMonsterMode();
		break;
	case MODE_IRONBAR:
		UpdateIronBarMode();
		//m_pBlockPlacer->Update_Placer(BLOCK_IRONBAR);
		//CBlockMgr::GetInstance()->Update(fTimeDelta);
		break;
	case MODE_TRIGGERBOX:
		UpdateTriggerBoxMode();
		break;
	default:
		break;
	}

	return iExit;
}

void CEditor::LateUpdate_Scene(const _float& fTimeDelta)
{
	Engine::CScene::LateUpdate_Scene(fTimeDelta);
}

void CEditor::Render_Scene()
{
	if (!m_bEditorMode)
	{
		return;
	}

	Render_MenuBar();
	Render_Hierarchy();
	Render_Inspector();
	Render_Viewport();

	//에디터냐 스테이지냐에 따라서 서로 다른 렌더링 방식 사용
	//DDS or png
	CBlockMgr::GetInstance()->Render();
}

HRESULT CEditor::Ready_Environment_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();
	
	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;

	// dynamicCamera
	_vec3   vEye{ 0.f, 10.f, -10.f };
	_vec3   vAt{ 0.f, 0.f, 0.f };
	_vec3   vUp{ 0.f, 1.f, 0.f };

	pGameObject = CDynamicCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);

	if (nullptr == pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
		return E_FAIL;

	m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

void CEditor::SetEditorMode(bool editorMode)
{
	m_bEditorMode = !m_bEditorMode;

	//에디터가 꺼질 경우 스테이지 진입하면서 LoadBlocks 호출
	CBlockMgr::GetInstance()->SetEditorMode(m_bEditorMode);

	if (m_bEditorMode)
	{
		LoadStageData(GetStagePath(m_eCurrentStage));
	}
}

const _tchar* CEditor::GetStagePath(eStageType eStage) const
{
	//스테이지 경로
	static const _tchar* s_aPath[STAGE_END] =
	{
		L"../Bin/Data/Stage1.dat", //Squid Coast
		L"../Bin/Data/Stage2.dat", //Camp
		L"../Bin/Data/Stage3.dat", //RedStone
		L"../Bin/Data/Stage4.dat" //Obsidian
	};
	
	if (eStage < 0 || eStage >= STAGE_END)
	{
		return s_aPath[0];
	}

	return s_aPath[eStage];
}

void CEditor::SwitchStage(eStageType eNewStage)
{
	if (eNewStage == m_eCurrentStage)
		return;

	//현재 스테이지 자동 저장
	SaveStageData(GetStagePath(m_eCurrentStage));
	
	//스테이지 전환
	m_eCurrentStage = eNewStage;

	//블럭 클리어 후 새 스테이지 로딩
	if (FAILED(LoadStageData(GetStagePath(m_eCurrentStage))))
	{
		//If there is no file, clear stage
		CBlockMgr::GetInstance()->ClearBlocks();
		m_mapMonsters.clear();
		m_mapIronBars.clear();
	}
}

void CEditor::Render_MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			//블럭들 삭제
			if (ImGui::MenuItem("New Scene")) 
			{
				CBlockMgr::GetInstance()->ClearBlocks();
			}
			//블럭 저장
			if (ImGui::MenuItem("Save Scene")) 
			{
				if (SaveStageData(GetStagePath(m_eCurrentStage)))
				{
					MSG_BOX("Save Failed");
				}
				else
				{
					MSG_BOX("Save Success");
				}
			}
			//블럭 불러오기
			if (ImGui::MenuItem("Load Scene"))
			{
				if (LoadStageData(GetStagePath(m_eCurrentStage)))
				{
					MSG_BOX("Load Failed");
				}
				else
				{
					MSG_BOX("Load Success");
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit Editor")) { m_bEditorMode = false; }
			ImGui::EndMenu();
		}
		//메뉴바 오른쪽에 현재 스테이지의 이름 표시
		static const char* s_aStageNames[STAGE_END] =
		{
			"Squid Coast", "Camp", "RedStone", "Obsidian"
		};
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 150.f);
		ImGui::TextDisabled("Stage : %s", s_aStageNames[m_eCurrentStage]);

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void CEditor::Render_Hierarchy()
{
	ImGui::Begin("Hierarchy");

	ImGui::Text("Block Count: %d", (int)CBlockMgr::GetInstance()->Get_Blocks().size());
	ImGui::Text("Monster Spawn: %d", (int)m_mapMonsters.size());
	ImGui::Text("IronBar: %d", (int)m_mapIronBars.size());
	ImGui::Text("TriggerBox: %d", (int)m_mapTriggerBoxes.size());

	ImGui::End();
}

void CEditor::Render_Inspector()
{
	ImGui::Begin("Inspector");

	//스테이지 선택 패널 최상단에 배치
	Render_StageSelector();

	ImGui::Separator();

	//Mode Tab Button
	const char* szModes[] =
	{
		"Block", "Monster", "IronBar", "TriggerBox"
	};

	for (int i = 0; i < 4; ++i)
	{
		if (i > 0) ImGui::SameLine();
		bool bActive = (m_eEditMode == (eEditMode)i);
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.f));
		if (ImGui::Button(szModes[i], ImVec2(70.f, 25.f)))
			m_eEditMode = (eEditMode)i;
		if (bActive)
			ImGui::PopStyleColor();
	}

	ImGui::Separator();

	switch (m_eEditMode)
	{
	case MODE_BLOCK:
		Render_BlockPalette();
		break;
	case MODE_MONSTER:
		Render_MonsterPalette();
		break;
	case MODE_IRONBAR:
		Render_IronBarPalette();
		break;
	case MODE_TRIGGERBOX:
		Render_TriggerPalette();
		break;
	default:
		break;
	}

	ImGui::End();
}

void CEditor::Render_Viewport()
{
	ImGui::Begin("Viewport");

	// TODO: 뷰포트 렌더 타겟 연결

	ImGui::End();
}

void CEditor::Render_StageSelector()
{
	static const char* s_aNames[STAGE_END] =
	{ "Squid Coast", "Camp", "RedStone", "Obsidian" };
	static const char* s_aIDs[STAGE_END] =
	{ "Squid Coast##stage", "Camp##stage", "RedStone##stage", "Obsidian##stage" };

	ImGui::Text("Stage");

	for (int i = 0; i < STAGE_END; ++i)
	{
		if (i > 0) ImGui::SameLine();

		bool bActive = (m_eCurrentStage == (eStageType)i);
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.6f, 0.1f, 1.f));

		if (ImGui::Button(s_aIDs[i], ImVec2(85.f, 24.f)))
		{
			if (!bActive)
				SwitchStage((eStageType)i);
		}

		if (bActive)
			ImGui::PopStyleColor();
	}

	ImGui::SameLine();
	ImGui::TextDisabled("(Stage%d.dat)", (int)m_eCurrentStage + 1);
}

void CEditor::Render_BlockPalette()
{
	struct BlockEntry { const char* label; const char* name; eBlockType eType; };
	// "##block" suffix → StageSelector의 "Obsidian##stage"와 ID 완전 분리
	static const BlockEntry palette[] =
	{
		{"Grass##block",      "Grass",      BLOCK_GRASS},
		{"Dirt##block",       "Dirt",       BLOCK_DIRT},
		{"Rock##block",       "Rock",       BLOCK_ROCK},
		{"Sand##block",       "Sand",       BLOCK_SAND},
		{"Bedrock##block",    "Bedrock",    BLOCK_BEDROCK},
		{"Obsidian##block",   "Obsidian",   BLOCK_OBSIDIAN},
		{"StoneBrick##block", "StoneBrick", BLOCK_STONEBRICK},
	};
	constexpr int iCount = (int)(sizeof(palette) / sizeof(palette[0]));

	ImGui::Text("Block Palette");
	for (int i = 0; i < iCount; ++i)
	{
		bool bSelected = (m_eSelectedBlock == palette[i].eType);
		if (bSelected)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.f));
		if (ImGui::Button(palette[i].label, ImVec2(120.f, 28.f)))
			m_eSelectedBlock = palette[i].eType;
		if (bSelected)
			ImGui::PopStyleColor();
	}
	ImGui::Separator();
	ImGui::Text("Selected: %s", palette[m_eSelectedBlock].name);
}

void CEditor::Render_MonsterPalette()
{
	ImGui::Text("Monster Spawn");
	ImGui::Separator();
	//TriggerID 설정 추가
	ImGui::Separator();
	ImGui::Text("TriggerID: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.f);
	ImGui::InputInt("##MonsterTriggerID", &m_iMonsterTriggerID, 0, 0);
	ImGui::SameLine();
	ImGui::TextDisabled("(-1 = 즉시스폰)");

	ImGui::Separator();
	//몬스터 타입 선택
	const char* szMonsters[] =
	{
		"Zombie", "Skeleton", "Creeper", "Spider"
	};

	ImGui::Text("Type:");
	
	for (int i = 0; i < 4; ++i)
	{
		//Allign Buttons to Coloum
		if (i > 0)
			ImGui::SameLine();
		bool bSelected = (m_iSelectedMonster == i);
		if (bSelected)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.f));
		if (ImGui::Button(szMonsters[i], ImVec2(75.f, 25.f)))
			m_iSelectedMonster = i;
		if (bSelected)
			ImGui::PopStyleColor();
	}
	ImGui::Separator();
	ImGui::Text("Spawns : %d", (int)m_mapMonsters.size());

	MonsterData* pPendingDelete = nullptr;

	//배치된 스폰 목록
	for (auto& pair : m_mapMonsters)
	{
		const MonsterData& tData = pair.first;
		ImGui::Text("[%s] (%d %d %d)",
			szMonsters[tData.iMonsterType],
			tData.x, tData.y, tData.z);

		ImGui::SameLine();

		// TriggerID 인라인 수정
		char szID[32];
		sprintf_s(szID, "##mid_%d_%d_%d", tData.x, tData.y, tData.z);
		ImGui::SetNextItemWidth(40.f);
		int iID = tData.iTriggerID;
		if (ImGui::InputInt(szID, &iID, 0, 0))
		{
			MonsterData tNew = tData;
			tNew.iTriggerID = iID;
			auto pObj = pair.second;
			m_mapMonsters.erase(pair.first);
			m_mapMonsters.insert({ tNew, pObj });
			break;  // iterator 무효화
		}

		ImGui::SameLine();
		ImGui::Text("(%d,%d,%d)", tData.x, tData.y, tData.z);
		ImGui::SameLine();

		char szBtn[32];
		sprintf_s(szBtn, "X##%d_%d_%d", tData.x, tData.y, tData.z);
		if (ImGui::Button(szBtn))
			pPendingDelete = const_cast<MonsterData*>(&pair.first);

	}
	
	if (pPendingDelete)
	{
		auto iter = m_mapMonsters.find(*pPendingDelete);
		if (iter != m_mapMonsters.end())
		{
			//CMonster Release
			Safe_Release(iter->second);
			m_mapMonsters.erase(iter);
		}
		pPendingDelete = nullptr;
	}
}

void CEditor::Render_IronBarPalette()
{
	ImGui::Text("IronBar Placement");
	ImGui::Separator();
	ImGui::Text("Placed: %d", (int)m_mapIronBars.size());

	IronBarData* pPendingDelete = nullptr;

	for (auto& pair : m_mapIronBars)
	{
		const IronBarData& tData = pair.first;

		ImGui::Text("TriggerID:[%d] (%d %d %d)",
			tData.iTriggerID, tData.x, tData.y, tData.z);

		ImGui::SameLine();

		char szBtn[32];
		sprintf_s(szBtn, "X##%d_%d_%d", tData.x, tData.y, tData.z);

		if (ImGui::Button(szBtn))
			pPendingDelete = const_cast<IronBarData*>(&tData);
	}

	if (pPendingDelete)
	{
		auto iter = m_mapIronBars.find(*pPendingDelete);
		if (iter != m_mapIronBars.end())
		{
			Safe_Release(iter->second);  // CIronBar 해제
			m_mapIronBars.erase(iter);
		}
		pPendingDelete = nullptr;
	}
}

void CEditor::Render_TriggerPalette()
{
	ImGui::Text("TriggerBox Placement");
	ImGui::Separator();
	//타입 선택
	const char* szTypes[] = { "IronBar", "Monster", "SceneChange" };
	ImGui::Text("Type:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::Combo("##TriggerType", &m_iTriggerType, szTypes, IM_ARRAYSIZE(szTypes));
	// TriggerID 설정
	ImGui::Text("TriggerID:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.f);
	ImGui::InputInt("##TriggerID", &m_iCurTriggerID), 0, 0;
	ImGui::SameLine();
	if (ImGui::Button("Auto"))  // 클릭마다 자동 증가
		m_iCurTriggerID++;

	ImGui::Separator();

	// 배치된 목록
	ImGui::Text("Placed: %d", (int)m_mapTriggerBoxes.size());

	TriggerBoxData* pPendingDelete = nullptr;

	for (auto& pair : m_mapTriggerBoxes)
	{
		TriggerBoxData& tData = const_cast<TriggerBoxData&>(pair.first);

		// 타입 표시 + 변경
		const char* szTypes2[] = { "IronBar", "Monster", "SceneChange" };
		char szCombo[32];
		sprintf_s(szCombo, "##type_%d_%d_%d", tData.x, tData.y, tData.z);

		ImGui::SetNextItemWidth(90.f);
		int iType = tData.iTriggerBoxType;
		if (ImGui::Combo(szCombo, &iType, szTypes2, IM_ARRAYSIZE(szTypes2)))
		{
			// map은 key 변경 불가 → 지우고 다시 삽입
			TriggerBoxData tNew = tData;
			tNew.iTriggerBoxType = iType;
			auto pObj = pair.second;
			m_mapTriggerBoxes.erase(pair.first);
			m_mapTriggerBoxes.insert({ tNew, pObj });
			break;  // iterator 무효화됐으니 이번 프레임 중단
		}

		ImGui::SameLine();

		// TriggerID 변경
		char szID[32];
		sprintf_s(szID, "##id_%d_%d_%d", tData.x, tData.y, tData.z);
		ImGui::SetNextItemWidth(40.f);
		int iID = tData.iTriggerID;
		if (ImGui::InputInt(szID, &iID, 0))
		{
			TriggerBoxData tNew = tData;
			tNew.iTriggerID = iID;
			auto pObj = pair.second;
			m_mapTriggerBoxes.erase(pair.first);
			m_mapTriggerBoxes.insert({ tNew, pObj });
			break;
		}

		ImGui::SameLine();
		ImGui::Text("(%d,%d,%d)", tData.x, tData.y, tData.z);
		ImGui::SameLine();

		// 삭제 버튼
		char szBtn[32];
		sprintf_s(szBtn, "X##tb_%d_%d_%d", tData.x, tData.y, tData.z);
		if (ImGui::Button(szBtn))
			pPendingDelete = &tData;
	}

	if (pPendingDelete)
	{
		auto iter = m_mapTriggerBoxes.find(*pPendingDelete);
		if (iter != m_mapTriggerBoxes.end())
		{
			Safe_Release(iter->second);
			m_mapTriggerBoxes.erase(iter);
		}
	}
}

void CEditor::UpdateMonsterMode()
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;
	
	bool bLBtn = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool bRBtn = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

	if (bLBtn && !m_bLBtnPrev)
	{
		_vec3 vRayPos, vRayDir;
		m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);
		
		BlockPos tHitPos;
		float fT = 0.f;
		if (CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos, vRayDir,
			&tHitPos, &fT))
		{
			MonsterData tData;
			tData.x = tHitPos.x;
			tData.y = tHitPos.y + 1;
			tData.z = tHitPos.z;
			tData.iMonsterType = m_iSelectedMonster;
			tData.iTriggerID = m_iMonsterTriggerID;
			
			//Add Monster
			_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
			CMonster* pMonster = CMonster::Create(m_pGraphicDev, (EMonsterType)m_iSelectedMonster ,vPos);
			m_mapMonsters.insert({ tData, pMonster });
		}
	}
	//Right Btn -> Select the closet spawn point
	if (bRBtn && !m_bRBtnPrev)
	{
		_vec3 vRayPos, vRayDir;
		m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);

		BlockPos tHitPos;
		float fT = 0.f;
		if (CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos, vRayDir,
			&tHitPos, &fT))
		{
			//delete the spawn point of hit point
			for (auto iter = m_mapMonsters.begin(); iter != m_mapMonsters.end(); ++iter)
			{
				const MonsterData& tData = iter->first;
				if (tData.x == tHitPos.x && tData.y == tHitPos.y && tData.z == tHitPos.z)
				{
					Safe_Release(iter->second);
					m_mapMonsters.erase(iter);
					break;
				}
			}
		}
	}
	m_bLBtnPrev = bLBtn;
	m_bRBtnPrev = bRBtn;
}

void CEditor::UpdateIronBarMode()
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	bool bLBtn = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool bRBtn = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

	if (bLBtn && !m_bLBtnPrev)
	{
		_vec3 vRayPos, vRayDir;
		m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);

		BlockPos tHitPos;
		float fT = 0.f;
		if (CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos, vRayDir, &tHitPos, &fT))
		{
			IronBarData tData;
			tData.x = tHitPos.x;
			tData.y = tHitPos.y + 1;
			tData.z = tHitPos.z;
			tData.iTriggerID = 0;

			//Add IronBar
			if (m_mapIronBars.find(tData) == m_mapIronBars.end())
			{
				_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
				CIronBar* pIronBar = CIronBar::Create(m_pGraphicDev, vPos);
				if (pIronBar)
					m_mapIronBars.insert({ tData, pIronBar });
			}

		}
	}

	if (bRBtn && !m_bRBtnPrev)
	{
		_vec3 vRayPos, vRayDir;
		m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);

		BlockPos tHitPos;
		float fT = 0.f;
		if (CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos, vRayDir, &tHitPos, &fT))
		{
			for (auto iter = m_mapIronBars.begin(); iter != m_mapIronBars.end(); ++iter)
			{
				const IronBarData& tData = iter->first;
				if (tData.x == tHitPos.x &&
					tData.y == tHitPos.y + 1 &&
					tData.z == tHitPos.z)
				{
					Safe_Release(iter->second);
					m_mapIronBars.erase(iter);
					break;
				}
			}
		}
	}

	m_bLBtnPrev = bLBtn;
	m_bRBtnPrev = bRBtn;
}

void CEditor::UpdateTriggerBoxMode()
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	bool bLBtn = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool bRBtn = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

	if (bLBtn && !m_bLBtnPrev)
	{
		_vec3 vRayPos, vRayDir;
		m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);

		BlockPos tHitPos;
		float fT = 0.f;
		if (CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos, vRayDir, &tHitPos, &fT))
		{
			TriggerBoxData tData;
			tData.x = tHitPos.x;
			tData.y = tHitPos.y + 1;
			tData.z = tHitPos.z;
			tData.width = m_iTriggerWidth;
			tData.height = m_iTriggerHeight;
			tData.depth = m_iTriggerDepth;
			tData.iTriggerBoxType = m_iTriggerType;
			tData.iTriggerID = m_iCurTriggerID;

			//Add Trigger Box
			_vec3 vPos = {(float)tData.x, (float)tData.y, (float)tData.z};
			CTriggerBox* pTriggerBox = CTriggerBox::Create(m_pGraphicDev, vPos,
				eTriggerBoxType(m_iTriggerType));
			if (pTriggerBox)
			{
				m_mapTriggerBoxes.insert({ tData, pTriggerBox });
			}
		}
	}
	
	if (bRBtn && !m_bRBtnPrev)
	{
		_vec3 vRayPos, vRayDir;
		m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);

		BlockPos tHitPos;
		float fT = 0.f;
		if (CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos, vRayDir, &tHitPos, &fT))
		{
			//delete same hit point trigger box data
			for (auto iter = m_mapTriggerBoxes.begin(); iter != m_mapTriggerBoxes.end(); ++iter)
			{
				const TriggerBoxData& tData = iter->first;
				if (tData.x == tHitPos.x && tData.y == tHitPos.y && tData.z == tHitPos.z)
				{
					Safe_Release(iter->second);
					m_mapTriggerBoxes.erase(iter);
					break;
				}
			}
		}
	}

	m_bLBtnPrev = bLBtn;
	m_bRBtnPrev = bRBtn;
}

HRESULT CEditor::SaveStageData(const _tchar* szPath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, szPath, L"wb");
	if (!pFile)
		return E_FAIL;
	//1. 블럭 저장
	CBlockMgr::GetInstance()->SaveBlocks(pFile);

	//2. 몬스터 저장
	int iCount = (int)m_mapMonsters.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& pair : m_mapMonsters)
	{
		MonsterData tData = pair.first;
		fwrite(&tData, sizeof(MonsterData), 1, pFile);
	}

	//3. 창살 저장
	iCount = (int)m_mapIronBars.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& pair : m_mapIronBars)
	{
		IronBarData tData = pair.first;
		fwrite(&tData, sizeof(IronBarData), 1, pFile);
	}

	//4. 트리거 박스 저장
	iCount = (int)m_mapTriggerBoxes.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& pair : m_mapTriggerBoxes)
	{
		TriggerBoxData tData = pair.first;
		fwrite(&tData, sizeof(TriggerBoxData), 1, pFile);
	}

	fclose(pFile);
	return S_OK;
}

HRESULT CEditor::LoadStageData(const _tchar* szPath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, szPath, L"rb");
	if (!pFile)
		return E_FAIL;
	//1. 블럭 로드
	CBlockMgr::GetInstance()->LoadBlocks(pFile);
	//2. 몬스터 로드 - 기존 몬스터 해제
	for (auto& pair : m_mapMonsters)
	{
		Safe_Release(pair.second);
	}
	m_mapMonsters.clear();
	
	int iCount = 0;
	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		MonsterData tData;
		fread(&tData, sizeof(MonsterData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
		CMonster* pMonster = CMonster::Create(m_pGraphicDev,
			(EMonsterType)tData.iMonsterType, vPos);
		if (pMonster)
			m_mapMonsters.insert({ tData, pMonster });
	}

	// 3. 창살
	for (auto& pair : m_mapIronBars)
	{
		Safe_Release(pair.second);
	}
	m_mapIronBars.clear();
	
	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		IronBarData tData;
		fread(&tData, sizeof(IronBarData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
		CIronBar* pIronBar = CIronBar::Create(m_pGraphicDev, vPos);
		if (pIronBar)
			m_mapIronBars.insert({ tData, pIronBar });
	}
	// 4. 트리거 박스
	for (auto& pair : m_mapTriggerBoxes)
		Safe_Release(pair.second);
	m_mapTriggerBoxes.clear();

	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		TriggerBoxData tData;
		fread(&tData, sizeof(TriggerBoxData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
		CTriggerBox* pTriggerBox = CTriggerBox::Create(m_pGraphicDev, vPos);
		if (pTriggerBox)
			m_mapTriggerBoxes.insert({ tData, pTriggerBox });
	}

	fclose(pFile);
	return S_OK;
}

CEditor* CEditor::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CEditor* pEditor = new CEditor(pGraphicDev);

	if (FAILED(pEditor->Ready_Scene()))
	{
		MSG_BOX("Editor Create Failed");
		Safe_Release(pEditor);
		return nullptr;
	}

	return pEditor;
}

void CEditor::Free()
{
	Safe_Release(m_pBlockPlacer);
	CScene::Free();
}





HRESULT CEditor::Ready_ProtoType()
{
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TriCol", Engine::CTriCol::Create(m_pGraphicDev))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RcCol", Engine::CRcCol::Create(m_pGraphicDev))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TerrainTex", Engine::CTerrainTex::Create(m_pGraphicDev))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CubeTex", Engine::CCubeTex::Create(m_pGraphicDev))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TerrainTexture",
		Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Terrain/Grass_%d.tga", 2))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SkyBoxTexture",
		Engine::CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/SkyBox/burger%d.dds", 4))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_EffectTexture",
		Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Explosion/Explosion%d.png", 90))))
		return E_FAIL;

	//오징어 해안 로딩 텍스쳐
	//if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SquidCoastLoadingTexture",
	//    Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Loading_Screen_Squid_Coast.png"))))
	//    return E_FAIL;

	//캠프 로딩 텍스쳐
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CampLoadingTexture",
		Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Loading_Screen_Lobby.png"))))
		return E_FAIL;

	//레드 스톤 로딩 텍스쳐
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneLoadingTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Fiery_Forge.png"))))
		return E_FAIL;

	//옵시디언 로딩 텍스쳐
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianLoadingTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Obsidian_Pinnacle.png"))))
		return E_FAIL;

	// 플레이어 텍스쳐
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_PlayerTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/steve_real.png"))))
		return E_FAIL;

	// 닭 텍스쳐
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ChickenTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/chicken.png"))))
		return E_FAIL;

	//블럭 텍스쳐
	//잔디 
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_GrassTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/GrassSideTexture.dds"))))
		return E_FAIL;
	//흙
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_DirtTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/DirtTexture.dds"))))
		return E_FAIL;
	//모래
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SandTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/SandTexture.dds"))))
		return E_FAIL;
	//돌
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RockTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/RockTexture.dds"))))
		return E_FAIL;
	//bedrock
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BedrockTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/BedrockTexture.dds"))))
		return E_FAIL;
	//obsidian
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/ObsidianTexture.dds"))))
		return E_FAIL;
	//stonebrick
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_StoneBrickTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/StoneBrickTexture.dds"))))
		return E_FAIL;

	//블럭 텍스쳐 아틀라스
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BlockAtlasTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/blocks/minecraft_block_atlas_4x4.png"))))
		return E_FAIL;

#pragma region 
	// 좀비 텍스처
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ZombieTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/zombie.png"))))
		return E_FAIL;

	// 좀비 파츠 버퍼
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_Head",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::HEAD))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_Body",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::BODY))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_RArm",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::R_ARM))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_LArm",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::L_ARM))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_RLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::R_LEG))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_LLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::L_LEG))))
		return E_FAIL;
#pragma endregion
#pragma region 스켈레톤
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SkeletonTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/skeleton.png"))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_Head",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::HEAD))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_Body",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::BODY))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_RArm",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::R_ARM))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_LArm",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::L_ARM))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_RLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::R_LEG))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_LLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::L_LEG))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowStandbyTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/bow_standby.png"))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowPullingTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/bow_pulling_0.png"))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ArrowTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/arrow.png"))))
		return E_FAIL;
#pragma endregion

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Transform", Engine::CTransform::Create(m_pGraphicDev))))
		return E_FAIL;

	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Calculator", Engine::CCalculator::Create(m_pGraphicDev))))
		return E_FAIL;

	return 0;
}