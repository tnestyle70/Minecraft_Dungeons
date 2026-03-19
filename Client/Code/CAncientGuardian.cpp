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
            // 현재 위치 기준으로 궤도 각도 초기화 - 순간이동 방지
            _vec3 vDiff = vMyPos - vPlayerPos;
            vDiff.y = 0.f;
            m_fOrbitAngle = atan2f(vDiff.z, vDiff.x);
            m_fOrbitHeight = vMyPos.y - vPlayerPos.y; // 수정 - 현재 Y 기준으로 고정

            m_eState = EPufferFishState::ORBIT;
            m_fChargeCooldown = m_fChargeCoolMax;
            if (m_pBodyCom->Get_Anim())
                m_pBodyCom->Get_Anim()->Set_State(EAGState::ORBIT);
        }
        break;

    case EPufferFishState::ORBIT:
        m_fHoverTime += fTimeDelta;
        m_pTransformCom->m_vAngle.x = sinf(m_fHoverTime * 1.5f) * 10.f;

        Update_Orbit(fTimeDelta); // 순간이동 방지 - ORBIT 상태일 때만 호출

        m_bFiring = true;
        Update_Beams(fTimeDelta);
        Update_Biomines(fTimeDelta);

        if (m_fChargeCooldown <= 0.f)
        {
            m_bFiring = false;

            _vec3 vChargeDir = vPlayerPos - vMyPos;
            vChargeDir.y = 0.f;
            D3DXVec3Normalize(&vChargeDir, &vChargeDir);

            m_vChargeTarget.x = vPlayerPos.x + vChargeDir.x * 15.f;
            m_vChargeTarget.y = vMyPos.y; // Y 고정 (위를 지나감)
            m_vChargeTarget.z = vPlayerPos.z + vChargeDir.z * 15.f;

            m_fCurSpeed = 0.f;
            m_bDropped = false; // 새 CHARGE 시작 시 투하 여부 초기화
            m_fDropTimer = 0.f;   // 투하 타이머 초기화

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
        Update_Charge(fTimeDelta); // Update_Orbit 호출 없음 - 순간이동 방지
        Update_Biomines(fTimeDelta);
        break;

    case EPufferFishState::REPOSITION: // 돌진 후 제자리 정지
        m_fHoverTime += fTimeDelta;
        Update_Reposition(fTimeDelta);
        Update_Biomines(fTimeDelta); // REPOSITION 중에도 바이오마인 업데이트
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

    m_fOrbitAngle += m_fOrbitSpeed * fTimeDelta;

    _vec3 vMyPos;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);

    _vec3 vTargetPos;
    vTargetPos.x = vPlayerPos.x + cosf(m_fOrbitAngle) * m_fOrbitRadius;
    vTargetPos.y = vPlayerPos.y + m_fOrbitHeight;
    vTargetPos.z = vPlayerPos.z + sinf(m_fOrbitAngle) * m_fOrbitRadius;

    vMyPos.x += (vTargetPos.x - vMyPos.x) * 5.f * fTimeDelta;
    //vMyPos.y += (vTargetPos.y - vMyPos.y) * 1.f * fTimeDelta; // 추가 - Y도 부드럽게 이동
    vMyPos.z += (vTargetPos.z - vMyPos.z) * 5.f * fTimeDelta;

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

    // 돌진 중 일정 간격으로 바이오마인 연속 투하
    m_fDropTimer += fTimeDelta;
    if (m_fDropTimer >= m_fDropInterval) // 0.3초마다 투하
    {
        m_fDropTimer = 0.f;
        Drop_Biomine(); // 연속 투하
    }

    // 목표 통과 후 REPOSITION으로 전환
    if (fDist < 1.5f)
    {
        m_eState = EPufferFishState::REPOSITION;
        m_fRepoTimer = 0.f; // 대기 타이머 초기화
        m_fDropTimer = 0.f; // 투하 타이머 초기화
        if (m_pBodyCom->Get_Anim())
            m_pBodyCom->Get_Anim()->Set_State(EAGState::IDLE); // 대기 애니메이션
        return;
    }

    // 빠른 돌진 속도
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
    m_fRepoTimer += fTimeDelta; // 대기 시간 누적

    // 제자리에서 플레이어 천천히 바라보기
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
            while (fDiffY > 180.f) fDiffY -= 360.f;
            while (fDiffY < -180.f) fDiffY += 360.f;

            float fMaxRot = 60.f * fTimeDelta; // 60도/초 (느린 회전)
            if (fabsf(fDiffY) < fMaxRot)
                m_pTransformCom->m_vAngle.y = fTargetAngleY;
            else
                m_pTransformCom->m_vAngle.y += (fDiffY > 0.f ? fMaxRot : -fMaxRot);
        }
    }

    // 1.5초 후 IDLE 복귀 - 다시 천천히 플레이어에게 접근 // 수정
    if (m_fRepoTimer >= m_fRepoMax)
    {
        m_eState = EPufferFishState::IDLE; // 수정 - ORBIT → IDLE
        if (m_pBodyCom->Get_Anim())
            m_pBodyCom->Get_Anim()->Set_State(EAGState::IDLE); // 수정
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
            ++iter;
    }
}

void CAncientGuardian::Drop_Biomine()
{
    if (!m_pTransformCom) return;

    // 보스 현재 위치에서 바이오마인 생성
    _vec3 vMyPos;
    m_pTransformCom->Get_Info(INFO_POS, &vMyPos);

    CBiomine* pMine = CBiomine::Create(m_pGraphicDev, vMyPos);
    if (pMine)
        m_vecBiomines.push_back(pMine);
}

void CAncientGuardian::Update_Biomines(const _float& fTimeDelta)
{
    for (auto iter = m_vecBiomines.begin(); iter != m_vecBiomines.end();)
    {
        (*iter)->Update_GameObject(fTimeDelta);
        (*iter)->LateUpdate_GameObject(fTimeDelta);

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
    // 가시 정리
    for (auto* pBeam : m_vecBeams)
        Safe_Release(pBeam);
    m_vecBeams.clear();

    // 바이오마인 정리
    for (auto* pMine : m_vecBiomines)
        Safe_Release(pMine);
    m_vecBiomines.clear();

    Safe_Release(m_pBodyCom);
    CDLCBoss::Free();
}