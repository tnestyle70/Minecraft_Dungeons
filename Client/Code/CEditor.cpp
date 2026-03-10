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

	//update block by selected pannel of the palette
	m_pBlockPlacer->Update_Placer(m_eSelectedBlock);

	CBlockMgr::GetInstance()->Update(fTimeDelta);

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
	CBlockMgr::GetInstance()->LoadBlocks(L"../Bin/Data/Stage.dat");
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
				if (FAILED(CBlockMgr::GetInstance()->SaveBlocks(L"../Bin/Data/Stage.dat")))
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
				if (FAILED(CBlockMgr::GetInstance()->LoadBlocks(L"../Bin/Data/Stage.dat")))
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

	// TODO: 씬 오브젝트 목록 출력

	ImGui::End();
}

void CEditor::Render_Inspector()
{
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
}

void CEditor::Render_Viewport()
{
	ImGui::Begin("Viewport");

	// TODO: 뷰포트 렌더 타겟 연결

	ImGui::End();
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
