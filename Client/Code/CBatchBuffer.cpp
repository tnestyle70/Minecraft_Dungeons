#include "pch.h"
#include "CBatchBuffer.h"
//블럭 중심 기준으로 +-0.5 정도의 위치로 월드 좌표를 직접 써넣기
//world matrix를 identity로 두고 그리기 떄문에 settransform이 필요 없음

//face offsets
static const _vec3 g_FaceVerts[CBatchBuffer::FACE_END][4] =
{
	// FACE_TOP    (+Y)
	{ {-0.5f, 0.5f,  0.5f}, { 0.5f, 0.5f,  0.5f}, { 0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f} },
	// FACE_BOTTOM (-Y)
	{ {-0.5f,-0.5f, -0.5f}, { 0.5f,-0.5f, -0.5f}, { 0.5f,-0.5f,  0.5f}, {-0.5f,-0.5f,  0.5f} },
	// FACE_RIGHT  (+X)
	{ { 0.5f, 0.5f, -0.5f}, { 0.5f, 0.5f,  0.5f}, { 0.5f,-0.5f,  0.5f}, { 0.5f,-0.5f, -0.5f} },
	// FACE_LEFT   (-X)
	{ {-0.5f, 0.5f,  0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f,-0.5f, -0.5f}, {-0.5f,-0.5f,  0.5f} },
	// FACE_FRONT  (+Z)
	{ {-0.5f, 0.5f,  0.5f}, { 0.5f, 0.5f,  0.5f}, { 0.5f,-0.5f,  0.5f}, {-0.5f,-0.5f,  0.5f} },  // 기존 Z+ 뒷면과 동일
	// FACE_BACK   (-Z)
	{ { 0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f,-0.5f, -0.5f}, { 0.5f,-0.5f, -0.5f} },  // 기존 Z- 앞면과 동일
};

static const _vec2 g_FaceUVs[4] =
{
	{0.f, 0.f}, //Left Top
	{1.f, 0.f}, //Right Top
	{1.f, 1.f}, //Right Bottom
	{0.f, 1.f}  //Left Bottom
};

CBatchBuffer::CBatchBuffer(LPDIRECT3DDEVICE9 pGraphicDev)
	:CVIBuffer(pGraphicDev)
{
}

CBatchBuffer::~CBatchBuffer()
{
}

TileUV CBatchBuffer::MakeTile(int col, int row)
{
	const float su = 1.f / 4.f;          // 가로 4칸
	const float sv = 1.f / 5.f;          // 세로 5칸 (64x80 아틀라스)
	const float insetU = 1.5f / 64.f;    // 반 픽셀 (가로)
	const float insetV = 1.5f / 80.f;    // 반 픽셀 (세로)

	return {
		col * su + insetU,
		row * sv + insetV,
		(col + 1) * su - insetU,
		(row + 1) * sv - insetV
	};

	//const float s = 1.f / 4.f;          // 타일 1칸 = 0.25
	//const float inset = 0.5f / 64.f;    // 반 픽셀 = 0.0078125

	//return {
	//col * s + inset,
	//row * s + inset,
	//(col + 1) * s - inset,
	//(row + 1) * s - inset
	//};
}

TileUV CBatchBuffer::GetTileUV(eBlockType eType, eFace eFace)
{
	switch (eType)
	{
	case BLOCK_GRASS:
		if (eFace == CBatchBuffer::FACE_TOP)
			return MakeTile(3, 0);
		if (eFace == CBatchBuffer::FACE_BOTTOM)
			return MakeTile(1, 0);
		return MakeTile(0, 1);
	case BLOCK_DIRT:
		return MakeTile(1, 0);
	case BLOCK_ROCK:
		return MakeTile(3, 1);
	case BLOCK_SAND:
		return MakeTile(0, 2);
	case BLOCK_BEDROCK:
		return MakeTile(0, 0);
	case BLOCK_OBSIDIAN:
		return MakeTile(2, 0);
	case BLOCK_STONEBRICK:
		return MakeTile(1, 2);
	case BLOCK_IRONBAR:
		return MakeTile(1, 2);
	case BLOCK_TNT:
		if (eFace == CBatchBuffer::FACE_TOP)
			return MakeTile(0, 3);
		if (eFace == CBatchBuffer::FACE_BOTTOM)
			return MakeTile(2, 2);
		return MakeTile(3, 2);
	case BLOCK_OAK:
		return MakeTile(3, 3);
	case BLOCK_OAK_LEAVES:
		return MakeTile(2, 3);
	case BLOCK_CHERRY_LEAVES:
		return MakeTile(1, 3);
	case BLOCK_LAVA:
		return MakeTile(2, 1);  
	case BLOCK_PLANKS_ACACIA:
		return MakeTile(3, 4);  
	case BLOCK_PLANKS_SPRUCE:
		return MakeTile(3, 3);  
	case BLOCK_OAKWOOD:
		return MakeTile(0, 4);
	case BLOCK_REDSTONE:
		return MakeTile(1, 4);
	default:
		return MakeTile(1, 0);
	}

	return MakeTile(1, 0);
}

HRESULT CBatchBuffer::Rebuild(const vector<_vec3>& vecPositions,
	const vector<eBlockType>& vecType, 
	const vector<bool>& vecFaceVisible)
{
	//블럭들의 위치 목록을 받아서 Vertex Buffer, Index Buffer를 다시 채우기
	//AddBlock / RemoveBlock / LoadBlocks 직후 CBlockMgr이 호출
	const DWORD dwBlockCount = static_cast<DWORD>(vecPositions.size());
	const bool bCulling = !vecFaceVisible.empty();
	
	//블럭이 0개면 버퍼를 비우고 종료
	if (dwBlockCount == 0)
	{
		Safe_Release(m_pVB);
		Safe_Release(m_pIB);
		m_dwVtxCnt = 0;
		m_dwTriCnt = 0;
		m_dwFaceCount = 0;
		return S_OK;
	}

	//Calculate the rendering faces
	DWORD dwVisibleFaces = 0;

	for (DWORD i = 0; i < dwBlockCount; ++i)
	{
		for (int face = 0; face < FACE_END; ++face)
		{
			bool bVisible = bCulling ? vecFaceVisible[i * FACE_END + face] : true;

			if (bVisible)
			{
				++dwVisibleFaces;
			}
		}
	}
	
	//if block's face stuck from other blocks, don't render
	if (dwVisibleFaces == 0)
	{
		Safe_Release(m_pVB);
		Safe_Release(m_pIB);
		m_dwVtxCnt = 0;
		m_dwTriCnt = 0;
		m_dwFaceCount = 0;
		return S_OK;
	}

	//Reallocate GPU buffer, when face actually change
	if (dwVisibleFaces != m_dwFaceCount)
	{
		if (FAILED(ReallocBuffers(dwVisibleFaces)))
			return E_FAIL;
	}
	
	//Buffer Lock -> Write Visible Faces
	VERTEXBLOCK* pVertex = nullptr;
	m_pVB->Lock(0, 0, reinterpret_cast<void**>(&pVertex), 0);
	
	INDEX32* pIndex = nullptr;
	m_pIB->Lock(0, 0, reinterpret_cast<void**>(&pIndex), 0);
	
	DWORD dwFaceSlot = 0;

	for (DWORD i = 0; i < dwBlockCount; ++i)
	{
		float batchX = vecPositions[i].x;
		float batchY = vecPositions[i].y;
		float batchZ = vecPositions[i].z;
		
		for (int face = 0; face < FACE_END; ++face)
		{
			bool bVisible = bCulling ? vecFaceVisible[i * FACE_END + face] : true;
			if (!bVisible)
				continue;

			WriteFace(pVertex, pIndex, dwFaceSlot, static_cast<eFace>(face),
				batchX, batchY, batchZ, vecType[i]);

			++dwFaceSlot;
		}
	}

	m_pVB->Unlock();
	m_pIB->Unlock();

	return S_OK;
}

//face count를 기준으로 VB/IB 크기를 결정해서 재할당
HRESULT CBatchBuffer::ReallocBuffers(DWORD dwFaceCount)
{
	Safe_Release(m_pVB);
	Safe_Release(m_pIB);

	m_dwVtxCnt = dwFaceCount * 4; // face vertex 4
	m_dwTriCnt = dwFaceCount * 2; // face tri 2
	m_dwVtxSize = sizeof(VERTEXBLOCK); 
	m_dwIdxSize = sizeof(INDEX32); //triangle size = dword * 3
	m_dwFVF = FVF_BLOCK;         
	m_IdxFmt = D3DFMT_INDEX32;

	if (FAILED(CVIBuffer::Ready_Buffer()))
		return E_FAIL;

	m_dwFaceCount = dwFaceCount;

	return S_OK;
}

void CBatchBuffer::WriteFace(VERTEXBLOCK* pVertex, INDEX32* pIndex, 
	DWORD dwFaceSlot, eFace eFace,
	float bakeX, float bakeY, float bakeZ, 
	eBlockType eType)
{
	//N번째 면의 인덱스는 N * 4부터 시작해야 함, 안 하면 모든 면이 첫번째 면 정점을 
	//공유해서 텍스쳐가 깨지게 됨
	const DWORD vtxBase = dwFaceSlot * 4;
	const DWORD idxBase = dwFaceSlot * 2;

	//해당 면의 아틀라스 범위 가지고 오기
	TileUV tile = GetTileUV(eType, eFace);

	//타일 범위로 UV 4개 계산
	_vec2 uvs[4] =
	{
		{tile.u0, tile.v0}, //LT
		{tile.u1, tile.v0}, //RT
		{tile.u1, tile.v1}, //RB
		{tile.u0, tile.v1} //LB
	};

	//write 4 vertexs, center pos -> offset -> world coord
	//정점 정보 자체에 월드 위치를 bake
	//vertex에 애초에 월드 위치를 저장했기 때문에, 월드 행렬을 곱해줄 필요가 없으므로 
	//단위 행렬을 곱해줌
	//모든 정점을 하나의 월드 좌표로 베이킹을 해서 한 번의 DrawCall을 호출
	for (int vtx = 0; vtx < 4; ++vtx)
	{
		pVertex[vtxBase + vtx].vPos =
		{
			bakeX + g_FaceVerts[eFace][vtx].x,
			bakeY + g_FaceVerts[eFace][vtx].y,
			bakeZ + g_FaceVerts[eFace][vtx].z
		};
		//pVertex[vtxBase + vtx].vUV = g_FaceUVs[vtx];
		pVertex[vtxBase + vtx].vUV = uvs[vtx];
	}
	//삼각형 2개 기록 - 덮어쓰기 방지
	pIndex[idxBase] = { vtxBase, vtxBase + 1, vtxBase + 2 };
	pIndex[idxBase + 1] = { vtxBase, vtxBase + 2, vtxBase + 3 };
}

void CBatchBuffer::Render_Buffer()
{
	// 블럭이 없으면 그리지 않는다
	if (m_dwFaceCount == 0 || !m_pVB || !m_pIB)
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
