#include "CBossTex.h"

CBossTex::CBossTex()
{
}

CBossTex::CBossTex(LPDIRECT3DDEVICE9 pGraphicDev)
	: CVIBuffer(pGraphicDev)
{
}

CBossTex::CBossTex(const CBossTex& rhs)
	: CVIBuffer(rhs)
{
}

CBossTex::~CBossTex()
{
}

HRESULT CBossTex::Ready_Buffer()
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

    // FRONT
    pVertex[0].vPosition = { -1.f,  0.8f, -0.5f };
    pVertex[1].vPosition = { 1.f,  0.8f, -0.5f };
    pVertex[2].vPosition = { 1.f, -0.8f, -0.5f };
    pVertex[3].vPosition = { -1.f, -0.8f, -0.5f };

    pVertex[0].vTexUV = { 20.f / 256.f, 20.f / 256.f };
    pVertex[1].vTexUV = { 60.f / 256.f, 20.f / 256.f };
    pVertex[2].vTexUV = { 60.f / 256.f, 52.f / 256.f };
    pVertex[3].vTexUV = { 20.f / 256.f, 52.f / 256.f };

    //// BACK
    pVertex[4].vPosition = { 1.f,  0.8f,  0.5f };
    pVertex[5].vPosition = { -1.f,  0.8f,  0.5f };
    pVertex[6].vPosition = { -1.f, -0.8f,  0.5f };
    pVertex[7].vPosition = { 1.f, -0.8f,  0.5f };

    pVertex[4].vTexUV = { 80.f / 256.f, 20.f / 256.f };
    pVertex[5].vTexUV = { 120.f / 256.f, 20.f / 256.f };
    pVertex[6].vTexUV = { 120.f / 256.f, 52.f / 256.f };
    pVertex[7].vTexUV = { 80.f / 256.f, 52.f / 256.f };

    //// LEFT
    pVertex[8].vPosition = { -1.f,  0.8f,  0.5f };
    pVertex[9].vPosition = { -1.f,  0.8f, -0.5f };
    pVertex[10].vPosition = { -1.f, -0.8f, -0.5f };
    pVertex[11].vPosition = { -1.f, -0.8f,  0.5f };

    pVertex[8].vTexUV = { 0.f / 256.f, 20.f / 256.f };
    pVertex[9].vTexUV = { 20.f / 256.f, 20.f / 256.f };
    pVertex[10].vTexUV = { 20.f / 256.f, 52.f / 256.f };
    pVertex[11].vTexUV = { 0.f / 256.f, 52.f / 256.f };

    //// RIGHT
    pVertex[12].vPosition = { 1.f,  0.8f, -0.5f };
    pVertex[13].vPosition = { 1.f,  0.8f,  0.5f };
    pVertex[14].vPosition = { 1.f, -0.8f,  0.5f };
    pVertex[15].vPosition = { 1.f, -0.8f, -0.5f };

    pVertex[12].vTexUV = { 60.f / 256.f, 20.f / 256.f };
    pVertex[13].vTexUV = { 80.f / 256.f, 20.f / 256.f };
    pVertex[14].vTexUV = { 80.f / 256.f, 52.f / 256.f };
    pVertex[15].vTexUV = { 60.f / 256.f, 52.f / 256.f };

    //// TOP
    pVertex[16].vPosition = { -1.f,  0.8f,  0.5f };
    pVertex[17].vPosition = { 1.f,  0.8f,  0.5f };
    pVertex[18].vPosition = { 1.f,  0.8f, -0.5f };
    pVertex[19].vPosition = { -1.f,  0.8f, -0.5f };

    pVertex[16].vTexUV = { 20.f / 256.f, 0.f / 256.f };
    pVertex[17].vTexUV = { 60.f / 256.f, 0.f / 256.f };
    pVertex[18].vTexUV = { 60.f / 256.f, 20.f / 256.f };
    pVertex[19].vTexUV = { 20.f / 256.f, 20.f / 256.f };

    //// BOTTOM
    pVertex[20].vPosition = { -1.f, -0.8f, -0.5f };
    pVertex[21].vPosition = { 1.f, -0.8f, -0.5f };
    pVertex[22].vPosition = { 1.f, -0.8f,  0.5f };
    pVertex[23].vPosition = { -1.f, -0.8f,  0.5f };

    pVertex[20].vTexUV = { 60.f / 256.f, 0.f / 256.f };
    pVertex[21].vTexUV = { 100.f / 256.f, 0.f / 256.f };
    pVertex[22].vTexUV = { 100.f / 256.f, 20.f / 256.f };
    pVertex[23].vTexUV = { 60.f / 256.f, 20.f / 256.f };

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

void CBossTex::Render_Buffer()
{
	CVIBuffer::Render_Buffer();
}

void CBossTex::Compute_MaxScale()
{
}

CBossTex* CBossTex::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CBossTex* pBossTex = new CBossTex(pGraphicDev);

	if (FAILED(pBossTex->Ready_Buffer()))
	{
		Safe_Release(pBossTex);
		MSG_BOX("BossTex Create Failed");
		return nullptr;
	}

	return pBossTex;
}

CComponent* CBossTex::Clone()
{
	return new CBossTex(*this);
}

void CBossTex::Free()
{
	CVIBuffer::Free();
}