#include "pch.h"
#include "CCMiniMap.h"

IMPLEMENT_SINGLETON(CCMiniMap)

CCMiniMap::CCMiniMap()
{
}

CCMiniMap::~CCMiniMap()
{
    Free();
}

HRESULT CCMiniMap::Ready_MiniMap(LPDIRECT3DDEVICE9 pGraphicDev)
{
    if (m_pGraphicDev)
        return S_OK;

    m_pGraphicDev = pGraphicDev;
    m_pGraphicDev->AddRef();

    return S_OK;
}

void CCMiniMap::Update(const _float& fTimeDelta)
{
    const auto& mapBlocks = CBlockMgr::GetInstance()->Get_Blocks();
    if (mapBlocks.empty()) return;

    // 1. 먼저 평균 위치 계산
    float fAvgX = 0.f, fAvgZ = 0.f;
    for (auto& pair : mapBlocks)
    {
        fAvgX += (float)pair.first.x;
        fAvgZ += (float)pair.first.z;
    }
    fAvgX /= (float)mapBlocks.size();
    fAvgZ /= (float)mapBlocks.size();

    // 2. 평균에서 너무 멀리 있는 블록 제외하고 범위 계산
    // 평균에서 200칸 이내 블록만 사용
    static constexpr float fMaxDist = 200.f;

    bool bFirst = true;
    for (auto& pair : mapBlocks)
    {
        float fX = (float)pair.first.x;
        float fZ = (float)pair.first.z;

        // 평균에서 너무 멀면 스킵
        if (fabsf(fX - fAvgX) > fMaxDist) continue;
        if (fabsf(fZ - fAvgZ) > fMaxDist) continue;

        if (bFirst)
        {
            m_fMinX = m_fMaxX = fX;
            m_fMinZ = m_fMaxZ = fZ;
            bFirst = false;
        }
        else
        {
            if (fX < m_fMinX) m_fMinX = fX;
            if (fX > m_fMaxX) m_fMaxX = fX;
            if (fZ < m_fMinZ) m_fMinZ = fZ;
            if (fZ > m_fMaxZ) m_fMaxZ = fZ;
        }
    }

    if (bFirst) return; // 유효한 블록 없음

    float fRangeX = m_fMaxX - m_fMinX;
    float fRangeZ = m_fMaxZ - m_fMinZ;
    if (fRangeX <= 0.f) fRangeX = 1.f;
    if (fRangeZ <= 0.f) fRangeZ = 1.f;

    float fScaleX = m_fMapWidth / fRangeX;
    float fScaleZ = m_fMapHeight / fRangeZ;
    m_fScale = min(fScaleX, fScaleZ);
    m_fScale *= 0.6f;

    float fUsedW = fRangeX * m_fScale;
    float fUsedH = fRangeZ * m_fScale;

    m_fCenterOffX = (m_fMapWidth - fUsedW) * 0.5f;
    m_fCenterOffY = (m_fMapHeight - fUsedH) * 0.5f;
}

void CCMiniMap::Render()
{
    if (!m_pGraphicDev) return;

    DWORD dwLighting, dwZEnable, dwAlpha;
    m_pGraphicDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);
    m_pGraphicDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
    m_pGraphicDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);

    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, FALSE);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_pGraphicDev->SetTexture(0, nullptr);
    m_pGraphicDev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

    Render_Background();
    Render_Tiles();
    Render_Player();

    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
    m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
}

void CCMiniMap::Render_Background()
{
    Draw_Rect(m_fMapX, m_fMapY, m_fMapWidth, m_fMapHeight,
        D3DCOLOR_ARGB(150, 0, 0, 0));
}

void CCMiniMap::Render_Tiles()
{
    const auto& mapBlocks = CBlockMgr::GetInstance()->Get_Blocks();
    if (mapBlocks.empty()) return;

    // 타일 1칸 = m_fScale 픽셀 (최소 2픽셀 보장)
    float fTileSize = max(2.f, m_fScale);

    for (auto& pair : mapBlocks)
    {
        float fWorldX = (float)pair.first.x;
        float fWorldZ = (float)pair.first.z;

        float fScreenX, fScreenY;
        World_To_MiniMap(fWorldX, fWorldZ, fScreenX, fScreenY);

        Draw_Rect(fScreenX, fScreenY, fTileSize, fTileSize,
            D3DCOLOR_ARGB(200, 150, 150, 150));
    }
}

void CCMiniMap::Render_Player()
{
    Engine::CComponent* pCom = CManagement::GetInstance()->Get_Component(
        ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Transform");
    Engine::CTransform* pTransform = dynamic_cast<Engine::CTransform*>(pCom);
    if (!pTransform) return;

    _vec3 vPlayerPos;
    pTransform->Get_Info(INFO_POS, &vPlayerPos);

    float fScreenX, fScreenY;
    World_To_MiniMap(vPlayerPos.x, vPlayerPos.z, fScreenX, fScreenY);

    // 빨간 점 (5x5 픽셀)
    Draw_Rect(fScreenX - 2.5f, fScreenY - 2.5f, 5.f, 5.f,
        D3DCOLOR_ARGB(255, 255, 0, 0));
}

void CCMiniMap::World_To_MiniMap(float fWorldX, float fWorldZ,
    float& fOutX, float& fOutY)
{
    float fRangeX = m_fMaxX - m_fMinX;
    float fRangeZ = m_fMaxZ - m_fMinZ;
    if (fRangeX <= 0.f) fRangeX = 1.f;
    if (fRangeZ <= 0.f) fRangeZ = 1.f;

    float fNormX = (fWorldX - m_fMinX) / fRangeX;
    float fNormZ = 1.f - ((fWorldZ - m_fMinZ) / fRangeZ);

    // 미니맵 중앙을 기준으로 타일 위치 계산
    float fMidX = m_fMapX + m_fMapWidth * 0.5f;
    float fMidY = m_fMapY + m_fMapHeight * 0.5f;

    float fHalfW = (fRangeX * m_fScale) * 0.5f;
    float fHalfH = (fRangeZ * m_fScale) * 0.5f;

    fOutX = fMidX - fHalfW + fNormX * fRangeX * m_fScale;
    fOutY = fMidY - fHalfH + fNormZ * fRangeZ * m_fScale;
}

void CCMiniMap::Draw_Rect(float fX, float fY, float fW, float fH, DWORD dwColor)
{
    struct Vertex { float x, y, z, rhw; DWORD color; };

    Vertex verts[4] =
    {
        { fX,      fY,      0.f, 1.f, dwColor },
        { fX + fW, fY,      0.f, 1.f, dwColor },
        { fX,      fY + fH, 0.f, 1.f, dwColor },
        { fX + fW, fY + fH, 0.f, 1.f, dwColor },
    };

    m_pGraphicDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(Vertex));
}

void CCMiniMap::Free()
{
    Safe_Release(m_pGraphicDev);
}
