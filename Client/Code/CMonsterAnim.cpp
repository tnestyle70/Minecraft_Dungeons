#include "pch.h"
#include "CMonsterAnim.h"

CMonsterAnim::CMonsterAnim(EMonsterType eType) : m_eType(eType)
{
    m_tPose.Initialize(6);
}

void CMonsterAnim::Update(const _float& fTimeDelta, bool bMoving, bool bAttack)
{
    KeyInput(); // 숫자키 테스트: 1(WALK), 2(ATTACK), 3(HIT), 4(DEAD)

    m_fStateTime += fTimeDelta;

    // WALK 상태일 때만 걷기 시간 누적
    if (m_eState == EMonsterState::WALK)
        m_fWalkTime += fTimeDelta;

    Update_Motion(fTimeDelta);
}

void CMonsterAnim::KeyInput()
{
    if (GetAsyncKeyState('1') & 0x8000) Set_State(EMonsterState::WALK);
    if (GetAsyncKeyState('2') & 0x8000) Set_State(EMonsterState::ATTACK);
    if (GetAsyncKeyState('3') & 0x8000) Set_State(EMonsterState::HIT);
    if (GetAsyncKeyState('4') & 0x8000) Set_State(EMonsterState::DEAD);
}

void CMonsterAnim::Set_State(EMonsterState eState)
{
    if (m_eState == eState) return;

    // 사망 상태에서는 WALK 외 다른 상태로 전환 불가
    if (m_eState == EMonsterState::DEAD && eState != EMonsterState::WALK) return;

    m_eState = eState;
    m_fStateTime = 0.f;
    m_fFlashTimer = 0.f;

    if (eState != EMonsterState::HIT)
        m_bHitFlash = false;
}

void CMonsterAnim::Update_Motion(const _float& fTimeDelta)
{
    switch (m_eState)
    {
    case EMonsterState::WALK:   Pose_Walk();            break;
    case EMonsterState::ATTACK: Pose_Attack();          break;
    case EMonsterState::HIT:    Pose_Hit(fTimeDelta);   break;
    case EMonsterState::DEAD:   Pose_Dead();            break;
    }
}

// 1. WALK: 다리 교차 흔들기
void CMonsterAnim::Pose_Walk()
{
    float fLegSwing = D3DXToRadian(sinf(m_fWalkTime * 5.f) * 30.f);

    m_tPose.SetRot(0, 0.f, 0.f, 0.f); // 머리
    m_tPose.SetRot(1, 0.f, 0.f, 0.f); // 몸통
    m_tPose.SetRot(2, D3DXToRadian(-90.f), 0.f, 0.f); // 오른팔
    m_tPose.SetRot(3, D3DXToRadian(-90.f), 0.f, 0.f); // 왼팔
    m_tPose.SetRot(4, -fLegSwing, 0.f, 0.f); // 오른다리
    m_tPose.SetRot(5, fLegSwing, 0.f, 0.f); // 왼다리
}


void CMonsterAnim::Pose_Attack()
{
    if (m_eType == EMonsterType::ZOMBIE)
    {
       
        float fArmRot = (m_fStateTime < 0.3f)
            ? D3DXToRadian(-180.f * (m_fStateTime / 0.3f))
            : D3DXToRadian(-90.f * (1.f - (m_fStateTime - 0.3f) / 0.3f));

        m_tPose.SetRot(2, fArmRot, 0.f, 0.f);
        m_tPose.SetRot(3, fArmRot, 0.f, 0.f);
    }
    else if (m_eType == EMonsterType::SKELETON)
    {
        // 오른팔 앞으로 고정
        m_tPose.SetRot(2, D3DXToRadian(-90.f), 0.f, 0.f);

        if (m_fStateTime < 0.3f)
        {
            // 0 ~ 0.3초: 왼팔 당기기
            float fPullRot = D3DXToRadian(-90.f * (m_fStateTime / 0.3f));
            m_tPose.SetRot(3, fPullRot, 0.f, 0.f);
        }
        else if (m_fStateTime < 0.8f)
        {
            // 0.3 ~ 0.8초: 당긴 상태 유지 (딜레이)
            m_tPose.SetRot(3, D3DXToRadian(-90.f), 0.f, 0.f);
        }
        else if (m_fStateTime < 1.1f)
        {
            // 0.8 ~ 1.1초: 팔 복귀
            float fT = (m_fStateTime - 0.8f) / 0.3f;
            float fPullRot = D3DXToRadian(-90.f * (1.f - fT));
            m_tPose.SetRot(3, fPullRot, 0.f, 0.f);
        }
    }

    if (m_fStateTime >= 1.1f)
        Set_State(EMonsterState::WALK);
}

void CMonsterAnim::Pose_Hit(const _float& fTimeDelta)
{
    
    m_fKnockbackDelta = (m_fStateTime < 0.5f) ? 5.f * fTimeDelta : 0.f;

    m_fFlashTimer += fTimeDelta;
    if (m_fFlashTimer >= 0.1f)
    {
        m_bHitFlash = !m_bHitFlash;
        m_fFlashTimer = 0.f;
    }

    for (int i = 0; i < 6; ++i)
        m_tPose.SetRot(i, 0.f, 0.f, 0.f);

    if (m_fStateTime >= 0.5f)
    {
        m_fKnockbackDelta = 0.f;
        Set_State(EMonsterState::WALK);
    }
}


void CMonsterAnim::Pose_Dead()
{
    if (m_eType == EMonsterType::ZOMBIE)
    {
        // 좀비 - 기존 전체 눕기
        float fProgress = min(m_fStateTime / 0.1f, 1.f);
        m_fDeadRotX = D3DXToRadian(-90.f * fProgress);
        for (int i = 0; i < 6; ++i)
            m_tPose.SetRot(i, 0.f, 0.f, 0.f);
    }
    else if (m_eType == EMonsterType::SKELETON)
    {
        float fFallProgress = min(m_fStateTime / 0.5f, 1.f);
        float fSplitProgress = max(m_fStateTime - 0.5f, 0.f) / 1.f;
        float fSplitClamped = fSplitProgress > 1.f ? 1.f : fSplitProgress; // [추가] 회전 고정

        m_fDeadRotX = D3DXToRadian(-90.f * fFallProgress);

        m_tPose.SetRot(0, D3DXToRadian(-270.f * fSplitClamped), 0.f, 0.f);
        m_tPose.SetRot(1, 0.f, 0.f, 0.f);
        m_tPose.SetRot(2, 0.f, 0.f, D3DXToRadian(200.f * fSplitClamped));
        m_tPose.SetRot(3, 0.f, 0.f, D3DXToRadian(-270.f * fSplitClamped));
        m_tPose.SetRot(4, D3DXToRadian(270.f * fSplitClamped), 0.f, 0.f);
        m_tPose.SetRot(5, D3DXToRadian(-270.f * fSplitClamped), 0.f, 0.f);

        // [수정] 머리 x 방향으로, 거리 +0.4
        m_vDeadOffset[0] = { 0.f,   0.5f, 0.f };
        m_vDeadOffset[1] = { 0.f,   0.f,  0.f };
        m_vDeadOffset[2] = { -1.0f,  0.f,  0.f }; // -0.6f → -1.0f
        m_vDeadOffset[3] = { 1.0f,  0.f,  0.f }; //  0.6f →  1.0f
        m_vDeadOffset[4] = { -0.9f,  0.f,  0.f }; // -0.5f → -0.9f
        m_vDeadOffset[5] = { 0.9f,  0.f,  0.f }; //  0.5f →  0.9f
    }
}