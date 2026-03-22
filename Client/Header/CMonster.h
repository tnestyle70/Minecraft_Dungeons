#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CMonsterBody.h"
#include "CMonsterAnim.h"
#include "CArrow.h"


class CMonster : public CGameObject
{
private:
    explicit CMonster(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CMonster(const CGameObject& rhs);
    virtual ~CMonster();
    
public:
    virtual HRESULT     Ready_GameObject(_vec3& vPos);
    virtual _int        Update_GameObject(const _float& fTimeDelta);
    virtual void        LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void        Render_GameObject();
    


    virtual bool        Is_Dead()                   override { return m_bDeadDone; } // 몬스터 삭제 
    void                Take_Damage(int iDamage);
    int                 Get_Hp()                    const { return m_iHp; }
    int                 Get_AtkDamage()             const { return m_iAtkDamage; }

    //Get, Set Monster Active
    bool IsActive() { return m_bActive; }
    void SetActive(bool bActive) { m_bActive = bActive; }

    CCollider* Get_Collider() const { return m_pColliderCom; }

private:
    HRESULT             Add_Component();
    void                Update_Arrow(const _float& fTimeDelta);
    void                Fire_Arrow();
    void                Render_Bow();
    // 중력
    void                Apply_Gravity(const _float& fTimeDelta);
    void                Resolve_BlockCollision();
    // AI 시스템 
    void                Update_AI(const _float& fTimeDelta);
private:
    CMonsterBody* m_pBodyCom = nullptr;
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CTexture* m_pTextureCom = nullptr;

    Engine::CCollider* m_pColliderCom = nullptr; 
    Engine::CCollider* m_pAtkColliderCom = nullptr;
    Engine::CCollider* m_pExplosionColliderCom = nullptr; // 크리퍼 폭발 범위
    
    //=========Death Effect===========//
    Engine::CParticleEmitter* m_pDeathEmitter = nullptr;

    //스켈레톤 활
    Engine::CRcTex* m_pBowBufferCom = nullptr;
    Engine::CTexture* m_pBowStandbyTex = nullptr;
    Engine::CTexture* m_pBowPullingTex = nullptr;

    vector<CArrow*>         m_vecArrows;
    bool                    m_bFired = false;

    EMonsterType            m_eType = EMonsterType::ZOMBIE;
    bool                    m_bIsMoving = false;

    float                   m_fKnockbackAccum = 0.f;
    float                   m_fVelocityY = 0.f;
    float                   m_fDeadAngleY = 0.f;
    bool                    m_bOnGround = false;  
    bool                    m_bExploded = false;                // 폭발 1회 처리용
    bool                    m_bDeadDone = false; // 몬스터 삭제용도 
    int                     m_iHp = 2;
    int                     m_iAtkDamage = 10;
    
    
    static constexpr float  m_fGravity = -20.f;
    static constexpr float  m_fMaxFall = -20.f; 

    // 사정거리 감지 
    float m_fDetectRange = 20.f; // 감지할 거리
    float m_fAttackRange = 2.f;  // 공격 사정거리 
    float m_fMoveSpeed   = 2.f;  // 이동 속도  

    bool m_bActive = false;

    //===이전 프레임 Player Collider와 충돌 상태 기억===
    bool m_bPrevAtkColliding = false;

public:
    static CMonster* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        EMonsterType eType = EMonsterType::ZOMBIE, _vec3 vPos = { -1.f, 5.f, 3.f });

private:
    virtual void Free();
};