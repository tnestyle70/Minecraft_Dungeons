#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CMonsterBody.h"
#include "CMonsterAnim.h"
#include "CCollider.h"
#include "CArrow.h"

class CMonster : public CGameObject
{
private:
    explicit CMonster(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CMonster(const CGameObject& rhs);
    virtual ~CMonster();

public:
    virtual HRESULT     Ready_GameObject();
    virtual _int        Update_GameObject(const _float& fTimeDelta);
    virtual void        LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void        Render_GameObject();

private:
    HRESULT             Add_Component();
    void                Update_Arrow(const _float& fTimeDelta);
    void                Fire_Arrow();
    void                Render_Bow();

    // [추가] 중력
    void                Apply_Gravity(const _float& fTimeDelta);
    void                Resolve_BlockCollision();

private:
    CMonsterBody* m_pBodyCom = nullptr;
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CTexture* m_pTextureCom = nullptr;
    CCollider* m_pColliderCom = nullptr; 

    Engine::CRcTex* m_pBowBufferCom = nullptr;
    Engine::CTexture* m_pBowStandbyTex = nullptr;
    Engine::CTexture* m_pBowPullingTex = nullptr;

    vector<CArrow*>         m_vecArrows;
    bool                    m_bFired = false;

    EMonsterType            m_eType = EMonsterType::ZOMBIE;
    bool                    m_bIsMoving = true;

    
    float                   m_fVelocityY = 0.f;
    bool                    m_bOnGround = false;

    static constexpr float  m_fGravity = -20.f;
    static constexpr float  m_fMaxFall = -20.f;

public:
    static CMonster* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        EMonsterType eType = EMonsterType::ZOMBIE);

private:
    virtual void Free();
};