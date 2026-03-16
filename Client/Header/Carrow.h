#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CArrow : public CGameObject
{
private:
    explicit CArrow(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CArrow(const CGameObject& rhs);
    virtual ~CArrow();

public:
    virtual HRESULT     Ready_GameObject();
    virtual _int        Update_GameObject(const _float& fTimeDelta);
    virtual void        LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void        Render_GameObject();

private:
    HRESULT             Add_Component();

private:
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CTexture* m_pTextureCom = nullptr;
    Engine::CRcTex* m_pBufferCom = nullptr;
    Engine::CCollider* m_pColliderCom = nullptr; // private 유지, Get_Collider()로 접근

    _vec3   m_vDir = { 0.f, 0.f, 1.f };
    float   m_fSpeed = 20.f;
    float   m_fLifeTime = 0.f;
    float   m_fMaxLifeTime = 3.f;
    bool    m_bDead = false;

public:
    void                Set_Direction(const _vec3& vDir) { m_vDir = vDir; }
    bool                Is_Dead() const { return m_bDead; }
    Engine::CCollider* Get_Collider() { return m_pColliderCom; } // Monster에서 화살 위치 접근용

public:
    static CArrow* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3& vStartPos, const _vec3& vDir);

private:
    virtual void Free();
};