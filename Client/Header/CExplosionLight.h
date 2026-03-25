#pragma once
#include "Engine_Define.h"
#include "CCubeTex.h"

class CExplosionLight
{
public:
    explicit    CExplosionLight(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);
    ~CExplosionLight();

public:
    void        Update(const _float& fTimeDelta);
    void        Render();
    bool        Is_Done() const { return m_bDone; }

private:
    LPDIRECT3DDEVICE9       m_pGraphicDev = nullptr;
    Engine::CCubeTex* m_pExplosionCube = nullptr;
    _vec3                   m_vPos = {};
    _float                  m_fTimer = 0.f;
    bool                    m_bDone = false;

   
    static constexpr int    m_iLightIndex = 7;
    static constexpr _float m_fFlashEnd = 0.05f;
    static constexpr _float m_fExpandEnd1 = 0.10f;
    static constexpr _float m_fExpandEnd2 = 0.40f;
    static constexpr _float m_fSphereEnd = 0.50f;
    static constexpr _float m_fLightEnd = 2.00f;
    static constexpr _float m_fMaxRange = 100.f;
    static constexpr _float m_fMaxScale = 15.f;
    static constexpr _float m_fMaxAlpha = 0.85f;
};