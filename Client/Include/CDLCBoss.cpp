#include "pch.h"
#include "CDLCBoss.h"
#include "CManagement.h"
#include "CRenderer.h"

CDLCBoss::CDLCBoss(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev) 
{
}

CDLCBoss::CDLCBoss(const CGameObject& rhs)
    : CGameObject(rhs) 
{
}

CDLCBoss::~CDLCBoss()
{
}

HRESULT CDLCBoss::Ready_GameObject()
{
    if (FAILED(Add_Component()))
        return E_FAIL;
    return S_OK;
}

_int CDLCBoss::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);
    return iExit;
}

void CDLCBoss::LateUpdate_GameObject(const _float& fTimeDelta)
{
 
    if (m_iHp > 0) {Update_AI(fTimeDelta);}
    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CDLCBoss::Take_Damage(int iDamage)
{
    m_iHp -= iDamage;
    if (m_iHp < 0) m_iHp = 0;
}

void CDLCBoss::Free()
{
    CGameObject::Free();
}