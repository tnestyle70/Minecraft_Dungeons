#include "pch.h"
#include "CEditor.h"
#include "CBlockMgr.h"
#include "CDynamicCamera.h"
#include "CMonsterUV.h"

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
		CBlockMgr::GetInstance()->LoadBlocks(GetStagePath(m_eCurrentStage));
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
	CBlockMgr::GetInstance()->SaveBlocks(GetStagePath(m_eCurrentStage));
	
	//스테이지 전환
	m_eCurrentStage = eNewStage;

	//블럭 클리어 후 새 스테이지 로딩
	CBlockMgr::GetInstance()->LoadBlocks(GetStagePath(m_eCurrentStage));
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
				if (FAILED(CBlockMgr::GetInstance()->SaveBlocks(GetStagePath(m_eCurrentStage))))
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
				if (FAILED(CBlockMgr::GetInstance()->LoadBlocks(GetStagePath(m_eCurrentStage))))
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
	ImGui::Text("Monster Spawn: %d", (int)m_vecMonsterData.size());
	ImGui::Text("IronBar: %d", (int)m_vecIronBarsData.size());
	ImGui::Text("TriggerBox: %d", (int)m_vecTriggerBoxData.size());

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
		Reneder_IronBarPalette();
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
			m_vecMonsterData.erase(m_vecMonsterData.begin() + i);
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