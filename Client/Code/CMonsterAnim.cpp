#include "pch.h"
#include "CMonsterAnim.h"
#include "CSoundMgr.h"

CMonsterAnim::CMonsterAnim(EMonsterType eType) : m_eType(eType)
{
    m_tPose.Initialize(6);
}

void CMonsterAnim::Update(const _float& fTimeDelta, bool bMoving, bool bAttack)
{
    m_fStateTime += fTimeDelta;

    if (m_eState == EMonsterState::IDLE && bMoving)
        Set_State(EMonsterState::WALK);
    else if (m_eState == EMonsterState::WALK && !bMoving)
        Set_State(EMonsterState::IDLE);

    KeyInput();

    if (m_eState == EMonsterState::WALK)
        m_fWalkTime += fTimeDelta;

    Update_Motion(fTimeDelta);
}

void CMonsterAnim::KeyInput()
{
    if (GetAsyncKeyState('1') & 0x8000) Set_State(EMonsterState::IDLE);
    if (GetAsyncKeyState('2') & 0x8000) Set_State(EMonsterState::WALK);
    if (GetAsyncKeyState('3') & 0x8000) Set_State(EMonsterState::ATTACK);
    if (GetAsyncKeyState('4') & 0x8000) Set_State(EMonsterState::HIT);
    if (GetAsyncKeyState('5') & 0x8000) Set_State(EMonsterState::DEAD);
}

void CMonsterAnim::Set_State(EMonsterState eState)
{
    if (m_eState == EMonsterState::DEAD) return;
    if (m_eState == eState) return;

    m_eState = eState;
    m_fStateTime = 0.f;
    m_fFlashTimer = 0.f;

    if (eState == EMonsterState::HIT)
        m_bHitFlash = true;
    else
        m_bHitFlash = false;
}

void CMonsterAnim::Update_Motion(const _float& fTimeDelta)
{
    switch (m_eState)
    {
    case EMonsterState::IDLE:   Pose_Idle();             break;
    case EMonsterState::WALK:   Pose_Walk();             break;
    case EMonsterState::ATTACK: Pose_Attack(fTimeDelta); break;
    case EMonsterState::HIT:    Pose_Hit(fTimeDelta);    break;
    case EMonsterState::DEAD:   Pose_Dead();             break;
    }
}

void CMonsterAnim::Pose_Idle()
{
    for (int i = 0; i < 6; ++i)
        m_tPose.SetRot(i, 0.f, 0.f, 0.f);
}

void CMonsterAnim::Pose_Walk()
{
    float fLegSwing = D3DXToRadian(sinf(m_fWalkTime * 5.f) * 30.f);

    m_tPose.SetRot(0, 0.f, 0.f, 0.f);
    m_tPose.SetRot(1, 0.f, 0.f, 0.f);

    if (m_eType == EMonsterType::CREEPER)
    {
        m_tPose.SetRot(2, -fLegSwing, 0.f, 0.f);
        m_tPose.SetRot(3, fLegSwing, 0.f, 0.f);
        m_tPose.SetRot(4, fLegSwing, 0.f, 0.f);
        m_tPose.SetRot(5, -fLegSwing, 0.f, 0.f);
    }
    else if (m_eType == EMonsterType::SPIDER)
    {
        m_tPose.SetRot(2, fLegSwing, 0.f, D3DXToRadian(45.f));
        m_tPose.SetRot(3, -fLegSwing, 0.f, D3DXToRadian(-45.f));
        m_tPose.SetRot(4, -fLegSwing, 0.f, D3DXToRadian(45.f));
        m_tPose.SetRot(5, fLegSwing, 0.f, D3DXToRadian(-45.f));
    }
    else
    {
        m_tPose.SetRot(2, D3DXToRadian(-90.f), 0.f, 0.f);
        m_tPose.SetRot(3, D3DXToRadian(-90.f), 0.f, 0.f);
        m_tPose.SetRot(4, -fLegSwing, 0.f, 0.f);
        m_tPose.SetRot(5, fLegSwing, 0.f, 0.f);
    }
}

void CMonsterAnim::Pose_Attack(const _float& fTimeDelta)
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
        m_tPose.SetRot(2, D3DXToRadian(-90.f), 0.f, 0.f);

        if (m_fStateTime < 0.3f)
        {
            float fPullRot = D3DXToRadian(-90.f * (m_fStateTime / 0.3f));
            m_tPose.SetRot(3, fPullRot, 0.f, 0.f);
        }
        else if (m_fStateTime < 0.8f)
        {
            m_tPose.SetRot(3, D3DXToRadian(-90.f), 0.f, 0.f);
        }
        else if (m_fStateTime < 1.1f)
        {
            float fT = (m_fStateTime - 0.8f) / 0.3f;
            float fPullRot = D3DXToRadian(-90.f * (1.f - fT));
            m_tPose.SetRot(3, fPullRot, 0.f, 0.f);
        }
    }
    else if (m_eType == EMonsterType::CREEPER)
    {
        m_fFlashTimer += fTimeDelta;
        if (m_fFlashTimer >= 0.1f)
        {
            m_bHitFlash = !m_bHitFlash;
            m_fFlashTimer = 0.f;
            // 설명 : 깜빡일 때마다 타이머 사운드 재생
            if (m_bHitFlash)
                CSoundMgr::GetInstance()->PlayEffect(L"Monster/Creeper_Timer.wav", 0.6f);
        }
    }
    else if (m_eType == EMonsterType::SPIDER)
    {
        if (m_fStateTime < 0.3f)
        {
            m_tPose.SetRot(0, 0.f, 0.f, 0.f);
            m_tPose.SetRot(1, 0.f, 0.f, 0.f);
            m_tPose.SetRot(2, D3DXToRadian(-80.f), 0.f, D3DXToRadian(30.f));
            m_tPose.SetRot(3, D3DXToRadian(-80.f), 0.f, D3DXToRadian(-30.f));
            m_tPose.SetRot(4, 0.f, 0.f, D3DXToRadian(45.f));
            m_tPose.SetRot(5, 0.f, 0.f, D3DXToRadian(-45.f));
        }
        else if (m_fStateTime < 0.6f)
        {
            for (int i = 0; i < 6; ++i)
                m_tPose.SetRot(i, 0.f, 0.f, 0.f);
        }
        else
        {
            Set_State(EMonsterState::WALK);
        }
    }

    if (m_eType != EMonsterType::CREEPER && m_fStateTime >= 1.1f)
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
        float fProgress = min(m_fStateTime / 0.1f, 1.f);
        m_fDeadRotX = D3DXToRadian(-90.f * fProgress);
        for (int i = 0; i < 6; ++i)
            m_tPose.SetRot(i, 0.f, 0.f, 0.f);

        if (fProgress >= 1.f) m_bDeadDone = true;
    }
    else if (m_eType == EMonsterType::SKELETON)
    {
        float fFallProgress = min(m_fStateTime / 0.5f, 1.f);
        float fSplitProgress = max(m_fStateTime - 0.5f, 0.f) / 1.f;
        float fSplitClamped = fSplitProgress > 1.f ? 1.f : fSplitProgress;

        m_fDeadRotX = D3DXToRadian(-90.f * fFallProgress);
        m_tPose.SetRot(0, D3DXToRadian(-270.f * fSplitClamped), 0.f, 0.f);
        m_tPose.SetRot(1, 0.f, 0.f, 0.f);
        m_tPose.SetRot(2, 0.f, 0.f, D3DXToRadian(200.f * fSplitClamped));
        m_tPose.SetRot(3, 0.f, 0.f, D3DXToRadian(-270.f * fSplitClamped));
        m_tPose.SetRot(4, D3DXToRadian(270.f * fSplitClamped), 0.f, 0.f);
        m_tPose.SetRot(5, D3DXToRadian(-270.f * fSplitClamped), 0.f, 0.f);
        m_vDeadOffset[0] = { 0.f,  0.5f, 0.f };
        m_vDeadOffset[1] = { 0.f,  0.f,  0.f };
        m_vDeadOffset[2] = { -1.0f, 0.f,  0.f };
        m_vDeadOffset[3] = { 1.0f, 0.f,  0.f };
        m_vDeadOffset[4] = { -0.9f, 0.f,  0.f };
        m_vDeadOffset[5] = { 0.9f, 0.f,  0.f };

        if (fSplitClamped >= 1.f) m_bDeadDone = true;
    }
    else if (m_eType == EMonsterType::CREEPER)
    {
        float fProgress = min(m_fStateTime / 0.3f, 1.f);
        m_fDeadRotX = D3DXToRadian(-90.f * fProgress);
        for (int i = 0; i < 6; ++i)
            m_tPose.SetRot(i, 0.f, 0.f, 0.f);

        if (fProgress >= 1.f) m_bDeadDone = true;
    }
    else if (m_eType == EMonsterType::SPIDER)
    {
        float fProgress = min(m_fStateTime / 0.3f, 1.f);
        m_fDeadRotX = D3DXToRadian(180.f * fProgress);
        for (int i = 0; i < 6; ++i)
            m_tPose.SetRot(i, 0.f, 0.f, 0.f);

        if (fProgress >= 1.f) m_bDeadDone = true;
    }
}