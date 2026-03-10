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

    _vec3   m_vDir = { 0.f, 0.f, 1.f }; // 날아갈 방향
    float   m_fSpeed = 20.f;               // 화살 속도
    float   m_fLifeTime = 0.f;                // 생존 시간
    float   m_fMaxLifeTime = 3.f;                // 최대 생존 시간 (3초 후 삭제)
    bool    m_bDead = false;

public:
    // 발사 방향 설정 (스켈레톤이 쏠 때 호출)
    void    Set_Direction(const _vec3& vDir) { m_vDir = vDir; }
    bool    Is_Dead() const { return m_bDead; }

public:
    static CArrow* Create(LPDIRECT3DDEVICE9 pGraphicDev, const _vec3& vStartPos, const _vec3& vDir);

private:
    virtual void Free();
};