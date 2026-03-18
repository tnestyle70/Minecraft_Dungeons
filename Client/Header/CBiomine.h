#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CCollider.h"
#include "CBlockMgr.h"

// CAncientGuardian 전용 낙하 폭탄
// CCreeper의 m_pExplosionColliderCom 패턴 + CMonster의 중력/블록충돌 패턴 조합
class CBiomine : public CGameObject
{
private:
    explicit CBiomine(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CBiomine(const CGameObject& rhs);
    virtual ~CBiomine();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float fTimeDelta); // CDLCBoss와 동일하게 레퍼런스 없음
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

private:
    HRESULT         Add_Component();
    void            Apply_Gravity(const _float fTimeDelta);    // CMonster와 동일한 패턴
    void            Resolve_BlockCollision();                  // CMonster와 동일한 패턴

private:
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CCollider* m_pColliderCom = nullptr; // 낙하 중 콜라이더
    Engine::CCollider* m_pExplosionColliderCom = nullptr; // 폭발 범위 콜라이더 (CCreeper 패턴)

    // 중력
    float   m_fVelocityY = 0.f;
    float   m_fGravity = -20.f;
    float   m_fMaxFall = -20.f;
    bool    m_bOnGround = false;

    // 폭발
    float   m_fExplosionTimer = 0.f;
    float   m_fExplosionMax = 1.5f; // 폭발 지속 시간
    bool    m_bExploded = false;
    bool    m_bDead = false;

public:
    bool    Is_Dead() const { return m_bDead; }

public:
    static CBiomine* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3& vStartPos);

private:
    virtual void Free();
};