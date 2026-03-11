#pragma once
#include "CVIBuffer.h"
#include "CBlockPlacer.h"

//여러 큐브의 정점들을 하나의 IB/VB에 넣어서 한 번에 렌더
//텍스쳐의 타입 별로 하나씩 만들어서 사용

//배치용 정점 구조체
struct VERTEXBLOCK
{
	_vec3 vPos;
	_vec2 vUV;
};

#define FVF_BLOCK (D3DFVF_XYZ | D3DFVF_TEX1)

//Atlas mapping
struct TileUV
{
	float u0, v0; //LeftTop
	float u1, v1; //Right Bottom
};

//N개 블럭의 정점을 하나의 VB/IB에 bake해서 DrawIndexedPrimitive를 1번만 호출

class CBatchBuffer : public CVIBuffer
{
public: //Face Culling
	enum eFace
	{
		FACE_TOP = 0, FACE_BOTTOM, FACE_RIGHT, FACE_LEFT, 
		FACE_FRONT, FACE_BACK, FACE_END
	};
private:
	explicit CBatchBuffer(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CBatchBuffer();
public:
	static TileUV MakeTile(int col, int row);
	static TileUV GetTileUV(eBlockType eType, eFace eFace);
public:
	//Positions : block world location 
	//vecFaceVisible : positions[i] visibility
	HRESULT Rebuild(const vector<_vec3>& vecPositions,
		const vector<eBlockType>& vecType,
		const vector<bool>& vecFaceVisible = {});

	void Render_Buffer() override;
public:
	static CBatchBuffer* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone() { return nullptr; }
private:
	//Reallocate Blocks by the actual rendering vertexs
	HRESULT ReallocBuffers(DWORD dwFaceCount);
	//큐브 1개의 정점 인덱스를 lock된 배열에 써 넣기
	void WriteFace(VERTEXBLOCK* pVertex, INDEX32* pIndex,
		DWORD dwFaceSlot, eFace eFace , 
		float bakeX, float bakeY, float bakeZ,
		eBlockType eType);
private:
	//face that allocate to face
	DWORD m_dwFaceCount = 0;
private:
	virtual void Free() override;
};
