#pragma once
#include "CBodyBase.h"
#include "CMonsterAnim.h"

// 몬스터 파츠 인덱스
namespace MonsterPart
{
    enum
    {
        HEAD = 0,
        BODY = 1,
        RIGHT_ARM = 2,
        LEFT_ARM = 3,
        RIGHT_LEG = 4,
        LEFT_LEG = 5,
    };
}

class CMonsterBody : public CBodyBase
{
public:
    explicit CMonsterBody(LPDIRECT3DDEVICE9 pGraphicDev, EMonsterType eType);
    virtual ~CMonsterBody();
    _matrix Get_PartWorld(EBodyPart iPartIndex, const _matrix* pParentWorld); 
    void Render_PartsWithOffset(const _matrix* pParentWorld,
        Engine::CTexture* pTexture, const _vec3* pOffsets);
public:
    HRESULT Ready_Body() override;

    // 치수 데이터 - 3마리 동일
    float m_fBodyHeight = 1.1f;
    float m_fBodyWidth = 0.9f;
    float m_fHeadHeight = 0.7f;
    float m_fArmWidth = 0.25f;
    float m_fArmHeight = 0.85f;
    float m_fLegHeight = 0.90f;

public:
    static CMonsterBody* Create(LPDIRECT3DDEVICE9 pGraphicDev, EMonsterType eType);

private:
    EMonsterType    m_eType;
    virtual void    Free();
};