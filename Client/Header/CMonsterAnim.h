#pragma once
#include "CBodyBase.h"


enum class EMonsterType
{
    ZOMBIE,
    SKELETON,
    CREEPER,
    SPIDER,

};

enum class EMonsterState { IDLE, WALK, ATTACK, HIT, DEAD, MAX };

class CMonsterAnim : public CBodyAnim

{
public:
    CMonsterAnim(EMonsterType);
    virtual ~CMonsterAnim() = default;

public:
    virtual void Update(const _float& fTimeDelta, bool bMoving, bool bAttack) override;
    virtual const BodyPose& Get_Pose() override { return m_tPose; }

    void Set_State(EMonsterState eState);
    void KeyInput();
    bool Is_HitFlash() const { return m_bHitFlash; }
    float Get_KnockbackDelta() const { return m_fKnockbackDelta; }
    float Get_DeadRotX() const { return m_fDeadRotX; } 
    EMonsterState Get_State() const { return m_eState; }  
    float Get_StateTime() const { return m_fStateTime; }
    const _vec3* Get_DeadOffsets() const { return m_vDeadOffset; } 
    bool Is_DeadDone() const { return m_bDeadDone; }// 몬스터 삭제 
  

private:
    void Update_Motion(const _float& fTimeDelta);

    void Pose_Idle();
    void Pose_Walk();
    void Pose_Attack(const _float& fTimeDelta);
    void Pose_Hit(const _float& fTimeDelta);
    void Pose_Dead(); 

   

private:
    EMonsterType  m_eType = EMonsterType::ZOMBIE;
    EMonsterState m_eState = EMonsterState::IDLE;

    BodyPose      m_tPose;


    float m_fDeadRotX = 0.f;
    float m_fKnockbackDelta = 0.f;
    
    float m_fWalkTime = 0.f;
    float m_fStateTime = 0.f;
    float m_fFlashTimer = 0.f;
    float m_fSpeed = 0.f;

    bool  m_bHitFlash = false;  

    bool m_bDeadDone = false; // 삭제 트리거 

    _vec3 m_vDeadOffset[6] = {
    { 0.f, 0.f, 0.f },
    { 0.f, 0.f, 0.f },
    { 0.f, 0.f, 0.f },
    { 0.f, 0.f, 0.f },
    { 0.f, 0.f, 0.f },
    { 0.f, 0.f, 0.f },
    };
};