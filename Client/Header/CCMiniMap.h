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

private:
    LPDIRECT3DDEVICE9 m_pGraphicDev = nullptr;

    // 미니맵 화면 위치/크기 - 왼쪽 상단 고정
    static constexpr float m_fMapX = 10.f;
    static constexpr float m_fMapY = 10.f;
    static constexpr float m_fMapWidth = 200.f;
    static constexpr float m_fMapHeight = 200.f;

    // 블록 범위
    float m_fMinX = 0.f;
    float m_fMaxX = 0.f;
    float m_fMinZ = 0.f;
    float m_fMaxZ = 0.f;

    // Update()에서 계산, World_To_MiniMap/Render_Tiles 에서 공유
    float m_fScale = 1.f;  // 월드 1유닛 = 몇 픽셀
    float m_fCenterOffX = 0.f;  // 미니맵 안에서 X 중앙 정렬 오프셋
    float m_fCenterOffY = 0.f;  // 미니맵 안에서 Y 중앙 정렬 오프셋

private:
    virtual void Free();
};