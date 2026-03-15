#include "pch.h"
#include "CStage.h"
#include "CBackGround.h"
#include "CProtoMgr.h"
#include "CMonster.h"
#include "CPlayer.h"
#include "CDynamicCamera.h"
#include "CTerrain.h"
#include "CSkyBox.h"
#include "CLightMgr.h"
#include "CEffect.h"
#include "CBlock.h"
#include "CBlockMgr.h" 
#include "CMonsterAnim.h"
#include "CSceneChanger.h"
#include "CRenderer.h"
#include "StageData.h"

CStage::CStage(LPDIRECT3DDEVICE9 pGraphicDev)
    : Engine::CScene(pGraphicDev)
{
}

CStage::~CStage()
{
}

HRESULT CStage::Ready_Scene()
{    
    if (FAILED(Ready_Light()))
        return E_FAIL;

    if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
        return E_FAIL;

    if (FAILED(Ready_GameLogic_Layer(L"GameLogic_Layer")))
        return E_FAIL;

    if (FAILED(Ready_UI_Layer(L"UI_Layer")))
        return E_FAIL;
    
    return S_OK;
}

_int CStage::Update_Scene(const _float& fTimeDelta)
{
    _int iExit = Engine::CScene::Update_Scene(fTimeDelta);

    CBlockMgr::GetInstance()->Update(fTimeDelta);

    if (GetAsyncKeyState(VK_RETURN))
    {
        //Render Group Clear Before Change Scene!!!!
        CRenderer::GetInstance()->Clear_RenderGroup();
        if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_LOGO)))
        {
            MSG_BOX("SquidCoast Create Failed");
            return -1;
        }
    }

    return iExit;
}

void CStage::LateUpdate_Scene(const _float& fTimeDelta)
{
    Engine::CScene::LateUpdate_Scene(fTimeDelta);
}

void CStage::Render_Scene()
{
    //����ȭ ���� �ּ� ����
    CBlockMgr::GetInstance()->Render();
}

HRESULT CStage::Ready_Environment_Layer(const _tchar* pLayerTag)
{
    CLayer* pLayer = CLayer::Create();

    if (nullptr == pLayer)
        return E_FAIL;

    CGameObject* pGameObject = nullptr;

    // dynamicCamera

    _vec3   vEye{ 0.f, 10.f, -10.f };
    _vec3   vAt{ 0.f, 0.f, 1.f };
    _vec3   vUp{ 0.f, 1.f, 0.f };

    pGameObject = CDynamicCamera::Create(m_pGraphicDev, &vEye, &vAt, &vUp);

    if (nullptr == pGameObject)
        return E_FAIL;

    if (FAILED(pLayer->Add_GameObject(L"DynamicCamera", pGameObject)))
        return E_FAIL;    

    // skyBox
    pGameObject = CSkyBox::Create(m_pGraphicDev);

    if (nullptr == pGameObject)
        return E_FAIL;

    if (FAILED(pLayer->Add_GameObject(L"SkyBox", pGameObject)))
        return E_FAIL;

    m_mapLayer.insert({ pLayerTag, pLayer });

    //ready blockmgr 
    if (FAILED(CBlockMgr::GetInstance()->Ready_BlockMgr(m_pGraphicDev)))
    {
        MSG_BOX("block mgr create failed");
        return E_FAIL;
    }

    CBlockMgr::GetInstance()->SetEditorMode(false); // ���� ��� ����

    FILE* pFile = nullptr;
    _wfopen_s(&pFile, L"../Bin/Data/Stage1.dat", L"rb");
    if (pFile)
    {
        CBlockMgr::GetInstance()->LoadBlocks(pFile); // ���ο��� Rebuild���� ó��
        fclose(pFile);
    }

    //SetEditorMode ���ο��� rebuildbatchmesh�� �ؾ� �ϴµ�,
    //�� �������� ������ ���� �����̹Ƿ�, LoadBlocks�� ���� �ؾ� ��
    //CBlockMgr::GetInstance()->SetEditorMode(false);

    return S_OK;
}

HRESULT CStage::Ready_GameLogic_Layer(const _tchar* pLayerTag)
{
    CLayer* pLayer = CLayer::Create();

    if (nullptr == pLayer)
        return E_FAIL;

    CGameObject* pGameObject = nullptr;

    // Terrain
    pGameObject = CTerrain::Create(m_pGraphicDev);

    if (nullptr == pGameObject)
        return E_FAIL;

    if (FAILED(pLayer->Add_GameObject(L"Terrain", pGameObject)))
        return E_FAIL;

    // player
    pGameObject = CPlayer::Create(m_pGraphicDev);

    if (nullptr == pGameObject)
        return E_FAIL;

    if (FAILED(pLayer->Add_GameObject(L"Player", pGameObject)))
        return E_FAIL;

 
    pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::ZOMBIE);
    if (nullptr == pGameObject)
        return E_FAIL;
    if (FAILED(pLayer->Add_GameObject(L"Monster_Zombie", pGameObject)))
        return E_FAIL;

    pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::SKELETON);
    if (nullptr == pGameObject)
        return E_FAIL;
    if (FAILED(pLayer->Add_GameObject(L"Monster_Skeleton", pGameObject)))
        return E_FAIL;

    pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::CREEPER);
    if (nullptr == pGameObject)
        return E_FAIL;
    if (FAILED(pLayer->Add_GameObject(L"Monster_creeper", pGameObject)))
        return E_FAIL; 

    pGameObject = CMonster::Create(m_pGraphicDev, EMonsterType::SPIDER);
    if (nullptr == pGameObject)
        return E_FAIL;
    if (FAILED(pLayer->Add_GameObject(L"Monster_Spider", pGameObject)))
        return E_FAIL;
    

   

    m_mapLayer.insert({ pLayerTag, pLayer });

    return S_OK;
}

HRESULT CStage::Ready_UI_Layer(const _tchar* pLayerTag)
{
    CLayer* pLayer = CLayer::Create();

    if (nullptr == pLayer)
        return E_FAIL;

    CGameObject* pGameObject = nullptr;

    //effect
    for (_uint i = 0; i < 50; ++i)
    {
        pGameObject = CEffect::Create(m_pGraphicDev);

        if (!pGameObject)
            return E_FAIL;
        if (FAILED(pLayer->Add_GameObject(L"Effect", pGameObject)))
            return E_FAIL;
    }

    m_mapLayer.insert({ pLayerTag, pLayer });

    return S_OK;
}

HRESULT CStage::Ready_Light()
{
    D3DLIGHT9   tLightInfo;
    ZeroMemory(&tLightInfo, sizeof(D3DLIGHT9));

    tLightInfo.Type = D3DLIGHT_DIRECTIONAL;

    tLightInfo.Diffuse  = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
    tLightInfo.Specular = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);
    tLightInfo.Ambient  = D3DXCOLOR(1.f, 1.f, 1.f, 1.f);

    tLightInfo.Direction = { 1.f, -1.f, 1.f };

    if (FAILED(CLightMgr::GetInstance()->Ready_Light(m_pGraphicDev, &tLightInfo, 0)))
        return E_FAIL;

    return S_OK;
}

CStage* CStage::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CStage* pLogo = new CStage(pGraphicDev);

    if (FAILED(pLogo->Ready_Scene()))
    {
        Safe_Release(pLogo);
        MSG_BOX("Stage Create Failed");
        return nullptr;
    }

    return pLogo;
}

void CStage::Free()
{
    CScene::Free();
}
