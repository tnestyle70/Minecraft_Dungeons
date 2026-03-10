#pragma once
#include "CBlockPlacer.h"
#include "CBlock.h"
#include "CIronBar.h"
#include "CBatchBuffer.h"

struct BlockData
{
	int x, y, z;
	int eType;
};

class CBlockMgr : public CBase
{
	DECLARE_SINGLETON(CBlockMgr)
private:
	explicit CBlockMgr();
	virtual ~CBlockMgr();
public:
	HRESULT Ready_BlockMgr(LPDIRECT3DDEVICE9 pGraphicDev);
	HRESULT Ready_Textures();
	void Update(const _float& fTimeDelta);
	void Render();
	
	const map<BlockPos, CBlock*>& Get_Blocks() { return m_mapBlocks; }
	void AddBlock(const _vec3& vPos, eBlockType eType);
	void RemoveBlock(const _vec3& vPos);
	void RemoveBlockByPos(const BlockPos& pos);
	void ClearBlocks();

	HRESULT SaveBlocks(const _tchar* pFilePath);
	HRESULT LoadBlocks(const _tchar* pFilePath);
public:
	bool RayAABBIntersect(const _vec3& vRayPos, const _vec3& vRayDir, 
		BlockPos* pOutBlockPos, float* pOutT);
	AABB Get_BlockAABB(const BlockPos& tPos);
	bool HasBlock(const BlockPos& tPos);
public: 
	BlockPos ToPos(const _vec3& vPos);
	//배치 매쉬 갱신 - 블럭 추가 삭제 로드 후 호출
	void RebuildBatchMesh();

private: //블럭의 위치 - 키, 실제 블럭 - 값으로 저장
	LPDIRECT3DDEVICE9 m_pGraphicDev;
	map<BlockPos, CBlock*> m_mapBlocks;
	CBatchBuffer* m_pBatchBuffer = nullptr;
	//타입별로 배치 버퍼를 분리
	map<eBlockType, CBatchBuffer*> m_mapBatchBuffers;
	map<eBlockType, CTexture*> m_mapTextures;
private:
	virtual void Free();
};