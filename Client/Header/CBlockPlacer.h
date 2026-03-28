#pragma once
#include "CBase.h"
#include "CCalculator.h"
#include <stack>
#include <algorithm>

struct BlockPos
{
	int x, y, z;
	bool operator<(const BlockPos& pos)const
	{
		if (x != pos.x)
			return x < pos.x;
		if (y != pos.y)
			return y < pos.y;
		return z < pos.z;
	}
};

enum eBlockType
{
	BLOCK_GRASS,
	BLOCK_DIRT,
	BLOCK_ROCK,
	BLOCK_SAND,
	BLOCK_BEDROCK,
	BLOCK_OBSIDIAN,
	BLOCK_STONEBRICK,
	BLOCK_IRONBAR,
	BLOCK_TNT,
	BLOCK_TRIGGERBOX,
	BLOCK_OAK,
	BLOCK_OAK_LEAVES,
	BLOCK_CHERRY_LEAVES,
	BLOCK_LAVA,
	BLOCK_PLANKS_ACACIA,
	BLOCK_PLANKS_SPRUCE,
	BLOCK_END
};

class CBlockPlacer : public CBase
{
public:
	explicit CBlockPlacer(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CBlockPlacer();
public:
	_int Update_Placer(eBlockType eType);
	void Render_Preview(eBlockType eType); // 추가 - 배치 위치 미리보기
public:
	void Compute_Ray(_vec3* pRayPos, _vec3* pRayDir);
	bool RayOnGround(_vec3* pRayPos, _vec3* pRayDir, _vec3* pHitOut);
	_vec3 SnapToGrid(_vec3* pHit, eBlockType eType);
	void Undo();
	BlockPos GetPlacePos(eBlockType eType); // 추가 - 현재 배치될 위치 반환
public:
	void SetPresetMode(bool bPreset, int iPreset = 0)
	{
		m_bPresetMode = bPreset;
		m_iSelectedPreset = iPreset;
	}
private:
	bool m_bPresetMode = false;
	int  m_iSelectedPreset = 0;
private:
	LPDIRECT3DDEVICE9 m_pGraphicDev;
	float             m_fBlockSize;
	bool              m_bLBtnPrev;
	bool              m_bRBtnPrev;
	bool              m_bZBtnPrev;
	stack<BlockPos>   m_undoStack;
private:
	void Free() override;
};