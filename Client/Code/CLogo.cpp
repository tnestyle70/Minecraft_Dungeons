#include "pch.h"
#include "CLogo.h"
#include "CBackGround.h"
#include "CProtoMgr.h"
#include "CMonster.h"
#include "CPlayer.h"
#include "CStage.h"
#include "CSquidCoast.h"
#include "CFontMgr.h"
#include "CDInputMgr.h"
#include "CEditor.h"
#include "CBlockMgr.h"
#include "CSceneChanger.h"
#include "CRenderer.h"
#include "CObjectEditor.h"
#include "CInventoryMgr.h"

CLogo::CLogo(LPDIRECT3DDEVICE9 pGraphicDev)
    : Engine::CScene(pGraphicDev), m_pEditor(nullptr), m_pObjectEditor(nullptr)
{
}

CLogo::~CLogo()
{
}

HRESULT CLogo::Ready_Scene()
{
    if (FAILED(Ready_Prototype()))
        return E_FAIL;

    if (FAILED(Ready_Environment_Layer(L"Environment_Layer")))
        return E_FAIL;

    return S_OK;
}

_int CLogo::Update_Scene(const _float& fTimeDelta)
{
    bool bF1 = CDInputMgr::GetInstance()->Get_DIKeyState(DIK_F1);
    bool bF2 = CDInputMgr::GetInstance()->Get_DIKeyState(DIK_F2);

    //에디터 모드로 변경
    if (bF1 && !m_bF1Toggle)
    {
        if (!m_pEditor)
        {
            //최초 에디터 생성 카메라 
            m_pEditor = CEditor::Create(m_pGraphicDev);
            if (!m_pEditor)
                return -1;
        }
        //처음 생성 이후 토글만 적용
        m_pEditor->SetEditorMode(!m_pEditor->IsEditorMode());
    }

    // 오브젝트 에디터 변경
    if (bF2 && !m_bF2Toggle)
    {
        if (!m_pObjectEditor)
        {
            CRenderer::GetInstance()->Clear_RenderGroup();

            if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, SCENE_OBJECT_EDITOR)))
            {
                MSG_BOX("ObjectEditor Scene Change Failed");
                return -1;
            }

            return 0;
        }
    }

    m_bF1Toggle = bF1;
    m_bF2Toggle = bF2;

    if (m_pEditor && m_pEditor->IsEditorMode())
    {
        m_pEditor->Update_Scene(fTimeDelta);
        return 0;
    }

    //로고 씬 업데이트
    _int iExit = Engine::CScene::Update_Scene(fTimeDelta);

    if (GetAsyncKeyState(VK_RETURN))
    {
        //Render Group Clear Before Change Scene!!!!
        CRenderer::GetInstance()->Clear_RenderGroup();

        if (FAILED(CSceneChanger::ChangeScene(m_pGraphicDev, eSceneType::SCENE_SQUIDCOAST)))
        {
            MSG_BOX("SquidCoast Create Failed");
            return -1;
        }
    }
    
    return iExit;
}

void CLogo::LateUpdate_Scene(const _float& fTimeDelta)
{
    if (m_pEditor && m_pEditor->IsEditorMode())
    {
        m_pEditor->LateUpdate_Scene(fTimeDelta);
        return;
    }

    Engine::CScene::LateUpdate_Scene(fTimeDelta);
}

void CLogo::Render_Scene()
{
    if (m_pEditor && m_pEditor->IsEditorMode())
    {
        m_pEditor->Render_Scene();
        return;
    }
}

void CLogo::Render_UI()
{
    _vec2 vPos{ 460.f, 550.f };
    CFontMgr::GetInstance()->Render_Font(
        L"Font_Minecraft", L"Please Press Any Keys", &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
}

HRESULT CLogo::Ready_Environment_Layer(const _tchar* pLayerTag)
{
    CLayer* pLayer = CLayer::Create();

    if (nullptr == pLayer)
        return E_FAIL;

    // 오브젝트 추가
    CGameObject* pGameObject = nullptr;

    // back ground
    pGameObject = CBackGround::Create(m_pGraphicDev, L"Proto_MainMenuTexture");

    if (nullptr == pGameObject)
        return E_FAIL;

    if (FAILED(pLayer->Add_GameObject(L"BackGround", pGameObject)))
        return E_FAIL;

    m_mapLayer.insert({ pLayerTag, pLayer });

    return S_OK;
}

HRESULT CLogo::Ready_Prototype()
{    
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_MainMenuTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/MainMenu_Screen2.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SquidCoastLoadingTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Loading_Screen_Squid_Coast.png"))))
        return E_FAIL;

    return S_OK;
}

CLogo* CLogo::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CLogo* pLogo = new CLogo(pGraphicDev);

    if (FAILED(pLogo->Ready_Scene()))
    {
        Safe_Release(pLogo);
        MSG_BOX("Logo Create Failed");
        return nullptr;
    }

    return pLogo;
}

void CLogo::Free()
{
    Safe_Release(m_pEditor);
    CScene::Free();
}
