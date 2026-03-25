#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CMonsterBody.h"
#include "CMonsterAnim.h"
#include "CArrow.h" 
#include "CBlockMgr.h"
#include <queue>

struct AStarNode
{
    BlockPos tPos;  // 현재 노드의 위치 
    float fG = 0.f; // 시작~ 현재 실제 비용 
    float fH = 0.f; // 현재 ~ 목표 예상 비용
    float fF = 0.f; // G + H
    BlockPos tParent; // 이 노드의 부모 위치 (역추적용)
};

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

    //A*
    vector<BlockPos>    Find_Path(BlockPos tStart, BlockPos tGoal);
    bool                IsPassable(BlockPos tPos);
    vector<BlockPos>    Get_Neighbors(BlockPos tCurrent);
    float               Heuristic(BlockPos a, BlockPos b);
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
    bool                    m_bDeadDone = false;               // 몬스터 삭제용도 
    int                     m_iHp = 2;
    int                     m_iAtkDamage = 10;
    float                   m_fExplodeDmg = 12.f;


    static constexpr float  m_fGravity = -20.f;
    static constexpr float  m_fMaxFall = -20.f;

    vector<BlockPos>       m_vecPath;               // 계산된 경로 
    int                    m_iPathIndex = 0;        // 현재 이동중인 노드 인덱스 
    float                  m_fPathTimer = 0.f;      // 경로 재계산 타이머 
    static constexpr float m_fPathInterval = 0.5f;  // 0.5초 마다 재계산

    // 사정거리 감지 
    float m_fDetectRange = 20.f; // 감지할 거리    // A* (감지거리, 탐색범위)
    float m_fAttackRange = 2.f;  // 공격 사정거리  // A* ( A* 시작조건)
    float m_fMoveSpeed = 2.f;  // 이동 속도      // 경로 이동 속도 

    bool m_bActive = false;
    bool m_bPrevMeleeColliding = false; // 몬스터 1회만 피격
    bool m_bPrevExplosionColliding = false; // 폭죽화살도 1회만 피격
public:
    static CMonster* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        EMonsterType eType = EMonsterType::ZOMBIE, _vec3 vPos = { -1.f, 5.f, 3.f });

private:
    virtual void Free();
};