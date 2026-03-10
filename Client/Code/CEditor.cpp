#include "pch.h"
#include "CEditor.h"
#include "CBlockMgr.h"
#include "CDynamicCamera.h"

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
	if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
		return E_FAIL;

	//blockmgr 초기화
	if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
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
		return 0;

	_int iExit = CScene::Update_Scene(fTimeDelta);

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
		m_pBlockPlacer->Update_Placer(BLOCK_IRONBAR);
		CBlockMgr::GetInstance()->Update(fTimeDelta);
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

	CBlockMgr::GetInstance()->LoadBlocks(L"../Bin/Data/Stage1.dat");
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
				if (FAILED(CBlockMgr::GetInstance()->SaveBlocks(L"../Bin/Data/Stage1.dat")))
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
				if (FAILED(CBlockMgr::GetInstance()->LoadBlocks(L"../Bin/Data/Stage1.dat")))
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
	ImGui::Text("Monster Spawn: %d", (int)m_vecMonsterData.size());
	ImGui::Text("IronBar: %d", (int)m_vecIronBarsData.size());
	ImGui::Text("TriggerBox: %d", (int)m_vecTriggerBoxData.size());

	ImGui::End();
}

void CEditor::Render_Inspector()
{
	ImGui::Begin("Inspector");

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
		Reneder_IronBarPalette();
		break;
	case MODE_TRIGGERBOX:
		Render_TriggerPalette();
		break;
	default:
		break;
	}

	ImGui::End();

	/*
	ImGui::Begin("Inspector");

	//Show all blocks can select on the palette
	ImGui::Text("Block Palette");
	ImGui::Separator();

	//Definition of palette - {name, type}
	struct BlockEntry 
	{ 
		const char* name;  
		eBlockType eType;
	};

	static const BlockEntry palette[] =
	{
		{"Grass", BLOCK_GRASS},
		{"Dirt", BLOCK_DIRT},
		{"Rock", BLOCK_ROCK},
		{"Sand", BLOCK_SAND},
		{"Bedrock", BLOCK_BEDROCK},
		{"Obsidian", BLOCK_OBSIDIAN},
		{"StoneBrick", BLOCK_STONEBRICK},
		{"IronBar", BLOCK_IRONBAR}
	};

	for (const auto& entry : palette)
	{
		//현재 선택된 블럭 강조
		bool bSelected = (m_eSelectedBlock == entry.eType);
		if (bSelected)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.f));
		if (ImGui::Button(entry.name, ImVec2(120.f, 30.f)))
			m_eSelectedBlock = entry.eType;
		if (bSelected)
			ImGui::PopStyleColor();
	}

	ImGui::Separator();
	ImGui::Text("Selected : %s", palette[m_eSelectedBlock].name);

	ImGui::End();
	*/
}

void CEditor::Render_Viewport()
{
	ImGui::Begin("Viewport");

	// TODO: 뷰포트 렌더 타겟 연결

	ImGui::End();
}

void CEditor::Render_BlockPalette()
{
	struct BlockEntry { const char* name; eBlockType eType; };
	static const BlockEntry palette[] =
	{
		{"Grass", BLOCK_GRASS},
		{"Dirt", BLOCK_DIRT},
		{"Rock", BLOCK_ROCK},
		{"Sand", BLOCK_SAND},
		{"Bedrock", BLOCK_BEDROCK},
		{"Obsidian", BLOCK_OBSIDIAN},
		{"StoneBrick", BLOCK_STONEBRICK}
	};
	ImGui::Text("Block Palette");

	for (const auto& entry : palette)
	{
		bool bSelected = (m_eSelectedBlock == entry.eType);
		if (bSelected)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.f));
		if (ImGui::Button(entry.name, ImVec2(120.f, 28.f)))
			m_eSelectedBlock = entry.eType;
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

	//몬스터 타입 선택
	const char* szMonsters[] =
	{
		"Zombie", "Creeper", "Skeleton"
	};
	ImGui::Text("Type:");
	for (int i = 0; i < 3; ++i)
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
	ImGui::Text("Spawns : %d", (int)m_vecMonsterData.size());
	//배치된 스폰 목록
	for (int i = 0; i < (int)m_vecMonsterData.size(); ++i)
	{
		auto& data = m_vecMonsterData[i];
		ImGui::Text("[%d] %s (%d %d %d)",
			i, szMonsters[data.iMonsterType],
			data.x, data.y, data.z);
		ImGui::SameLine();
		char szBtn[32];
		sprintf_s(szBtn, "X %d", i);
		if (ImGui::Button(szBtn))
		{
			m_vecMonsterData.erase(m_vecMonsterData.begin() + 1);
			break;
		}
	}
}

void CEditor::Reneder_IronBarPalette()
{
	ImGui::Text("IronBar Placement");
	ImGui::Separator();
	ImGui::Text("LBtn: Place  RBtn: Remove");
	ImGui::Text("Placed: %d", (int)m_vecIronBarsData.size());

	for (int i = 0; i < (int)m_vecIronBarsData.size(); ++i)
	{
		auto& d = m_vecIronBarsData[i];
		ImGui::Text("[%d] (%d,%d,%d) TrigID:%d", i, d.x, d.y, d.z, d.iTriggerID);
		ImGui::SameLine();
		char szBtn[32];
		sprintf_s(szBtn, "X##b%d", i);
		if (ImGui::Button(szBtn))
		{
			m_vecIronBarsData.erase(m_vecIronBarsData.begin() + i);
			break;
		}
	}
}

void CEditor::Render_TriggerPalette()
{
	ImGui::Text("TriggerBox Placement");
	ImGui::Separator();

	// 트리거 박스 크기 설정
	ImGui::Text("Size:");
	ImGui::InputInt("W", &m_iTriggerWidth);
	ImGui::InputInt("H", &m_iTriggerHeight);
	ImGui::InputInt("D", &m_iTriggerDepth);

	ImGui::Separator();
	ImGui::Text("Triggers: %d", (int)m_vecTriggerBoxData.size());

	for (int i = 0; i < (int)m_vecTriggerBoxData.size(); ++i)
	{
		auto& d = m_vecTriggerBoxData[i];
		ImGui::Text("[%d] (%d,%d,%d) %dx%dx%d",
			i, d.x, d.y, d.z, d.width, d.height, d.depth);
		ImGui::SameLine();
		char szBtn[32];
		sprintf_s(szBtn, "X##t%d", i);
		if (ImGui::Button(szBtn))
		{
			m_vecTriggerBoxData.erase(m_vecTriggerBoxData.begin() + i);
			break;
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
			tData.y = tHitPos.y;
			tData.z = tHitPos.z;
			tData.iMonsterType = m_iSelectedMonster;
			m_vecMonsterData.push_back(tData);
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
			auto iter = remove_if(m_vecMonsterData.begin(), m_vecMonsterData.end(),
				[&](const MonsterData& data)
				{
					return data.x == tHitPos.x && data.y == tHitPos.y &&
						data.z == tHitPos.z;
				});
			m_vecMonsterData.erase(iter, m_vecMonsterData.end());
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
			m_vecTriggerBoxData.push_back(tData);
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
			auto it = remove_if(m_vecTriggerBoxData.begin(), m_vecTriggerBoxData.end(),
				[&](const TriggerBoxData& d) {
					return d.x == tHitPos.x &&
						d.y == tHitPos.y + 1 &&
						d.z == tHitPos.z;
				});
			m_vecTriggerBoxData.erase(it, m_vecTriggerBoxData.end());
		}
	}

	m_bLBtnPrev = bLBtn;
	m_bRBtnPrev = bRBtn;
}

HRESULT CEditor::SaveStageData(const _tchar* szPath)
{
	CBlockMgr::GetInstance()->SaveBlocks(szPath);

	return S_OK;
	/*
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, szPath, L"wb");
	if (!pFile)
		return E_FAIL;
	//블럭 저장
	int iCount = (int)CBlockMgr::GetInstance()->Get_Blocks().size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& pair : CBlockMgr::GetInstance()->Get_Blocks())
	{
		BlockData tData = { pair.first.x, pair.first.y, pair.first.z,
		(int)pair.second->GetBlockType() };
		fwrite(&tData, sizeof(BlockData), 1, pFile);
	}
	//몬스터 저장
	iCount = (int)m_vecMonsterData.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& d : m_vecMonsterData)
		fwrite(&d, sizeof(MonsterData), 1, pFile);

	// 창살 저장
	iCount = (int)m_vecIronBarsData.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& d : m_vecIronBarsData)
		fwrite(&d, sizeof(IronBarsData), 1, pFile);

	// 트리거 저장
	iCount = (int)m_vecTriggerBoxData.size();
	fwrite(&iCount, sizeof(int), 1, pFile);
	for (auto& d : m_vecTriggerBoxData)
		fwrite(&d, sizeof(TriggerBoxData), 1, pFile);

	fclose(pFile);
	return S_OK;
	*/
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
