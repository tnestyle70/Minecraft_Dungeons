#include "pch.h"
#include "CMainApp.h"
#include "CLogo.h"
#include "CProtoMgr.h"
#include "CFontMgr.h"
#include "CRenderer.h"
#include "CDInputMgr.h"
#include "CLightMgr.h"
#include <ctime>
#include "CSoundMgr.h"
#include "CBlockMgr.h"
#include "CParticleMgr.h"
#include "CMonsterMgr.h"
#include "CIronBarMgr.h"
#include "CTriggerBoxMgr.h"
#include "CInventoryMgr.h"
#include "CCursorMgr.h"
#include "CDamageMgr.h"
#include "CEnvironmentMgr.h"
#include "CNetworkMgr.h"
#include "CCMiniMap.h"


CMainApp::CMainApp()
    : m_pDeviceClass(nullptr), m_pGraphicDev(nullptr)
    , m_pManagementClass(CManagement::GetInstance())
{
}

CMainApp::~CMainApp()
{
}

HRESULT CMainApp::Ready_MainApp()
{
    srand(unsigned(time(NULL)));

    if (FAILED(Ready_DefaultSetting(&m_pGraphicDev)))
        return E_FAIL;

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX9_Init(m_pGraphicDev);

    if (FAILED(Ready_Scene(m_pGraphicDev)))
        return E_FAIL;

    return S_OK;
}

int CMainApp::Update_MainApp(const float& fTimeDelta)
{
    CDInputMgr::GetInstance()->Update_InputDev();

    CDamageMgr::GetInstance()->Update(fTimeDelta);

    CCursorMgr::GetInstance()->Update(fTimeDelta);

    m_pManagementClass->Update_Scene(fTimeDelta);

    return 0;
}

void CMainApp::LateUpdate_MainApp(const float& fTimeDelta)
{
    m_pManagementClass->LateUpdate_Scene(fTimeDelta);

    /*_ulong dwData = 0;

    if (dwData = CDInputMgr::GetInstance()->Get_DIMouseMove(DIMS_Z))
    {
        INT A = 0;
    }*/

    //Show FPS
    static float fTime = 0.f;
    static int iFrame = 0;
    fTime += fTimeDelta;
    iFrame++;

    if (fTime >= 1.f)
    {
        TCHAR szFPS[64];
        wsprintf(szFPS, L"FPS: %d", iFrame);
        SetWindowText(g_hWnd, szFPS);
        fTime = 0.f;
        iFrame = 0;
    }
}

void CMainApp::Render_MainApp()
{
    m_pDeviceClass->Render_Begin(D3DXCOLOR(0.f, 0.f, 1.f, 1.f));

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    m_pManagementClass->Render_Scene(m_pGraphicDev);

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    CDamageMgr::GetInstance()->Render();

    CCursorMgr::GetInstance()->Render();

    CCMiniMap::GetInstance()->Render();

    m_pDeviceClass->Render_End();
}

HRESULT CMainApp::Ready_DefaultSetting(LPDIRECT3DDEVICE9* ppGraphicDev)
{
    if (FAILED(CGraphicDev::GetInstance()->Ready_GraphicDev(g_hWnd, MODE_WIN, WINCX, WINCY, &m_pDeviceClass)))
        return E_FAIL;

    m_pDeviceClass->AddRef();

    (*ppGraphicDev) = m_pDeviceClass->Get_GraphicDev();
    (*ppGraphicDev)->AddRef();

    (*ppGraphicDev)->SetRenderState(D3DRS_LIGHTING, FALSE);

    (*ppGraphicDev)->SetRenderState(D3DRS_ZENABLE, TRUE);      
    (*ppGraphicDev)->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);  

    //MineCraftStyle Font Add
    if (0 == AddFontResourceEx(
        L"../Bin/Resource/Font/minecraft.ttf",
        FR_PRIVATE, nullptr))
    {
        MSG_BOX("Minecraft font load Failded");
        return E_FAIL;
    }

    if (FAILED(CFontMgr::GetInstance()->Ready_Font(m_pGraphicDev,
        L"Font_Minecraft", L"Minecraft", 16, 30, FW_NORMAL)))
        return E_FAIL;

    if (FAILED(CFontMgr::GetInstance()->Ready_Font(m_pGraphicDev, L"Font_Default", L"????", 15, 20, FW_HEAVY)))
        return E_FAIL;

    if (FAILED(CFontMgr::GetInstance()->Ready_Font(m_pGraphicDev, L"Font_Jinji", L"???", 30, 30, FW_HEAVY)))
        return E_FAIL;
  
    if (FAILED(CDInputMgr::GetInstance()->Ready_InputDev(g_hInst, g_hWnd)))
        return E_FAIL;

    //SoundMgr
    CSoundMgr::GetInstance()->Initialize();

    //CSoundMgr::GetInstance()->PlayBGM(L"BGM/Title.wav", 2.f);

    //RcTex
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RcTex", Engine::CRcTex::Create(m_pGraphicDev))))
        return E_FAIL;
    //Cursor - Default
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CursorTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/T_defaultCursor.png"))))
        return E_FAIL;
    //Cursor - Hover
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CursorClickTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/T_defaultCursorInteract.png"))))
        return E_FAIL;
    //Attack Cursor - Default
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_AttackCursorTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/T_actionCursor.png"))))
        return E_FAIL;
    //Attack Cursor - Hover
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_AttackCursorHoverTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/T_actionCursorInteract.png"))))
        return E_FAIL;

    //CursorMgr
    if (FAILED(CCursorMgr::GetInstance()->Ready_CursorMgr(m_pGraphicDev)))
        return E_FAIL;

    //DamageMgr
    if (FAILED(CDamageMgr::GetInstance()->Ready_DamageMgr(m_pGraphicDev)))
        return E_FAIL;

    (*ppGraphicDev)->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    (*ppGraphicDev)->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    return S_OK;
}

HRESULT CMainApp::Ready_Scene(LPDIRECT3DDEVICE9 pGraphicDev)
{
    Engine::CScene* pLogo = CLogo::Create(pGraphicDev);

    if (nullptr == pLogo)
        return E_FAIL;
    
    if (FAILED(m_pManagementClass->Set_Scene(pLogo)))
    {
        Safe_Release(pLogo);
        MSG_BOX("Logo Create Failed");
        return E_FAIL;
    }

    return S_OK;
}

CMainApp* CMainApp::Create()
{
    CMainApp* pInstance = new CMainApp;

    if (FAILED(pInstance->Ready_MainApp()))
    {
        Safe_Release(pInstance);
        MSG_BOX("MainApp Create Failed");
        return nullptr;
    }

    return pInstance;
}

void CMainApp::Free()
{
    // 1. ImGui 먼저 (DX9 디바이스 쓰는 것 중 가장 먼저)
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // 2. 씬/렌더 → 게임오브젝트들이 여기서 Free()됨
    CManagement::GetInstance()->DestroyInstance();

    // 3. 렌더 그룹 정리 후 렌더러 해제
    CRenderer::GetInstance()->Clear_RenderGroup();
    CRenderer::GetInstance()->DestroyInstance();

    // 4. 게임 오브젝트를 관리하는 매니저들
    CMonsterMgr::GetInstance()->DestroyInstance();
    CIronBarMgr::GetInstance()->DestroyInstance();
    CTriggerBoxMgr::GetInstance()->DestroyInstance();
    CParticleMgr::GetInstance()->DestroyInstance();
    CBlockMgr::GetInstance()->DestroyInstance();
    CInventoryMgr::GetInstance()->DestroyInstance();
    CCursorMgr::GetInstance()->DestroyInstance();
    CDamageMgr::GetInstance()->DestroyInstance();
    CEnvironmentMgr::GetInstance()->DestroyInstance();
    CCMiniMap::GetInstance()->DestroyInstance();

    // 5. 엔진 서비스 매니저들
    CSoundMgr::GetInstance()->DestroyInstance();
    CLightMgr::GetInstance()->DestroyInstance();
    CDInputMgr::GetInstance()->DestroyInstance();
    CFontMgr::GetInstance()->DestroyInstance();
    CProtoMgr::GetInstance()->DestroyInstance();
    CFrameMgr::GetInstance()->DestroyInstance();
    CTimerMgr::GetInstance()->DestroyInstance();

    //네트워크 매니저 해제
    CNetworkMgr::GetInstance()->DestroyInstance();

    // 6. DX9 디바이스는 무조건 마지막
    Safe_Release(m_pDeviceClass);
    Safe_Release(m_pGraphicDev);
}







