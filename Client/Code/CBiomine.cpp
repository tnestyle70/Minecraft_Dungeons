#include "pch.h"
#include "CBiomine.h"
#include "CRenderer.h"
#include "CCollider.h"

CBiomine::CBiomine(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CBiomine::CBiomine(const CGameObject& rhs)
    : CGameObject(rhs)
{
}

CBiomine::~CBiomine()
{
}

HRESULT CBiomine::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    return S_OK;
}

_int CBiomine::Update_GameObject(const _float fTimeDelta)
{
    return 0;
}

void CBiomine::LateUpdate_GameObject(const _float& fTimeDelta)
{
}

void CBiomine::Render_GameObject()
{
}

HRESULT CBiomine::Add_Component()
{
    return S_OK;
}

void CBiomine::Apply_Gravity(const _float fTimeDelta)
{
}

void CBiomine::Resolve_BlockCollision()
{
}

CBiomine* CBiomine::Create(LPDIRECT3DDEVICE9 pGraphicDev, const _vec3& vStartPos)
{
    CBiomine* pMine = new CBiomine(pGraphicDev);

    if (FAILED(pMine->Ready_GameObject()))
    {
        Safe_Release(pMine);
        MSG_BOX("CBiomine Create Failed");
        return nullptr;
    }

    return pMine;
}

void CBiomine::Free()
{
    CGameObject::Free();
}