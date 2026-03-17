#pragma once
#include "CBodyBase.h"
#include "CAGAnim.h"

// CMonsterBody 와 동일한 구조 - CBodyBase 상속
// 파츠 인덱스
enum EAGPart
{
    AG_HEAD = 0,
    AG_HEAD_SPIKE_TOP,  // 1

    AG_TAIL1,           // 2
    AG_TAIL1_SPIKE_TOP, // 3

    AG_TAIL2,           // 4
    AG_TAIL2_SPIKE_TOP, // 5

    AG_TAIL3,           // 6
    AG_TAIL3_SPIKE_TOP, // 7

    AG_PART_END         // 8
};



class CAGBody : public CBodyBase
{
public:
    explicit CAGBody(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CAGBody() = default;

public:
    HRESULT     Ready_Body();
    CAGAnim* Get_Anim() { return dynamic_cast<CAGAnim*>(m_pAnim); }

public:
    static CAGBody* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free() override;
};