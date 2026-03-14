#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CPlayerBody.h"

class CPlayerArrow : public CGameObject
{
public:
    CPlayerArrow(LPDIRECT3DDEVICE9 pGraphicDev);
    CPlayerArrow(const CGameObject& rhs);
    virtual ~CPlayerArrow();

public:
    virtual HRESULT Ready_GameObject()                          override;
    virtual _int    Update_GameObject(const _float& fTimeDelta) override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject()                         override;
    virtual void    Free()                                      override;

    bool            Is_Dead() const { return m_bDead; }
private:
    HRESULT Add_Component();

private:
    CTransform* m_pTransformCom = nullptr;
    CRcTex* m_pBufferCom = nullptr;
    CTexture* m_pTextureCom = nullptr;
    CCollider* m_pColliderCom = nullptr;

    _vec3  m_vDir = {};
    float  m_fSpeed = 3.f;
    float  m_fDamage = 0.f;
    float  m_fLifeTime = 0.f;
    bool   m_bDead = false;

public:
    static CPlayerArrow* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3& vPos,
        const _vec3& vDir,
        float fCharge);
};