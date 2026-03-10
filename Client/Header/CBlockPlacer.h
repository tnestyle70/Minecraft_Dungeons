#pragma once
#include "CBase.h"
#include "CCalculator.h"
#include <stack>
#include <algorithm>

struct BlockPos
{
	int x, y, z;
	//이게 무슨 역할을 하는 거지?
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
	BLOCK_TRIGGERBOX,
	BLOCK_END
};

class CBlockPlacer : public CBase
{
public:
	explicit CBlockPlacer(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CBlockPlacer();
public:
	_int Update_Placer(eBlockType eType);
public:
	//마우스에서 발사하는 Ray
	void Compute_Ray(_vec3* pRayPos, _vec3* pRayDir);
	//Ray와 Y = 0 평면의 교차
	bool RayOnGround(_vec3* pRayPos, _vec3* pRayDir, _vec3* pHitOut);
	//그리드 스냅
	_vec3 SnapToGrid(_vec3* pHit, eBlockType eType);

	void Undo();
private: 
	LPDIRECT3DDEVICE9 m_pGraphicDev;
	float m_fBlockSize;
	bool m_bLBtnPrev;
	bool m_bRBtnPrev;
	bool m_bZBtnPrev;
	stack<BlockPos> m_undoStack;
private:
	void Free() override;
};