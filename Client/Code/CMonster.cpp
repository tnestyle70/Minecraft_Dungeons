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

    if (m_eType == EMonsterType::SKELETON)
        CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);
    else
        CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();

    if (pAtkCollider && pAnim
        && pAnim->Get_State() != EMonsterState::HIT
        && pAnim->Get_State() != EMonsterState::DEAD
        && pPlayer && pPlayer->Get_AtkColliderActive()
        && m_pColliderCom->IsColliding(pAtkCollider->Get_AABB()))
    {
        m_iHp -= 1;

        if (m_iHp <= 0)
            pAnim->Set_State(EMonsterState::DEAD);
        else
            pAnim->Set_State(EMonsterState::HIT);
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

    float fDist = D3DXVec3Length(&(vPlayerPos - vMyPos));

    CMonsterAnim* pAnim = dynamic_cast<CMonsterAnim*>(m_pBodyCom->Get_Anim());
    if (!pAnim) return;
    if (pAnim->Get_State() == EMonsterState::DEAD) return;

    // 항상 플레이어 방향으로 회전
    _vec3 vLookDir = vPlayerPos - vMyPos;
    vLookDir.y = 0.f;
    D3DXVec3Normalize(&vLookDir, &vLookDir);
    m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vLookDir.x, vLookDir.z));

    // 스켈레톤은 기존 원거리 로직 유지
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

    // 1. DetectRange 밖 → IDLE, A* 안 돌림
    if (fDist > m_fDetectRange)
    {
        m_bIsMoving = false;
        m_vecPath.clear();
        m_iPathIndex = 0;
        pAnim->Set_State(EMonsterState::IDLE);
        return;
    }

    // 2. AttackRange 안 → 제자리 공격
    if (fDist <= m_fAttackRange)
    {
        m_bIsMoving = false;
        m_vecPath.clear();
        m_iPathIndex = 0;
        pAnim->Set_State(EMonsterState::ATTACK);
        return;
    }

    // 3. DetectRange 안 + AttackRange 밖 → A* 경로 추적으로 이동
    pAnim->Set_State(EMonsterState::WALK);

    m_fPathTimer += fTimeDelta;
    if (m_fPathTimer >= m_fPathInterval || m_vecPath.empty())
    {
        m_fPathTimer = 0.f;
        BlockPos tStart = { (int)roundf(vMyPos.x),    (int)roundf(vMyPos.y),    (int)roundf(vMyPos.z) };
        BlockPos tGoal = { (int)roundf(vPlayerPos.x),(int)roundf(vPlayerPos.y),(int)roundf(vPlayerPos.z) };
        m_vecPath = Find_Path(tStart, tGoal);
        m_iPathIndex = 0;
    }

    // 경로 없으면 직선 이동 (임시 폴백)
    if (m_vecPath.empty())
    {
        m_bIsMoving = true;
        m_pTransformCom->Move_Pos(&vLookDir, m_fMoveSpeed, fTimeDelta);
        return;
    }

    // 경로 따라 이동
    if (m_iPathIndex < (int)m_vecPath.size())
    {
        BlockPos tNext = m_vecPath[m_iPathIndex];
        _vec3    vNextPos = { (float)tNext.x, (float)tNext.y, (float)tNext.z };

        _vec3 vMoveDir = vNextPos - vMyPos;
        float fNodeDist = D3DXVec3Length(&vMoveDir);

        if (fNodeDist < 0.3f)
        {
            m_iPathIndex++;
        }
        else
        {
            _vec3 vFlatDir = { vMoveDir.x, 0.f, vMoveDir.z };
            D3DXVec3Normalize(&vFlatDir, &vFlatDir);

            _vec3 vNextWorldPos = vMyPos + vFlatDir * m_fMoveSpeed * 0.1f;
            BlockPos tNextCheck = {
                (int)roundf(vNextWorldPos.x),
                (int)roundf(vNextWorldPos.y) - 1,
                (int)roundf(vNextWorldPos.z)
            };

            if (!CBlockMgr::GetInstance()->HasBlock(tNextCheck))
            {
                m_vecPath.clear();
                m_iPathIndex = 0;
                m_bIsMoving = false;
                return;
            }

            m_bIsMoving = true;
            m_pTransformCom->Move_Pos(&vFlatDir, m_fMoveSpeed, fTimeDelta);
        }
    }
    else
    {
        m_vecPath.clear();
        m_iPathIndex = 0;
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
// A* 메인 - 시작→목표 경로 반환, 경로 없으면 빈 벡터
vector<BlockPos> CMonster::Find_Path(BlockPos tStart, BlockPos tGoal)
{
    // F값 오름차순 정렬 (priority_queue는 큰 값이 top이라 역순 비교)
    auto Compare = [](const AStarNode& a, const AStarNode& b) {return a.fF > b.fF; };

    priority_queue<AStarNode, vector<AStarNode>, decltype(Compare) > openList(Compare);
    map<BlockPos, float>    mapGCost;   // 각 노드까지의 G값
    map<BlockPos, BlockPos> mapParent;  // 경로 역추적용 부모

    // 시작 노드 추가
    AStarNode tStartNode;
    tStartNode.tPos = tStart;
    tStartNode.fG = 0.f;
    tStartNode.fH = Heuristic(tStart, tGoal);
    tStartNode.fF = tStartNode.fH;
    tStartNode.tParent = tStart;

    openList.push(tStartNode);
    mapGCost[tStart] = 0.f;
    mapParent[tStart] = tStart;

    while (!openList.empty())
    {
        AStarNode tCurrent = openList.top();
        openList.pop();

        // 목표 도달 → 역추적으로 경로 완성
        if (tCurrent.tPos.x == tGoal.x &&
            tCurrent.tPos.y == tGoal.y &&
            tCurrent.tPos.z == tGoal.z)
        {
            vector<BlockPos> vecPath;
            BlockPos tTrace = tGoal;

            while (!(tTrace.x == tStart.x &&
                tTrace.y == tStart.y &&
                tTrace.z == tStart.z))
            {
                vecPath.push_back(tTrace);
                tTrace = mapParent[tTrace];
            }

            reverse(vecPath.begin(), vecPath.end());
            return vecPath;
        }

        // 이웃 노드 탐색
        for (auto& tNeighbor : Get_Neighbors(tCurrent.tPos))
        {
            // m_fDetectRange 재활용 - 탐색 범위 제한
            // 전체 맵 탐색 대신 몬스터 주변 DetectRange 칸만 탐색
            if (abs(tNeighbor.x - tStart.x) > (int)m_fDetectRange ||
                abs(tNeighbor.y - tStart.y) > (int)m_fDetectRange ||
                abs(tNeighbor.z - tStart.z) > (int)m_fDetectRange)
                continue;

            float fNewG = tCurrent.fG + 1.f;

            auto iter = mapGCost.find(tNeighbor);
            if (iter != mapGCost.end() && fNewG >= iter->second)
                continue; // 이미 더 좋은 경로로 방문했으면 스킵

            mapGCost[tNeighbor] = fNewG;
            mapParent[tNeighbor] = tCurrent.tPos;

            AStarNode tNeighborNode;
            tNeighborNode.tPos = tNeighbor;
            tNeighborNode.fG = fNewG;
            tNeighborNode.fH = Heuristic(tNeighbor, tGoal);
            tNeighborNode.fF = fNewG + tNeighborNode.fH;
            tNeighborNode.tParent = tCurrent.tPos;

            openList.push(tNeighborNode);
        }
    }

    return {}; // 경로 없음 → IDLE 유지
}

// 발판 있고(y-1 블록) + 몸통/머리 공간 비어있어야 이동 가능
bool CMonster::IsPassable(BlockPos tPos)
{
    BlockPos tFoot = { tPos.x, tPos.y - 1, tPos.z };
    BlockPos tBody = { tPos.x, tPos.y, tPos.z };
    BlockPos tHead = { tPos.x, tPos.y - 1, tPos.z };

    return CBlockMgr::GetInstance()->HasBlock(tFoot) &&
        !CBlockMgr::GetInstance()->HasBlock(tBody) &&
        !CBlockMgr::GetInstance()->HasBlock(tHead);
}

vector<BlockPos> CMonster::Get_Neighbors(BlockPos tCurrent)
{
    vector<BlockPos> vecNeighbors;

    static const int dx[] = { 1, -1, 0,  0 };
    static const int dz[] = { 0,  0, 1, -1 };

    for (int i = 0; i < 4; ++i)
    {
        int nx = tCurrent.x + dx[i];
        int nz = tCurrent.z + dz[i];

        // 수평 이동
        BlockPos tHoriz = { nx, tCurrent.y,     nz };
        if (IsPassable(tHoriz)) vecNeighbors.push_back(tHoriz);

        // 계단 오르기 (y+1)
        BlockPos tUp = { nx, tCurrent.y + 1, nz };
        if (IsPassable(tUp))    vecNeighbors.push_back(tUp);

        // 계단 내려가기 (y-1)
        BlockPos tDown = { nx, tCurrent.y - 1, nz };
        if (IsPassable(tDown))  vecNeighbors.push_back(tDown);

    }
    return vecNeighbors;
}

float CMonster::Heuristic(BlockPos a, BlockPos b)
{
    return (float)(abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z));
}
