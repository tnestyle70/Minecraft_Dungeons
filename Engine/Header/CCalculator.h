#pragma once
#include "CComponent.h"

BEGIN(Engine)

class CTerrainTex;
class CTransform;

class ENGINE_DLL CCalculator : public CComponent
{
private:
	explicit CCalculator(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CCalculator(const CCalculator& rhs);
	virtual ~CCalculator();

public:
	HRESULT			Ready_Calculator();

	_float			Compute_HeightOnTerrain(const _vec3* pPos,		// 플레이어 위치
											const _vec3* pTerrainVtxPos,	// 평면 상에 놓인 버텍스 포지션
											const _ulong& dwCntX,
											const _ulong& dwCntZ,
											const _ulong& dwVtxItv = 1);
	
	_vec3			Picking_OnTerrain(HWND hWnd, 
		CTerrainTex* pTerrainBufferCom, 
		CTransform* pTerrainTransformCom);
	
	//레이 생성(스크린 -> 월드) - Add
	void ComputePickRay(HWND hWnd, _vec3* pRayPos, _vec3* pRayDir);

public:
	static CCalculator* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void	Free();
};

END
