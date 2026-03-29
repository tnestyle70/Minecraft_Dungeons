#include "pch.h"
#include "CEditor.h"
#include "CBlockMgr.h"
#include "CDynamicCamera.h"
#include "CMonsterUV.h"
#include "CSceneChanger.h"
#include "CTriggerBox.h"
#include "CBlockPreset.h"

CEditor::CEditor(LPDIRECT3DDEVICE9 pGraphicDev)
	:CScene(pGraphicDev), m_bEditorMode(false)
{
	//m_pGraphicDev->AddRef();
	ZeroMemory(&m_tEditDesc, sizeof(ParticleDesc));
	m_tEditDesc.iMaxParticles = 10;
	m_tEditDesc.fMaxSize = 8.f;
	m_tEditDesc.fMinSize = 4.f;
	m_tEditDesc.fMaxLifeTime = 1.f;
	m_tEditDesc.fMinLifeTime = 0.5f;
	m_tEditDesc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
	m_tEditDesc.fSpreadAngle = D3DX_PI * 0.3f;
	m_tEditDesc.colorStart = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
	m_tEditDesc.colorEnd = D3DXCOLOR(1.f, 1.f, 1.f, 0.f);
	m_tEditDesc.bLoop = true;
	m_tEditDesc.fEmitRate = 30.f;
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

	m_pBlockPlacer = new CBlockPlacer(m_pGraphicDev);

	if (!m_pBlockPlacer)
	{
		MSG_BOX("block placer create failed");
		return E_FAIL;
	}

	D3DXCreateTextureFromFile(m_pGraphicDev,
		L"../Bin/Resource/Texture/Effect/FootPrint_Small.png",
		&m_pPreviewTexture);
	

	return S_OK;
}

_int CEditor::Update_Scene(const _float& fTimeDelta)
{
	if (!m_bEditorMode)
		return 0;

	_int iExit = CScene::Update_Scene(fTimeDelta);

	CBlockMgr::GetInstance()->Update(fTimeDelta);

	for (auto& pair : m_mapTriggerBoxes)
		pair.second->Update_GameObject(fTimeDelta);
	for (auto& pair : m_mapMonsters)
		pair.second->Update_GameObject(fTimeDelta);
	for (auto& pair : m_mapIronBars)
		pair.second->Update_GameObject(fTimeDelta);
	for (auto& pair : m_mapJumpingTraps)
		pair.second->Update_GameObject(fTimeDelta);

	//CParticleMgr::GetInstance()->Update(fTimeDelta);

	switch (m_eEditMode)
	{
	case MODE_BLOCK:
		UpdateBlockMode(fTimeDelta); // 수정 - 블록 모드 통합
		break;
	case MODE_MONSTER:
		UpdateMonsterMode();
		break;
	case MODE_IRONBAR:
		UpdateIronBarMode();
		break;
	case MODE_TRIGGERBOX:
		UpdateTriggerBoxMode();
		break;
	case MODE_JUMPINGTRAP:
		UpdateJumpingTrapMode();
		break;
	case MODE_PARTICLE:
		UpdateParticleMode();
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
		return;

	CBlockMgr::GetInstance()->Render();
	//ParticleMgr::GetInstance()->Render();

	Render_SelectionBox();

	// 추가 - 블록 모드일 때 배치 미리보기
	if (m_eEditMode == MODE_BLOCK && !m_bSelecting)
		m_pBlockPlacer->Render_Preview(m_eSelectedBlock);

	Render_MenuBar();
	Render_Hierarchy();
	Render_Inspector();
	Render_Viewport();
}
//Scene 깡통
void CEditor::Render_UI()
{
}

HRESULT CEditor::Ready_Environment_Layer(const _tchar* pLayerTag)
{
	CLayer* pLayer = CLayer::Create();

	if (!pLayer)
		return E_FAIL;

	CGameObject* pGameObject = nullptr;
	//카메라
	_vec3 vEye{ 0.f, 10.f, -10.f };
	_vec3 vAt{ 0.f, 0.f, 0.f };
	_vec3 vUp{ 0.f, 1.f, 0.f };

	pGameObject = CDynamicCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);
	if (nullptr == pGameObject)
		return E_FAIL;

	if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
		return E_FAIL;
	//드래곤
	m_pDragon = CDragon::Create(m_pGraphicDev);
	if (!m_pDragon)
	{
		MSG_BOX("Dragon Create Failed");
		return E_FAIL;
	}
	pLayer->Add_GameObject(L"Dragon", m_pDragon);
	m_pDragon->Set_MoveTarget(_vec3(0.f, 20.f, 0.f));

	m_mapLayer.insert({ pLayerTag, pLayer });

	//플레이어
	//m_pPlayer = CPlayer::Create(m_pGraphicDev);
	//if (!m_pPlayer)
	//{
	//	MSG_BOX("Player Create Failed");
	//	return E_FAIL;
	//}
	//pLayer->Add_GameObject(L"Player", m_pPlayer);

	//m_mapLayer.insert({ pLayerTag, pLayer });

	return S_OK;
}

void CEditor::SetEditorMode(bool editorMode)
{
	m_bEditorMode = editorMode;

	if (m_bEditorMode)
		LoadStageData(GetStagePath(m_eCurrentStage));
}

const _tchar* CEditor::GetStagePath(eStageType eStage) const
{
	static const _tchar* s_aPath[STAGE_END] =
	{
		L"../Bin/Data/Stage1.dat",
		L"../Bin/Data/Stage2.dat",
		L"../Bin/Data/Stage3.dat",
		L"../Bin/Data/Stage4.dat",
		L"../Bin/Data/Stage5.dat",
		L"../Bin/Data/Stage6.dat",
		L"../Bin/Data/Stage7.dat",
		L"../Bin/Data/Stage8.dat"

	};

	if (eStage < 0 || eStage >= STAGE_END)
		return s_aPath[0];

	return s_aPath[eStage];
}

void CEditor::SwitchStage(eStageType eNewStage)
{
	if (eNewStage == m_eCurrentStage)
		return;

	SaveStageData(GetStagePath(m_eCurrentStage));
	m_eCurrentStage = eNewStage;

	if (FAILED(LoadStageData(GetStagePath(m_eCurrentStage))))
	{
		CBlockMgr::GetInstance()->ClearBlocks();
		for (auto& pair : m_mapMonsters)     Safe_Release(pair.second);
		for (auto& pair : m_mapIronBars)     Safe_Release(pair.second);
		for (auto& pair : m_mapTriggerBoxes) Safe_Release(pair.second);
		for (auto& pair : m_mapJumpingTraps) Safe_Release(pair.second);
		m_mapMonsters.clear();
		m_mapIronBars.clear();
		m_mapTriggerBoxes.clear();
		m_mapJumpingTraps.clear();
	}
}

void CEditor::Render_MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene"))
				CBlockMgr::GetInstance()->ClearBlocks();

			if (ImGui::MenuItem("Save Scene"))
			{
				if (SaveStageData(GetStagePath(m_eCurrentStage)))
					MSG_BOX("Save Failed");
				else
					MSG_BOX("Save Success");
			}

			if (ImGui::MenuItem("Load Scene"))
			{
				if (LoadStageData(GetStagePath(m_eCurrentStage)))
					MSG_BOX("Load Failed");
				else
					MSG_BOX("Load Success");
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Exit Editor")) { m_bEditorMode = false; }
			ImGui::EndMenu();
		}

		static const char* s_aStageNames[STAGE_END] =
		{ "Squid Coast", "Camp", "RedStone", "Obsidian" };
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 150.f);
		ImGui::TextDisabled("Stage : %s", s_aStageNames[m_eCurrentStage]);

		if (ImGui::BeginMenu("Edit"))
			ImGui::EndMenu();

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
	ImGui::Text("JumpingTrap: %d", (int)m_mapJumpingTraps.size());

	ImGui::Separator();

	// 수정 - 법선 기반 커서 위치 HUD
	_vec3 vRayPos, vRayDir;
	m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);
	BlockPos tHitPos, tNormal;
	float fT = 0.f;
	if (CBlockMgr::GetInstance()->RayAABBIntersectWithNormal(vRayPos, vRayDir, &tHitPos, &fT, &tNormal))
		ImGui::Text("Cursor: (%d, %d, %d)",
			tHitPos.x + tNormal.x,
			tHitPos.y + tNormal.y,
			tHitPos.z + tNormal.z);
	else
		ImGui::TextDisabled("Cursor: --");

	// 추가 - 선택 영역 정보
	if (m_bHasSel)
	{
		int iW = abs(m_tSelPos2.x - m_tSelPos1.x) + 1;
		int iH = abs(m_tSelPos2.y - m_tSelPos1.y) + 1;
		int iD = abs(m_tSelPos2.z - m_tSelPos1.z) + 1;
		ImGui::Text("Selection: %dx%dx%d (%d blocks)", iW, iH, iD, iW * iH * iD);
	}

	ImGui::Separator();

	ImGui::Text("Undo: %d / Redo: %d", (int)m_undoStack.size(), (int)m_redoStack.size());

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Dragon Debug"))
	{
		static float fTarget[3] = { 0.f, 8.f, 0.f };
		ImGui::SliderFloat3("MoveTarget", fTarget, -30.f, 30.f);
		if (m_pDragon)
			m_pDragon->Set_MoveTarget(_vec3(fTarget[0], fTarget[1], fTarget[2]));
	}

	ImGui::End();
}

void CEditor::Render_Inspector()
{
	ImGui::Begin("Inspector");

	Render_StageSelector();
	ImGui::Separator();

	const char* szModes[] = { "Block", "Monster", "IronBar", "TriggerBox", "JumpingTrap" ,"Particle"};
	for (int i = 0; i < 6; ++i)
	{
		if (i > 0) ImGui::SameLine();
		bool bActive = (m_eEditMode == (eEditMode)i);
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.f));
		if (ImGui::Button(szModes[i], ImVec2(60.f, 25.f)))
			m_eEditMode = (eEditMode)i;
		if (bActive)
			ImGui::PopStyleColor();
	}

	ImGui::Separator();

	switch (m_eEditMode)
	{
	case MODE_BLOCK:      Render_BlockPalette();   break;
	case MODE_MONSTER:    Render_MonsterPalette();  break;
	case MODE_IRONBAR:    Render_IronBarPalette();  break;
	case MODE_TRIGGERBOX: Render_TriggerPalette();  break;
	case MODE_JUMPINGTRAP: Render_JumpingTrapPalette();  break;
	case MODE_PARTICLE:   Render_ParticleEditor();  break;
	default: break;
	}

	ImGui::End();
}

void CEditor::Render_Viewport()
{
	ImGui::Begin("Viewport");
	ImGui::End();
}

void CEditor::Render_StageSelector()
{
	static const char* s_aIDs[STAGE_END] =
	{ "Squid Coast##stage", "Camp##stage", "RedStone##stage", "Obsidian##stage",
	"JS##stage", "TG##stage","CY##stage","GB##stage" };

	ImGui::Text("Stage");
	for (int i = 0; i < STAGE_END; ++i)
	{
		if (i > 0) ImGui::SameLine();
		bool bActive = (m_eCurrentStage == (eStageType)i);
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.6f, 0.1f, 1.f));
		if (ImGui::Button(s_aIDs[i], ImVec2(85.f, 24.f)))
			if (!bActive) SwitchStage((eStageType)i);
		if (bActive)
			ImGui::PopStyleColor();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("(Stage%d.dat)", (int)m_eCurrentStage + 1);
}

void CEditor::Render_BlockPalette()
{
	struct BlockEntry { const char* label; const char* name; eBlockType eType; };
	static const BlockEntry palette[] =
	{
		{"Grass##block",      "Grass",      BLOCK_GRASS},
		{"Dirt##block",       "Dirt",       BLOCK_DIRT},
		{"Rock##block",       "Rock",       BLOCK_ROCK},
		{"Sand##block",       "Sand",       BLOCK_SAND},
		{"Bedrock##block",    "Bedrock",    BLOCK_BEDROCK},
		{"Obsidian##block",   "Obsidian",   BLOCK_OBSIDIAN},
		{"StoneBrick##block", "StoneBrick", BLOCK_STONEBRICK},
		{"Lava##block",         "Lava",         BLOCK_LAVA},
        {"PlankAcacia##block",  "PlankAcacia",  BLOCK_PLANKS_ACACIA},
        {"PlankSpruce##block",  "PlankSpruce",  BLOCK_PLANKS_SPRUCE},
		{"OakWood##block",  "OakWood",  BLOCK_OAKWOOD},
		{"Redstone##block",  "Redstone",  BLOCK_REDSTONE}
	};
	constexpr int iCount = (int)(sizeof(palette) / sizeof(palette[0]));

	ImGui::Text("Block Palette");
	for (int i = 0; i < iCount; ++i)
	{
		bool bSelected = (m_eSelectedBlock == palette[i].eType);
		if (bSelected)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.f));
		if (ImGui::Button(palette[i].label, ImVec2(120.f, 28.f)))
		{
			m_eSelectedBlock = palette[i].eType;
			m_pBlockPlacer->SetPresetMode(false);
		}
		if (bSelected)
			ImGui::PopStyleColor();
	}

	ImGui::Separator();
	
	enum ePresetType { PRESET_OAK, PRESET_CHERRY, PRESET_DRAGON, PRESET_END };
	struct PresetEntry { const char* label; const char* name; ePresetType eType; };
	static const PresetEntry presets[] =
	{
		{"OakTree##preset",    "Oak Tree",    PRESET_OAK},
		{"CherryTree##preset", "Cherry Tree", PRESET_CHERRY},
		{"Dragon##preset",     "Dragon",      PRESET_DRAGON}
	};
	constexpr int iPCount = (int)(sizeof(presets) / sizeof(presets[0]));

	ImGui::Text("[ Preset ]");
	for (int i = 0; i < iPCount; ++i)
	{
		bool bSelected = (m_eSelectedBlockPreset == presets[i].eType && m_bPresetMode);
		if (bSelected) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.0f, 1.f));
		if (ImGui::Button(presets[i].label, ImVec2(120.f, 28.f)))
		{
			m_eSelectedBlockPreset = presets[i].eType;
			m_bPresetMode = true;
			m_pBlockPlacer->SetPresetMode(true, i);
		}
		if (bSelected) ImGui::PopStyleColor();
	}

	ImGui::Separator();
	if (m_bPresetMode)
		ImGui::Text("Selected: %s", presets[m_eSelectedBlockPreset].name);
	else
		ImGui::Text("Selected: %s", palette[m_eSelectedPreset].name);

	// 선택 영역 조작 UI
	ImGui::Separator();
	ImGui::Text("[ Selection ]");
	ImGui::TextDisabled("Shift+Drag: Select Area");

	if (m_bHasSel)
	{
		if (ImGui::Button("Fill Selection", ImVec2(120.f, 28.f)))
			FillSelection();
		ImGui::SameLine();
		if (ImGui::Button("Erase", ImVec2(60.f, 28.f)))
			EraseSelection();

		if (ImGui::Button("Copy (Ctrl+C)", ImVec2(120.f, 28.f)))
			CopySelection();

		if (!m_vecClipboard.empty())
		{
			ImGui::SameLine();
			if (ImGui::Button("Paste (Ctrl+V)", ImVec2(120.f, 28.f)))
				PasteClipboard();
		}

		if (ImGui::Button("Deselect (ESC)", ImVec2(120.f, 28.f)))
			m_bHasSel = false;
	}

	ImGui::Separator();
	ImGui::TextDisabled("Ctrl+Z: Undo  Ctrl+Y: Redo");

	// 추가 - 범위 직접 입력으로 채우기
	ImGui::Separator();
	ImGui::Text("[ Range Fill ]");
	ImGui::TextDisabled("숫자로 범위 지정 후 Fill");

	// X 범위
	ImGui::SetNextItemWidth(55.f);
	ImGui::InputInt("##MinX", &m_iRangeMinX, 0, 0);
	ImGui::SameLine(); ImGui::Text("~"); ImGui::SameLine();
	ImGui::SetNextItemWidth(55.f);
	ImGui::InputInt("X##MaxX", &m_iRangeMaxX, 0, 0);

	// Y 범위
	ImGui::SetNextItemWidth(55.f);
	ImGui::InputInt("##MinY", &m_iRangeMinY, 0, 0);
	ImGui::SameLine(); ImGui::Text("~"); ImGui::SameLine();
	ImGui::SetNextItemWidth(55.f);
	ImGui::InputInt("Y##MaxY", &m_iRangeMaxY, 0, 0);

	// Z 범위
	ImGui::SetNextItemWidth(55.f);
	ImGui::InputInt("##MinZ", &m_iRangeMinZ, 0, 0);
	ImGui::SameLine(); ImGui::Text("~"); ImGui::SameLine();
	ImGui::SetNextItemWidth(55.f);
	ImGui::InputInt("Z##MaxZ", &m_iRangeMaxZ, 0, 0);

	// 크기 미리보기
	int iW = abs(m_iRangeMaxX - m_iRangeMinX) + 1;
	int iH = abs(m_iRangeMaxY - m_iRangeMinY) + 1;
	int iD = abs(m_iRangeMaxZ - m_iRangeMinZ) + 1;
	ImGui::TextDisabled("Size: %dx%dx%d (%d blocks)", iW, iH, iD, iW * iH * iD);

	if (ImGui::Button("Range Fill", ImVec2(120.f, 28.f)))
	{
		BlockPos tMin = { min(m_iRangeMinX, m_iRangeMaxX),
						  min(m_iRangeMinY, m_iRangeMaxY),
						  min(m_iRangeMinZ, m_iRangeMaxZ) };
		BlockPos tMax = { max(m_iRangeMinX, m_iRangeMaxX),
						  max(m_iRangeMinY, m_iRangeMaxY),
						  max(m_iRangeMinZ, m_iRangeMaxZ) };

		EditorAction tAction;
		tAction.vecRemoved = CBlockMgr::GetInstance()->GetBlocksInRange(tMin, tMax);
		CBlockMgr::GetInstance()->EraseBlocks(tMin, tMax);
		CBlockMgr::GetInstance()->FillBlocks(tMin, tMax, m_eSelectedBlock);
		tAction.vecAdded = CBlockMgr::GetInstance()->GetBlocksInRange(tMin, tMax);
		Push_UndoAction(tAction);
	}
	ImGui::SameLine();
	if (ImGui::Button("Range Erase", ImVec2(100.f, 28.f)))
	{
		BlockPos tMin = { min(m_iRangeMinX, m_iRangeMaxX),
						  min(m_iRangeMinY, m_iRangeMaxY),
						  min(m_iRangeMinZ, m_iRangeMaxZ) };
		BlockPos tMax = { max(m_iRangeMinX, m_iRangeMaxX),
						  max(m_iRangeMinY, m_iRangeMaxY),
						  max(m_iRangeMinZ, m_iRangeMaxZ) };

		EditorAction tAction;
		tAction.vecRemoved = CBlockMgr::GetInstance()->GetBlocksInRange(tMin, tMax);
		CBlockMgr::GetInstance()->EraseBlocks(tMin, tMax);
		Push_UndoAction(tAction);
	}

	// 커서 위치를 Min으로 가져오기
	if (ImGui::Button("Get Cursor -> Min", ImVec2(130.f, 28.f)))
	{
		_vec3 vRayPos, vRayDir;
		m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);
		BlockPos tHitPos;
		float fT = 0.f;
		if (CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos, vRayDir, &tHitPos, &fT))
		{
			m_iRangeMinX = tHitPos.x;
			m_iRangeMinY = tHitPos.y + 1;
			m_iRangeMinZ = tHitPos.z;
		}
	}
	ImGui::SameLine();
	// 커서 위치를 Max로 가져오기
	if (ImGui::Button("Get Cursor -> Max", ImVec2(130.f, 28.f)))
	{
		_vec3 vRayPos, vRayDir;
		m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);
		BlockPos tHitPos;
		float fT = 0.f;
		if (CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos, vRayDir, &tHitPos, &fT))
		{
			m_iRangeMaxX = tHitPos.x;
			m_iRangeMaxY = tHitPos.y + 1;
			m_iRangeMaxZ = tHitPos.z;
		}
	}
	ImGui::TextDisabled("블록 클릭으로 Min/Max 좌표 자동 입력");
}

void CEditor::Render_MonsterPalette()
{
	ImGui::Text("Monster Spawn");
	ImGui::Separator();
	ImGui::Text("TriggerID: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.f);
	ImGui::InputInt("##MonsterTriggerID", &m_iMonsterTriggerID, 0, 0);
	ImGui::SameLine();
	ImGui::TextDisabled("(-1 = 즉시스폰)");

	ImGui::Separator();
	const char* szMonsters[] = { "Zombie", "Skeleton", "Creeper", "Spider" };
	ImGui::Text("Type:");
	for (int i = 0; i < 4; ++i)
	{
		if (i > 0) ImGui::SameLine();
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

	for (auto& pair : m_mapMonsters)
	{
		const MonsterData& tData = pair.first;
		ImGui::Text("[%s] (%d %d %d)", szMonsters[tData.iMonsterType],
			tData.x, tData.y, tData.z);
		ImGui::SameLine();

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
			break;
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
	ImGui::Text("TriggerID:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.f);
	ImGui::InputInt("##IronBarTriggerID", &m_iCurTriggerID, 0, 0);
	if (m_iCurTriggerID < 0) m_iCurTriggerID = 0;
	ImGui::SameLine();
	if (ImGui::Button("Auto##ib")) m_iCurTriggerID++;

	ImGui::Separator();
	ImGui::Text("Placed: %d", (int)m_mapIronBars.size());
	ImGui::Separator();

	using Pair = pair<const IronBarData*, CIronBar*>;
	vector<Pair> vecSorted;
	vecSorted.reserve(m_mapIronBars.size());
	for (auto& pair : m_mapIronBars)
		vecSorted.push_back({ &pair.first, pair.second });

	sort(vecSorted.begin(), vecSorted.end(), [](const Pair& a, const Pair& b)
		{
			if (a.first->iTriggerID != b.first->iTriggerID)
				return a.first->iTriggerID < b.first->iTriggerID;
			if (a.first->x != b.first->x) return a.first->x < b.first->x;
			if (a.first->y != b.first->y) return a.first->y < b.first->y;
			return a.first->z < b.first->z;
		});

	IronBarData* pPendingDelete = nullptr;
	for (auto& entry : vecSorted)
	{
		const IronBarData& tData = *entry.first;
		ImGui::Text("(%d, %d, %d)", tData.x, tData.y, tData.z);
		ImGui::SameLine();
		ImGui::Text("ID:");
		ImGui::SameLine();

		char szID[32];
		sprintf_s(szID, "##ibid_%d_%d_%d", tData.x, tData.y, tData.z);
		ImGui::SetNextItemWidth(40.f);
		int iID = tData.iTriggerID;
		if (ImGui::InputInt(szID, &iID, 0, 0))
		{
			IronBarData tNew = tData;
			tNew.iTriggerID = iID;
			auto pObj = entry.second;
			m_mapIronBars.erase(tData);
			m_mapIronBars.insert({ tNew, pObj });
			break;
		}
		ImGui::SameLine();
		char szBtn[32];
		sprintf_s(szBtn, "X##ib_%d_%d_%d", tData.x, tData.y, tData.z);
		if (ImGui::Button(szBtn))
			pPendingDelete = const_cast<IronBarData*>(&tData);
	}

	if (pPendingDelete)
	{
		auto iter = m_mapIronBars.find(*pPendingDelete);
		if (iter != m_mapIronBars.end())
		{
			Safe_Release(iter->second);
			m_mapIronBars.erase(iter);
		}
	}
}

void CEditor::Render_TriggerPalette()
{
	ImGui::Text("TriggerBox Placement");
	ImGui::Separator();
	const char* szTypes[] = { "IronBar", "Monster", "SceneChange", "JumpingTrap"};
	ImGui::Text("Type:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::Combo("##TriggerType", &m_iTriggerType, szTypes, IM_ARRAYSIZE(szTypes));
	ImGui::Text("TriggerID:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.f);
	ImGui::InputInt("##TriggerID", &m_iCurTriggerID), 0, 0;
	ImGui::SameLine();
	if (ImGui::Button("Auto")) m_iCurTriggerID++;

	ImGui::Separator();
	ImGui::Text("Placed: %d", (int)m_mapTriggerBoxes.size());

	TriggerBoxData* pPendingDelete = nullptr;
	for (auto& pair : m_mapTriggerBoxes)
	{
		TriggerBoxData& tData = const_cast<TriggerBoxData&>(pair.first);
		const char* szTypes2[] = { "IronBar", "Monster", "SceneChange", "JumpingTrap"};
		char szCombo[32];
		sprintf_s(szCombo, "##type_%d_%d_%d", tData.x, tData.y, tData.z);
		ImGui::SetNextItemWidth(90.f);
		int iType = tData.iTriggerBoxType;
		if (ImGui::Combo(szCombo, &iType, szTypes2, IM_ARRAYSIZE(szTypes2)))
		{
			TriggerBoxData tNew = tData;
			tNew.iTriggerBoxType = iType;
			auto pObj = pair.second;
			m_mapTriggerBoxes.erase(pair.first);
			m_mapTriggerBoxes.insert({ tNew, pObj });
			break;
		}
		ImGui::SameLine();
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

void CEditor::Render_JumpingTrapPalette()
{
	ImGui::Text("JumpingTrap Placement");
	ImGui::Separator();
	ImGui::Text("TriggerID:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.f);
	ImGui::InputInt("##JTrapTriggerID", &m_iCurTriggerID, 0, 0);
	if (m_iCurTriggerID < 0) m_iCurTriggerID = 0;
	ImGui::SameLine();
	if (ImGui::Button("Auto##jt")) m_iCurTriggerID++;

	ImGui::Separator();
	ImGui::Text("Placed: %d", (int)m_mapJumpingTraps.size());
	ImGui::Separator();

	// TriggerID → 좌표 순으로 정렬해서 표시
	using Pair = pair<const JumpingTrapData*, CJumpingTrap*>;
	vector<Pair> vecSorted;
	vecSorted.reserve(m_mapJumpingTraps.size());
	for (auto& pair : m_mapJumpingTraps)
		vecSorted.push_back({ &pair.first, pair.second });

	sort(vecSorted.begin(), vecSorted.end(), [](const Pair& a, const Pair& b)
		{
			if (a.first->iTriggerID != b.first->iTriggerID)
				return a.first->iTriggerID < b.first->iTriggerID;
			if (a.first->x != b.first->x) return a.first->x < b.first->x;
			if (a.first->y != b.first->y) return a.first->y < b.first->y;
			return a.first->z < b.first->z;
		});

	// 삭제 대상을 값으로 보관 (포인터 dangling 방지)
	bool bPendingDelete = false;
	JumpingTrapData tPendingDeleteData{};

	for (auto& entry : vecSorted)
	{
		const JumpingTrapData& tData = *entry.first;

		ImGui::Text("(%d, %d, %d)", tData.x, tData.y, tData.z);
		ImGui::SameLine();
		ImGui::Text("ID:");
		ImGui::SameLine();

		char szID[64];
		sprintf_s(szID, "##jtid_%d_%d_%d", tData.x, tData.y, tData.z);
		ImGui::SetNextItemWidth(40.f);
		int iID = tData.iTriggerID;
		if (ImGui::InputInt(szID, &iID, 0, 0))
		{
			JumpingTrapData tNew = tData;
			tNew.iTriggerID = iID;
			auto pObj = entry.second;
			m_mapJumpingTraps.erase(tData);
			m_mapJumpingTraps.insert({ tNew, pObj });
			break; // map 변경 후 vecSorted 포인터 무효화 → 즉시 break
		}

		ImGui::SameLine();
		char szBtn[64];
		sprintf_s(szBtn, "X##jt_%d_%d_%d", tData.x, tData.y, tData.z);
		if (ImGui::Button(szBtn))
		{
			// 포인터 대신 값을 복사해서 보관 → vecSorted 해제 후에도 안전
			bPendingDelete = true;
			tPendingDeleteData = tData;
		}
	}

	if (bPendingDelete)
	{
		auto iter = m_mapJumpingTraps.find(tPendingDeleteData);
		if (iter != m_mapJumpingTraps.end())
		{
			Safe_Release(iter->second);
			m_mapJumpingTraps.erase(iter);
		}
	}
}

void CEditor::Render_ParticleEditor()
{
	ImGui::Begin("Particle Editor");
	const char* szPathNames[] = { "FOOTSTEP", "HIT", "FIREWORK", "DYNAMITE", "BOSSATTACK" };
	if (ImGui::Combo("Preset", (int*)&m_eSelectedPreset, szPathNames, IM_ARRAYSIZE(szPathNames)))
		Load_ParticlePreset(m_eSelectedPreset, m_tEditDesc);

	ImGui::Separator();
	ImGui::BeginDisabled(true);
	ImGui::DragFloat3("EmitPos (picking only)", (float*)&m_tEditDesc.vEmitPos, 0.1f);
	ImGui::EndDisabled();
	ImGui::DragFloat3("EmitDir", (float*)&m_tEditDesc.vEmitDir, 0.01f, -1.f, 1.f);
	ImGui::SliderFloat("SpreadAngle", &m_tEditDesc.fSpreadAngle, 0.f, D3DX_PI);
	ImGui::Separator();
	ImGui::DragFloat2("Speed  (min/max)", &m_tEditDesc.fMinSpeed, 0.1f, 0.f, 30.f);
	ImGui::DragFloat2("Life   (min/max)", &m_tEditDesc.fMinLifeTime, 0.05f, 0.f, 5.f);
	ImGui::DragFloat2("Size   (min/max)", &m_tEditDesc.fMinSize, 0.5f, 0.f, 128.f);
	ImGui::Separator();
	ImGui::ColorEdit4("ColorStart", (float*)&m_tEditDesc.colorStart);
	ImGui::ColorEdit4("ColorEnd", (float*)&m_tEditDesc.colorEnd);
	ImGui::Checkbox("UseTextureAsIs", (bool*)&m_tEditDesc.bUseTextureAsIs);
	ImGui::Separator();
	ImGui::DragInt("MaxParticles", &m_tEditDesc.iMaxParticles, 1, 1, 500);
	ImGui::DragFloat("EmitRate", &m_tEditDesc.fEmitRate, 1.f, 0.f, 200.f);
	ImGui::Checkbox("Loop", (bool*)&m_tEditDesc.bLoop);
	ImGui::DragFloat("Gravity", &m_tEditDesc.fGravity, 0.1f, 0.f, 20.f);
	ImGui::Separator();
	if (ImGui::Button("Spawn Preview"))
		Respawn_PreviewParticle();
	ImGui::SameLine();
	ImGui::Text("Emitters: %d", CParticleMgr::GetInstance()->Get_EmitterCount());
	ImGui::End();
}

void CEditor::Respawn_PreviewParticle()
{
	CParticleMgr::GetInstance()->Clear_Emitters();
}

// 추가 - 블록 모드 통합 업데이트
void CEditor::UpdateBlockMode(const _float& fTimeDelta)
{
	bool bShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
	bool bCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
	bool bEsc = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;

	// ESC - 선택 해제
	if (bEsc && m_bHasSel)
		m_bHasSel = false;

	// Ctrl+Z Undo / Ctrl+Y Redo
	bool bZKey = (GetAsyncKeyState('Z') & 0x8000) != 0;
	bool bYKey = (GetAsyncKeyState('Y') & 0x8000) != 0;
	if (bCtrl && bZKey && !m_bZKeyPrev) Undo();
	if (bCtrl && bYKey && !m_bYKeyPrev) Redo();
	m_bZKeyPrev = bZKey;
	m_bYKeyPrev = bYKey;

	if (bShift)
	{
		// Shift 누름 → 드래그 선택 모드
		UpdateSelectionMode();
	}
	else
	{
		// 기존 블록 배치 모드
		m_pBlockPlacer->Update_Placer(m_eSelectedBlock);
		//CBlockMgr::GetInstance()->Update(fTimeDelta);
	}

	// 선택 영역이 있을 때 단축키 처리
	if (m_bHasSel)
	{
		bool bCKey = (GetAsyncKeyState('C') & 0x8000) != 0;
		bool bVKey = (GetAsyncKeyState('V') & 0x8000) != 0;
		bool bDel = (GetAsyncKeyState(VK_DELETE) & 0x8000) != 0;

		if (bCtrl && bCKey && !m_bCKeyPrev) CopySelection();
		if (bCtrl && bVKey && !m_bVKeyPrev) PasteClipboard();
		if (bDel && !m_bDelKeyPrev)         EraseSelection();

		m_bCKeyPrev = bCKey;
		m_bVKeyPrev = bVKey;
		m_bDelKeyPrev = bDel;
	}
}

// 추가 - Shift 드래그 선택
void CEditor::UpdateSelectionMode()
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	bool bLBtn = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	_vec3 vRayPos, vRayDir;
	m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);

	BlockPos tCurPos;
	float fT = 0.f;
	bool bHit = CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos, vRayDir, &tCurPos, &fT);

	if (!bHit)
	{
		_vec3 vGroundHit;
		if (m_pBlockPlacer->RayOnGround(&vRayPos, &vRayDir, &vGroundHit))
		{
			tCurPos = CBlockMgr::GetInstance()->ToPos(vGroundHit);
			bHit = true;
		}
	}

	if (bHit)
	{
		// 드래그 시작
		if (bLBtn && !m_bLBtnPrev)
		{
			m_tSelPos1 = tCurPos;
			m_tSelPos2 = tCurPos;
			m_bSelecting = true;
			m_bHasSel = false;
		}

		// 드래그 중 - 끝점 실시간 갱신
		if (bLBtn && m_bSelecting)
			m_tSelPos2 = tCurPos;

		// 드래그 끝 - 선택 완료
		if (!bLBtn && m_bLBtnPrev && m_bSelecting)
		{
			m_tSelPos2 = tCurPos;
			m_bSelecting = false;
			m_bHasSel = true;
		}
	}

	m_bLBtnPrev = bLBtn;
}

// 추가 - 선택 영역 와이어프레임 렌더링
void CEditor::Render_SelectionBox()
{
	if (!m_bHasSel && !m_bSelecting) return;

	float fMinX = (float)min(m_tSelPos1.x, m_tSelPos2.x) - 0.5f;
	float fMinY = (float)min(m_tSelPos1.y, m_tSelPos2.y) - 0.5f;
	float fMinZ = (float)min(m_tSelPos1.z, m_tSelPos2.z) - 0.5f;
	float fMaxX = (float)max(m_tSelPos1.x, m_tSelPos2.x) + 0.5f;
	float fMaxY = (float)max(m_tSelPos1.y, m_tSelPos2.y) + 0.5f;
	float fMaxZ = (float)max(m_tSelPos1.z, m_tSelPos2.z) + 0.5f;

	DWORD dwColor = m_bSelecting ? 0xFFFFFF00 : 0xFF00FF00;

	struct LineVertex { float x, y, z; DWORD color; };

	LineVertex verts[24] =
	{
		{fMinX, fMinY, fMinZ, dwColor}, {fMaxX, fMinY, fMinZ, dwColor},
		{fMaxX, fMinY, fMinZ, dwColor}, {fMaxX, fMinY, fMaxZ, dwColor},
		{fMaxX, fMinY, fMaxZ, dwColor}, {fMinX, fMinY, fMaxZ, dwColor},
		{fMinX, fMinY, fMaxZ, dwColor}, {fMinX, fMinY, fMinZ, dwColor},
		{fMinX, fMaxY, fMinZ, dwColor}, {fMaxX, fMaxY, fMinZ, dwColor},
		{fMaxX, fMaxY, fMinZ, dwColor}, {fMaxX, fMaxY, fMaxZ, dwColor},
		{fMaxX, fMaxY, fMaxZ, dwColor}, {fMinX, fMaxY, fMaxZ, dwColor},
		{fMinX, fMaxY, fMaxZ, dwColor}, {fMinX, fMaxY, fMinZ, dwColor},
		{fMinX, fMinY, fMinZ, dwColor}, {fMinX, fMaxY, fMinZ, dwColor},
		{fMaxX, fMinY, fMinZ, dwColor}, {fMaxX, fMaxY, fMinZ, dwColor},
		{fMaxX, fMinY, fMaxZ, dwColor}, {fMaxX, fMaxY, fMaxZ, dwColor},
		{fMinX, fMinY, fMaxZ, dwColor}, {fMinX, fMaxY, fMaxZ, dwColor},
	};

	// 현재 렌더 상태 저장 // 수정
	DWORD dwLighting, dwZWrite;
	m_pGraphicDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);   // 추가
	m_pGraphicDev->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWrite); // 추가

	_matrix matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matIdentity);
	m_pGraphicDev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	m_pGraphicDev->SetTexture(0, nullptr);
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE); // 추가
	m_pGraphicDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, verts, sizeof(LineVertex));

	// 저장된 상태로 복구 // 수정
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, dwLighting);    // 수정
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, dwZWrite);  // 추가
}

// 추가 - 선택 영역 복사
void CEditor::CopySelection()
{
	m_vecClipboard.clear();

	vector<BlockData> vecBlocks = CBlockMgr::GetInstance()->GetBlocksInRange(m_tSelPos1, m_tSelPos2);
	if (vecBlocks.empty()) return;

	int iOriginX = min(m_tSelPos1.x, m_tSelPos2.x);
	int iOriginY = min(m_tSelPos1.y, m_tSelPos2.y);
	int iOriginZ = min(m_tSelPos1.z, m_tSelPos2.z);

	for (auto& tData : vecBlocks)
	{
		ClipboardBlock cb;
		cb.rx = tData.x - iOriginX; // 기준점 상대 좌표
		cb.ry = tData.y - iOriginY;
		cb.rz = tData.z - iOriginZ;
		cb.eType = (eBlockType)tData.eType;
		m_vecClipboard.push_back(cb);
	}
}

// 추가 - 마우스 위치에 붙여넣기
void CEditor::PasteClipboard()
{
	if (m_vecClipboard.empty()) return;

	_vec3 vRayPos, vRayDir;
	m_pBlockPlacer->Compute_Ray(&vRayPos, &vRayDir);

	BlockPos tPasteBase, tNormal;
	float fT = 0.f;

	// 수정 - 법선 기반으로 붙여넣기 위치 계산
	bool bHit = CBlockMgr::GetInstance()->RayAABBIntersectWithNormal(
		vRayPos, vRayDir, &tPasteBase, &fT, &tNormal);

	if (bHit)
	{
		// 법선 방향으로 붙여넣기
		tPasteBase.x += tNormal.x;
		tPasteBase.y += tNormal.y;
		tPasteBase.z += tNormal.z;
	}
	else
	{
		_vec3 vGroundHit;
		if (!m_pBlockPlacer->RayOnGround(&vRayPos, &vRayDir, &vGroundHit)) return;
		tPasteBase = CBlockMgr::GetInstance()->ToPos(vGroundHit);
	}

	EditorAction tAction;

	for (auto& cb : m_vecClipboard)
	{
		int x = tPasteBase.x + cb.rx;
		int y = tPasteBase.y + cb.ry;
		int z = tPasteBase.z + cb.rz;

		if (!CBlockMgr::GetInstance()->HasBlock({ x, y, z }))
		{
			CBlockMgr::GetInstance()->AddBlock(x, y, z, cb.eType);
			BlockData tData;
			tData.x = x; tData.y = y; tData.z = z;
			tData.eType = (int)cb.eType;
			tAction.vecAdded.push_back(tData);
		}
	}

	if (!tAction.vecAdded.empty())
		Push_UndoAction(tAction);
}

// 추가 - 선택 영역 지우기
void CEditor::EraseSelection()
{
	vector<BlockData> vecBlocks = CBlockMgr::GetInstance()->GetBlocksInRange(m_tSelPos1, m_tSelPos2);
	if (vecBlocks.empty()) return;

	EditorAction tAction;
	tAction.vecRemoved = vecBlocks; // Undo 시 복원할 블록들

	CBlockMgr::GetInstance()->EraseBlocks(m_tSelPos1, m_tSelPos2);
	Push_UndoAction(tAction);

	m_bHasSel = false;
}

// 추가 - 선택 영역 채우기
void CEditor::FillSelection()
{
	if (!m_bHasSel) return;

	EditorAction tAction;

	// Undo를 위해 기존 블록 기록
	tAction.vecRemoved = CBlockMgr::GetInstance()->GetBlocksInRange(m_tSelPos1, m_tSelPos2);

	// 지우고 채우기
	CBlockMgr::GetInstance()->EraseBlocks(m_tSelPos1, m_tSelPos2);
	CBlockMgr::GetInstance()->FillBlocks(m_tSelPos1, m_tSelPos2, m_eSelectedBlock);

	// 추가된 블록 기록
	tAction.vecAdded = CBlockMgr::GetInstance()->GetBlocksInRange(m_tSelPos1, m_tSelPos2);

	Push_UndoAction(tAction);
}

// 추가 - Undo 스택에 작업 저장
void CEditor::Push_UndoAction(const EditorAction& tAction)
{
	m_undoStack.push_back(tAction);
	if ((int)m_undoStack.size() > MAX_UNDO)
		m_undoStack.pop_front(); // 가장 오래된 것 제거

	m_redoStack.clear(); // 새 작업 시 Redo 스택 초기화
}

// 추가 - Undo (Ctrl+Z)
void CEditor::Undo()
{
	if (m_undoStack.empty()) return;

	EditorAction tAction = m_undoStack.back();
	m_undoStack.pop_back();

	// 추가됐던 블록 제거
	for (auto& tData : tAction.vecAdded)
		CBlockMgr::GetInstance()->RemoveBlockByPos({ tData.x, tData.y, tData.z });

	// 제거됐던 블록 복원
	for (auto& tData : tAction.vecRemoved)
		CBlockMgr::GetInstance()->AddBlock(tData.x, tData.y, tData.z, (eBlockType)tData.eType);

	m_redoStack.push_back(tAction);
}

// 추가 - Redo (Ctrl+Y)
void CEditor::Redo()
{
	if (m_redoStack.empty()) return;

	EditorAction tAction = m_redoStack.back();
	m_redoStack.pop_back();

	// 제거됐던 것 다시 제거
	for (auto& tData : tAction.vecRemoved)
		CBlockMgr::GetInstance()->RemoveBlockByPos({ tData.x, tData.y, tData.z });

	// 추가됐던 것 다시 추가
	for (auto& tData : tAction.vecAdded)
		CBlockMgr::GetInstance()->AddBlock(tData.x, tData.y, tData.z, (eBlockType)tData.eType);

	m_undoStack.push_back(tAction);
}

void CEditor::UpdateMonsterMode()
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
			MonsterData tData;
			tData.x = tHitPos.x;
			tData.y = tHitPos.y + 1;
			tData.z = tHitPos.z;
			tData.iMonsterType = m_iSelectedMonster;
			tData.iTriggerID = m_iMonsterTriggerID;

			_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
			CMonster* pMonster = CMonster::Create(m_pGraphicDev, (EMonsterType)m_iSelectedMonster, vPos);
			pMonster->SetActive(true);
			m_mapMonsters.insert({ tData, pMonster });
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
			tData.iTriggerID = m_iCurTriggerID;

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
				if (tData.x == tHitPos.x && tData.y == tHitPos.y + 1 && tData.z == tHitPos.z)
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

			_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
			CTriggerBox* pTriggerBox = CTriggerBox::Create(m_pGraphicDev, vPos, eTriggerBoxType(m_iTriggerType));
			if (pTriggerBox)
				m_mapTriggerBoxes.insert({ tData, pTriggerBox });
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

void CEditor::UpdateJumpingTrapMode()
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
			JumpingTrapData tData = {};
			tData.x = tHitPos.x;
			tData.y = tHitPos.y + 1;
			tData.z = tHitPos.z;
			tData.iTriggerID = m_iCurTriggerID;

			if (m_mapJumpingTraps.find(tData) == m_mapJumpingTraps.end())
			{
				_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
				CJumpingTrap* pJumpingTrap = CJumpingTrap::Create(m_pGraphicDev, vPos);
				if (pJumpingTrap)
					m_mapJumpingTraps.insert({ tData, pJumpingTrap });
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
			for (auto iter = m_mapJumpingTraps.begin(); iter != m_mapJumpingTraps.end(); ++iter)
			{
				const JumpingTrapData& tData = iter->first;
				if (tData.x == tHitPos.x && tData.y == tHitPos.y + 1 && tData.z == tHitPos.z)
				{
					Safe_Release(iter->second);
					m_mapJumpingTraps.erase(iter);
					break;
				}
			}
		}
	}

	m_bLBtnPrev = bLBtn;
	m_bRBtnPrev = bRBtn;
}

void CEditor::UpdateParticleMode()
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
			_vec3 vPos = { (float)tHitPos.x, (float)tHitPos.y + 1.f, (float)tHitPos.z };
			ParticleDesc tDescriptor = m_tEditDesc;
			tDescriptor.vEmitPos = vPos;
			CParticleEmitter* pEmitter = CParticleEmitter::Create(
				m_pGraphicDev, m_eSelectedPreset, tDescriptor, m_pPreviewTexture);
			if (pEmitter)
				CParticleMgr::GetInstance()->Add_Emitter(pEmitter);
		}
	}

	if (bRBtn && !m_bRBtnPrev)
		CParticleMgr::GetInstance()->Clear_Emitters();

	m_bLBtnPrev = bLBtn;
	m_bRBtnPrev = bRBtn;
}

void CEditor::Load_ParticlePreset(eParticlePreset eType, ParticleDesc& outDesc)
{
	_vec3 vSavedPos = outDesc.vEmitPos;
	ParticleDesc desc;
	ZeroMemory(&desc, sizeof(ParticleDesc));
	desc.vEmitPos = vSavedPos;

	switch (eType)
	{
	case PARTICLE_FOOTSTEP:
		desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
		desc.fSpreadAngle = D3DX_PI * 0.4f;
		desc.fMinSpeed = 0.1f;  desc.fMaxSpeed = 0.3f;
		desc.fMinLifeTime = 0.5f; desc.fMaxLifeTime = 0.8f;
		desc.fMinSize = 0.1f;   desc.fMaxSize = 2.f;
		desc.colorStart = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
		desc.colorEnd = D3DXCOLOR(1.f, 1.f, 1.f, 0.f);
		desc.bUseTextureAsIs = true;
		desc.iMaxParticles = 10;
		desc.fEmitRate = 20.f;
		desc.bLoop = false;
		desc.fGravity = 4.f;
		break;
	case PARTICLE_HIT:
		desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
		desc.fSpreadAngle = D3DX_PI;
		desc.fMinSpeed = 2.f;   desc.fMaxSpeed = 7.f;
		desc.fMinLifeTime = 0.1f; desc.fMaxLifeTime = 3.4f;
		desc.fMinSize = 1.f;    desc.fMaxSize = 3.f;
		desc.colorStart = D3DXCOLOR(1.f, 1.f, 0.6f, 1.f);
		desc.colorEnd = D3DXCOLOR(1.f, 0.7f, 0.f, 0.f);
		desc.iMaxParticles = 300;
		desc.fEmitRate = 0.f;
		desc.bLoop = true;
		desc.fGravity = 6.f;
		break;
	case PARTICLE_FIREWORK:
		desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
		desc.fSpreadAngle = D3DX_PI;
		desc.fMinSpeed = 3.f;   desc.fMaxSpeed = 7.f;
		desc.fMinLifeTime = 0.5f; desc.fMaxLifeTime = 2.2f;
		desc.fMinSize = 2.f;    desc.fMaxSize = 6.f;
		desc.colorStart = D3DXCOLOR(1.f, 0.8f, 0.f, 1.f);
		desc.colorEnd = D3DXCOLOR(1.f, 0.2f, 0.f, 0.f);
		desc.iMaxParticles = 300;
		desc.fEmitRate = 0.f;
		desc.bLoop = true;
		desc.fGravity = 3.f;
		break;
	case PARTICLE_BOSS_ATTACK:
		desc.vEmitDir = D3DXVECTOR3(0.f, 1.f, 0.f);
		desc.fSpreadAngle = D3DX_PI * 0.6f;
		desc.fMinSpeed = 1.f;   desc.fMaxSpeed = 3.f;
		desc.fMinLifeTime = 0.3f; desc.fMaxLifeTime = 0.8f;
		desc.fMinSize = 0.15f;  desc.fMaxSize = 0.35f;
		desc.colorStart = D3DXCOLOR(0.5f, 0.f, 1.f, 1.f);
		desc.colorEnd = D3DXCOLOR(0.2f, 0.f, 0.5f, 0.f);
		desc.iMaxParticles = 80;
		desc.fEmitRate = 30.f;
		desc.bLoop = true;
		desc.fGravity = 0.f;
		break;
	default:
		break;
	}
	outDesc = desc;
}

HRESULT CEditor::SaveStageData(const _tchar* szPath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, szPath, L"wb");
	if (!pFile) return E_FAIL;

	CBlockMgr::GetInstance()->SaveBlocks(pFile);

	int iCount = (int)m_mapMonsters.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& pair : m_mapMonsters)
	{
		MonsterData tData = pair.first;
		fwrite(&tData, sizeof(MonsterData), 1, pFile);
	}

	iCount = (int)m_mapIronBars.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& pair : m_mapIronBars)
	{
		IronBarData tData = pair.first;
		fwrite(&tData, sizeof(IronBarData), 1, pFile);
	}

	iCount = (int)m_mapTriggerBoxes.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& pair : m_mapTriggerBoxes)
	{
		TriggerBoxData tData = pair.first;
		fwrite(&tData, sizeof(TriggerBoxData), 1, pFile);
	}

	iCount = (int)m_mapJumpingTraps.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& pair : m_mapJumpingTraps)
	{
		JumpingTrapData tData = pair.first;
		fwrite(&tData, sizeof(JumpingTrapData), 1, pFile);
	}

	fclose(pFile);
	return S_OK;
}

HRESULT CEditor::LoadStageData(const _tchar* szPath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, szPath, L"rb");
	if (!pFile) return E_FAIL;
	//블럭
	CBlockMgr::GetInstance()->LoadBlocks(pFile);
	//몬스터
	for (auto& pair : m_mapMonsters)
		Safe_Release(pair.second);
	m_mapMonsters.clear();

	int iCount = 0;
	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		MonsterData tData{};
		fread(&tData, sizeof(MonsterData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
		CMonster* pMonster = CMonster::Create(m_pGraphicDev, (EMonsterType)tData.iMonsterType, vPos);
		pMonster->SetActive(true);
		if (pMonster)
			m_mapMonsters.insert({ tData, pMonster });
	}
	//아이언바
	for (auto& pair : m_mapIronBars)
		Safe_Release(pair.second);
	m_mapIronBars.clear();

	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		IronBarData tData{};
		fread(&tData, sizeof(IronBarData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
		CIronBar* pIronBar = CIronBar::Create(m_pGraphicDev, vPos);
		if (pIronBar)
			m_mapIronBars.insert({ tData, pIronBar });
	}
	//트리거 박스
	for (auto& pair : m_mapTriggerBoxes)
		Safe_Release(pair.second);
	m_mapTriggerBoxes.clear();

	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		TriggerBoxData tData{};
		fread(&tData, sizeof(TriggerBoxData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
		CTriggerBox* pTriggerBox = CTriggerBox::Create(m_pGraphicDev, vPos);
		if (pTriggerBox)
			m_mapTriggerBoxes.insert({ tData, pTriggerBox });
	}
	//점핑 트랩
	for (auto& pair : m_mapJumpingTraps)
		Safe_Release(pair.second);
	m_mapJumpingTraps.clear();

	fread(&iCount, sizeof(int), 1, pFile);
	for (int i = 0; i < iCount; ++i)
	{
		JumpingTrapData tData{};
		fread(&tData, sizeof(JumpingTrapData), 1, pFile);
		_vec3 vPos = { (float)tData.x, (float)tData.y, (float)tData.z };
		CJumpingTrap* pJumpingTrap = CJumpingTrap::Create(m_pGraphicDev, vPos);
		if (pJumpingTrap)
			m_mapJumpingTraps.insert({ tData, pJumpingTrap });
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
	Safe_Release(m_pPreviewTexture);
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
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemBodyTex", Engine::CRedStoneGolemBodyTex::Create(m_pGraphicDev))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemHeadTex", Engine::CRedStoneGolemHeadTex::Create(m_pGraphicDev))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemShoulderTex", Engine::CRedStoneGolemShoulderTex::Create(m_pGraphicDev))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemHipTex", Engine::CRedStoneGolemHipTex::Create(m_pGraphicDev))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemCoreTex", Engine::CRedStoneGolemCoreTex::Create(m_pGraphicDev))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemArmTex", Engine::CRedStoneGolemArmTex::Create(m_pGraphicDev))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemLegTex", Engine::CRedStoneGolemLegTex::Create(m_pGraphicDev))))
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
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Boss/T_RedStone_Golem.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CampLoadingTexture",
		Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Loading_Screen_Lobby.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneLoadingTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Fiery_Forge.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianLoadingTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Obsidian_Pinnacle.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_PlayerTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/steve_real.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SwordTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Player/iron_sword.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ChickenTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/chicken.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_GrassTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/GrassSideTexture.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_DirtTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/DirtTexture.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SandTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/SandTexture.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RockTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/RockTexture.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BedrockTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/BedrockTexture.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/ObsidianTexture.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianPngTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/blocks/obsidian.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_StoneBrickTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/StoneBrickTexture.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_OakTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/OakTexture.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_OakLeavesTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/OakLeaves.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/RedStone_Block.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_OakWoodTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/Oak_Wood.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CherryLeavesTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/CherryLeaves.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BlockAtlasTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/blocks/minecraft_block_atlas_4x4.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ZombieTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/zombie.png"))))
		return E_FAIL;
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
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeperTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/creeper.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_Head",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::HEAD))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_Body",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::BODY))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_RFLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::LEG))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_LFLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::LEG))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_RBLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::LEG))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_LBLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::LEG))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SpiderTexture",
		CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/T_Spider_Skin.png"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_Head",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::HEAD))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_Body",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::BODY))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_RFLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::LEG))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_LFLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::LEG))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_RBLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::LEG))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_LBLeg",
		Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::LEG))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Transform",
		Engine::CTransform::Create(m_pGraphicDev))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Calculator",
		Engine::CCalculator::Create(m_pGraphicDev))))
		return E_FAIL; 
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LavaTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE,
			L"../Bin/Resource/Texture/blocks/lava.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_PlankAcaciaTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE,
			L"../Bin/Resource/Texture/blocks/planks_acacia.dds"))))
		return E_FAIL;
	if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_PlankSpruceTexture",
		CTexture::Create(m_pGraphicDev, TEX_CUBE,
			L"../Bin/Resource/Texture/blocks/planks_spruce.dds"))))
		return E_FAIL;


	return 0;
}