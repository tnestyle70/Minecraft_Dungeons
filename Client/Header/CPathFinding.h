#pragma once
#include "CBase.h"
#include "CProtoMgr.h"
#include "CNavNode.h"

class CPathFinding : public CBase
{
	DECLARE_SINGLETON(CPathFinding)
public:
	explicit CPathFinding();
	virtual ~CPathFinding();
public:
	HRESULT Ready_PathFinder(LPDIRECT3DDEVICE9 pGraphicDev);
	//시작 / 끝 월드 좌표를 받아서 경로 반환
	vector<_vec3> FindPath(const _vec3& vStart, const _vec3& vEnd);
private:
	//해당 X,Z에 블럭이 없고, 발 아래에는 블럭이 존재
	bool IsWalkable(int x, int z);
	//이웃 노드 8 방향 탐색
	vector<NavNode*> GetNeighbors(NavNode* pNode);
	//휴리스틱 - 유클리드 거리
	float Heuristic(int x1, int z1, int x2, int z2);
	//노드 -> 월드 _vec3 변환(Y는 지형 높이로 보장)
	_vec3 NodeToWorld(int x, int z);
	//경로 역추적
	vector<_vec3> ReconstructPath(NavNode* pEndNode);
	//노드 메모리 정리
	void ClearNodes();
	vector<_vec3> SmoothPath(const vector<_vec3>& vecPath);
	bool HasLineOfSight(const _vec3& vFrom, const _vec3& vTo);
private:
	LPDIRECT3DDEVICE9 m_pGraphicDev;
	//탐색 노드용 컨테이너
	vector<NavNode*> m_vecAllocNodes;
	//탐색 범위 제한
	static constexpr int MAX_ITERATORS = 2000;
	//8방향 오프셋
	static const int DIRS[8][2];
	//탐색 기준 y
	int m_iSearchY = 0;
	//블럭 격자 셀 크기(BlockPlacer와 동일하게 1유닛)
	static constexpr int m_iCellSize = 1;
private:
	virtual void Free() override;
};