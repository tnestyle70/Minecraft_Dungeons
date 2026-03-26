#include "pch.h"
#include "CAGAnim.h"
#include "CAGBody.h"
#include "CSoundMgr.h"

CAGAnim::CAGAnim()
{
    m_tPose.Initialize(8);
}

void CAGAnim::Update(const _float& fTimeDelta, bool bMoving, bool bAttack)
{
    m_fStateTime += fTimeDelta;
    m_fWaveTime += fTimeDelta;

    if (m_bHitFlash)
    {
        m_fHitTimer += fTimeDelta;
        if (m_fHitTimer >= m_fHitDuration)
        {
            m_bHitFlash = false;
            m_fHitTimer = 0.f;
            if (m_eState == EAGState::HIT)
                Set_State(EAGState::ORBIT);
        }
    }

    Update_Motion(fTimeDelta);

    if (m_eState != EAGState::DEAD)
        Update_TailWave();
}

void CAGAnim::Set_State(EAGState eState)
{
    if (m_eState == eState)         return;
    if (m_eState == EAGState::DEAD) return;

    m_eState = eState;
    m_fStateTime = 0.f;

    if (eState == EAGState::HIT)
    {
        m_bHitFlash = true;
        m_fHitTimer = 0.f;
        // 추가 : 피격 사운드
        CSoundMgr::GetInstance()->PlayEffect(L"Monster/AG_HIT.wav", 0.7f);
    }
}

void CAGAnim::Update_Motion(const _float& fTimeDelta)
{
    switch (m_eState)
    {
    case EAGState::IDLE:   Pose_Idle();             break;
    case EAGState::ORBIT:  Pose_Orbit(fTimeDelta);  break;
    case EAGState::CHARGE: Pose_Charge(fTimeDelta); break;
    case EAGState::HIT:    Pose_Hit(fTimeDelta);    break;
    case EAGState::DEAD:   Pose_Dead(fTimeDelta);   break;
    }
}

void CAGAnim::Pose_Idle()
{
    m_tPose.SetRot(AG_HEAD, 0.f, 0.f, 0.f);
    m_tPose.SetRot(AG_HEAD_SPIKE_TOP, 0.f, 0.f, 0.f);
}

void CAGAnim::Pose_Orbit(const _float& fTimeDelta)
{
    m_tPose.SetRot(AG_HEAD, 0.f, 0.f, 0.f);
    m_tPose.SetRot(AG_HEAD_SPIKE_TOP, 0.f, 0.f, 0.f);
}

void CAGAnim::Pose_Charge(const _float& fTimeDelta)
{
    m_tPose.SetRot(AG_HEAD, 0.f, 0.f, 0.f);
    m_tPose.SetRot(AG_HEAD_SPIKE_TOP, 0.f, 0.f, 0.f);
}

void CAGAnim::Pose_Hit(const _float& fTimeDelta)
{
    m_tPose.SetRot(AG_HEAD, 0.f, 0.f, 0.f);
    m_tPose.SetRot(AG_HEAD_SPIKE_TOP, 0.f, 0.f, 0.f);
}

void CAGAnim::Pose_Dead(const _float& fTimeDelta)
{
    float fRatio = min(m_fStateTime / 1.5f, 1.f);

    m_tPose.SetRot(AG_HEAD,
        D3DXToRadian(-30.f * fRatio), 0.f, 0.f);
    m_tPose.SetRot(AG_HEAD_SPIKE_TOP,
        D3DXToRadian(-45.f * fRatio), 0.f, 0.f);
    m_tPose.SetRot(AG_TAIL1,
        D3DXToRadian(20.f * fRatio), 0.f, 0.f);
    m_tPose.SetRot(AG_TAIL2,
        D3DXToRadian(40.f * fRatio), 0.f, 0.f);
    m_tPose.SetRot(AG_TAIL3,
        D3DXToRadian(60.f * fRatio), 0.f, 0.f);
    m_tPose.SetRot(AG_TAIL1_SPIKE_TOP,
        0.f, 0.f, D3DXToRadian(30.f * fRatio));
    m_tPose.SetRot(AG_TAIL2_SPIKE_TOP,
        0.f, 0.f, D3DXToRadian(45.f * fRatio));
    m_tPose.SetRot(AG_TAIL3_SPIKE_TOP,
        0.f, 0.f, D3DXToRadian(60.f * fRatio));

    if (m_fStateTime >= 1.5f)
        m_bDeadDone = true;
}

void CAGAnim::Update_TailWave()
{
    float fWave1 = sinf(m_fWaveTime * 2.5f) * D3DXToRadian(15.f);
    float fWave2 = sinf(m_fWaveTime * 2.5f + 1.0f) * D3DXToRadian(22.f);
    float fWave3 = sinf(m_fWaveTime * 2.5f + 2.0f) * D3DXToRadian(30.f);

    m_tPose.SetRot(AG_TAIL1, 0.f, fWave1, 0.f);
    m_tPose.SetRot(AG_TAIL1_SPIKE_TOP, 0.f, fWave1, 0.f);
    m_tPose.SetRot(AG_TAIL2, 0.f, fWave2, 0.f);
    m_tPose.SetRot(AG_TAIL2_SPIKE_TOP, 0.f, fWave2, 0.f);
    m_tPose.SetRot(AG_TAIL3, 0.f, fWave3, 0.f);
    m_tPose.SetRot(AG_TAIL3_SPIKE_TOP, 0.f, fWave3, 0.f);
}