#pragma once
#include "IGolemState.h"

// -----------------------------------------------
class CGolemState_Idle : public IGolemState
{
public:
    void Enter(CRedStoneGolem* pGolem) override;
    void Update(CRedStoneGolem* pGolem, const _float& fTimeDelta) override;
    void Exit(CRedStoneGolem* pGolem) override {}
};

// -----------------------------------------------
class CGolemState_Walk : public IGolemState
{
public:
    void Enter(CRedStoneGolem* pGolem) override;
    void Update(CRedStoneGolem* pGolem, const _float& fTimeDelta) override;
    void Exit(CRedStoneGolem* pGolem) override {}
};

// -----------------------------------------------
class CGolemState_Attack : public IGolemState
{
public:
    void Enter(CRedStoneGolem* pGolem) override;
    void Update(CRedStoneGolem* pGolem, const _float& fTimeDelta) override;
    void Exit(CRedStoneGolem* pGolem) override;

    // 핵심: 공격 중엔 전환 불가
    bool Can_Transition() const override { return m_bFinished; }

private:
    bool m_bFinished = false;
    bool m_bHit = false;
};

// -----------------------------------------------
class CGolemState_Skill : public IGolemState
{
public:
    void Enter(CRedStoneGolem* pGolem) override;
    void Update(CRedStoneGolem* pGolem, const _float& fTimeDelta) override;
    void Exit(CRedStoneGolem* pGolem) override;

    bool Can_Transition() const override { return m_bFinished; }

private:
    bool m_bFinished = false;
};

// -----------------------------------------------
class CGolemState_Hit : public IGolemState
{
public:
    void Enter(CRedStoneGolem* pGolem) override;
    void Update(CRedStoneGolem* pGolem, const _float& fTimeDelta) override;
    void Exit(CRedStoneGolem* pGolem) override;

    bool Can_Transition() const override { return true; }

private:
    bool m_bFinished = false;
};

// -----------------------------------------------
class CGolemState_Dead : public IGolemState
{
public:
    void Enter(CRedStoneGolem* pGolem) override;
    void Update(CRedStoneGolem* pGolem, const _float& fTimeDelta) override;
    void Exit(CRedStoneGolem* pGolem) override {}

    // 사망은 영구 전환 불가
    bool Can_Transition() const override { return false; }
};