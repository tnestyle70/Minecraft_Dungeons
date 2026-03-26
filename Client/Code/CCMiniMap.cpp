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
    m_bCacheBuilt = false;
    m_vecBatchVerts.clear();

    if (!m_pGraphicDev)
    {
        m_pGraphicDev = pGraphicDev;
        m_pGraphicDev->AddRef();
    }

    return S_OK;
}

void CCMiniMap::Build_Cache()
{
    m_vecBatchVerts.clear();

    const auto& mapBlocks = CBlockMgr::GetInstance()->Get_Blocks();
    if (mapBlocks.empty()) return;

    // 평균 좌표 계산 (너무 먼 블록 필터링용)
    float fAvgX = 0.f, fAvgZ = 0.f;
    for (auto& pair : mapBlocks)
    {
        fAvgX += (float)pair.first.x;
        fAvgZ += (float)pair.first.z;
    }
    fAvgX /= (float)mapBlocks.size();
    fAvgZ /= (float)mapBlocks.size();

    static constexpr float fMaxDist = 400.f;
    bool bFirst = true;

    // min/max 계산
    for (auto& pair : mapBlocks)
    {
        float fX = (float)pair.first.x;
        float fZ = (float)pair.first.z;

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

    if (bFirst) return;

    float fRangeX = m_fMaxX - m_fMinX;
    float fRangeZ = m_fMaxZ - m_fMinZ;
    if (fRangeX <= 0.f) fRangeX = 1.f;
    if (fRangeZ <= 0.f) fRangeZ = 1.f;

    float fScaleX = m_fMapWidth  / fRangeX;
    float fScaleZ = m_fMapHeight / fRangeZ;
    m_fScale = min(fScaleX, fScaleZ) * 0.6f;

    float fTileSize = max(2.f, m_fScale);
    DWORD dwColor   = D3DCOLOR_ARGB(200, 150, 150, 150);

    // 블록마다 버텍스 미리 계산 (TRIANGLELIST: 사각형 1개 = 버텍스 6개)
    for (auto& pair : mapBlocks)
    {
        float fWX = (float)pair.first.x;
        float fWZ = (float)pair.first.z;

        if (fabsf(fWX - fAvgX) > fMaxDist) continue;
        if (fabsf(fWZ - fAvgZ) > fMaxDist) continue;

        float fSX, fSY;
        World_To_MiniMap(fWX, fWZ, fSX, fSY);

        float fX = fSX, fY = fSY, fW = fTileSize, fH = fTileSize;

        m_vecBatchVerts.push_back({ fX,      fY,      0.f, 1.f, dwColor });
        m_vecBatchVerts.push_back({ fX + fW, fY,      0.f, 1.f, dwColor });
        m_vecBatchVerts.push_back({ fX,      fY + fH, 0.f, 1.f, dwColor });
        m_vecBatchVerts.push_back({ fX + fW, fY,      0.f, 1.f, dwColor });
        m_vecBatchVerts.push_back({ fX + fW, fY + fH, 0.f, 1.f, dwColor });
        m_vecBatchVerts.push_back({ fX,      fY + fH, 0.f, 1.f, dwColor });
    }

    m_bCacheBuilt = true;
}

void CCMiniMap::Update(const _float& fTimeDelta)
{
    if (!m_bCacheBuilt)
        Build_Cache();
}

void CCMiniMap::Render()
{
    if (!m_pGraphicDev) return;
    if (!m_bCacheBuilt) return;

    DWORD dwLighting = 0, dwZEnable = 0, dwAlpha = 0;
    DWORD dwSrcBlend = 0, dwDstBlend = 0, dwFVF = 0;
    DWORD dwScissor = 0;

    m_pGraphicDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);
    m_pGraphicDev->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
    m_pGraphicDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
    m_pGraphicDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
    m_pGraphicDev->GetRenderState(D3DRS_DESTBLEND, &dwDstBlend);
    m_pGraphicDev->GetFVF(&dwFVF);
    m_pGraphicDev->GetRenderState(D3DRS_SCISSORTESTENABLE, &dwScissor);

    // 설명 : 미니맵 영역 밖은 렌더 안 함 → 잘림 방지
    RECT tScissor = {
        (LONG)m_fMapX,
        (LONG)m_fMapY,
        (LONG)(m_fMapX + m_fMapWidth),
        (LONG)(m_fMapY + m_fMapHeight)
    };
    m_pGraphicDev->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    m_pGraphicDev->SetScissorRect(&tScissor);

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

    // 설명 : 렌더 상태 전부 원복
    m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
    m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, dwZEnable);
    m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
    m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
    m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, dwDstBlend);
    m_pGraphicDev->SetFVF(dwFVF);
    m_pGraphicDev->SetRenderState(D3DRS_SCISSORTESTENABLE, dwScissor);
}

void CCMiniMap::Render_Background()
{
    Draw_Rect(m_fMapX, m_fMapY, m_fMapWidth, m_fMapHeight,
        D3DCOLOR_ARGB(150, 0, 0, 0));
}

void CCMiniMap::Render_Tiles()
{
    if (m_vecBatchVerts.empty()) return;

    // 캐싱된 버텍스 전체를 DrawCall 1번으로 처리
    m_pGraphicDev->DrawPrimitiveUP(
        D3DPT_TRIANGLELIST,
        (UINT)m_vecBatchVerts.size() / 3,
        m_vecBatchVerts.data(),
        sizeof(MapVertex));
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

    float fMidX   = m_fMapX + m_fMapWidth  * 0.5f;
    float fMidY   = m_fMapY + m_fMapHeight * 0.5f;
    float fHalfW  = (fRangeX * m_fScale) * 0.5f;
    float fHalfH  = (fRangeZ * m_fScale) * 0.5f;

    fOutX = fMidX - fHalfW + fNormX * fRangeX * m_fScale;
    fOutY = fMidY - fHalfH + fNormZ * fRangeZ * m_fScale;
}

void CCMiniMap::Draw_Rect(float fX, float fY, float fW, float fH, DWORD dwColor)
{
    MapVertex verts[4] =
    {
        { fX,      fY,      0.f, 1.f, dwColor },
        { fX + fW, fY,      0.f, 1.f, dwColor },
        { fX,      fY + fH, 0.f, 1.f, dwColor },
        { fX + fW, fY + fH, 0.f, 1.f, dwColor },
    };

    m_pGraphicDev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(MapVertex));
}

void CCMiniMap::Free()
{
    m_vecBatchVerts.clear();
    Safe_Release(m_pGraphicDev);
}
