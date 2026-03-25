#include "pch.h"
#include "COceanBuffer.h"

namespace
{
    template <typename T>
    auto ResolvePatchSize(const T& desc, int) -> decltype((void)desc.fPatchSize, _float())
    {
        return desc.fPatchSize;
    }

    template <typename T>
    _float ResolvePatchSize(const T&, long)
    {
        // OCEAN_DESC 정의 충돌/구버전 헤더 대응 기본값
        return 512.f;
    }
}

COceanBuffer::COceanBuffer(LPDIRECT3DDEVICE9 pGraphicDev)
	: CVIBuffer(pGraphicDev)
{}

COceanBuffer::COceanBuffer(const COceanBuffer & rhs)
	: CVIBuffer(rhs)
{}

COceanBuffer::~COceanBuffer()
{}

HRESULT COceanBuffer::Ready_Buffer(const OCEAN_DESC & desc)
{
    m_dwVtxCntX = desc.dwVtxCntX;
    m_dwVtxCntZ = desc.dwVtxCntZ;
    m_fPatchSize = ResolvePatchSize(desc, 0);

    m_dwVtxCnt = m_dwVtxCntX * m_dwVtxCntZ;
    m_dwTriCnt = (m_dwVtxCntX - 1) * (m_dwVtxCntZ - 1) * 2;
    m_dwVtxSize = sizeof(VTXTEX);
    m_dwIdxSize = sizeof(INDEX32);
    m_dwFVF = FVF_TEX;
    m_IdxFmt = D3DFMT_INDEX32;

    // 기본 격자 CPU 배열 할당
    m_pBasePos = new D3DXVECTOR3[m_dwVtxCnt];
    m_pBaseUV = new D3DXVECTOR2[m_dwVtxCnt];

    Compute_BaseGrid();

    // Dynamic VB 생성
    if (FAILED(Create_DynamicVB()))
        return E_FAIL;

    // IB 생성 (고정)
    if (FAILED(m_pGraphicDev->CreateIndexBuffer(
        m_dwTriCnt * m_dwIdxSize,
        0,
        m_IdxFmt,
        D3DPOOL_MANAGED,
        &m_pIB,
        nullptr)))
        return E_FAIL;

    if (FAILED(Build_IndexBuffer()))
        return E_FAIL;

    return S_OK;
}

void COceanBuffer::Update_Vertices(const float* pHeight, const float* pDisplX, 
    const float* pDisplZ, const D3DXVECTOR3* pNormals)
{
    VTXTEX* pVertex = nullptr;

    // D3DLOCK_DISCARD: 이전 프레임 데이터 버리고 새 메모리 받음
    // 드라이버가 GPU가 읽는 중인 버퍼와 다른 메모리 블록을 줌
    // → CPU/GPU 스톨(stall) 없이 바로 쓸 수 있음
    if (FAILED(m_pVB->Lock(0, 0, (void**)&pVertex, D3DLOCK_DISCARD)))
        return;

    bool bChoppy = (pDisplX != nullptr && pDisplZ != nullptr);

    for (_ulong i = 0; i < m_dwVtxCnt; ++i)
    {
        pVertex[i].vPosition = m_pBasePos[i];
        pVertex[i].vPosition.y = pHeight[i];

        // Choppy (수평 변위) 적용
        if (bChoppy)
        {
            pVertex[i].vPosition.x += pDisplX[i];
            pVertex[i].vPosition.z += pDisplZ[i];
        }

        pVertex[i].vNormal = pNormals[i];
        pVertex[i].vTexUV = m_pBaseUV[i];
    }

    m_pVB->Unlock();
}

void COceanBuffer::Render_Buffer()
{
    CVIBuffer::Render_Buffer();
}

HRESULT COceanBuffer::Create_DynamicVB()
{
    if (FAILED(m_pGraphicDev->CreateVertexBuffer(
        m_dwVtxCnt * m_dwVtxSize,
        D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,  // 매 프레임 쓰기 전용
        m_dwFVF,
        D3DPOOL_DEFAULT,    // GPU 메모리 → Lock이 빠름
        &m_pVB,
        nullptr)))
        return E_FAIL;

    // 초기 버텍스 데이터 기록 (Y = 0 평면)
    VTXTEX* pVertex = nullptr;
    // D3DLOCK_DISCARD: 기존 내용 버리고 새 메모리 받음 (드라이버 최적화 핵심)
    m_pVB->Lock(0, 0, (void**)&pVertex, D3DLOCK_DISCARD);

    for (_ulong i = 0; i < m_dwVtxCnt; ++i)
    {
        pVertex[i].vPosition = m_pBasePos[i];
        pVertex[i].vNormal = D3DXVECTOR3(0.f, 1.f, 0.f);
        pVertex[i].vTexUV = m_pBaseUV[i];
    }

    m_pVB->Unlock();
    return S_OK;
}

HRESULT COceanBuffer::Build_IndexBuffer()
{
    INDEX32* pIndex = nullptr;
    m_pIB->Lock(0, 0, (void**)&pIndex, 0);

    _ulong dwTri = 0;
    for (_ulong iz = 0; iz < m_dwVtxCntZ - 1; ++iz)
    {
        for (_ulong ix = 0; ix < m_dwVtxCntX - 1; ++ix)
        {
            _ulong base = iz * m_dwVtxCntX + ix;

            // 상삼각 (위쪽)
            pIndex[dwTri]._0 = base + m_dwVtxCntX;
            pIndex[dwTri]._1 = base + m_dwVtxCntX + 1;
            pIndex[dwTri]._2 = base + 1;
            ++dwTri;

            // 하삼각 (아래쪽)
            pIndex[dwTri]._0 = base + m_dwVtxCntX;
            pIndex[dwTri]._1 = base + 1;
            pIndex[dwTri]._2 = base;
            ++dwTri;
        }
    }

    m_pIB->Unlock();
    return S_OK;
}

void COceanBuffer::Compute_BaseGrid()
{
    _float stepX = m_fPatchSize / (_float)(m_dwVtxCntX - 1);
    _float stepZ = m_fPatchSize / (_float)(m_dwVtxCntZ - 1);
    _float offX = -m_fPatchSize * 0.5f;
    _float offZ = -m_fPatchSize * 0.5f;

    for (_ulong iz = 0; iz < m_dwVtxCntZ; ++iz)
    {
        for (_ulong ix = 0; ix < m_dwVtxCntX; ++ix)
        {
            _ulong idx = iz * m_dwVtxCntX + ix;
            m_pBasePos[idx] = D3DXVECTOR3(
                offX + ix * stepX,
                0.f,
                offZ + iz * stepZ);

            // UV: 타일링 × 2 (텍스처 반복)
            m_pBaseUV[idx] = D3DXVECTOR2(
                (_float)ix / (_float)(m_dwVtxCntX - 1) * 2.f,
                (_float)iz / (_float)(m_dwVtxCntZ - 1) * 2.f);
        }
    }
}

COceanBuffer* COceanBuffer::Create(LPDIRECT3DDEVICE9 pGraphicDev, const OCEAN_DESC & desc)
{
    COceanBuffer* pBuffer = new COceanBuffer(pGraphicDev);
    if (FAILED(pBuffer->Ready_Buffer(desc)))
    {
        Safe_Release(pBuffer);
        MSG_BOX("COceanBuffer Create Failed");
        return nullptr;
    }
    return pBuffer;
}

CComponent* COceanBuffer::Clone()
{
    return new COceanBuffer(*this);
}

void COceanBuffer::Free()
{
    if (false == m_bClone)
    {
        Safe_Delete_Array(m_pBasePos);
        Safe_Delete_Array(m_pBaseUV);
    }
    CVIBuffer::Free();
}
