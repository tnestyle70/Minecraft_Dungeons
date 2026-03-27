#include "pch.h"
#include "CDLCBoss.h"
#include "CManagement.h"
#include "CRenderer.h" 
#include "CMonsterMgr.h"
#include "CPlayer.h"
#include "CCollider.h"
#include "CDamageMgr.h"

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

    
    if (m_pColliderCom && m_pTransformCom && m_iHp > 0)
    {
        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        m_pColliderCom->Update_AABB(vPos);

        CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
        if (pPlayer && pPlayer->Get_AtkColliderActive())
        {
            CCollider* pAtkCollider = dynamic_cast<CCollider*>(
                pPlayer->Get_Component(ID_STATIC, L"Com_AtkCollider"));

            if (pAtkCollider &&
                m_pColliderCom->IsColliding(pAtkCollider->Get_AABB()))
            {
                Take_Damage((int)pPlayer->Get_MeleeDmg());
            }
        }
    }

   
    CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
    if (pPlayer)
    {
        for (auto& pArrow : pPlayer->Get_Arrows())
        {
            if (pArrow->Is_Dead()) continue;
            CCollider* pArrowCollider = dynamic_cast<CCollider*>(
                pArrow->Get_Component(ID_STATIC, L"Com_Collider"));
            if (!pArrowCollider) continue;
            if (m_pColliderCom && m_pColliderCom->IsColliding(pArrowCollider->Get_AABB()))
            {
                if (pArrow->Is_Firework())
                {
                    pArrow->Trigger_Explode();
                    Take_Damage((int)(pPlayer->Get_BowDmg() * 3.f));
                }
                else
                {
                    Take_Damage((int)pPlayer->Get_BowDmg());
                    pArrow->Set_Dead();
                }
                break;
            }
        }
    }

    return iExit;
}

void CDLCBoss::LateUpdate_GameObject(const _float& fTimeDelta)
{
  
    Update_AI(fTimeDelta);

    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CDLCBoss::Take_Damage(int iDamage)
{
   
    m_iHp -= iDamage;
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    CDamageMgr::GetInstance()->AddDamage(vPos, iDamage);
    if (m_iHp < 0) m_iHp = 0;
}
void CDLCBoss::Free()
{
    Safe_Release(m_pColliderCom);
    Safe_Release(m_pAtkColliderCom);
    CGameObject::Free();
}