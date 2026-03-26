#include "pch.h"
#include "CGolemStates.h"
#include "CRedStoneGolem.h"
#include "CPlayer.h"
#include "CMonsterMgr.h"
#include "CSoundMgr.h"

// ====================== IDLE ======================
void CGolemState_Idle::Enter(CRedStoneGolem* pGolem)
{
    // Idle 진입 시 특별한 처리 없음
}

void CGolemState_Idle::Update(CRedStoneGolem* pGolem, const _float& fTimeDelta)
{
    pGolem->Anim_Idle();
    pGolem->Check_Distance();   // 거리 조건에 따라 전환 요청
}

// ====================== WALK ======================
void CGolemState_Walk::Enter(CRedStoneGolem* pGolem)
{
}

void CGolemState_Walk::Update(CRedStoneGolem* pGolem, const _float& fTimeDelta)
{
    pGolem->Anim_Walk();
    pGolem->Chase_Player(fTimeDelta);
    pGolem->Check_Distance();
}

// ====================== ATTACK ======================
void CGolemState_Attack::Enter(CRedStoneGolem* pGolem)
{
    m_bFinished = false;
    pGolem->Reset_Pose();
    pGolem->Set_AnimTime(0.f);
    pGolem->LookAt_Player();
}

void CGolemState_Attack::Update(CRedStoneGolem* pGolem, const _float& fTimeDelta)
{
    pGolem->Anim_NormalAttack();

    if (pGolem->Get_AnimTime() > 0.45f && pGolem->Get_AnimTime() < 0.5f && !m_bHit)
    {
        if (pGolem->Check_AttackHit())
        {
            CPlayer* pPlayer = nullptr;
            pPlayer = CMonsterMgr::GetInstance()->Get_Player();

            if (pPlayer)
                pPlayer->Hit(pGolem->Get_Atk());

            CSoundMgr::GetInstance()->PlayEffect(L"Golem/sfx_mob_redstoneGolemAttack-001_soundWave.wav", 0.6f);
        }

        m_bHit = true;
    }

    if (pGolem->Get_AnimTime() >= 1.2f)
    {
        m_bFinished = true;
        m_bHit = false;
        pGolem->Change_State(GOLEM_STATE_IDLE);
    }
}

void CGolemState_Attack::Exit(CRedStoneGolem* pGolem)
{
    m_bFinished = false;
}

// ====================== SKILL ======================
void CGolemState_Skill::Enter(CRedStoneGolem* pGolem)
{
    m_bFinished = false;
    pGolem->Reset_Pose();
    pGolem->Set_AnimTime(0.f);
}

void CGolemState_Skill::Update(CRedStoneGolem* pGolem, const _float& fTimeDelta)
{
    pGolem->Anim_Skill();

    if (pGolem->Get_AnimTime() >= 2.4f)
    {
        m_bFinished = true;
        pGolem->Change_State(GOLEM_STATE_IDLE);
    }
}

void CGolemState_Skill::Exit(CRedStoneGolem* pGolem)
{
    m_bFinished = false;
}

// ====================== Hit ======================
void CGolemState_Hit::Enter(CRedStoneGolem* pGolem)
{
    m_bFinished = false;
    pGolem->Reset_Pose();
    pGolem->Set_AnimTime(0.f);

    pGolem->LookAt_Player();
}

void CGolemState_Hit::Update(CRedStoneGolem* pGolem, const _float& fTimeDelta)
{
    pGolem->Anim_Hit();

    if (pGolem->Get_AnimTime() >= 0.4f)
    {
        m_bFinished = true;
        pGolem->Change_State(GOLEM_STATE_IDLE);
    }
}

void CGolemState_Hit::Exit(CRedStoneGolem* pGolem)
{
    m_bFinished = false;
}

// ====================== DEAD ======================
void CGolemState_Dead::Enter(CRedStoneGolem* pGolem)
{
    CSoundMgr::GetInstance()->PlayEffect(L"Golem/sfx_mob_redstoneGolemDeathHeavy-001_soundWave.wav", 0.6f);

    pGolem->Reset_Pose();
    pGolem->Set_AnimTime(0.f);
}

void CGolemState_Dead::Update(CRedStoneGolem* pGolem, const _float& fTimeDelta)
{
    pGolem->Anim_Dead();
}