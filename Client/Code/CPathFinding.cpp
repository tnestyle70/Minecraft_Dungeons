#include "pch.h"
#include "CPathFinding.h"
#include "CBlockMgr.h"
#include "CCalculator.h"
#include <queue>
#include <algorithm>
#include <map>


IMPLEMENT_SINGLETON(CPathFinding)

//8방향(상하좌우 + 대각선)용 배열 x z 기준
const int CPathFinding::DIRS[8][2] =
{
	{0, 1}, {0, -1}, {1, 0},{-1, 0}, //상하좌우
	{1, 1}, {1, -1}, {-1, 1}, {-1, -1} //대각선
};

CPathFinding::CPathFinding()
{
}

CPathFinding::~CPathFinding()
{
}

HRESULT CPathFinding::Ready_PathFinder(LPDIRECT3DDEVICE9 pGraphicDev)
{
	m_pGraphicDev = pGraphicDev;
	m_pGraphicDev->AddRef();
	return S_OK;
}

vector<_vec3> CPathFinding::FindPath(const _vec3& vStart, const _vec3& vEnd)
{
	ClearNodes();

	//월드 좌표 -> 격자 좌표 변환
	int iStartX = (int)floorf(vStart.x / m_iCellSize);
	int iStartZ = (int)floorf(vStart.z / m_iCellSize);
	int iEndX = (int)floorf(vEnd.x / m_iCellSize);
	int iEndZ = (int)floorf(vEnd.z / m_iCellSize);
	//도착했을 경우 return
	if (iStartX == iEndX && iStartZ == iEndZ)
		return {};
	//탐색 기준 Y : 
	m_iSearchY = (int)floorf(vStart.y) - 1;
	//오픈 리스트 : f 값 기준 최소힙 정렬
	auto cmp = [](NavNode* a, NavNode* b) {return a->f > b->f; };
	priority_queue<NavNode*, vector<NavNode*>, decltype(cmp)> openQueue(cmp);
	
	//좌표 기반 룩업용 맵 - map의 키를 X, Z 좌표로 사용하기 위해서 pair로 설정
	map<pair<int, int>, NavNode*> openSet;
	map<pair<int, int>, bool> closedSet;

	//시작 노드 생성
	NavNode* pStartNode = new NavNode();
	pStartNode->x = iStartX;
	pStartNode->z = iStartZ;
	pStartNode->g = 0.f;
	pStartNode->h = Heuristic(iStartX, iStartZ, iEndX, iEndZ);
	pStartNode->f = pStartNode->h;
	pStartNode->pParent = nullptr;

	m_vecAllocNodes.push_back(pStartNode);
	openQueue.push(pStartNode);
	openSet[{iStartX, iStartZ}] = pStartNode;
	
	int iIterCount = 0;
	//목적지까지의 좌표 하나 하나 오픈 노드에서 가지고 오기 
	while (!openQueue.empty() && iIterCount < MAX_ITERATORS)
	{
		++iIterCount;

		NavNode* pCurrent = openQueue.top();
		openQueue.pop();

		//map에 저장할 pair<int, int>키를 위한 make_pair
		auto curKey = make_pair(pCurrent->x, pCurrent->z);

		//이미 처리된 노드 스킵 - curKey가 이미 있는지 검증
		if (closedSet.count(curKey))
			continue;

		closedSet[curKey] = true;
		openSet.erase(curKey);

		//도착 확인
		if (pCurrent->x == iEndX && pCurrent->z == iEndZ)
		{
			return ReconstructPath(pCurrent);
		}
		
		//8방향 이웃 탐색
		vector<NavNode*> vecNeighbors = GetNeighbors(pCurrent);

		for (NavNode* pNeighbor : vecNeighbors)
		{
			auto neighborKey = make_pair(pNeighbor->x, pNeighbor->z);

			//이미 처리 완료된 노드는 스킵
			if (closedSet.count(neighborKey))
			{
				delete pNeighbor;
				continue;
			}
			//직선 1.0, 대각선 1.414 이동
			int dx = abs(pNeighbor->x - pCurrent->x);
			int dz = abs(pNeighbor->z - pCurrent->z);
			float fMoveCost = (dx + dz == 2) ? 1.414f : 1.f;
			float fNewG = pCurrent->g + fMoveCost;

			auto it = openSet.find(neighborKey);
			if (it == openSet.end())
			{
				//새 노드 - 오픈 리스트에 추가
				pNeighbor->g = fNewG;
				pNeighbor->h = Heuristic(pNeighbor->x, pNeighbor->z,
					iEndX, iEndZ);
				pNeighbor->f = pNeighbor->g + pNeighbor->h;
				pNeighbor->pParent = pCurrent;

				m_vecAllocNodes.push_back(pNeighbor);
				openQueue.push(pNeighbor);
				openSet[neighborKey] = pNeighbor;
			}
			else if (fNewG < it->second->g)
			{
				//더 좋은 경로 발견 - 기존 노드 갱신 후 큐에 재삽입
				it->second->g = fNewG;
				it->second->f = fNewG + it->second->h;
				it->second->pParent = pCurrent;
				openQueue.push(it->second);
				delete pNeighbor;
			}
			else
			{
				delete pNeighbor;
			}
		}
	}
	//경로 없음
	return {};
}

bool CPathFinding::IsWalkable(int x, int z)
{
	//격자 인덱스 -> 실제 월드 좌표(BlockMgr에 저장된 정수 좌표)
	int worldX = x * m_iCellSize;
	int worldZ = z * m_iCellSize;

	//m_iSearchY 기준으로 위 판단(계단 넘을 수 있도록 설정)
	for (int dy = -1; dy <= 3; ++dy)
	{
		int groundY = m_iSearchY - dy;
		//캐릭터 크기 기준 겹치는 블럭이 있는지 검증
		//** 3D DirectX 좌표계이므로 +Y
		BlockPos tGround = { worldX, groundY, worldZ };
		BlockPos tFeet = { worldX, groundY + 1, worldZ };
		BlockPos tHead = { worldX, groundY + 2, worldZ };

		//땅에 블럭이 있고, -1, -2에 블럭이 없을 경우 이동 가능
		if (CBlockMgr::GetInstance()->HasBlock(tGround) &&
			!CBlockMgr::GetInstance()->HasBlock(tFeet) &&
			!CBlockMgr::GetInstance()->HasBlock(tHead))
		{
			return true;
		}
	}
	return false;
}

vector<NavNode*> CPathFinding::GetNeighbors(NavNode* pNode)
{
	vector<NavNode*> vecNeighbors;

	for (int i = 0; i < 8; ++i)
	{
		int nextX = pNode->x + DIRS[i][0];
		int nextZ = pNode->z + DIRS[i][1];

		//대각선 이동시 양쪽 직선 방향도 통과 가능하도록 설정(코너 끼임 방지)
		if (i >= 4)
		{
			if (!IsWalkable(pNode->x + DIRS[i][0], pNode->z) ||
				!IsWalkable(pNode->x, pNode->z + DIRS[i][1]))
				continue;
		}
		if (!IsWalkable(nextX, nextZ))
			continue;
		NavNode* pNeighbor = new NavNode();
		pNeighbor->x = nextX;
		pNeighbor->z = nextZ;
		vecNeighbors.push_back(pNeighbor);
	}

	return vecNeighbors;
}

float CPathFinding::Heuristic(int x1, int z1, int x2, int z2)
{
	float dx = (float)(x2 - x1) * m_iCellSize;
	float dz = (float)(z2 - z1) * m_iCellSize;
	return sqrtf(dx * dx + dz * dz);
}

_vec3 CPathFinding::NodeToWorld(int x, int z)
{
	//격자 인덱스를 실제 좌표로 변환해서 BlockMgr 조회
	int worldX = x * m_iCellSize;
	int worldZ = z * m_iCellSize;

	//걷기 가능한 실제 Y 탐색
	for (int dy = -1; dy <= 3; ++dy)
	{
		int groundY = m_iSearchY - dy;

		BlockPos tGround = { worldX, groundY, worldZ };
		BlockPos tFeet = { worldX, groundY + 1, worldZ };

		if (CBlockMgr::GetInstance()->HasBlock(tGround) &&
			!CBlockMgr::GetInstance()->HasBlock(tFeet))
		{
			//발 위치 Y 반환
			return _vec3((float)worldX, (float)(groundY + 1), (float)worldZ);
		}
	}
	//못 찾았을 경우 기준 Y + 1
	return _vec3((float)worldX, (float)(m_iSearchY + 1), (float)worldZ);
}

vector<_vec3> CPathFinding::ReconstructPath(NavNode* pEndNode)
{
	// 1. 원래 경로 역추적
	vector<_vec3> vecPath;
	NavNode* pCurrent = pEndNode;

	while (pCurrent != nullptr)
	{
		vecPath.push_back(NodeToWorld(pCurrent->x, pCurrent->z));
		pCurrent = pCurrent->pParent;
	}

	// 역순 → 순방향
	reverse(vecPath.begin(), vecPath.end());

	// 2. 스무딩 적용 후 반환
	return SmoothPath(vecPath);
	/*
	//끝점에서 시작점까지 계산한 노드를 뒤집어서 pushback 
	vector<_vec3> vecPath;

	NavNode* pCurrent = pEndNode;
	while (pCurrent != nullptr)
	{
		vecPath.push_back(NodeToWorld(pCurrent->x, pCurrent->z));
		pCurrent = pCurrent->pParent;
	}
	//역순 -> 순방향 정렬
	reverse(vecPath.begin(), vecPath.end());

	return vecPath;
	*/
}

void CPathFinding::ClearNodes()
{
	for (NavNode* pNode : m_vecAllocNodes)
		delete pNode;

	m_vecAllocNodes.clear();
}

vector<_vec3> CPathFinding::SmoothPath(const vector<_vec3>& vecPath)
{
	if (vecPath.size() <= 2)
		return vecPath;

	vector<_vec3> vecSmoothed;
	vecSmoothed.push_back(vecPath.front());

	int iCheck = 0;

	while (iCheck < (int)vecPath.size() - 1)
	{
		// 가능한 한 멀리 직선으로 갈 수 있는지 체크
		int iFurthest = iCheck + 1;

		for (int i = (int)vecPath.size() - 1; i > iCheck + 1; --i)
		{
			if (HasLineOfSight(vecPath[iCheck], vecPath[i]))
			{
				iFurthest = i;
				break;
			}
		}

		vecSmoothed.push_back(vecPath[iFurthest]);
		iCheck = iFurthest;
	}

	return vecSmoothed;
}

bool CPathFinding::HasLineOfSight(const _vec3& vFrom, const _vec3& vTo)
{
	_vec3 vDir = vTo - vFrom;
	float fDist = D3DXVec3Length(&vDir);
	D3DXVec3Normalize(&vDir, &vDir);

	for (float t = 0.f; t < fDist; t += 0.5f)
	{
		_vec3 vSample = vFrom + vDir * t;

		// 발 위치 체크
		BlockPos tFeet = {
			(int)floorf(vSample.x),
			(int)floorf(vSample.y),
			(int)floorf(vSample.z)
		};
		// 머리 위치 체크 (발 + 1)
		BlockPos tHead = {
			(int)floorf(vSample.x),
			(int)floorf(vSample.y) + 1,
			(int)floorf(vSample.z)
		};

		if (CBlockMgr::GetInstance()->HasBlock(tFeet) ||
			CBlockMgr::GetInstance()->HasBlock(tHead))
			return false;
	}
	return true;
}

void CPathFinding::Free()
{
	ClearNodes();
	Safe_Release(m_pGraphicDev);
}
