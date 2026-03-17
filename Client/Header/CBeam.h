#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CCollider.h"

// CArrow와 동일한 구조 - 보스 전용 빔 투사체
// CAncientGuardian, CVengefulHeartOfEnder 공용으로 사용
class CBeam : public CGameObject
{
private:
    explicit CBeam(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CBeam(const CGameObject& rhs);
    virtual ~CBeam();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float fTimeDelta); // CDLCBoss와 동일하게 레퍼런스 없음
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();

private:
    HRESULT         Add_Component();

private:
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CCollider* m_pColliderCom = nullptr;

    _vec3   m_vDir = { 0.f, 0.f, 1.f }; // 날아갈 방향
    float   m_fSpeed = 15.f;               // 이동 속도
    float   m_fLifeTime = 0.f;                // 현재 생존 시간
    float   m_fMaxLifeTime = 4.f;                // 최대 생존 시간
    bool    m_bDead = false;              // 삭제 플래그

public:
    void    Set_Direction(const _vec3& vDir) { m_vDir = vDir; }
    bool    Is_Dead() const { return m_bDead; }

public:
    static CBeam* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3& vStartPos, const _vec3& vDir);

private:
    virtual void Free();
};