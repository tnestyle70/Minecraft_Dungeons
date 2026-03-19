#include "CRedStoneGolemShoulderTex.h"

CRedStoneGolemShoulderTex::CRedStoneGolemShoulderTex()
{
}

CRedStoneGolemShoulderTex::CRedStoneGolemShoulderTex(LPDIRECT3DDEVICE9 pGraphicDev)
    : CVIBuffer(pGraphicDev)
{
}

CRedStoneGolemShoulderTex::CRedStoneGolemShoulderTex(const CRedStoneGolemShoulderTex& rhs)
    : CVIBuffer(rhs)
{
}

CRedStoneGolemShoulderTex::~CRedStoneGolemShoulderTex()
{
}

HRESULT CRedStoneGolemShoulderTex::Ready_Buffer()
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

    const _float fHalfX = 0.29167f;
    const _float fHalfY = 0.5f;
    const _float fHalfZ = 0.25f;

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
    pVertex[16].vPosition = { -fHalfX,  fHalfY,  fHalfZ };
    pVertex[17].vPosition = { fHalfX,  fHalfY,  fHalfZ };
    pVertex[18].vPosition = { fHalfX,  fHalfY, -fHalfZ };
    pVertex[19].vPosition = { -fHalfX,  fHalfY, -fHalfZ };

    //// BOTTOM
    pVertex[20].vPosition = { -fHalfX, -fHalfY, -fHalfZ };
    pVertex[21].vPosition = { fHalfX, -fHalfY, -fHalfZ };
    pVertex[22].vPosition = { fHalfX, -fHalfY,  fHalfZ };
    pVertex[23].vPosition = { -fHalfX, -fHalfY,  fHalfZ };

    // FRONT:v0~3  BACK:v4~7  LEFT:v8~11  RIGHT:v12~15  TOP:v16~19  BOTTOM:v20~23
    // FRONT  (px: 12,64 ~ 26,88)
    pVertex[0].vTexUV = { 0.04688f, 0.25000f };
    pVertex[1].vTexUV = { 0.10156f, 0.25000f };
    pVertex[2].vTexUV = { 0.10156f, 0.34375f };
    pVertex[3].vTexUV = { 0.04688f, 0.34375f };

    // BACK  (px: 38,64 ~ 52,88)
    pVertex[4].vTexUV = { 0.14844f, 0.25000f };
    pVertex[5].vTexUV = { 0.20313f, 0.25000f };
    pVertex[6].vTexUV = { 0.20313f, 0.34375f };
    pVertex[7].vTexUV = { 0.14844f, 0.34375f };

    // LEFT  (px: 0,64 ~ 12,88)
    pVertex[8].vTexUV = { 0.00000f, 0.25000f };
    pVertex[9].vTexUV = { 0.04688f, 0.25000f };
    pVertex[10].vTexUV = { 0.04688f, 0.34375f };
    pVertex[11].vTexUV = { 0.00000f, 0.34375f };

    // RIGHT  (px: 26,64 ~ 38,88)
    pVertex[12].vTexUV = { 0.10156f, 0.25000f };
    pVertex[13].vTexUV = { 0.14844f, 0.25000f };
    pVertex[14].vTexUV = { 0.14844f, 0.34375f };
    pVertex[15].vTexUV = { 0.10156f, 0.34375f };

    // TOP  (px: 12,52 ~ 26,64)
    pVertex[16].vTexUV = { 0.04688f, 0.20313f };
    pVertex[17].vTexUV = { 0.10156f, 0.20313f };
    pVertex[18].vTexUV = { 0.10156f, 0.25000f };
    pVertex[19].vTexUV = { 0.04688f, 0.25000f };

    // BOTTOM  (px: 26,52 ~ 40,64)
    pVertex[20].vTexUV = { 0.10156f, 0.20313f };
    pVertex[21].vTexUV = { 0.15625f, 0.20313f };
    pVertex[22].vTexUV = { 0.15625f, 0.25000f };
    pVertex[23].vTexUV = { 0.10156f, 0.25000f };


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

    return S_OK;
}

void CRedStoneGolemShoulderTex::Render_Buffer()
{
    CVIBuffer::Render_Buffer();
}

CRedStoneGolemShoulderTex* CRedStoneGolemShoulderTex::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CRedStoneGolemShoulderTex* pRedStoneGolemShoulderTex = new CRedStoneGolemShoulderTex(pGraphicDev);

    if (FAILED(pRedStoneGolemShoulderTex->Ready_Buffer()))
    {
        Safe_Release(pRedStoneGolemShoulderTex);
        MSG_BOX("RedStoneGolemShoulderTex Create Failed");
        return nullptr;
    }

    return pRedStoneGolemShoulderTex;
}

CComponent* CRedStoneGolemShoulderTex::Clone()
{
    return new CRedStoneGolemShoulderTex(*this);
}

void CRedStoneGolemShoulderTex::Free()
{
    CVIBuffer::Free();
}