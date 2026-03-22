#include "CLampHeadTex.h"

CLampHeadTex::CLampHeadTex()
{
}

CLampHeadTex::CLampHeadTex(LPDIRECT3DDEVICE9 pGraphicDev)
    : CVIBuffer(pGraphicDev)
{
}

CLampHeadTex::CLampHeadTex(const CLampHeadTex& rhs)
    : CVIBuffer(rhs)
{
}

CLampHeadTex::~CLampHeadTex()
{
}

HRESULT CLampHeadTex::Ready_Buffer()
{
    m_dwVtxSize = sizeof(VTXCUBEBODY);
    m_dwVtxCnt = 24;
    m_dwTriCnt = 12;
    m_dwFVF = FVF_CUBEBODY;

    m_dwIdxSize = sizeof(INDEX32);
    m_IdxFmt = D3DFMT_INDEX32;

    if (FAILED(CVIBuffer::Ready_Buffer()))
        return E_FAIL;

    VTXCUBEBODY* pVertex = NULL;

    m_pVB->Lock(0, 0, (void**)&pVertex, 0);

    const float fHalfX = 0.30000f;  // 6/10
    const float fHalfY = 0.50000f;  // 10/10
    const float fHalfZ = 0.35000f;  // 7/10

    // FRONT
    pVertex[0].vPosition = { -fHalfX,  fHalfY, fHalfZ };
    pVertex[1].vPosition = { fHalfX,  fHalfY, fHalfZ };
    pVertex[2].vPosition = { fHalfX, -fHalfY, fHalfZ };
    pVertex[3].vPosition = { -fHalfX, -fHalfY, fHalfZ };

    //// BACK
    pVertex[4].vPosition = { fHalfX,  fHalfY,  -fHalfZ };
    pVertex[5].vPosition = { -fHalfX,  fHalfY,  -fHalfZ };
    pVertex[6].vPosition = { -fHalfX, -fHalfY,  -fHalfZ };
    pVertex[7].vPosition = { fHalfX, -fHalfY,  -fHalfZ };

    //// LEFT
    pVertex[8].vPosition = { -fHalfX,  fHalfY,  -fHalfZ };
    pVertex[9].vPosition = { -fHalfX,  fHalfY, fHalfZ };
    pVertex[10].vPosition = { -fHalfX, -fHalfY, fHalfZ };
    pVertex[11].vPosition = { -fHalfX, -fHalfY,  -fHalfZ };

    //// RIGHT
    pVertex[12].vPosition = { fHalfX,  fHalfY, fHalfZ };
    pVertex[13].vPosition = { fHalfX,  fHalfY,  -fHalfZ };
    pVertex[14].vPosition = { fHalfX, -fHalfY,  -fHalfZ };
    pVertex[15].vPosition = { fHalfX, -fHalfY, fHalfZ };

    //// TOP
    pVertex[16].vPosition = { -fHalfX,  fHalfY,  -fHalfZ };
    pVertex[17].vPosition = { fHalfX,  fHalfY,  -fHalfZ };
    pVertex[18].vPosition = { fHalfX,  fHalfY, fHalfZ };
    pVertex[19].vPosition = { -fHalfX,  fHalfY, fHalfZ };

    //// BOTTOM
    pVertex[20].vPosition = { -fHalfX, -fHalfY, fHalfZ };
    pVertex[21].vPosition = { fHalfX, -fHalfY, fHalfZ };
    pVertex[22].vPosition = { fHalfX, -fHalfY,  -fHalfZ };
    pVertex[23].vPosition = { -fHalfX, -fHalfY,  -fHalfZ };

    // FRONT:v0~3  BACK:v4~7  LEFT:v8~11  RIGHT:v12~15  TOP:v16~19  BOTTOM:v20~23
// FRONT  (px: 38,0 ~ 44,10)
    pVertex[0].vTexUV = { 0.59375f, 0.00000f };
    pVertex[1].vTexUV = { 0.68750f, 0.00000f };
    pVertex[2].vTexUV = { 0.68750f, 0.31250f };
    pVertex[3].vTexUV = { 0.59375f, 0.31250f };

    // BACK  (px: 50,0 ~ 56,10)
    pVertex[4].vTexUV = { 0.78125f, 0.00000f };
    pVertex[5].vTexUV = { 0.87500f, 0.00000f };
    pVertex[6].vTexUV = { 0.87500f, 0.31250f };
    pVertex[7].vTexUV = { 0.78125f, 0.31250f };

    // LEFT  (px: 32,0 ~ 38,10)
    pVertex[8].vTexUV = { 0.50000f, 0.00000f };
    pVertex[9].vTexUV = { 0.59375f, 0.00000f };
    pVertex[10].vTexUV = { 0.59375f, 0.31250f };
    pVertex[11].vTexUV = { 0.50000f, 0.31250f };

    // RIGHT  (px: 44,0 ~ 50,10)
    pVertex[12].vTexUV = { 0.68750f, 0.00000f };
    pVertex[13].vTexUV = { 0.78125f, 0.00000f };
    pVertex[14].vTexUV = { 0.78125f, 0.31250f };
    pVertex[15].vTexUV = { 0.68750f, 0.31250f };

    // TOP  (px: 35,10 ~ 42,17)
    pVertex[16].vTexUV = { 0.54688f, 0.31250f };
    pVertex[17].vTexUV = { 0.65625f, 0.31250f };
    pVertex[18].vTexUV = { 0.65625f, 0.53125f };
    pVertex[19].vTexUV = { 0.54688f, 0.53125f };

    // BOTTOM  (px: 42,10 ~ 49,17)
    pVertex[20].vTexUV = { 0.65625f, 0.31250f };
    pVertex[21].vTexUV = { 0.76563f, 0.31250f };
    pVertex[22].vTexUV = { 0.76563f, 0.53125f };
    pVertex[23].vTexUV = { 0.65625f, 0.53125f };

    m_pVB->Unlock();

    INDEX32* pIndex = NULL;
    m_pIB->Lock(0, 0, (void**)&pIndex, 0);

    for (int i = 0; i < 6; ++i)
    {
        int v = i * 4;
        int t = i * 2;

        pIndex[t]._0 = v + 0;
        pIndex[t]._1 = v + 1;
        pIndex[t]._2 = v + 2;

        pIndex[t + 1]._0 = v + 0;
        pIndex[t + 1]._1 = v + 2;
        pIndex[t + 1]._2 = v + 3;
    }

    m_pIB->Unlock();
}

void CLampHeadTex::Render_Buffer()
{
    CVIBuffer::Render_Buffer();
}

CLampHeadTex* CLampHeadTex::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CLampHeadTex* pLampHeadTex = new CLampHeadTex(pGraphicDev);

    if (FAILED(pLampHeadTex->Ready_Buffer()))
    {
        Safe_Release(pLampHeadTex);
        MSG_BOX("LampHeadTex Create Failed");
        return nullptr;
    }

    return pLampHeadTex;
}

CComponent* CLampHeadTex::Clone()
{
    return new CLampHeadTex(*this);
}

void CLampHeadTex::Free()
{
    CVIBuffer::Free();
}