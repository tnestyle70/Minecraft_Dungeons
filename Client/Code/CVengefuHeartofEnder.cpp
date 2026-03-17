#include "pch.h"
#include "CVengefuHeartofEnder.h"
#include "CRenderer.h"
#include "CManagement.h"

CVengefulHeartOfEnder::CVengefulHeartOfEnder(LPDIRECT3DDEVICE9 pGraphicDev)
    : CDLCBoss(pGraphicDev)
{
}

CVengefulHeartOfEnder::~CVengefulHeartOfEnder()
{
}

HRESULT CVengefulHeartOfEnder::Ready_GameObject()
{
    if (FAILED(CDLCBoss::Ready_GameObject()))
        return E_FAIL;

    m_vecSegmentPos.resize(m_iSegmentCount, _vec3(0.f, 0.f, 0.f));

    return S_OK;
}

_int CVengefulHeartOfEnder::Update_GameObject(const _float fTimeDelta)
{
    _int iExit = CDLCBoss::Update_GameObject(fTimeDelta);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CVengefulHeartOfEnder::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CDLCBoss::LateUpdate_GameObject(fTimeDelta);
}

HRESULT CVengefulHeartOfEnder::Add_Component()
{
    return S_OK;
}

void CVengefulHeartOfEnder::Update_AI(const _float& fTimeDelta)
{
}

void CVengefulHeartOfEnder::Render_GameObject()
{
}

void CVengefulHeartOfEnder::Update_Segments()
{
}

void CVengefulHeartOfEnder::Update_Rush(const _float& fTimeDelta)
{
}

void CVengefulHeartOfEnder::Update_VoidSpew(const _float& fTimeDelta)
{
}

void CVengefulHeartOfEnder::Fire_Beam()
{
}

void CVengefulHeartOfEnder::Update_Beams(const _float& fTimeDelta)
{
}

CVengefulHeartOfEnder* CVengefulHeartOfEnder::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{
    CVengefulHeartOfEnder* pInstance = new CVengefulHeartOfEnder(pGraphicDev);

    if (FAILED(pInstance->Ready_GameObject()))
    {
        Safe_Release(pInstance);
        MSG_BOX("CVengefulHeartOfEnder Create Failed");
        return nullptr;
    }

    pInstance->m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

    return pInstance;
}

void CVengefulHeartOfEnder::Free()
{
    for (auto* pBeam : m_vecBeams)
        Safe_Release(pBeam);
    m_vecBeams.clear();

    CDLCBoss::Free();
}