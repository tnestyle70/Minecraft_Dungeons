#pragma once
#include "CGameObject.h"
#include "CCollider.h"
#include "CTransform.h"
#include "CTexture.h"
#include "CCubeBodyTex.h"
#include "CExplosionLight.h"

class CTNT : public CGameObject
{
private:
    explicit CTNT(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CTNT();
public:
    virtual HRESULT Ready_GameObject(_vec3 vPos);
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
    virtual void    Render_GameObject();
private:
    HRESULT Add_Component();
public:
    bool Is_Dead() const { return m_bDead; }
    bool Is_PickedUp() const { return m_bPickedUp; }
    void PickUp() { m_bPickedUp = true; }
    void Throw(_vec3 vPos, _vec3 vDir, float fPower);
private:
    Engine::CTransform* m_pTransformCom = nullptr;
    Engine::CTexture* m_pTopTex = nullptr;
    CCollider* m_pColliderCom = nullptr; 
    CExplosionLight* m_pExplosionLight = nullptr;

    bool  m_bDead = false;
    bool  m_bPickedUp = false;
    bool  m_bThrown = false;
    _vec3 m_vVelocity = {};
    float m_fGravity = -20.f;

    bool  m_bFusing = false;
    float m_fFuseTimer = 3.f;
    float m_fMaxFuseTime = 3.f;


    Engine::CCubeBodyTex* m_pTopCubeCom = nullptr;

    CCollider* m_pExplodeColliderCom = nullptr;
    bool m_bExploding = false;
    float m_fExplodeTimer = 0.f;

public:
    Engine::CTransform* Get_Transform() const { return m_pTransformCom; }

public:
    static CTNT* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);

    bool Is_Exploding() const { return m_bExploding; }
    CCollider* Get_ExplodeCollider() const { return m_pExplodeColliderCom; }

private:
    virtual void Free();
};