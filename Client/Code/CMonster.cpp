#include "pch.h"
#include "CMonster.h"
#include "CManagement.h"
#include "CProtoMgr.h"
#include "CRenderer.h"
#include "CBlockMgr.h"
#include "CCollider.h"
#include "CParticleMgr.h"
#include "CPlayer.h"
#include "CMonsterMgr.h"
#include "CDamageMgr.h"
#include "CPlayerArrow.h"
#include "CTNT.h"

CMonster::CMonster(LPDIRECT3DDEVICE9 pGraphicDev)
    : CGameObject(pGraphicDev)
{
}

CMonster::CMonster(const CGameObject& rhs)
    : CGameObject(rhs)
{
}

CMonster::~CMonster()
{
} 



HRESULT CMonster::Ready_GameObject(_vec3& vPos)
{
    if (FAILED(Add_Component()))
        return E_FAIL;

    switch (m_eType)
    {
    case EMonsterType::ZOMBIE:   m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z); 
        m_iHp = 20;
        m_iAtkDamage = 10;
        break;        
    case EMonsterType::SKELETON: m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z); 
        m_iHp = 20;
        m_iAtkDamage = 0;
        break;
    case EMonsterType::CREEPER:  m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z); 
        m_iHp = 10;
        m_iAtkDamage = 100;
        break;
    case EMonsterType::SPIDER:   m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
        m_iHp = 20;
        m_iAtkDamage = 10;
    }

    //=====Effect Emitter Connect======// 
    LPDIRECT3DTEXTURE9 pDeathEffectTexture = nullptr;
    D3DXCreateTextureFromFile(m_pGraphicDev,
        L"../Bin/Resource/Texture/Effect/Smoke.png", &pDeathEffectTexture);

    m_pDeathEmitter = CParticleEmitter::Create(
        m_pGraphicDev, PARTICLE_FOOTSTEP, _vec3(0.f, 0.f, 0.f), pDeathEffectTexture);

    CParticleMgr::GetInstance()->Add_Emitter(m_pDeathEmitter);

    return S_OK;
}

_int CMonster::Update_GameObject(const _float& fTimeDelta)
{
    if (!m_bActive)
        return 0;

    _int iExit = CGameObject::Update_GameObject(fTimeDelta);
    Apply_Gravity(fTimeDelta);
    Resolve_BlockCollision();

    CMonsterAnim* pAnim = dynamic_cast<CMonsterAnim*>(m_pBodyCom->Get_Anim());

    if (pAnim && pAnim->Get_State() == EMonsterState::DEAD && pAnim->Get_DeadRotX() != 0.f)
    {
        if (m_fDeadAngleY == 0.f)
            m_fDeadAngleY = D3DXToRadian(m_pTransformCom->m_vAngle.y);

        D3DXMATRIX matRotY, matRotX, matWorld;
        D3DXMatrixRotationY(&matRotY, m_fDeadAngleY);
        D3DXMatrixRotationX(&matRotX, pAnim->Get_DeadRotX());

        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        matWorld = matRotX * matRotY;
        matWorld._41 = vPos.x;
        matWorld._42 = vPos.y;
        matWorld._43 = vPos.z;
        m_pTransformCom->Set_World(&matWorld);

        //=======Death Effect========//
        if (m_pDeathEmitter)
        {
            _vec3 vPos;
            m_pTransformCom->Get_Info(INFO_POS, &vPos);
            vPos.y += vPos.y + 1.f;
            m_pDeathEmitter->Set_Position(vPos);
        }
    }

    if (pAnim && pAnim->Get_KnockbackDelta() > 0.f)
    {
        Engine::CComponent* pCom = CManagement::GetInstance()->Get_Component(
            ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
        Engine::CTransform* pPlayerTrans = dynamic_cast<Engine::CTransform*>(pCom);

        if (pPlayerTrans)
        {
            _vec3 vMyPos, vPlayerPos;
            m_pTransformCom->Get_Info(INFO_POS, &vMyPos);
            pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

            _vec3 vKnockDir = vMyPos - vPlayerPos;
            vKnockDir.y = 0.f;
            D3DXVec3Normalize(&vKnockDir, &vKnockDir);

            float fMove = min(pAnim->Get_KnockbackDelta(), 2.f - m_fKnockbackAccum);
            if (fMove > 0.f)
            {
                m_fKnockbackAccum += fMove;
                vMyPos += vKnockDir * fMove;
                m_pTransformCom->Set_Pos(vMyPos.x, vMyPos.y, vMyPos.z);
            }
        }
    }
    else
    {
        m_fKnockbackAccum = 0.f;
    }

    if (pAnim && pAnim->Is_DeadDone())
    {
        m_bDeadDone = true;
        return true;
    }

    // 좀비/스파이더 근접 공격 콜라이더 갱신
    if (m_pAtkColliderCom && pAnim &&
        (m_eType == EMonsterType::ZOMBIE || m_eType == EMonsterType::SPIDER) &&
        pAnim->Get_State() == EMonsterState::ATTACK)
    {
        _vec3 vPos, vLook;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        m_pTransformCom->Get_Info(INFO_LOOK, &vLook);
        D3DXVec3Normalize(&vLook, &vLook);
        _vec3 vAtkPos = vPos + vLook * 0.8f;
        vAtkPos.y = vPos.y + 0.9f;
        m_pAtkColliderCom->Update_AABB(vAtkPos);
    }

    //몬스터 공격 - 플레이어 피격 충돌판정
    if (m_pAtkColliderCom)
    {
        CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
        if (pPlayer)
        {
            Engine::CCollider* pPlayerCollider = dynamic_cast<Engine::CCollider*>(
                pPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
            if (pPlayerCollider && m_pAtkColliderCom->IsColliding(pPlayerCollider->Get_AABB()))
                pPlayer->Hit();
        }
    }


    // 크리퍼 폭발 처리
    if (m_eType == EMonsterType::CREEPER && !m_bExploded)
    {
        if (pAnim && pAnim->Get_State() == EMonsterState::ATTACK
            && pAnim->Get_StateTime() >= 2.f)
        {
            _vec3 vPos;
            m_pTransformCom->Get_Info(INFO_POS, &vPos);
            m_pExplosionColliderCom->Update_AABB(vPos);
            m_bExploded = true;
            pAnim->Set_State(EMonsterState::DEAD);
        }
    }

    // 크리퍼 폭발 → 플레이어 피격
    if (m_eType == EMonsterType::CREEPER && m_bExploded && m_pExplosionColliderCom)
    {
        CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
        if (pPlayer)
        {
            Engine::CCollider* pPlayerCollider = dynamic_cast<Engine::CCollider*>(
                pPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
            if (pPlayerCollider && m_pExplosionColliderCom->IsColliding(pPlayerCollider->Get_AABB()))
                pPlayer->Hit();
        }
    }

    // 플레이어 공격 콜라이더와 충돌 체크 (몬스터 피격)
    Engine::CComponent* pAtkCom = CManagement::GetInstance()->Get_Component(
        ID_STATIC, L"GameLogic_Layer", L"Player", L"Com_AtkCollider");
    Engine::CCollider* pAtkCollider = dynamic_cast<Engine::CCollider*>(pAtkCom);

    CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();

    //====Editor용======//
    if (!pPlayer)
        return 0;

    bool bCurColliding = pPlayer->Get_AtkColliderActive() &&
        m_pColliderCom->IsColliding(pAtkCollider->Get_AABB());

    if (pAtkCollider && pAnim
        && pAnim->Get_State() != EMonsterState::HIT
        && pAnim->Get_State() != EMonsterState::DEAD
        && bCurColliding && !m_bPrevAtkColliding)
    {
        m_iHp -= 1;

        //DamageText
        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        vPos.y += 1.5f;
        CDamageMgr::GetInstance()->AddDamage(vPos, pPlayer->Get_MeleeDmg());

        if (m_iHp <= 0)
            pAnim->Set_State(EMonsterState::DEAD);
        else
            pAnim->Set_State(EMonsterState::HIT);
    }
    //===이전 프레임 상태====
    m_bPrevAtkColliding = bCurColliding;

    //if (pAtkCollider && pAnim
    //    && pAnim->Get_State() != EMonsterState::HIT
    //    && pAnim->Get_State() != EMonsterState::DEAD
    //    && pPlayer && pPlayer->Get_AtkColliderActive()
    //    && m_pColliderCom->IsColliding(pAtkCollider->Get_AABB()))
    //{
    //    m_iHp -= 1;

    //    //DamageText
    //    _vec3 vPos;
    //    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    //    vPos.y += 1.5f;
    //    CDamageMgr::GetInstance()->AddDamage(vPos, pPlayer->Get_MeleeDmg());

    //    if (m_iHp <= 0)
    //        pAnim->Set_State(EMonsterState::DEAD);
    //    else
    //        pAnim->Set_State(EMonsterState::HIT);
    //}

    // 기본화살/폭죽화살 충돌
    if (pPlayer && pAnim
        && pAnim->Get_State() != EMonsterState::HIT
        && pAnim->Get_State() != EMonsterState::DEAD)
    {
        for (auto& pArrow : pPlayer->Get_Arrows())
        {
            if (pArrow->Is_Dead() && !pArrow->Is_Exploding()) continue;
            if (pArrow->Is_Dead()) continue;

            CCollider* pArrowCollider = dynamic_cast<CCollider*>(
                pArrow->Get_Component(ID_STATIC, L"Com_Collider"));
            if (!pArrowCollider) continue;

            if (m_pColliderCom->IsColliding(pArrowCollider->Get_AABB()))
            {
                if (pArrow->Is_Firework())
                {
                    _vec3 vArrowPos;
                    AABB tAABB = pArrowCollider->Get_AABB();
                    vArrowPos = (tAABB.vMin + tAABB.vMax) * 0.5f;
                    CParticleMgr::GetInstance()->Add_Emitter(
                        CParticleEmitter::Create(m_pGraphicDev, PARTICLE_FIREWORK, vArrowPos, nullptr));
                    pArrow->Trigger_Explode();
                }
                else
                {
                    m_iHp -= 10;
                    if (m_iHp <= 0)
                        pAnim->Set_State(EMonsterState::DEAD);
                    else
                        pAnim->Set_State(EMonsterState::HIT);
                    pArrow->Set_Dead();
                }
                break;
            }
        }
    }

    //폭발 콜라이더 충돌
    if (pPlayer && pAnim
        && pAnim->Get_State() != EMonsterState::DEAD)
    {
        for (auto& pArrow : pPlayer->Get_Arrows())
        {
            if (!pArrow->Is_Exploding()) continue;

            CCollider* pExplodeCollider = pArrow->Get_ExplodeCollider();
            if (!pExplodeCollider) continue;

            if (m_pColliderCom->IsColliding(pExplodeCollider->Get_AABB()))
            {
                m_iHp -= 30;
                if (m_iHp <= 0)
                    pAnim->Set_State(EMonsterState::DEAD);
                else
                    pAnim->Set_State(EMonsterState::HIT);
            }
        }
    }

    // TNT 폭발 충돌
    if (pPlayer && pAnim && pAnim->Get_State() != EMonsterState::DEAD)
    {
        for (auto& pTNT : pPlayer->Get_TNTs())
        {
            if (!pTNT->Is_Exploding()) continue;
            CCollider* pExplodeCollider = pTNT->Get_ExplodeCollider();
            if (!pExplodeCollider) continue;
            if (m_pColliderCom->IsColliding(pExplodeCollider->Get_AABB()))
            {
                m_iHp -= 50;
                if (m_iHp <= 0)
                    pAnim->Set_State(EMonsterState::DEAD);
                else
                    pAnim->Set_State(EMonsterState::HIT);
            }
        }
    }

    m_pBodyCom->Update_Body(fTimeDelta, m_bIsMoving, false);

    // 스켈레톤 화살 처리
    if (m_eType == EMonsterType::SKELETON)
    {
        if (pAnim && pAnim->Get_State() == EMonsterState::ATTACK)
        {
            if (!m_bFired)
            {
                Fire_Arrow();
                m_bFired = true;
            }
        }
        else
        {
            m_bFired = false;
        }
        Update_Arrow(fTimeDelta);
    }

    if (m_eType == EMonsterType::SKELETON)
        CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);
    else
        CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CMonster::LateUpdate_GameObject(const _float& fTimeDelta)
{
    if (!m_bActive)
        return;

    Update_AI(fTimeDelta);

    CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CMonster::Render_GameObject()
{
    if (!m_bActive)
        return;

    CMonsterAnim* pAnim = dynamic_cast<CMonsterAnim*>(m_pBodyCom->Get_Anim());

    if (pAnim && pAnim->Is_HitFlash())
    {
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        m_pGraphicDev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_XRGB(255, 0, 0));
    }

    if (m_eType == EMonsterType::SKELETON)
    {
        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

        if (pAnim && pAnim->Get_State() == EMonsterState::DEAD)
        {
            m_pBodyCom->Render_PartsWithOffset(
                m_pTransformCom->Get_World(),
                m_pTextureCom,
                pAnim->Get_DeadOffsets()
            );
        }
        else
        {
            m_pBodyCom->Render_Body(m_pTransformCom->Get_World(), m_pTextureCom);
            Render_Bow();
        }

        m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }
    else
    {
        m_pBodyCom->Render_Body(m_pTransformCom->Get_World(), m_pTextureCom);
    }

    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    for (auto* pArrow : m_vecArrows)
    {
        if (pArrow && !pArrow->Is_Dead())
            pArrow->Render_GameObject();
    }

    if (m_pColliderCom)
        m_pColliderCom->Render_Collider();

    // 좀비/스파이더 공격 콜라이더 디버그 렌더
    if (m_pAtkColliderCom && pAnim &&
        (m_eType == EMonsterType::ZOMBIE || m_eType == EMonsterType::SPIDER) &&
        pAnim->Get_State() == EMonsterState::ATTACK)
    {
        m_pAtkColliderCom->Render_Collider();
    }

    // 크리퍼 폭발 콜라이더 디버그 렌더
    if (m_eType == EMonsterType::CREEPER && m_bExploded && m_pExplosionColliderCom)
    {
        m_pExplosionColliderCom->Render_Collider();
    }

    // 스켈레톤 화살 콜라이더 디버그 렌더
    if (m_eType == EMonsterType::SKELETON && m_pAtkColliderCom)
    {
        m_pAtkColliderCom->Render_Collider();
    }
}

void CMonster::Render_Bow()
{
    if (!m_pBowBufferCom) return;

    CMonsterAnim* pAnim = dynamic_cast<CMonsterAnim*>(m_pBodyCom->Get_Anim());
    if (!pAnim) return;

    _matrix matRArm = m_pBodyCom->Get_PartWorld(
        MonsterPart::RIGHT_ARM,
        m_pTransformCom->Get_World()
    );

    _matrix matView;
    m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);

    _matrix matBillboard;
    D3DXMatrixIdentity(&matBillboard);
    matBillboard._11 = matView._11;
    matBillboard._12 = matView._21;
    matBillboard._13 = matView._31;
    matBillboard._21 = matView._12;
    matBillboard._22 = matView._22;
    matBillboard._23 = matView._32;
    matBillboard._31 = matView._13;
    matBillboard._32 = matView._23;
    matBillboard._33 = matView._33;

    _matrix matScale, matOffset, matRotZ, matBow;
    D3DXMatrixScaling(&matScale, 0.5f, 0.5f, 0.5f);
    D3DXMatrixTranslation(&matOffset, 0.f, -0.5f, 0.f);
    D3DXMatrixRotationZ(&matRotZ, D3DXToRadian(150.f));

    matBow = matScale * matRotZ * matBillboard * matOffset * matRArm;

    Engine::CTexture* pBowTex = (pAnim->Get_State() == EMonsterState::ATTACK)
        ? m_pBowPullingTex : m_pBowStandbyTex;

    if (!pBowTex) return;

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    m_pGraphicDev->SetTransform(D3DTS_WORLD, &matBow);
    pBowTex->Set_Texture(0);
    m_pBowBufferCom->Render_Buffer();

    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CMonster::Fire_Arrow()
{
    Engine::CComponent* pCom = CManagement::GetInstance()->Get_Component(
        ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
    Engine::CTransform* pPlayerTrans = dynamic_cast<Engine::CTransform*>(pCom);
    if (!pPlayerTrans) return;

    _matrix matRArm = m_pBodyCom->Get_PartWorld(
        MonsterPart::RIGHT_ARM,
        m_pTransformCom->Get_World()
    );
    _vec3 vStartPos = { matRArm._41, matRArm._42, matRArm._43 };

    _vec3 vPlayerPos;
    pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

    _vec3 vDir = vPlayerPos - vStartPos;
    D3DXVec3Normalize(&vDir, &vDir);

    CArrow* pArrow = CArrow::Create(m_pGraphicDev, vStartPos, vDir);
    pArrow->m_iDamage = 10;
    if (pArrow)
        m_vecArrows.push_back(pArrow);
} 
void CMonster::Take_Damage(int iDamage)
{
    if (m_bDeadDone) return;

    m_iHp -= iDamage; 

    CMonsterAnim* pAnim = dynamic_cast<CMonsterAnim*>(m_pBodyCom->Get_Anim());
    if (!pAnim) return;

    //Damage Text
    _vec3 vPos = m_pTransformCom->m_vInfo[INFO_POS];
    vPos.y += 1.5f;
    CDamageMgr::GetInstance()->AddDamage(vPos, iDamage);

    if (m_iHp <= 0)
    {
        m_iHp = 0;
        pAnim->Set_State(EMonsterState::DEAD);
    }
    else
    {
        pAnim->Set_State(EMonsterState::HIT);
    }
}

void CMonster::Update_Arrow(const _float& fTimeDelta)
{
    for (auto* pArrow : m_vecArrows)
    {
        if (pArrow && !pArrow->Is_Dead())
        {
            pArrow->Update_GameObject(fTimeDelta);

            // 화살 위치로 Com_AtkCollider 갱신
            // 플레이어가 다른 몬스터와 동일하게 Com_AtkCollider로 받아감
            AABB tAABB = pArrow->Get_Collider()->Get_AABB();
            _vec3 vArrowPos = (tAABB.vMin + tAABB.vMax) * 0.5f;
            m_pAtkColliderCom->Update_AABB(vArrowPos);
        }
    }

    m_vecArrows.erase(
        remove_if(m_vecArrows.begin(), m_vecArrows.end(),
            [](CArrow* p) {
                if (p && p->Is_Dead()) { Safe_Release(p); return true; }
                return false;
            }),
        m_vecArrows.end());
}

void CMonster::Apply_Gravity(const _float& fTimeDelta)
{
    if (m_bOnGround) return;
    m_fVelocityY += m_fGravity * fTimeDelta;
    if (m_fVelocityY < m_fMaxFall)
        m_fVelocityY = m_fMaxFall;

    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    vPos.y += m_fVelocityY * fTimeDelta;
    m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
}

void CMonster::Resolve_BlockCollision()
{
    _vec3 vPos;
    m_pTransformCom->Get_Info(INFO_POS, &vPos);
    m_pColliderCom->Update_AABB(vPos);
    AABB tMonsterAABB = m_pColliderCom->Get_AABB();

    m_bOnGround = false;

    int iMinX = (int)floorf(tMonsterAABB.vMin.x);
    int iMaxX = (int)ceilf(tMonsterAABB.vMax.x);
    int iMinY = (int)floorf(tMonsterAABB.vMin.y) - 3;
    int iMaxY = (int)ceilf(tMonsterAABB.vMax.y);
    int iMinZ = (int)floorf(tMonsterAABB.vMin.z);
    int iMaxZ = (int)ceilf(tMonsterAABB.vMax.z);

    for (int y = iMinY; y <= iMaxY; ++y)
        for (int x = iMinX; x <= iMaxX; ++x)
            for (int z = iMinZ; z <= iMaxZ; ++z)
            {
                BlockPos tBlockPos = { x, y, z };
                if (!CBlockMgr::GetInstance()->HasBlock(tBlockPos)) continue;

                AABB tBlockAABB = CBlockMgr::GetInstance()->Get_BlockAABB(tBlockPos);
                if (!m_pColliderCom->IsColliding(tBlockAABB)) continue;

                _vec3 vResolve = m_pColliderCom->Resolve(tBlockAABB);
                vPos += vResolve;
                m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);
                m_pColliderCom->Update_AABB(vPos);
                tMonsterAABB = m_pColliderCom->Get_AABB();

                if (vResolve.y > 0.f)
                {
                    m_bOnGround = true;
                    m_fVelocityY = 0.f;
                }
                else if (fabsf(vResolve.y) > 0.f)
                {
                    m_fVelocityY = 0.f;
                }
            }
}

void CMonster::Update_AI(const _float& fTimeDelta)
{
    Engine::CComponent* pCom = CManagement::GetInstance()->Get_Component(
        ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
    Engine::CTransform* pPlayerTrans = dynamic_cast<Engine::CTransform*>(pCom);
    if (!pPlayerTrans) return;

    _vec3 vMyPos, vPlayerPos;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);
    pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

    _vec3 vDir = vPlayerPos - vMyPos;
    float fDist = D3DXVec3Length(&vDir);

    CMonsterAnim* pAnim = dynamic_cast<CMonsterAnim*>(m_pBodyCom->Get_Anim());
    if (!pAnim) return;
    if (pAnim->Get_State() == EMonsterState::DEAD) return;

    _vec3 vLookDir = vPlayerPos - vMyPos;
    vLookDir.y = 0.f;
    D3DXVec3Normalize(&vLookDir, &vLookDir);
    m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vLookDir.x, vLookDir.z));

    if (m_eType == EMonsterType::SKELETON)
    {
        if (fDist <= m_fDetectRange)
        {
            m_bIsMoving = false;
            pAnim->Set_State(EMonsterState::ATTACK);
        }
        else
        {
            m_bIsMoving = false;
            pAnim->Set_State(EMonsterState::IDLE);
        }
        return;
    }

    if (fDist <= m_fAttackRange)
    {
        pAnim->Set_State(EMonsterState::ATTACK);

        if (m_eType == EMonsterType::CREEPER)
        {
            m_bIsMoving = true;
            m_pTransformCom->Move_Pos(&vLookDir, m_fMoveSpeed * 2.5f, fTimeDelta);
        }
        else
        {
            m_bIsMoving = false;
        }
    }
    else if (fDist <= m_fDetectRange)
    {
        m_bIsMoving = true;
        m_pTransformCom->Move_Pos(&vLookDir, m_fMoveSpeed, fTimeDelta);
    }
    else
    {
        m_bIsMoving = false;
        pAnim->Set_State(EMonsterState::IDLE);
    }
}

HRESULT CMonster::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    // Transform
    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    // 텍스처
    const _tchar* pTexTag = nullptr;
    switch (m_eType)
    {
    case EMonsterType::ZOMBIE:   pTexTag = L"Proto_ZombieTexture";   break;
    case EMonsterType::SKELETON: pTexTag = L"Proto_SkeletonTexture"; break;
    case EMonsterType::CREEPER:  pTexTag = L"Proto_creeperTexture";  break;
    case EMonsterType::SPIDER:   pTexTag = L"Proto_SpiderTexture";   break;
    default: return E_FAIL;
    }

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(pTexTag));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    // 스켈레톤 활 관련
    if (m_eType == EMonsterType::SKELETON)
    {
        m_pBowStandbyTex = dynamic_cast<Engine::CTexture*>
            (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BowStandbyTexture"));
        m_pBowPullingTex = dynamic_cast<Engine::CTexture*>
            (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BowPullingTexture"));
        m_pBowBufferCom = dynamic_cast<Engine::CRcTex*>
            (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    }

    // 몸체
    m_pBodyCom = CMonsterBody::Create(m_pGraphicDev, m_eType);
    if (!m_pBodyCom) return E_FAIL;

    // 몸통 콜라이더
    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(1.0f, 1.8f, 1.0f),
        _vec3(0.f, 0.9f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    // 공격 콜라이더 - 4종 모두 Com_AtkCollider로 등록
    // 플레이어가 타입 구분 없이 Com_AtkCollider 하나로 받아감
    if (m_eType == EMonsterType::ZOMBIE || m_eType == EMonsterType::SPIDER)
    {
        // 근접 공격 콜라이더
        m_pAtkColliderCom = CCollider::Create(m_pGraphicDev,
            _vec3(1.2f, 0.8f, 1.2f),
            _vec3(0.f, 0.9f, 0.f));
        if (!m_pAtkColliderCom) return E_FAIL;
        m_mapComponent[ID_STATIC].insert({ L"Com_AtkCollider", m_pAtkColliderCom });
    }
    else if (m_eType == EMonsterType::CREEPER)
    {
        // 폭발 콜라이더를 AtkCollider로 등록
        m_pExplosionColliderCom = CCollider::Create(m_pGraphicDev,
            _vec3(3.f, 3.f, 3.f),
            _vec3(0.f, 0.f, 0.f));
        if (!m_pExplosionColliderCom) return E_FAIL;
        m_mapComponent[ID_STATIC].insert({ L"Com_AtkCollider", m_pExplosionColliderCom });
    }
    else if (m_eType == EMonsterType::SKELETON)
    {
        // 화살 콜라이더 - Update_Arrow()에서 화살 위치로 매 프레임 갱신
        m_pAtkColliderCom = CCollider::Create(m_pGraphicDev,
            _vec3(0.3f, 0.3f, 0.3f),
            _vec3(0.f, 0.f, 0.f));
        if (!m_pAtkColliderCom) return E_FAIL;
        m_mapComponent[ID_STATIC].insert({ L"Com_AtkCollider", m_pAtkColliderCom });
    }

    return S_OK;
}

CMonster* CMonster::Create(LPDIRECT3DDEVICE9 pGraphicDev, EMonsterType eType, _vec3 vPos)
{
    CMonster* pMonster = new CMonster(pGraphicDev);
    pMonster->m_eType = eType;

    if (FAILED(pMonster->Ready_GameObject(vPos)))
    {
        Safe_Release(pMonster);
        MSG_BOX("pMonster Create Failed");
        return nullptr;
    }

    return pMonster;
}

void CMonster::Free()
{
    //======Effect Release======//
    Safe_Release(m_pDeathEmitter);

    for (auto* pArrow : m_vecArrows)
        Safe_Release(pArrow);
    m_vecArrows.clear();

    Safe_Release(m_pExplosionColliderCom);
    Safe_Release(m_pAtkColliderCom);
    Safe_Release(m_pBowStandbyTex);
    Safe_Release(m_pBowPullingTex);
    Safe_Release(m_pBowBufferCom);
    Safe_Release(m_pBodyCom);

    CGameObject::Free();
}