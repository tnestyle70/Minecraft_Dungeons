#include "pch.h"
#include "AncientGuardian.h"
#include "CRenderer.h"
#include "CManagement.h"

CAncientGuardian::CAncientGuardian(LPDIRECT3DDEVICE9 pGraphicDev)
    : CDLCBoss(pGraphicDev)
{
}

CAncientGuardian::~CAncientGuardian()
{
}

HRESULT CAncientGuardian::Ready_GameObject()
{
    if (FAILED(CDLCBoss::Ready_GameObject()))
        return E_FAIL;

    return S_OK;
}

_int CAncientGuardian::Update_GameObject(const _float fTimeDelta)
{
    _int iExit = CDLCBoss::Update_GameObject(fTimeDelta);

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CAncientGuardian::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CDLCBoss::LateUpdate_GameObject(fTimeDelta);
}

HRESULT CAncientGuardian::Add_Component()
{
    return S_OK;
}

void CAncientGuardian::Update_AI(const _float& fTimeDelta)
{
}

void CAncientGuardian::Render_GameObject()
{
}

void CAncientGuardian::Update_Orbit(const _float& fTimeDelta)
{
}

void CAncientGuardian::Update_Charge(const _float& fTimeDelta)
{
}

void CAncientGuardian::Fire_Beam()
{
}

void CAncientGuardian::Update_Beams(const _float& fTimeDelta)
{
}

void CAncientGuardian::Drop_Biomine()
{
}

void CAncientGuardian::Update_Biomines(const _float& fTimeDelta)
{
}

CAncientGuardian* CAncientGuardian::Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos)
{
    CAncientGuardian* pInstance = new CAncientGuardian(pGraphicDev);

    if (FAILED(pInstance->Ready_GameObject()))
    {
        Safe_Release(pInstance);
        MSG_BOX("CAncientGuardian Create Failed");
        return nullptr;
    }

    pInstance->m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

    return pInstance;
}

void CAncientGuardian::Free()
{
    CDLCBoss::Free();
}