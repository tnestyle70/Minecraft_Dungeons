#include "pch.h"
#include "CMonster.h"
#include "CManagement.h"
#include "CProtoMgr.h"
#include "CRenderer.h"
#include "CBlockMgr.h"
#include "CCollider.h"

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
    case EMonsterType::ZOMBIE:   m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z); break;
    case EMonsterType::SKELETON: m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z); break; 
    case EMonsterType::CREEPER:  m_pTransformCom->Set_Pos(-2.f, 10.f, 3.f); break;
    case EMonsterType::SPIDER:   m_pTransformCom->Set_Pos(2.f, 10.f, 3.f); break;
    }

    return S_OK;
}

_int CMonster::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CGameObject::Update_GameObject(fTimeDelta);
    Apply_Gravity(fTimeDelta);
    Resolve_BlockCollision();

    CMonsterAnim* pAnim = dynamic_cast<CMonsterAnim*>(m_pBodyCom->Get_Anim());

    if (pAnim && pAnim->Get_State() == EMonsterState::DEAD && pAnim->Get_DeadRotX() != 0.f)
    {
        // 사망 첫 프레임에 방향 저장
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

    m_pBodyCom->Update_Body(fTimeDelta, m_bIsMoving, false);
    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CMonster::LateUpdate_GameObject(const _float& fTimeDelta)
{
    Update_AI(fTimeDelta);

    CGameObject::LateUpdate_GameObject(fTimeDelta);


}

void CMonster::Render_GameObject() 
{
    
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
    else if (m_eType == EMonsterType::CREEPER)
    {
        m_pBodyCom->Render_Body(m_pTransformCom->Get_World(), m_pTextureCom); 

    } 
    else if (m_eType == EMonsterType::SPIDER)
    {
        m_pBodyCom->Render_Body(m_pTransformCom->Get_World(), m_pTextureCom);
    }
    else  // ZOMBIE
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
        ID_DYNAMIC,
        L"GameLogic_Layer",
        L"Player",
        L"Com_Transform");

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
    if (pArrow)
        m_vecArrows.push_back(pArrow);
}

void CMonster::Update_Arrow(const _float& fTimeDelta)
{
    for (auto* pArrow : m_vecArrows)
    {
        if (pArrow && !pArrow->Is_Dead())
            pArrow->Update_GameObject(fTimeDelta);
    }

    m_vecArrows.erase(
        remove_if(m_vecArrows.begin(), m_vecArrows.end(),
            [](CArrow* p) {
                if (p && p->Is_Dead()) { Safe_Release(p); return true; }
                return false;
            }),
        m_vecArrows.end());
}

// [추가] 중력 적용
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

// [추가] 블록 충돌
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
    Engine::CComponent* pCom = CManagement::GetInstance()->Get_Component(ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
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

    // 항상 플레이어 방향 바라보기
    _vec3 vLookDir = vPlayerPos - vMyPos;
    vLookDir.y = 0.f;
    D3DXVec3Normalize(&vLookDir, &vLookDir);
    m_pTransformCom->m_vAngle.y = D3DXToDegree(atan2f(vLookDir.x, vLookDir.z));

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

    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

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

    if (m_eType == EMonsterType::SKELETON)
    {
        m_pBowStandbyTex = dynamic_cast<Engine::CTexture*>
            (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BowStandbyTexture"));

        m_pBowPullingTex = dynamic_cast<Engine::CTexture*>
            (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BowPullingTexture"));

        m_pBowBufferCom = dynamic_cast<Engine::CRcTex*>
            (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
    }

    m_pBodyCom = CMonsterBody::Create(m_pGraphicDev, m_eType);
    if (!m_pBodyCom) return E_FAIL;

    // [추가] 콜라이더
    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(0.5f, 1.8f, 0.5f),  
        _vec3(0.f, 0.9f, 0.f));  
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

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
    for (auto* pArrow : m_vecArrows)
        Safe_Release(pArrow);
    m_vecArrows.clear();

    Safe_Release(m_pBowStandbyTex);
    Safe_Release(m_pBowPullingTex);
    Safe_Release(m_pBowBufferCom);
    Safe_Release(m_pBodyCom);
    CGameObject::Free();
}