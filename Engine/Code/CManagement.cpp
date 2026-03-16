#include "CManagement.h"
#include "CRenderer.h"

IMPLEMENT_SINGLETON(CManagement)

CManagement::CManagement() : m_pScene(nullptr)
{
}

CManagement::~CManagement()
{
    Free();
}

CComponent* CManagement::Get_Component(COMPONENTID eID, const _tchar* pLayerTag, const _tchar* pObjTag, const _tchar* pComponentTag)
{
    if (nullptr == m_pScene)
        return nullptr;

    return m_pScene->Get_Component(eID, pLayerTag, pObjTag, pComponentTag);
}

HRESULT CManagement::Set_Scene(CScene* pScene)
{
    if (nullptr == pScene)
        return  E_FAIL;

    Safe_Release(m_pScene);

    m_pScene = pScene;

    return S_OK;
}

_int CManagement::Update_Scene(const _float& fTimeDelta)
{
    if (nullptr == m_pScene)
        return -1;

    return m_pScene->Update_Scene(fTimeDelta);
}

void CManagement::LateUpdate_Scene(const _float& fTimeDelta)
{
    m_pScene->LateUpdate_Scene(fTimeDelta);
}

void CManagement::Render_Scene(LPDIRECT3DDEVICE9 pGraphicDev)
{
    Engine::CRenderer* pRenderer = Engine::CRenderer::GetInstance();

    // 1. Render main world passes (Priority, NonAlpha, Alpha)
    pRenderer->Render_Priority(pGraphicDev);
    pRenderer->Render_NonAlpha(pGraphicDev);
    pRenderer->Render_Alpha(pGraphicDev);

    // 2. Render manual scene content (teammate's manual rendering)
    if (m_pScene)
        m_pScene->Render_Scene();

    // 3. Render UI last to ensure it is always on top
    pRenderer->Render_UI(pGraphicDev);

    // 4. Clear the render group for the next frame
    pRenderer->Clear_RenderGroup();
}

void CManagement::Free()
{
    Safe_Release(m_pScene);
}
