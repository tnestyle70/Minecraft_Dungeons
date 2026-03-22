#pragma once
#include "CProtoMgr.h"
#include "CBlockPlacer.h"
#include "CBlock.h"
#include "CBatchBuffer.h"

enum eRenderMode
{
    RENDER_EDITOR,
    RENDER_BATCH,
    RENDER_QUADTREE
};

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
    void Rendre_QuadTree();
public:
    void SetRenderMode(eRenderMode eMode);
    eRenderMode GetRenderMode() { return m_eRenderMode; }
    void BuildQuadTree();
    const map<BlockPos, CBlock*>& Get_Blocks() { return m_mapBlocks; }
    void AddBlock(const _vec3& vPos, eBlockType eType);
    void AddBlock(int x, int y, int z, eBlockType eType);
    void RemoveBlock(const _vec3& vPos);
    void RemoveBlockByPos(const BlockPos& pos);
    void ClearBlocks();
    HRESULT SaveBlocks(FILE* pFile);
    HRESULT LoadBlocks(FILE* pFile);
    // 영역 채우기/지우기/조회
    void FillBlocks(BlockPos pos1, BlockPos pos2, eBlockType eType);
    void EraseBlocks(BlockPos pos1, BlockPos pos2);
    vector<BlockData> GetBlocksInRange(BlockPos pos1, BlockPos pos2);
public:
    // 기존 - 단순 충돌 체크 (선택 모드, 몬스터 배치 등에서 사용)
    bool RayAABBIntersect(const _vec3& vRayPos, const _vec3& vRayDir,
        BlockPos* pOutBlockPos, float* pOutT);
    // 추가 - 법선 포함 충돌 체크 (블록 배치 미리보기, 옆면 배치에 사용)
    bool RayAABBIntersectWithNormal(const _vec3& vRayPos, const _vec3& vRayDir,
        BlockPos* pOutBlockPos, float* pOutT, BlockPos* pOutNormal);
    AABB Get_BlockAABB(const BlockPos& tPos);
    bool HasBlock(const BlockPos& tPos);
public:
    BlockPos ToPos(const _vec3& vPos);
    void RebuildBatchMesh();
private:
    LPDIRECT3DDEVICE9      m_pGraphicDev;
    map<BlockPos, CBlock*> m_mapBlocks;
    CTexture* m_pTexture = nullptr;
    CBatchBuffer* m_pBatchBuffer = nullptr;
    eRenderMode            m_eRenderMode = eRenderMode::RENDER_EDITOR;
private:
    virtual void Free();
};