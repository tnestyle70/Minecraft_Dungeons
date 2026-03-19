#include "CRedStoneGolemArmTex.h"

CRedStoneGolemArmTex::CRedStoneGolemArmTex()
{
}

CRedStoneGolemArmTex::CRedStoneGolemArmTex(LPDIRECT3DDEVICE9 pGraphicDev)
    : CVIBuffer(pGraphicDev)
{
}

CRedStoneGolemArmTex::CRedStoneGolemArmTex(const CRedStoneGolemArmTex& rhs)
    : CVIBuffer(rhs)
{
}

CRedStoneGolemArmTex::~CRedStoneGolemArmTex()
{
}

HRESULT CRedStoneGolemArmTex::Ready_Buffer()
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

    const _float fHalfX = 0.25f;
    const _float fHalfY = 0.5f;
    const _float fHalfZ = 0.27273f;

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
    // FRONT  (px: 15,100 ~ 26,122)
    pVertex[0].vTexUV = { 0.05859f, 0.39063f };
    pVertex[1].vTexUV = { 0.10156f, 0.39063f };
    pVertex[2].vTexUV = { 0.10156f, 0.47656f };
    pVertex[3].vTexUV = { 0.05859f, 0.47656f };

    // BACK  (px: 38,100 ~ 49,122)
    pVertex[4].vTexUV = { 0.14844f, 0.39063f };
    pVertex[5].vTexUV = { 0.19141f, 0.39063f };
    pVertex[6].vTexUV = { 0.19141f, 0.47656f };
    pVertex[7].vTexUV = { 0.14844f, 0.47656f };

    // LEFT  (px: 3,100 ~ 15,122)
    pVertex[8].vTexUV = { 0.01172f, 0.39063f };
    pVertex[9].vTexUV = { 0.05859f, 0.39063f };
    pVertex[10].vTexUV = { 0.05859f, 0.47656f };
    pVertex[11].vTexUV = { 0.01172f, 0.47656f };

    // RIGHT  (px: 26,100 ~ 38,122)
    pVertex[12].vTexUV = { 0.10156f, 0.39063f };
    pVertex[13].vTexUV = { 0.14844f, 0.39063f };
    pVertex[14].vTexUV = { 0.14844f, 0.47656f };
    pVertex[15].vTexUV = { 0.10156f, 0.47656f };

    // TOP  (px: 15,88 ~ 26,100)
    pVertex[16].vTexUV = { 0.05859f, 0.34375f };
    pVertex[17].vTexUV = { 0.10156f, 0.34375f };
    pVertex[18].vTexUV = { 0.10156f, 0.39063f };
    pVertex[19].vTexUV = { 0.05859f, 0.39063f };

    // BOTTOM  (px: 26,88 ~ 37,100)
    pVertex[20].vTexUV = { 0.10156f, 0.34375f };
    pVertex[21].vTexUV = { 0.14453f, 0.34375f };
    pVertex[22].vTexUV = { 0.14453f, 0.39063f };
    pVertex[23].vTexUV = { 0.10156f, 0.39063f };


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

void CRedStoneGolemArmTex::Render_Buffer()
{
    CVIBuffer::Render_Buffer();
}

CRedStoneGolemArmTex* CRedStoneGolemArmTex::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
    CRedStoneGolemArmTex* pRedStoneGolemArmTex = new CRedStoneGolemArmTex(pGraphicDev);

    if (FAILED(pRedStoneGolemArmTex->Ready_Buffer()))
    {
        Safe_Release(pRedStoneGolemArmTex);
        MSG_BOX("RedStoneGolemArmTex Create Failed");
        return nullptr;
    }

    return pRedStoneGolemArmTex;
}

CComponent* CRedStoneGolemArmTex::Clone()
{
    return new CRedStoneGolemArmTex(*this);
}

void CRedStoneGolemArmTex::Free()
{
    CVIBuffer::Free();
}