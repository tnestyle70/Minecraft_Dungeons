#include "pch.h"
#include "CBeam.h"
#include "CRenderer.h"
#include "CCollider.h"

CBeam::CBeam(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CBeam::CBeam(const CGameObject& rhs)
    : CGameObject(rhs)
{
}

CBeam::~CBeam()
{
}

HRESULT CBeam::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    return S_OK;
}

_int CBeam::Update_GameObject(const _float fTimeDelta)
{
    return 0;
}

void CBeam::LateUpdate_GameObject(const _float& fTimeDelta)
{
}

void CBeam::Render_GameObject()
{
}

HRESULT CBeam::Add_Component()
{
    return S_OK;
}

CBeam* CBeam::Create(LPDIRECT3DDEVICE9 pGraphicDev,
    const _vec3& vStartPos, const _vec3& vDir)
{
    CBeam* pBeam = new CBeam(pGraphicDev);

    if (FAILED(pBeam->Ready_GameObject()))
    {
        Safe_Release(pBeam);
        MSG_BOX("CBeam Create Failed");
        return nullptr;
    }

    return pBeam;
}

void CBeam::Free()
{
    CGameObject::Free();
}