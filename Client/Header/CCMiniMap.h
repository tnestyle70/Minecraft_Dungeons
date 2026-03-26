#pragma once
#include "CBase.h"
#include "CBlockMgr.h"
#include "CManagement.h"

class CCMiniMap : public CBase
{
    DECLARE_SINGLETON(CCMiniMap)
private:
    explicit CCMiniMap();
    virtual ~CCMiniMap();

public:
    HRESULT Ready_MiniMap(LPDIRECT3DDEVICE9 pGraphicDev);
    void    Update(const _float& fTimeDelta);
    void    Render();

private:
    void    Render_Background();
    void    Render_Tiles();
    void    Render_Player();
    void    World_To_MiniMap(float fWorldX, float fWorldZ,
                float& fOutX, float& fOutY);
    void    Draw_Rect(float fX, float fY, float fW, float fH, DWORD dwColor);
    void    Build_Cache();

private:
    struct MapVertex { float x, y, z, rhw; DWORD color; };

private:
    LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;

    static constexpr float m_fMapX      = 10.f;
    static constexpr float m_fMapY      = 10.f;
    static constexpr float m_fMapWidth  = 200.f;
    static constexpr float m_fMapHeight = 200.f;
    
    float m_fMinX  = 0.f;
    float m_fMaxX  = 0.f;
    float m_fMinZ  = 0.f;
    float m_fMaxZ  = 0.f;
    float m_fScale = 1.f;

    vector<MapVertex> m_vecBatchVerts;
    bool              m_bCacheBuilt = false;

private:
    virtual void Free();
};