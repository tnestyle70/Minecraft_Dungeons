#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CCollider.h"
#include "CBlockMgr.h"
#include "CExplosionLight.h"


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
    void    Apply_Gravity(const _float& fTimeDelta);   
    void    Resolve_BlockCollision();                  
    void    Explode();                                 

private:
    Engine::CTransform* m_pTransformCom = nullptr; 
    Engine::CTexture* m_pTextureCom = nullptr; 
    CCubeBodyTex* m_pBufferCom = nullptr; 
    Engine::CCollider* m_pColliderCom = nullptr; 
    Engine::CCollider* m_pExplosionColliderCom = nullptr; 
    CExplosionLight* m_pExplosionLight = nullptr;

    
    float m_fVelocityY = 0.f;   
    float m_fGravity = -20.f;   
    float m_fMaxFall = -20.f;   
    bool  m_bOnGround = false;  

    
    float m_fExplodeTimer = 0.f; 
    float m_fExplodeMax = 2.f;   
    bool  m_bExploded = false;   
    bool  m_bDead = false;       

    float m_fFlashTimer = 0.f;  
    bool  m_bFlash = false;     

public:
    int   m_iDamage = 25;   
    int   Get_Damage() const { return m_iDamage; }
    void  Set_Damage(int iDamage) { m_iDamage = iDamage; }
    bool  Is_Dead()  const { return m_bDead; }
    bool  Is_Exploded() const { return m_bExploded; }                   
    Engine::CCollider* Get_ExplosionCollider() { return m_pExplosionColliderCom; } 

public:
    static CBiomine* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3& vStartPos); 
private:
    virtual void Free();
};