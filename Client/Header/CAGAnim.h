#pragma once
#include "CBodyBase.h"

enum class EAGState
{
    IDLE,
    ORBIT,
    CHARGE,
    DEAD,
};

class CAGAnim : public CBodyAnim
{
public:
    CAGAnim();
    virtual ~CAGAnim() = default;

public:
    virtual void            Update(const _float& fTimeDelta, bool bMoving, bool bAttack) override;
    virtual const BodyPose& Get_Pose() override { return m_tPose; }

    void        Set_State(EAGState eState);
    EAGState    Get_State()     const { return m_eState; }
    float       Get_StateTime() const { return m_fStateTime; }
    bool        Is_Dead()       const { return m_bDeadDone; }

private:
    void        Update_Motion(const _float& fTimeDelta);
    void        Update_TailWave();
    void        Pose_Idle();
    void        Pose_Orbit(const _float& fTimeDelta);
    void        Pose_Charge(const _float& fTimeDelta);
    void        Pose_Dead(const _float& fTimeDelta);

private:
    EAGState    m_eState = EAGState::IDLE;
    BodyPose    m_tPose;
    float       m_fStateTime = 0.f;
    float       m_fWaveTime = 0.f;
    bool        m_bDeadDone = false;
};