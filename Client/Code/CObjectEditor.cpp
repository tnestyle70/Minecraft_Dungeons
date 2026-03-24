#include "pch.h"
#include "CObjectEditor.h"
#include "CBox.h"
#include "CLamp.h"
#include "CDynamicCamera.h"
#include "CBlockMgr.h"
#include "CMonsterUV.h"

CObjectEditor::CObjectEditor(LPDIRECT3DDEVICE9 pGraphicDev)
    : CScene(pGraphicDev)
{
}

CObjectEditor::~CObjectEditor()
{
}

HRESULT CObjectEditor::Ready_Scene()
{
    if (FAILED(Ready_Prototype()))
        return E_FAIL;

    if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
        return E_FAIL;

    if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
    {
        MSG_BOX("block mgr create failed");
        return E_FAIL;
    }

    LoadStageData("Stage1");

    return S_OK;
}

_int CObjectEditor::Update_Scene(const _float& fTimeDelta)
{
    _int iExit = CScene::Update_Scene(fTimeDelta);

    CBlockMgr::GetInstance()->Update(fTimeDelta);

    for (auto& pair : m_mapEditObject)
    {
        pair.second->Update_GameObject(fTimeDelta);
    }

    if (m_pPreviewObject)
    {
        _vec3 vPos = Get_MouseWorldPos();

        CTransform* pTrans = dynamic_cast<CTransform*>(
            m_pPreviewObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

        if (pTrans)
            pTrans->Set_Pos(vPos.x, vPos.y, vPos.z);
    }

    if (m_pPreviewObject)
    {
        m_pPreviewObject->Update_GameObject(fTimeDelta);
    }
     
    Editor_Input();

    return iExit;
}

void CObjectEditor::LateUpdate_Scene(const _float& fTimeDelta)
{
    CScene::LateUpdate_Scene(fTimeDelta);

    for (auto& pair : m_mapEditObject)
    {
        pair.second->LateUpdate_GameObject(fTimeDelta);
    }

    if (m_pPreviewObject)
    {
        m_pPreviewObject->LateUpdate_GameObject(fTimeDelta);
    }
}

void CObjectEditor::Render_Scene()
{
    for (auto& pair : m_mapEditObject)
    {
        pair.second->Render_GameObject();
    }

    if (m_pPreviewObject)
    {
        m_pPreviewObject->Render_GameObject();
    }

    if (!m_pSelectedObject) return;

    CTransform* pTrans = dynamic_cast<CTransform*>(
        m_pSelectedObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

    CCollider* pCol = dynamic_cast<CCollider*>(
        m_pSelectedObject->Get_Component(ID_STATIC, L"Com_Collider"));

    if (!pTrans || !pCol) return;

    AABB aabb = pCol->Get_AABB();
    _vec3 vMin = aabb.vMin;
    _vec3 vMax = aabb.vMax;

    D3DXVECTOR3 v[8] =
    {
        { vMin.x, vMin.y, vMin.z }, { vMax.x, vMin.y, vMin.z },
        { vMax.x, vMin.y, vMax.z }, { vMin.x, vMin.y, vMax.z },
        { vMin.x, vMax.y, vMin.z }, { vMax.x, vMax.y, vMin.z },
        { vMax.x, vMax.y, vMax.z }, { vMin.x, vMax.y, vMax.z },
    };

    D3DXVECTOR3 lines[24] =
    {
        v[0],v[1], v[1],v[2], v[2],v[3], v[3],v[0],
        v[4],v[5], v[5],v[6], v[6],v[7], v[7],v[4],
        v[0],v[4], v[1],v[5], v[2],v[6], v[3],v[7],
    };

    DWORD dwLighting, dwFVF;
    m_pGraphicDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);
    m_pGraphicDev->GetFVF(&dwFVF);

    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pGraphicDev->SetFVF(D3DFVF_XYZ);

    D3DXMATRIX matWorld;
    D3DXMatrixIdentity(&matWorld);
    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

    m_pGraphicDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, lines, sizeof(D3DXVECTOR3));

    // ¬ø√∏¬∑¬° ¬ª√≥√Ö√Ç ¬∫¬π¬±¬∏
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, TRUE);
    m_pGraphicDev->SetFVF(dwFVF);
}

void CObjectEditor::Render_UI()
{
    CBlockMgr::GetInstance()->Render_Stage();

    // Object List
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_Once);

    ImGui::Begin("Object List");

    ImGui::BeginChild("ObjectListChild", ImVec2(0, 0), true);

    for (auto& pair : m_mapEditObject)
    {
        std::string str(pair.first.begin(), pair.first.end());

        if (ImGui::Selectable(str.c_str(),
            m_pSelectedObject == pair.second))
        {
            m_pSelectedObject = pair.second;
        }
    }

    ImGui::EndChild();
    ImGui::End();

    Render_CreateUI();
    Render_Inspector();
    Render_SaveLoad();
}

HRESULT CObjectEditor::SaveObjectData(const char* pFileName)
{
    FILE* pFile = nullptr;
    if (fopen_s(&pFile, pFileName, "wb") != 0)
        return E_FAIL;

    int iCount = static_cast<int>(m_mapEditObject.size());
    fwrite(&iCount, sizeof(int), 1, pFile);

    for (auto& pair : m_mapEditObject)
    {
        CGameObject* pObj = pair.second;
        if (!pObj) continue;

        CTransform* pTrans = dynamic_cast<CTransform*>(pObj->Get_Component(ID_DYNAMIC, L"Com_Transform"));
        if (!pTrans) continue;

        _vec3 vPos, vRot;
        pTrans->Get_Info(INFO_POS, &vPos);
        vRot = pTrans->Get_Rotation();

        OBJECT_DATA data;
        // ≈∏¿‘ √ﬂ√‚
        if (dynamic_cast<CBox*>(pObj)) data.eType = OBJECT_BOX;
        else if (dynamic_cast<CLamp*>(pObj)) data.eType = OBJECT_LAMP;
        else data.eType = OBJECT_END;

        data.vPos[0] = vPos.x;
        data.vPos[1] = vPos.y;
        data.vPos[2] = vPos.z;

        data.vRot[0] = vRot.x;
        data.vRot[1] = vRot.y;
        data.vRot[2] = vRot.z;

        fwrite(&data, sizeof(OBJECT_DATA), 1, pFile);
    }

    fclose(pFile);
    return S_OK;
}

HRESULT CObjectEditor::LoadObjectData(const char* pFileName)
{
    FILE* pFile = nullptr;
    if (fopen_s(&pFile, pFileName, "rb") != 0)
        return E_FAIL;

    // ±‚¡∏ ø¿∫Í¡ß∆Æ ¡¶∞≈
    for (auto& pair : m_mapEditObject)
        Safe_Release(pair.second);
    m_mapEditObject.clear();
    m_pSelectedObject = nullptr;

    int iCount = 0;
    fread(&iCount, sizeof(int), 1, pFile);

    for (int i = 0; i < iCount; ++i)
    {
        OBJECT_DATA data;
        fread(&data, sizeof(OBJECT_DATA), 1, pFile);

        CGameObject* pObj = nullptr;
        switch (data.eType)
        {
        case OBJECT_BOX:
            pObj = CBox::Create(m_pGraphicDev);
            break;
        case OBJECT_LAMP:
            pObj = CLamp::Create(m_pGraphicDev);
            break;
        default:
            continue;
        }

        if (!pObj) continue;

        CTransform* pTrans = dynamic_cast<CTransform*>(pObj->Get_Component(ID_DYNAMIC, L"Com_Transform"));
        if (pTrans)
        {
            pTrans->Set_Pos(data.vPos[0], data.vPos[1], data.vPos[2]);
            pTrans->Set_Rotation(ROT_X, data.vRot[0]);
            pTrans->Set_Rotation(ROT_Y, data.vRot[1]);
            pTrans->Set_Rotation(ROT_Z, data.vRot[2]);
        }

        // ≈∞ ª˝º∫
        static int iID = 0;
        wstring key = L"Obj_" + to_wstring(iID++);
        m_mapEditObject.insert({ key, pObj });
    }

    fclose(pFile);
    return S_OK;
}

HRESULT CObjectEditor::LoadStageData(const char* pFileName)
{
    CBlockMgr::GetInstance()->ClearBlocks();

    for (auto& pair : m_mapEditObject)
    {
        Safe_Release(pair.second);
    }
    m_mapEditObject.clear();

    std::string fileName = "../Bin/Data/";
    fileName += pFileName;
    fileName += ".dat";

    FILE* pMap = nullptr;
    fopen_s(&pMap, fileName.c_str(), "rb");

    if (!pMap)
        return E_FAIL;

    CBlockMgr::GetInstance()->LoadBlocks(pMap);
    fclose(pMap);

    return S_OK;
}

void CObjectEditor::Render_CreateUI()
{
    ImGui::SetNextWindowPos(ImVec2(970, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Once);

    ImGui::Begin("Create");

    if (ImGui::Button("Box"))
    {
        Start_CreateMode(L"Box");
    }

    if (ImGui::Button("Lamp"))
    {
        Start_CreateMode(L"Lamp");
    }

    ImGui::End();
}

void CObjectEditor::Render_Inspector()
{
    ImGui::SetNextWindowPos(ImVec2(970, 170), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);

    ImGui::Begin("Inspector");

    if (!m_pSelectedObject)
    {
        ImGui::Text("No Object Selected");
        ImGui::End();
        return;
    }

    CTransform* pTrans = dynamic_cast<CTransform*>(
        m_pSelectedObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

    if (!pTrans)
    {
        ImGui::End();
        return;
    }

    _vec3 vPos;
    pTrans->Get_Info(INFO_POS, &vPos);

    float pos[3] = { vPos.x, vPos.y, vPos.z };

    if (ImGui::DragFloat3("Position", pos, 0.1f))
    {
        pTrans->Set_Pos(pos[0], pos[1], pos[2]);
    }

    ImGui::End();
}

void CObjectEditor::Render_SaveLoad()
{
    ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(200, 120), ImGuiCond_Once);

    ImGui::Begin("Object Save / Load");

    // Ω∫≈◊¿Ã¡ˆ º±≈√øÎ Combo Box
    static int stageIndex = 0;
    const char* stages[] = { "Stage1", "Stage2", "Stage3", "Stage4"};
    ImGui::Combo("Stage", &stageIndex, stages, IM_ARRAYSIZE(stages));

    // º±≈√µ» Ω∫≈◊¿Ã¡ˆø° µ˚∂Û ∆ƒ¿œ ¿Ã∏ß ∞·¡§
    std::string fileName = "../Bin/Data/";
    fileName += stages[stageIndex];
    fileName += "Object.dat";

    if (ImGui::Button("Save"))
    {
        if (FAILED(SaveObjectData(fileName.c_str())))
        {
            MSG_BOX("Failed to Save Object Data!");
        }
    }

    if (ImGui::Button("Load"))
    {
        if (FAILED(LoadObjectData(fileName.c_str())))
        {
            MSG_BOX("Failed to Load Object Data!");
        }
    }

    if (ImGui::Button("Load Stage"))
    {
        if (FAILED(LoadStageData(stages[stageIndex])))
        {
            MSG_BOX("Failed to Load Stage!");
        }
    }

    ImGui::End();
}

void CObjectEditor::Create_Object(const wstring& type)
{
    if (!m_pPreviewObject)
        return;

    CGameObject* pObj = nullptr;

    if (type == L"Box")
        pObj = CBox::Create(m_pGraphicDev);
    else if (type == L"Lamp")
        pObj = CLamp::Create(m_pGraphicDev);

    if (!pObj) return;

    CTransform* pPreviewTrans = dynamic_cast<CTransform*>(
        m_pPreviewObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

    CTransform* pNewTrans = dynamic_cast<CTransform*>(
        pObj->Get_Component(ID_DYNAMIC, L"Com_Transform"));

    if (pPreviewTrans && pNewTrans)
    {
        _vec3 vPos;
        pPreviewTrans->Get_Info(INFO_POS, &vPos);

        pNewTrans->Set_Pos(vPos.x, vPos.y, vPos.z);
    }

    static int iID = 0;
    wstring key = type + L"_" + to_wstring(iID++);

    m_mapEditObject.insert({ key, pObj });

    m_pSelectedObject = pObj;
}

void CObjectEditor::Start_CreateMode(const wstring& type)
{
    m_wstrCreateType = type;

    if (m_pPreviewObject)
        Safe_Release(m_pPreviewObject);

    if (type == L"Box")
        m_pPreviewObject = CBox::Create(m_pGraphicDev);
    else if (type == L"Lamp")
        m_pPreviewObject = CLamp::Create(m_pGraphicDev);
}

HRESULT CObjectEditor::Ready_Environment_Layer(const _tchar* pLayerTag)
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

    m_mapLayer.insert({ pLayerTag, pLayer });

    return S_OK;
}

HRESULT CObjectEditor::Ready_Prototype()
{
    // Transform
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Transform", Engine::CTransform::Create(m_pGraphicDev))))
        return E_FAIL;

    // Box
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BoxBottomTex", Engine::CBoxBottomTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BoxTopTex", Engine::CBoxTopTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BoxTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Object/T_LargeBoxChest.png"))))
        return E_FAIL;

    // Lamp
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LampBodyTex", Engine::CLampBodyTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LampHeadTex", Engine::CLampHeadTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LampTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Object/T_Lamp.png"))))
        return E_FAIL;

    //
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TriCol", Engine::CTriCol::Create(m_pGraphicDev))))
        return E_FAIL;
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RcCol", Engine::CRcCol::Create(m_pGraphicDev))))
        return E_FAIL;
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TerrainTex", Engine::CTerrainTex::Create(m_pGraphicDev))))
        return E_FAIL;
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CubeTex", Engine::CCubeTex::Create(m_pGraphicDev))))
    //    return E_FAIL;
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemBodyTex", Engine::CRedStoneGolemBodyTex::Create(m_pGraphicDev))))
    //    return E_FAIL;
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemHeadTex", Engine::CRedStoneGolemHeadTex::Create(m_pGraphicDev))))
    //    return E_FAIL;
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemShoulderTex", Engine::CRedStoneGolemShoulderTex::Create(m_pGraphicDev))))
    //    return E_FAIL;
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemHipTex", Engine::CRedStoneGolemHipTex::Create(m_pGraphicDev))))
    //    return E_FAIL;
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemCoreTex", Engine::CRedStoneGolemCoreTex::Create(m_pGraphicDev))))
    //    return E_FAIL;
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemArmTex", Engine::CRedStoneGolemArmTex::Create(m_pGraphicDev))))
    //    return E_FAIL;
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemLegTex", Engine::CRedStoneGolemLegTex::Create(m_pGraphicDev))))
    //    return E_FAIL;
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TerrainTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Terrain/Grass_%d.tga", 2))))
        return E_FAIL;
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SkyBoxTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/SkyBox/burger%d.dds", 4))))
        return E_FAIL;
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_EffectTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Explosion/Explosion%d.png", 90))))
    //    return E_FAIL;
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemTexture",
    //    CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Boss/T_RedStone_Golem.png"))))
    //    return E_FAIL;
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
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Transform",
    //    Engine::CTransform::Create(m_pGraphicDev))))
    //    return E_FAIL;
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Calculator",
        Engine::CCalculator::Create(m_pGraphicDev))))
        return E_FAIL;

    return S_OK;
}

void CObjectEditor::Editor_Input()
{
    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        return;

    bool bLButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    bool bRButtonDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

    bool bLButtonClicked = bLButtonDown && !m_bLButtonPrev;
    bool bRButtonClicked = bRButtonDown && !m_bRButtonPrev;

    m_bLButtonPrev = bLButtonDown;
    m_bRButtonPrev = bRButtonDown;

    if (bLButtonClicked)
    {
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
        {
            _vec3 vOrigin, vDir;
            Get_MouseRay(vOrigin, vDir);

            CGameObject* pPicked = nullptr;
            _float fMinT = FLT_MAX;

            for (auto& pair : m_mapEditObject)
            {
                CGameObject* pObj = pair.second;

                CTransform* pTrans = dynamic_cast<CTransform*>(
                    pObj->Get_Component(ID_DYNAMIC, L"Com_Transform"));

                CCollider* pCol = dynamic_cast<CCollider*>(
                    pObj->Get_Component(ID_STATIC, L"Com_Collider"));

                if (!pTrans || !pCol) continue;

                AABB aabb = pCol->Get_AABB();

                _float t = 0.f;
                if (IntersectRayAABB(vOrigin, vDir, aabb.vMin, aabb.vMax, t))
                {
                    if (t < fMinT)
                    {
                        fMinT = t;
                        pPicked = pObj;
                    }
                }
            }

            m_pSelectedObject = pPicked;
        }
    }

    if (bRButtonClicked)
    {
        if (m_pSelectedObject)
        {
            for (auto iter = m_mapEditObject.begin(); iter != m_mapEditObject.end(); )
            {
                if (iter->second == m_pSelectedObject)
                {
                    Safe_Release(iter->second);
                    iter = m_mapEditObject.erase(iter);
                }
                else
                    ++iter;
            }

            m_pSelectedObject = nullptr;
        }
    }

    if (GetAsyncKeyState(VK_RIGHT))
    {
        if (m_pSelectedObject)
        {
            CTransform* pTrans = dynamic_cast<CTransform*>(
                m_pSelectedObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

            pTrans->Rotation(ROT_Y, -3.f);
        }
    }
    else if (GetAsyncKeyState(VK_LEFT))
    {
        if (m_pSelectedObject)
        {
            CTransform* pTrans = dynamic_cast<CTransform*>(
                m_pSelectedObject->Get_Component(ID_DYNAMIC, L"Com_Transform"));

            pTrans->Rotation(ROT_Y, 3.f);
        }
    }

    if (bLButtonClicked && !m_wstrCreateType.empty())
    {
        Create_Object(m_wstrCreateType);

        Safe_Release(m_pPreviewObject);
        m_pPreviewObject = nullptr;
        m_wstrCreateType = L"";

        return;
    }
}

_vec3 CObjectEditor::Get_MouseWorldPos()
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(g_hWnd, &pt);

    D3DVIEWPORT9 vp;
    m_pGraphicDev->GetViewport(&vp);

    D3DXVECTOR3 vNear(pt.x, pt.y, 0.f);
    D3DXVECTOR3 vFar(pt.x, pt.y, 1.f);

    D3DXMATRIX matProj, matView, matWorld;
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
    D3DXMatrixIdentity(&matWorld);

    D3DXVec3Unproject(&vNear, &vNear, &vp, &matProj, &matView, &matWorld);
    D3DXVec3Unproject(&vFar, &vFar, &vp, &matProj, &matView, &matWorld);

    float t = -vNear.y / (vFar.y - vNear.y);

    return vNear + (vFar - vNear) * t;
}

void CObjectEditor::Get_MouseRay(_vec3& vOrigin, _vec3& vDir)
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(g_hWnd, &pt);

    D3DVIEWPORT9 vp;
    m_pGraphicDev->GetViewport(&vp);

    D3DXVECTOR3 vNear(pt.x, pt.y, 0.f);
    D3DXVECTOR3 vFar(pt.x, pt.y, 1.f);

    D3DXMATRIX matProj, matView, matWorld;
    m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
    D3DXMatrixIdentity(&matWorld);

    D3DXVec3Unproject(&vNear, &vNear, &vp, &matProj, &matView, &matWorld);
    D3DXVec3Unproject(&vFar, &vFar, &vp, &matProj, &matView, &matWorld);

    vOrigin = vNear;
    vDir = vFar - vNear;
    D3DXVec3Normalize(&vDir, &vDir);
}

bool CObjectEditor::IntersectRayAABB(const _vec3& rayOrigin, const _vec3& rayDir, const _vec3& min, const _vec3& max, _float& t)
{
    float tmin = (min.x - rayOrigin.x) / rayDir.x;
    float tmax = (max.x - rayOrigin.x) / rayDir.x;

    if (tmin > tmax) swap(tmin, tmax);

    float tymin = (min.y - rayOrigin.y) / rayDir.y;
    float tymax = (max.y - rayOrigin.y) / rayDir.y;

    if (tymin > tymax) swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    tmin = max(tmin, tymin);
    tmax = min(tmax, tymax);

    float tzmin = (min.z - rayOrigin.z) / rayDir.z;
    float tzmax = (max.z - rayOrigin.z) / rayDir.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    tmin = max(tmin, tzmin);

    t = tmin;
    return true;
}

CObjectEditor* CObjectEditor::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CObjectEditor* pObjectEditor = new CObjectEditor(pGraphicDev);

    if (FAILED(pObjectEditor->Ready_Scene()))
    {
        Safe_Release(pObjectEditor);
        MSG_BOX("ObjectEditor Create Failed");
        return nullptr;
    }

    return pObjectEditor;
}

void CObjectEditor::Free()
{
    for (auto& pair : m_mapEditObject)
    {
        Safe_Release(pair.second);
    }
    m_mapEditObject.clear();

    CScene::Free();
}