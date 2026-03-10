#include "pch.h"
#include "CBatchBuffer.h"

//블럭 중심 기준으로 +-0.5 정도의 위치로 월드 좌표를 직접 써넣기
//world matrix를 identity로 두고 그리기 떄문에 settransform이 필요 없음

CBatchBuffer::CBatchBuffer(LPDIRECT3DDEVICE9 pGraphicDev)
	:CVIBuffer(pGraphicDev)
{
}

CBatchBuffer::~CBatchBuffer()
{
}

HRESULT CBatchBuffer::Rebuild(const vector<_vec3>& vecPositions)
{
	//블럭들의 위치 목록을 받아서 Vertex Buffer, Index Buffer를 다시 채우기
	//AddBlock / RemoveBlock / LoadBlocks 직후 CBlockMgr이 호출
	const DWORD dwCount = static_cast<DWORD>(vecPositions.size());

	//블럭이 0개면 버퍼를 비우고 종료
	if (dwCount == 0)
	{
		Safe_Release(m_pVB);
		Safe_Release(m_pIB);
		m_dwVtxCnt = 0;
		m_dwTriCnt = 0;
		m_dwBlockCount = 0;
		return S_OK;
	}
	//블럭 수가 달라졌을 때만 GPU 버퍼를 재할당
	if (dwCount != m_dwBlockCount)
	{
		if (FAILED(ReallocBuffers(dwCount)))
			return E_FAIL;
	}
	//정점 인덱스 채우기
	VTXBLOCK* pVertex = nullptr;
	m_pVB->Lock(0, 0, reinterpret_cast<void**>(&pVertex), 0);
	
	INDEX32* pIndex = nullptr;
	m_pIB->Lock(0, 0, reinterpret_cast<void**>(&pIndex), 0);

	for (DWORD i = 0; i < dwCount; ++i)
	{
		WriteBlockMesh(pVertex, pIndex, i,
			vecPositions[i].x,
			vecPositions[i].y,
			vecPositions[i].z);
	}
	m_pIB->Unlock();
	m_pVB->Unlock();

	return S_OK;
}

HRESULT CBatchBuffer::ReallocBuffers(DWORD dwBlockCount)
{
	Safe_Release(m_pVB);
	Safe_Release(m_pIB);

	m_dwVtxCnt = dwBlockCount * 24; // 면당 4정점 × 6면
	m_dwTriCnt = dwBlockCount * 12; // 면당 2삼각형 × 6면 (동일)
	m_dwVtxSize = sizeof(VTXBLOCK);  //  변경
	m_dwIdxSize = sizeof(INDEX32);
	m_dwFVF = FVF_BLOCK;         //  변경
	m_IdxFmt = D3DFMT_INDEX32;

	if (FAILED(CVIBuffer::Ready_Buffer()))
		return E_FAIL;

	m_dwBlockCount = dwBlockCount;
	return S_OK;
}

void CBatchBuffer::WriteBlockMesh(VTXBLOCK* pVtx, INDEX32* pIndex,
	DWORD blockIndex, float bx, float by, float bz)
{
	const DWORD vBase = blockIndex * 24;
	const DWORD iBase = blockIndex * 12;

	// 각 면: 정점 4개, UV (0,0)(1,0)(1,1)(0,1) 순서
	// 인덱스: 0,1,2 / 0,2,3 패턴

	// Z- 앞면 (0,0,0 기준)
	pVtx[vBase + 0] = { {bx - 0.5f, by + 0.5f, bz - 0.5f}, {0,0} };
	pVtx[vBase + 1] = { {bx + 0.5f, by + 0.5f, bz - 0.5f}, {1,0} };
	pVtx[vBase + 2] = { {bx + 0.5f, by - 0.5f, bz - 0.5f}, {1,1} };
	pVtx[vBase + 3] = { {bx - 0.5f, by - 0.5f, bz - 0.5f}, {0,1} };

	// Z+ 뒷면
	pVtx[vBase + 4] = { {bx + 0.5f, by + 0.5f, bz + 0.5f}, {0,0} };
	pVtx[vBase + 5] = { {bx - 0.5f, by + 0.5f, bz + 0.5f}, {1,0} };
	pVtx[vBase + 6] = { {bx - 0.5f, by - 0.5f, bz + 0.5f}, {1,1} };
	pVtx[vBase + 7] = { {bx + 0.5f, by - 0.5f, bz + 0.5f}, {0,1} };

	// X+ 오른쪽
	pVtx[vBase + 8] = { {bx + 0.5f, by + 0.5f, bz - 0.5f}, {0,0} };
	pVtx[vBase + 9] = { {bx + 0.5f, by + 0.5f, bz + 0.5f}, {1,0} };
	pVtx[vBase + 10] = { {bx + 0.5f, by - 0.5f, bz + 0.5f}, {1,1} };
	pVtx[vBase + 11] = { {bx + 0.5f, by - 0.5f, bz - 0.5f}, {0,1} };

	// X- 왼쪽
	pVtx[vBase + 12] = { {bx - 0.5f, by + 0.5f, bz + 0.5f}, {0,0} };
	pVtx[vBase + 13] = { {bx - 0.5f, by + 0.5f, bz - 0.5f}, {1,0} };
	pVtx[vBase + 14] = { {bx - 0.5f, by - 0.5f, bz - 0.5f}, {1,1} };
	pVtx[vBase + 15] = { {bx - 0.5f, by - 0.5f, bz + 0.5f}, {0,1} };

	// Y+ 위
	pVtx[vBase + 16] = { {bx - 0.5f, by + 0.5f, bz + 0.5f}, {0,0} };
	pVtx[vBase + 17] = { {bx + 0.5f, by + 0.5f, bz + 0.5f}, {1,0} };
	pVtx[vBase + 18] = { {bx + 0.5f, by + 0.5f, bz - 0.5f}, {1,1} };
	pVtx[vBase + 19] = { {bx - 0.5f, by + 0.5f, bz - 0.5f}, {0,1} };

	// Y- 아래
	pVtx[vBase + 20] = { {bx - 0.5f, by - 0.5f, bz - 0.5f}, {0,0} };
	pVtx[vBase + 21] = { {bx + 0.5f, by - 0.5f, bz - 0.5f}, {1,0} };
	pVtx[vBase + 22] = { {bx + 0.5f, by - 0.5f, bz + 0.5f}, {1,1} };
	pVtx[vBase + 23] = { {bx - 0.5f, by - 0.5f, bz + 0.5f}, {0,1} };

	// 인덱스 - 면당 0,1,2 / 0,2,3 패턴
	for (DWORD f = 0; f < 6; ++f)
	{
		DWORD v = vBase + f * 4;
		pIndex[iBase + f * 2 + 0] = { v + 0, v + 1, v + 2 };
		pIndex[iBase + f * 2 + 1] = { v + 0, v + 2, v + 3 };
	}
}

void CBatchBuffer::Render_Buffer()
{
	// 블럭이 없으면 그리지 않는다
	if (m_dwBlockCount == 0 || !m_pVB || !m_pIB)
		return;
	// 부모(CVIBuffer)의 구현을 그대로 사용한다.
	// SetStreamSource → SetFVF → SetIndices → DrawIndexedPrimitive
	CVIBuffer::Render_Buffer();
}

CBatchBuffer* CBatchBuffer::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	// 생성 시점에는 블럭 수가 0이므로 버퍼를 만들지 않는다.
	// Rebuild()가 처음 호출될 때 ReallocBuffers()가 실행된다.
	return new CBatchBuffer(pGraphicDev);
}

void CBatchBuffer::Free()
{
	CVIBuffer::Free();
}