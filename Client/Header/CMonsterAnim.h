#pragma once
#include "CBodyBase.h"


enum class EMonsterType
{
    ZOMBIE,
    
};

class CMonsterAnim : public CBodyAnim
{
public:
    CMonsterAnim(EMonsterType eType)
        : m_eType(eType)
    {
        m_tPose.Initialize(6);
    }
    virtual ~CMonsterAnim() {}

public:
    virtual void Update(const _float& fTimeDelta, bool bMoving, bool bAttack) override
    {
        if (bMoving)
            m_fWalkTime += fTimeDelta;

        switch (m_eType)
        {
        case EMonsterType::ZOMBIE:
            Update_Zombie(bMoving);
            break;
        }
    }

    virtual const BodyPose& Get_Pose() override
    {
        return m_tPose;
    }

private:
    
    void Update_Zombie(bool bMoving)
    {
        float fArmFixed = D3DXToRadian(-90.f);

        float fLegSwing = bMoving
            ? D3DXToRadian(sinf(m_fWalkTime * 5.f) * 30.f)
            : 0.f;

        m_tPose.SetRot(0, 0.f, 0.f, 0.f);
        m_tPose.SetRot(1, 0.f, 0.f, 0.f);
        m_tPose.SetRot(2, fArmFixed, 0.f, 0.f);
        m_tPose.SetRot(3, fArmFixed, 0.f, 0.f);
        m_tPose.SetRot(4, -fLegSwing, 0.f, 0.f);
        m_tPose.SetRot(5, fLegSwing, 0.f, 0.f);
    }

 

private:
    EMonsterType    m_eType;
    BodyPose        m_tPose;
    float           m_fWalkTime = 0.f;
};