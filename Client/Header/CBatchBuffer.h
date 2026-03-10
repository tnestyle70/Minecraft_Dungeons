#pragma once
#include "CVIBuffer.h"

struct VTXBLOCK
{
	D3DXVECTOR3 vPosition;
	D3DXVECTOR2 vTexUV;     //2D UV
};
#define FVF_BLOCK (D3DFVF_XYZ | D3DFVF_TEX1)

//N개 블럭의 정점을 하나의 VB/IB에 bake해서 DrawIndexedPrimitive를 1번만 호출

class CBatchBuffer : public CVIBuffer
{
private:
	explicit CBatchBuffer(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CBatchBuffer();
public:
	//블럭 위치 목록을 가지고 VB/IB를 새로 빌드
	//CBlockMgr::RebuildMesh에서 호출
	HRESULT Rebuild(const vector<_vec3>& vecPositions);

	void Render_Buffer() override;
public:
	static CBatchBuffer* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	CComponent* Clone() override { return nullptr; };
private:
	virtual void Free() override;
private:
	//블럭 수가 바뀌었을 때만 GPU 버퍼를 재할당
	HRESULT ReallocBuffers(DWORD dwBlockCount);
	//큐브 1개의 정점 인덱스를 lock된 배열에 써 넣기
	void WriteBlockMesh(VTXBLOCK* pVtx, INDEX32* pIndex,
		DWORD blockIndex, float bx, float by, float bz);
private:
	DWORD m_dwBlockCount = 0;
};