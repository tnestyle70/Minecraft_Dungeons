#include "pch.h"
#include "CAncientGuardian.h"
#include "CRenderer.h"
#include "CManagement.h"
#include "CFontMgr.h"
#include "CDamageMgr.h"
#include "CMonsterMgr.h"
#include "CPlayer.h"
#include "CCollider.h"

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

    // AG 보스 개별 체력 설정
    m_iHp = 300;
    m_iMaxHp = 300;

    return S_OK;
}

_int CAncientGuardian::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CDLCBoss::Update_GameObject(fTimeDelta);

    if (!m_pBodyCom) return iExit;
    m_pBodyCom->Update_Body(fTimeDelta, false, false);

    // 추가 - DEAD 상태면 Z축 회전 + 낙하 처리
    if (m_pBodyCom->Get_Anim() &&
        dynamic_cast<CAGAnim*>(m_pBodyCom->Get_Anim())->Get_State() == EAGState::DEAD)
    {
        // Z축 회전 0 → 90도
        m_fDeadRotZ += 90.f * fTimeDelta;
        if (m_fDeadRotZ > 90.f) m_fDeadRotZ = 90.f;
        m_pTransformCom->m_vAngle.z = m_fDeadRotZ;

        // 중력 낙하
        m_fDeadVelY += m_fDeadGravity * fTimeDelta;
        _vec3 vPos;
        m_pTransformCom->Get_Info(INFO_POS, &vPos);
        vPos.y += m_fDeadVelY * fTimeDelta;
        m_pTransformCom->Set_Pos(vPos.x, vPos.y, vPos.z);

        // Y < -10 이하 → 삭제
        if (vPos.y < -10.f)
            m_bDeadDone = true;
    }

    CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

    return iExit;
}

void CAncientGuardian::LateUpdate_GameObject(const _float& fTimeDelta)
{
    CDLCBoss::LateUpdate_GameObject(fTimeDelta);
}

HRESULT CAncientGuardian::Add_Component()
{
    Engine::CComponent* pComponent = nullptr;

    pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

    pComponent = m_pTextureCom = dynamic_cast<Engine::CTexture*>
        (CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_AncientGuardianTexture"));
    if (!pComponent) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

    m_pBodyCom = CAGBody::Create(m_pGraphicDev);
    if (!m_pBodyCom) return E_FAIL;

    // 몸통 콜라이더
    m_pColliderCom = CCollider::Create(m_pGraphicDev,
        _vec3(3.f, 3.f, 3.f),
        _vec3(0.f, 0.f, 0.f));
    if (!m_pColliderCom) return E_FAIL;
    m_mapComponent[ID_STATIC].insert({ L"Com_Collider", m_pColliderCom });

    return S_OK;
}

void CAncientGuardian::Update_AI(const _float& fTimeDelta)
{
    if (!m_pTransformCom) return;

    // HP 0 → DEAD 전환
    if (m_iHp <= 0)
    {
        if (m_pBodyCom->Get_Anim())
            m_pBodyCom->Get_Anim()->Set_State(EAGState::DEAD);
        return;
    }

    Engine::CTransform* pPlayerTrans = dynamic_cast<Engine::CTransform*>(
        CManagement::GetInstance()->Get_Component(
            ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));
    if (!pPlayerTrans) return;

    _vec3 vMyPos, vPlayerPos;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);
    pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

    _vec3 vDir = vPlayerPos - vMyPos;
    vDir.y = 0.f;
    float fDist = D3DXVec3Length(&vDir);

    if (m_fChargeCooldown > 0.f) m_fChargeCooldown -= fTimeDelta;

    switch (m_eState)
    {
    case EPufferFishState::IDLE:
        m_fHoverTime += fTimeDelta;
        m_pTransformCom->m_vAngle.x = sinf(m_fHoverTime * 1.5f) * 20.f;
        {
            float fTargetY = 12.5f + sinf(m_fHoverTime * 1.5f) * 2.5f;
            vMyPos.y += (fTargetY - vMyPos.y) * 3.f * fTimeDelta;
            if (vMyPos.y < 10.f) vMyPos.y = 10.f;
            if (vMyPos.y > 15.f) vMyPos.y = 15.f;
        }
        m_pTransformCom->Set_Pos(vMyPos.x, vMyPos.y, vMyPos.z);
        if (fDist < m_fDetectRange)
        {
            _vec3 vDiff = vMyPos - vPlayerPos;
            vDiff.y = 0.f;
            m_fOrbitAngle = atan2f(vDiff.z, vDiff.x);
            m_fOrbitHeight = vMyPos.y - vPlayerPos.y;
            m_eState = EPufferFishState::ORBIT;
            m_fChargeCooldown = m_fChargeCoolMax;
            if (m_pBodyCom->Get_Anim())
                m_pBodyCom->Get_Anim()->Set_State(EAGState::ORBIT);
        }
        break;

    case EPufferFishState::ORBIT:
        m_fHoverTime += fTimeDelta;
        m_pTransformCom->m_vAngle.x = sinf(m_fHoverTime * 1.5f) * 10.f;
        Update_Orbit(fTimeDelta);
        m_bFiring = true;
        Update_Beams(fTimeDelta);
        if (m_fChargeCooldown <= 0.f)
        {
            m_bFiring = false;
            _vec3 vChargeDir = vPlayerPos - vMyPos;
            vChargeDir.y = 0.f;
            D3DXVec3Normalize(&vChargeDir, &vChargeDir);
            m_vChargeTarget.x = vPlayerPos.x + vChargeDir.x * 15.f;
            m_vChargeTarget.y = vMyPos.y;
            m_vChargeTarget.z = vPlayerPos.z + vChargeDir.z * 15.f;
            m_fCurSpeed = 0.f;
            m_bDropped = false;
            m_fDropTimer = 0.f;
            m_eState = EPufferFishState::CHARGE;
            if (m_pBodyCom->Get_Anim())
                m_pBodyCom->Get_Anim()->Set_State(EAGState::CHARGE);
        }
        if (fDist >= m_fDetectRange)
        {
            m_bFiring = false;
            m_eState = EPufferFishState::IDLE;
            if (m_pBodyCom->Get_Anim())
                m_pBodyCom->Get_Anim()->Set_State(EAGState::IDLE);
        }
        break;

    case EPufferFishState::CHARGE:
        m_fHoverTime += fTimeDelta;
        Update_Charge(fTimeDelta);
        break;

    case EPufferFishState::REPOSITION:
        m_fHoverTime += fTimeDelta;
        Update_Reposition(fTimeDelta);
        break;
    }

    // 항상 바이오마인 업데이트
    Update_Biomines(fTimeDelta);
}

void CAncientGuardian::Render_GameObject()
{
    if (!m_pBodyCom || !m_pTransformCom || !m_pTextureCom) return;

    // 추가 - 피격 점멸 처리
    CAGAnim* pAnim = m_pBodyCom->Get_Anim();
    if (pAnim && pAnim->Is_HitFlash())
    {
        float fBlink = sinf(pAnim->Get_HitTimer() * D3DX_PI * 10.f);
        if (fBlink > 0.f)
        {
            m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
            m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
            m_pGraphicDev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(255, 0, 0, 255));
        }
    }

    m_pBodyCom->Render_Body(m_pTransformCom->Get_World(), m_pTextureCom);

    // 추가 - 점멸 렌더 상태 복구
    if (pAnim && pAnim->Is_HitFlash())
    {
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pGraphicDev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    }

    if (m_eState != EPufferFishState::IDLE)
    {
        _vec2 vPos{ 500.f, 10.f };
        CFontMgr::GetInstance()->Render_Font(
            L"Font_Minecraft", L"Ancient Guardian", &vPos, D3DXCOLOR(1.f, 1.f, 1.f, 1.f));
    }
}

void CAncientGuardian::Update_Orbit(const _float& fTimeDelta)
{
    Engine::CTransform* pPlayerTrans = dynamic_cast<Engine::CTransform*>(
        CManagement::GetInstance()->Get_Component(
            ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));
    if (!pPlayerTrans) return;

    _vec3 vPlayerPos;
    pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

    m_fOrbitAngle += m_fOrbitSpeed * fTimeDelta;

    _vec3 vMyPos;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);

    _vec3 vTargetPos;
    vTargetPos.x = vPlayerPos.x + cosf(m_fOrbitAngle) * m_fOrbitRadius;
    vTargetPos.y = vPlayerPos.y + m_fOrbitHeight;
    vTargetPos.z = vPlayerPos.z + sinf(m_fOrbitAngle) * m_fOrbitRadius;

    vMyPos.x += (vTargetPos.x - vMyPos.x) * 5.f * fTimeDelta;
    vMyPos.z += (vTargetPos.z - vMyPos.z) * 5.f * fTimeDelta;

    _vec3 vLookDir = vPlayerPos - vMyPos;
    vLookDir.y = 0.f;
    if (D3DXVec3Length(&vLookDir) > 0.1f)
    {
        float fTargetAngleY = D3DXToDegree(atan2f(vLookDir.x, vLookDir.z)) + 180.f;
        float fCurAngleY = m_pTransformCom->m_vAngle.y;
        float fDiffY = fTargetAngleY - fCurAngleY;
        while (fDiffY > 180.f)  fDiffY -= 360.f;
        while (fDiffY < -180.f) fDiffY += 360.f;
        float fMaxRot = 150.f * fTimeDelta;
        if (fabsf(fDiffY) < fMaxRot)
            m_pTransformCom->m_vAngle.y = fTargetAngleY;
        else
            m_pTransformCom->m_vAngle.y += (fDiffY > 0.f ? fMaxRot : -fMaxRot);
    }

    m_pTransformCom->Set_Pos(vMyPos.x, vMyPos.y, vMyPos.z);
}

void CAncientGuardian::Update_Charge(const _float& fTimeDelta)
{
    _vec3 vMyPos;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);

    _vec3 vDir = m_vChargeTarget - vMyPos;
    vDir.y = 0.f;
    float fDist = D3DXVec3Length(&vDir);

    m_fDropTimer += fTimeDelta;
    if (m_fDropTimer >= m_fDropInterval)
    {
        m_fDropTimer = 0.f;
        Drop_Biomine();
    }

    if (fDist < 1.5f)
    {
        m_eState = EPufferFishState::REPOSITION;
        m_fRepoTimer = 0.f;
        m_fDropTimer = 0.f;
        if (m_pBodyCom->Get_Anim())
            m_pBodyCom->Get_Anim()->Set_State(EAGState::IDLE);
        return;
    }

    float fMaxSpeed = 50.f;
    if (fDist > 5.f)
        m_fCurSpeed = min(m_fCurSpeed + m_fAccel * fTimeDelta, fMaxSpeed);
    else
        m_fCurSpeed = max(m_fCurSpeed - m_fBrake * fTimeDelta, 5.f);

    D3DXVec3Normalize(&vDir, &vDir);
    vMyPos.x += vDir.x * m_fCurSpeed * fTimeDelta;
    vMyPos.z += vDir.z * m_fCurSpeed * fTimeDelta;

    float fTargetAngleY = D3DXToDegree(atan2f(vDir.x, vDir.z)) + 180.f;
    m_pTransformCom->m_vAngle.y = fTargetAngleY;

    m_pTransformCom->Set_Pos(vMyPos.x, vMyPos.y, vMyPos.z);
}

void CAncientGuardian::Update_Reposition(const _float& fTimeDelta)
{
    m_fRepoTimer += fTimeDelta;

    Engine::CTransform* pPlayerTrans = dynamic_cast<Engine::CTransform*>(
        CManagement::GetInstance()->Get_Component(
            ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));
    if (pPlayerTrans)
    {
        _vec3 vMyPos, vPlayerPos;
        m_pTransformCom->Get_Info(INFO_POS, &vMyPos);
        pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

        _vec3 vLookDir = vPlayerPos - vMyPos;
        vLookDir.y = 0.f;
        if (D3DXVec3Length(&vLookDir) > 0.1f)
        {
            float fTargetAngleY = D3DXToDegree(atan2f(vLookDir.x, vLookDir.z)) + 180.f;
            float fCurAngleY = m_pTransformCom->m_vAngle.y;
            float fDiffY = fTargetAngleY - fCurAngleY;
            while (fDiffY > 180.f)  fDiffY -= 360.f;
            while (fDiffY < -180.f) fDiffY += 360.f;
            float fMaxRot = 60.f * fTimeDelta;
            if (fabsf(fDiffY) < fMaxRot)
                m_pTransformCom->m_vAngle.y = fTargetAngleY;
            else
                m_pTransformCom->m_vAngle.y += (fDiffY > 0.f ? fMaxRot : -fMaxRot);
        }
    }

    if (m_fRepoTimer >= m_fRepoMax)
    {
        m_eState = EPufferFishState::IDLE;
        if (m_pBodyCom->Get_Anim())
            m_pBodyCom->Get_Anim()->Set_State(EAGState::IDLE);
    }
}

void CAncientGuardian::Fire_Beam()
{
    if (!m_pTransformCom) return;

    _vec3 vMyPos, vLook;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);
    m_pTransformCom->Get_Info(INFO_LOOK, &vLook);

    _vec3 vHeadPos = vMyPos + vLook * 1.5f;
    vHeadPos.y += 1.5f;

    Engine::CTransform* pPlayerTrans = dynamic_cast<Engine::CTransform*>(
        CManagement::GetInstance()->Get_Component(
            ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));
    if (!pPlayerTrans) return;

    _vec3 vPlayerPos;
    pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

    _vec3 vDir = vPlayerPos - vHeadPos;
    D3DXVec3Normalize(&vDir, &vDir);

    CBeam* pBeam = CBeam::Create(m_pGraphicDev, vHeadPos, vDir);
    if (pBeam)
        m_vecBeams.push_back(pBeam);
}

void CAncientGuardian::Update_Beams(const _float& fTimeDelta)
{
    if (m_bFiring)
    {
        m_fFireTimer += fTimeDelta;
        if (m_fFireTimer >= m_fFireInterval)
        {
            m_fFireTimer = 0.f;
            Fire_Beam();
            m_iFireCount++;
            if (m_iFireCount >= m_iFireMax)
                m_iFireCount = 0;
        }
    }

    CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
    CCollider* pPlayerCollider = nullptr;
    if (pPlayer)
    {
        pPlayerCollider = dynamic_cast<CCollider*>(
            pPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
    }

    for (auto iter = m_vecBeams.begin(); iter != m_vecBeams.end();)
    {
        (*iter)->Update_GameObject(fTimeDelta);
        (*iter)->LateUpdate_GameObject(fTimeDelta);

        if (pPlayer && pPlayerCollider && !(*iter)->Is_Dead())
        {
            Engine::CCollider* pBeamCollider = (*iter)->Get_Collider();
            if (pBeamCollider &&
                pBeamCollider->IsColliding(pPlayerCollider->Get_AABB()))
            {
                pPlayer->Hit();
                (*iter)->Set_Dead();
            }
        }

        if ((*iter)->Is_Dead())
        {
            Safe_Release(*iter);
            iter = m_vecBeams.erase(iter);
        }
        else
            ++iter;
    }
}

void CAncientGuardian::Drop_Biomine()
{
    if (!m_pTransformCom) return;

    _vec3 vMyPos;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);

    CBiomine* pMine = CBiomine::Create(m_pGraphicDev, vMyPos);
    if (pMine)
        m_vecBiomines.push_back(pMine);
}

void CAncientGuardian::Update_Biomines(const _float& fTimeDelta)
{
    CPlayer* pPlayer = CMonsterMgr::GetInstance()->Get_Player();
    CCollider* pPlayerCollider = nullptr;
    if (pPlayer)
    {
        pPlayerCollider = dynamic_cast<CCollider*>(
            pPlayer->Get_Component(ID_STATIC, L"Com_Collider"));
    }

    for (auto iter = m_vecBiomines.begin(); iter != m_vecBiomines.end();)
    {
        (*iter)->Update_GameObject(fTimeDelta);
        (*iter)->LateUpdate_GameObject(fTimeDelta);

        if (pPlayer && pPlayerCollider && (*iter)->Is_Exploded())
        {
            Engine::CCollider* pExplosionCollider = (*iter)->Get_ExplosionCollider();
            if (pExplosionCollider &&
                pExplosionCollider->IsColliding(pPlayerCollider->Get_AABB()))
            {
                pPlayer->Hit();
            }
        }

        if (!(*iter)->Is_Dead())
            CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, *iter);

        if ((*iter)->Is_Dead())
        {
            Safe_Release(*iter);
            iter = m_vecBiomines.erase(iter);
        }
        else
            ++iter;
    }
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
    for (auto* pBeam : m_vecBeams)
        Safe_Release(pBeam);
    m_vecBeams.clear();

    for (auto* pMine : m_vecBiomines)
        Safe_Release(pMine);
    m_vecBiomines.clear();

    Safe_Release(m_pBodyCom);
    CDLCBoss::Free();
}