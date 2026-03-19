#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CCollider.h"
#include "CBlockMgr.h"

// CAncientGuardian CHARGE 중 투하하는 폭발 지뢰
// 땅에 박힌 후 일정 시간 뒤 크리퍼처럼 폭발
class CBiomine : public CGameObject
{
private:
    explicit CBiomine(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CBiomine(const CGameObject& rhs);
    virtual ~CBiomine();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

private:
    HRESULT Add_Component();
    void    Apply_Gravity(const _float& fTimeDelta);   // CMonster 중력 코드 재사용
    void    Resolve_BlockCollision();                  // 블록 충돌 - 땅에 박히는 처리
    void    Explode();                                 // 폭발 처리 - 범위 콜라이더 활성화

private:
    Engine::CTransform* m_pTransformCom = nullptr; // 위치, 회전, 크기
    Engine::CTexture* m_pTextureCom = nullptr; // AG 텍스처 재사용
    CCubeBodyTex* m_pBufferCom = nullptr; // AG_Spike 버퍼 재사용
    Engine::CCollider* m_pColliderCom = nullptr; // 자신 충돌 콜라이더
    Engine::CCollider* m_pExplosionColliderCom = nullptr; // 폭발 범위 콜라이더

    // 낙하
    float m_fVelocityY = 0.f;   // 낙하 속도
    float m_fGravity = -20.f; // 중력 가속도
    float m_fMaxFall = -20.f; // 최대 낙하 속도
    bool  m_bOnGround = false; // 땅에 박혔는지

    // 폭발
    float m_fExplodeTimer = 0.f;  // 폭발까지 남은 시간 누적
    float m_fExplodeMax = 2.f;  // 2초 후 폭발
    bool  m_bExploded = false; // 폭발 1회 처리용
    bool  m_bDead = false; // 삭제 플래그

    // 폭발 이펙트용 플래시 타이머 (크리퍼처럼 깜빡임)
    float m_fFlashTimer = 0.f;  // 깜빡임 누적 시간
    bool  m_bFlash = false; // 현재 깜빡임 상태

public:
    int   m_iDamage = 25;   // 폭발 데미지 - 플레이어 팀원이 Get해서 사용
    bool  Is_Dead()  const { return m_bDead; }
    bool  Is_Exploded() const { return m_bExploded; }                    // 폭발 여부
    Engine::CCollider* Get_ExplosionCollider() { return m_pExplosionColliderCom; } // 폭발 범위 콜라이더

public:
    static CBiomine* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3& vStartPos); // 보스 위치에서 생성

private:
    virtual void Free();
};