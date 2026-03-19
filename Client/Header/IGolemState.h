#pragma once
#include "Engine_Define.h"

class CRedStoneGolem;

class IGolemState
{
public:
	virtual ~IGolemState() = default;

public:
	virtual void Enter(CRedStoneGolem* pGolem) PURE;
	virtual void Update(CRedStoneGolem* pGolem, const _float& fTimeDelta) PURE;
	virtual void Exit(CRedStoneGolem* pGolem) PURE;

	virtual _bool Can_Transition() const { return true; }
};