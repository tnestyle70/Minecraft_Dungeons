#pragma once
#include "CProtoMgr.h"
#include "CBlockPlacer.h"
#include "CBlock.h"
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
public:
	void Render_Editor();
	void Render_Stage();
public:
	void SetEditorMode(bool bEditor);
	bool IsEditorMode() { return m_bEditorMode; };

	const map<BlockPos, CBlock*>& Get_Blocks() { return m_mapBlocks; }
	void AddBlock(const _vec3& vPos, eBlockType eType);
	void RemoveBlock(const _vec3& vPos);
	void RemoveBlockByPos(const BlockPos& pos);
	void ClearBlocks();

	HRESULT SaveBlocks(FILE* pFile);
	HRESULT LoadBlocks(FILE* pFile);
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
	//Atlas Texture
	CTexture* m_pTexture = nullptr;
	CBatchBuffer* m_pBatchBuffer = nullptr;
	bool m_bEditorMode = true;
private:
	virtual void Free();
};