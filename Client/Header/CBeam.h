#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CCollider.h"
class CBeam : public CGameObject
{
private:
    explicit CBeam(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CBeam(const CGameObject& rhs);
    virtual ~CBeam();
public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float fTimeDelta);      
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta); 
    virtual void    Render_GameObject();                             
private:
    HRESULT         Add_Component();
private:
    Engine::CTransform* m_pTransformCom = nullptr; 
    Engine::CCollider* m_pColliderCom = nullptr;   
    Engine::CTexture* m_pTextureCom = nullptr;     
    CCubeBodyTex* m_pBufferCom = nullptr;          
    _vec3   m_vDir = { 0.f, 0.f, 1.f };            
    float   m_fSpeed = 15.f;                       
    float   m_fLifeTime = 0.f;                     
    float   m_fMaxLifeTime = 4.f;                  
    bool    m_bDead = false;                       
public:
    void    Set_Direction(const _vec3& vDir) { m_vDir = vDir; } 
    bool    Is_Dead() const { return m_bDead; }                 
    int     m_iDamage = 15;                                     
    int     Get_Damage() const { return m_iDamage; }
    void    Set_Damage(int iDamage) { m_iDamage = iDamage; }
    Engine::CCollider* Get_Collider() { return m_pColliderCom; }
    void   Set_Dead() { m_bDead = true; }
public:
    static CBeam* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3& vStartPos, const _vec3& vDir);
private:
    virtual void Free();
};