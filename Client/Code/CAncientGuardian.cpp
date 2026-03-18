#include "pch.h"
#include "CAncientGuardian.h"
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

_int CAncientGuardian::Update_GameObject(const _float& fTimeDelta)
{
    _int iExit = CDLCBoss::Update_GameObject(fTimeDelta);

    if (!m_pBodyCom) return iExit;
    m_pBodyCom->Update_Body(fTimeDelta, false, false);

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

    return S_OK;
}

void CAncientGuardian::Update_AI(const _float& fTimeDelta)
{
    if (!m_pTransformCom) return;

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
        if (fDist > 2.f)
        {
            D3DXVec3Normalize(&vDir, &vDir);
            float fTargetAngleY = D3DXToDegree(atan2f(vDir.x, vDir.z)) + 180.f;
            float fCurAngleY = m_pTransformCom->m_vAngle.y;
            float fDiffY = fTargetAngleY - fCurAngleY;
            while (fDiffY > 180.f) fDiffY -= 360.f;
            while (fDiffY < -180.f) fDiffY += 360.f;
            float fMaxRot = 120.f * fTimeDelta;
            if (fabsf(fDiffY) < fMaxRot)
                m_pTransformCom->m_vAngle.y = fTargetAngleY;
            else
                m_pTransformCom->m_vAngle.y += (fDiffY > 0.f ? fMaxRot : -fMaxRot);
            vMyPos.x += vDir.x * 3.f * fTimeDelta;
            vMyPos.z += vDir.z * 3.f * fTimeDelta;
        }

        // 헤엄 모션 - IDLE에서만
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

        m_bFiring = true; // 수정 - ORBIT 중 항상 발사
        Update_Beams(fTimeDelta);

        if (m_fChargeCooldown <= 0.f)
        {
            m_bFiring = false; // 추가 - CHARGE 진입 시 발사 중지
            m_eState = EPufferFishState::CHARGE;
            m_vChargeTarget = vPlayerPos;
            m_fCurSpeed = 0.f;
            if (m_pBodyCom->Get_Anim())
                m_pBodyCom->Get_Anim()->Set_State(EAGState::CHARGE);
        }

        if (fDist >= m_fDetectRange)
        {
            m_bFiring = false; // 추가 - IDLE 복귀 시 발사 중지
            m_eState = EPufferFishState::IDLE;
            if (m_pBodyCom->Get_Anim())
                m_pBodyCom->Get_Anim()->Set_State(EAGState::IDLE);
        }
        break;

    case EPufferFishState::CHARGE:
        m_fHoverTime += fTimeDelta;
        Update_Charge(fTimeDelta);
        Update_Beams(fTimeDelta);
        break;
    }
}

void CAncientGuardian::Render_GameObject()
{
    if (!m_pBodyCom || !m_pTransformCom || !m_pTextureCom) return;
    m_pBodyCom->Render_Body(m_pTransformCom->Get_World(), m_pTextureCom);
}

void CAncientGuardian::Update_Orbit(const _float& fTimeDelta)
{
    Engine::CTransform* pPlayerTrans = dynamic_cast<Engine::CTransform*>(
        CManagement::GetInstance()->Get_Component(
            ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform"));
    if (!pPlayerTrans) return;

    _vec3 vPlayerPos;
    pPlayerTrans->Get_Info(INFO_POS, &vPlayerPos);

    // 각도 누적
    m_fOrbitAngle += m_fOrbitSpeed * fTimeDelta;

    // 플레이어 주변 원형 궤도 위치 계산
    _vec3 vMyPos;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);

    _vec3 vTargetPos;
    vTargetPos.x = vPlayerPos.x + cosf(m_fOrbitAngle) * m_fOrbitRadius;
    vTargetPos.y = vPlayerPos.y + m_fOrbitHeight;
    vTargetPos.z = vPlayerPos.z + sinf(m_fOrbitAngle) * m_fOrbitRadius;

    // 부드럽게 이동
    vMyPos.x += (vTargetPos.x - vMyPos.x) * 5.f * fTimeDelta;
    vMyPos.z += (vTargetPos.z - vMyPos.z) * 5.f * fTimeDelta;

    // 항상 플레이어를 바라보도록 Y 회전
    _vec3 vLookDir = vPlayerPos - vMyPos;
    vLookDir.y = 0.f;
    if (D3DXVec3Length(&vLookDir) > 0.1f)
    {
        float fTargetAngleY = D3DXToDegree(atan2f(vLookDir.x, vLookDir.z)) + 180.f;
        float fCurAngleY = m_pTransformCom->m_vAngle.y;
        float fDiffY = fTargetAngleY - fCurAngleY;
        while (fDiffY > 180.f) fDiffY -= 360.f;
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

    // 목표 도달 → ORBIT 복귀
    if (fDist < 1.5f)
    {
        m_eState = EPufferFishState::ORBIT;
        m_fChargeCooldown = m_fChargeCoolMax;
        if (m_pBodyCom->Get_Anim())
            m_pBodyCom->Get_Anim()->Set_State(EAGState::ORBIT);
        return;
    }

    // 가속 후 감속
    float fMaxSpeed = 20.f;
    if (fDist > 5.f)
        m_fCurSpeed = min(m_fCurSpeed + m_fAccel * fTimeDelta, fMaxSpeed);
    else
        m_fCurSpeed = max(m_fCurSpeed - m_fBrake * fTimeDelta, 3.f);

    D3DXVec3Normalize(&vDir, &vDir);
    vMyPos.x += vDir.x * m_fCurSpeed * fTimeDelta;
    vMyPos.z += vDir.z * m_fCurSpeed * fTimeDelta;

    // 돌진 방향으로 즉시 회전
    float fTargetAngleY = D3DXToDegree(atan2f(vDir.x, vDir.z)) + 180.f;
    m_pTransformCom->m_vAngle.y = fTargetAngleY;

    m_pTransformCom->Set_Pos(vMyPos.x, vMyPos.y, vMyPos.z);
}

void CAncientGuardian::Fire_Beam()
{
    if (!m_pTransformCom) return;

    _vec3 vMyPos, vLook;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);
    m_pTransformCom->Get_Info(INFO_LOOK, &vLook);

    // 머리 위치 계산
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
    // 연속 발사 처리
    if (m_bFiring)
    {
        m_fFireTimer += fTimeDelta;
        if (m_fFireTimer >= m_fFireInterval)
        {
            m_fFireTimer = 0.f;
            Fire_Beam();
            m_iFireCount++;

            if (m_iFireCount >= m_iFireMax) // 10발 다 쏘면 카운트 리셋
            {
                m_iFireCount = 0; // 수정 - false 대신 카운트만 리셋 (ORBIT 중 계속 발사)
            }
        }
    }

    // 가시 업데이트 + 죽은 것 제거
    for (auto iter = m_vecBeams.begin(); iter != m_vecBeams.end();)
    {
        (*iter)->Update_GameObject(fTimeDelta);
        (*iter)->LateUpdate_GameObject(fTimeDelta);

        if ((*iter)->Is_Dead())
        {
            Safe_Release(*iter);
            iter = m_vecBeams.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
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
    // 가시 정리
    for (auto* pBeam : m_vecBeams)
        Safe_Release(pBeam);
    m_vecBeams.clear();

    Safe_Release(m_pBodyCom);
    CDLCBoss::Free();
}