#include "CVIBuffer.h"

CVIBuffer::CVIBuffer() : m_pVB(nullptr), m_pIB(nullptr)
, m_dwVtxSize(0)
, m_dwVtxCnt(0)
, m_dwTriCnt(0)
, m_dwIdxSize(0)
{
}

CVIBuffer::CVIBuffer(LPDIRECT3DDEVICE9 pGraphicDev)
	: CComponent(pGraphicDev), m_pVB(nullptr), m_pIB(nullptr)
	, m_dwVtxSize(0)
	, m_dwVtxCnt(0)
	, m_dwTriCnt(0)
	, m_dwFVF(0)
	, m_dwIdxSize(0)
{
}

CVIBuffer::CVIBuffer(const CVIBuffer& rhs)
	:CComponent(rhs), m_pVB(rhs.m_pVB), m_pIB(rhs.m_pIB)
	, m_dwVtxSize(rhs.m_dwVtxSize)
	, m_dwVtxCnt(rhs.m_dwVtxCnt)
	, m_dwTriCnt(rhs.m_dwTriCnt)
	, m_dwFVF(rhs.m_dwFVF)
	, m_dwIdxSize(rhs.m_dwIdxSize)
	, m_IdxFmt(rhs.m_IdxFmt)
{
	m_pVB->AddRef();
	m_pIB->AddRef();
}

CVIBuffer::~CVIBuffer()
{
}

HRESULT CVIBuffer::Ready_Buffer()
{
	// D3DPOOL_DEFAULT : 그래픽 메모리
	// D3DPOOL_MANAGED : 그래픽 메모리에 저장, 메인 메모리 백업
	// D3DPOOL_SYSTEMMEM : 메인 메모리에 저장
	// D3DPOOL_SCRATCH : 메인 메모리에 저장(DX 장치로 접근 불가)

	if (FAILED(m_pGraphicDev->CreateVertexBuffer(m_dwVtxCnt * m_dwVtxSize,	// 버텍스 버퍼의 크기
												0,		// 0인 경우 정적 버퍼, D3DUSAGE_DYNAMIC인 경우 동적 버퍼
												m_dwFVF, // 버텍스 속성
												D3DPOOL_MANAGED, // 정적 버퍼인 경우 MANAGED
												&m_pVB,			// 버텍스 버퍼 객체 생성
												NULL)))			// 공유할 일 없기 때문에 null
												return E_FAIL;

	if (FAILED(m_pGraphicDev->CreateIndexBuffer(m_dwTriCnt * m_dwIdxSize,	// 인덱스 버퍼의 크기
												0,		// 0인 경우 정적 버퍼, D3DUSAGE_DYNAMIC인 경우 동적 버퍼
												m_IdxFmt, // 인덱스 속성
												D3DPOOL_MANAGED, // 정적 버퍼인 경우 MANAGED
												&m_pIB,			// 버텍스 버퍼 객체 생성
												NULL)))			// 공유할 일 없기 때문에 null
												return E_FAIL;

	return S_OK;
}

void CVIBuffer::Render_Buffer()
{
	m_pGraphicDev->SetStreamSource(0, m_pVB, 0, m_dwVtxSize);

	m_pGraphicDev->SetFVF(m_dwFVF);

	m_pGraphicDev->SetIndices(m_pIB);

	// m_pGraphicDev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_dwTriCnt);

	m_pGraphicDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_dwVtxCnt, 0, m_dwTriCnt);
}

void CVIBuffer::Free()
{
	Safe_Release(m_pIB);
	Safe_Release(m_pVB);

	CComponent::Free();
}
