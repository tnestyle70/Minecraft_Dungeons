#include "pch.h"
#include "CBlockMgr.h"
#include "CRenderer.h"

IMPLEMENT_SINGLETON(CBlockMgr)

CBlockMgr::CBlockMgr()
	: m_pGraphicDev(nullptr)
{
}

CBlockMgr::~CBlockMgr()
{
	Free();
}

HRESULT CBlockMgr::Ready_BlockMgr(LPDIRECT3DDEVICE9 pGraphicDev)
{
	if (m_pGraphicDev)
		return S_OK;

	m_pGraphicDev = pGraphicDev;
	m_pGraphicDev->AddRef();

	m_pTexture = dynamic_cast<CTexture*>(
		CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_BlockAtlasTexture"));
	
	if (!m_pTexture)
	{
		MSG_BOX("Atlas Create Failed");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CBlockMgr::Ready_Textures()
{
	return S_OK;
}

void CBlockMgr::Update(const _float& fTimeDelta)
{
	// dirty 체크 → rebuild
	if (m_bDirty)
	{
		if (m_eRenderMode == RENDER_BATCH)
			RebuildBatchMesh();
		else if (m_eRenderMode == RENDER_QUADTREE)
			BuildQuadTree();
		m_bDirty = false;
	}

	for (auto& pair : m_mapBlocks)
		pair.second->Update_GameObject(fTimeDelta);
}

void CBlockMgr::Render()
{
	switch (m_eRenderMode)
	{
	case RENDER_EDITOR:   Render_Stage();    break;
	case RENDER_BATCH:    Render_Stage();     break;
	case RENDER_QUADTREE: Render_QuadTree(); break;
	default: break;
	}
}

void CBlockMgr::Render_Editor()
{
	for (auto& pair : m_mapBlocks)
		pair.second->Render_GameObject();
}

void CBlockMgr::Render_Stage()
{
	if (!m_pBatchBuffer)
		return;

	_matrix matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matIdentity);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	//블럭 깨짐 방지 - UV Clamping
	m_pGraphicDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	m_pGraphicDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	m_pGraphicDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	if (m_pTexture)
		m_pTexture->Set_Texture(0);

	m_pBatchBuffer->Render_Buffer();

	//UV 복구
	m_pGraphicDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pGraphicDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	m_pGraphicDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void CBlockMgr::BuildQuadTree()
{
	Destroy_Node(m_pQuadRoot);
	m_pQuadRoot = nullptr;

	if (m_mapBlocks.empty())
		return;
	//현재 배치된 블럭들을 감쌀 가장 작은 사각형을 만드는 과정
	//X, Y의 min max를 검증하는 것이 핵심
	float fMinX = FLT_MAX, fMinZ = FLT_MAX;	
	float fMaxX = -FLT_MAX, fMaxZ = -FLT_MAX;

	vector<CBlock*> vecAll;

	for (auto& pair : m_mapBlocks)
	{
		float fX = (float)pair.first.x;
		float fZ = (float)pair.first.z;
		fMinX = min(fMinX, fX); fMaxX = max(fMaxX, fX);
		fMinZ = min(fMinZ, fZ); fMaxZ = max(fMaxZ, fZ);
		vecAll.push_back(pair.second);
	}

	m_pQuadRoot = new QuadNode();

	m_pQuadRoot->fMinX = fMinX - 1.f;
	m_pQuadRoot->fMinZ = fMinZ - 1.f;
	m_pQuadRoot->fMaxX = fMaxX + 1.f;
	m_pQuadRoot->fMaxZ = fMaxZ + 1.f;

	Build_Node(m_pQuadRoot, vecAll, 0);
}

void CBlockMgr::Build_Node(QuadNode* pNode, vector<CBlock*>& vecBlocks, int iDepth)
{
	// 블록 8개 이하이거나 최대 깊이 도달 → 리프
	if (vecBlocks.size() <= 8 || iDepth >= 6)
	{
		pNode->bLeaf = true;
		pNode->vecBlocks = vecBlocks;
		return;
	}

	float fMidX = (pNode->fMinX + pNode->fMaxX) * 0.5f;
	float fMidZ = (pNode->fMinZ + pNode->fMaxZ) * 0.5f;

	// 자식 4개 영역 정의 (XZ 2분할)
	//  [0] 좌전  [1] 우전
	//  [2] 좌후  [3] 우후
	float fChildBounds[4][4] =
	{
		{ pNode->fMinX, pNode->fMinZ, fMidX,         fMidZ         },
		{ fMidX,        pNode->fMinZ, pNode->fMaxX,  fMidZ         },
		{ pNode->fMinX, fMidZ,        fMidX,         pNode->fMaxZ  },
		{ fMidX,        fMidZ,        pNode->fMaxX,  pNode->fMaxZ  }
	};

	vector<CBlock*> vecChild[4];

	for (CBlock* pBlock : vecBlocks)
	{
		_vec3 vPos = pBlock->Get_Pos();
		for (int i = 0; i < 4; ++i)
		{
			if (vPos.x >= fChildBounds[i][0] && vPos.x <= fChildBounds[i][2] &&
				vPos.z >= fChildBounds[i][1] && vPos.z <= fChildBounds[i][3])
			{
				vecChild[i].push_back(pBlock);
				break;
			}
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		if (vecChild[i].empty())
			continue;

		pNode->pChild[i] = new QuadNode();
		pNode->pChild[i]->fMinX = fChildBounds[i][0];
		pNode->pChild[i]->fMinZ = fChildBounds[i][1];
		pNode->pChild[i]->fMaxX = fChildBounds[i][2];
		pNode->pChild[i]->fMaxZ = fChildBounds[i][3];

		Build_Node(pNode->pChild[i], vecChild[i], iDepth + 1);
	}
}

void CBlockMgr::Render_QuadTree()
{
	if (!m_pQuadRoot)
		return;

	// VP 행렬에서 절두체 6면 추출
	_matrix matView, matProj, matVP;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);
	D3DXMatrixMultiply(&matVP, &matView, &matProj);

	D3DXPLANE planes[6];
	// Left
	planes[0] = D3DXPLANE(matVP._14 + matVP._11, matVP._24 + matVP._21,
		matVP._34 + matVP._31, matVP._44 + matVP._41);
	// Right
	planes[1] = D3DXPLANE(matVP._14 - matVP._11, matVP._24 - matVP._21,
		matVP._34 - matVP._31, matVP._44 - matVP._41);
	// Bottom
	planes[2] = D3DXPLANE(matVP._14 + matVP._12, matVP._24 + matVP._22,
		matVP._34 + matVP._32, matVP._44 + matVP._42);
	// Top
	planes[3] = D3DXPLANE(matVP._14 - matVP._12, matVP._24 - matVP._22,
		matVP._34 - matVP._32, matVP._44 - matVP._42);
	// Near
	planes[4] = D3DXPLANE(matVP._13, matVP._23, matVP._33, matVP._43);
	// Far
	planes[5] = D3DXPLANE(matVP._14 - matVP._13, matVP._24 - matVP._23,
		matVP._34 - matVP._33, matVP._44 - matVP._43);

	for (int i = 0; i < 6; ++i)
		D3DXPlaneNormalize(&planes[i], &planes[i]);

	Render_Node(m_pQuadRoot, planes);
}


void CBlockMgr::Render_Node(QuadNode * pNode, const D3DXPLANE * pPlanes)
{
	if (!IsAABBInFrustum(pPlanes, pNode->fMinX, pNode->fMinZ,
		pNode->fMaxX, pNode->fMaxZ))
		return;

	if (pNode->bLeaf)
	{
		for (CBlock* pBlock : pNode->vecBlocks)
			pBlock->Render_GameObject();
		return;
	}

	for (int i = 0; i < 4; ++i)
		if (pNode->pChild[i])
			Render_Node(pNode->pChild[i], pPlanes);
}

void CBlockMgr::Destroy_Node(QuadNode * pNode)
{
	if (!pNode)
		return;
	for (int i = 0; i < 4; ++i)
		Destroy_Node(pNode->pChild[i]);
	delete pNode;
}

bool CBlockMgr::IsAABBInFrustum(const D3DXPLANE* pPlanes, 
	float fMinX, float fMinZ, float fMaxX, float fMaxZ)
{
	const float MARGIN = 20.f;

	D3DXVECTOR3 corners[8] =
	{
		{fMinX - MARGIN, -100.f, fMinZ - MARGIN},
		{fMaxX + MARGIN, -100.f, fMinZ - MARGIN},
		{fMinX - MARGIN,  100.f, fMinZ - MARGIN},
		{fMaxX + MARGIN,  100.f, fMinZ - MARGIN},
		{fMinX - MARGIN, -100.f, fMaxZ + MARGIN},
		{fMaxX + MARGIN, -100.f, fMaxZ + MARGIN},
		{fMinX - MARGIN,  100.f, fMaxZ + MARGIN},
		{fMaxX + MARGIN,  100.f, fMaxZ + MARGIN}
	};

	for (int p = 0; p < 6; ++p)
	{
		bool bAllOut = true;
		for (int c = 0; c < 8; ++c)
		{
			if (D3DXPlaneDotCoord(&pPlanes[p], &corners[c]) >= 0.f)
			{
				bAllOut = false;
				break;
			}
		}
		if (bAllOut) return false;
	}
	return true;
}

void CBlockMgr::SetRenderMode(eRenderMode eMode)
{
	m_eRenderMode = eMode;

	if (eMode == RENDER_BATCH)
		RebuildBatchMesh();
	else if (eMode == RENDER_QUADTREE)
		BuildQuadTree();
}

void CBlockMgr::RebuildBatchMesh()
{
	static const BlockPos s_NeighborDir[CBatchBuffer::FACE_END] =
	{
		{0,  1, 0},  //FACE_TOP
		{0, -1, 0},  //FACE_BOTTOM
		{1,  0, 0},  //FACE_RIGHT
		{-1, 0, 0},  //FACE_LEFT
		{0,  0, 1},  //FACE_FRONT
		{0,  0, -1}  //FACE_BACK
	};

	if (!m_pBatchBuffer)
	{
		m_pBatchBuffer = CBatchBuffer::Create(m_pGraphicDev);
		if (!m_pBatchBuffer)
			return;
	}

	if (m_mapBlocks.empty())
	{
		m_pBatchBuffer->Rebuild({}, {}, {});
		return;
	}

	vector<_vec3>      vecPos;
	vector<eBlockType> vecType;
	vector<bool>       vecFaceVisible;

	for (const auto& pair : m_mapBlocks)
	{
		const BlockPos& pos = pair.first;
		eBlockType      eType = pair.second->GetBlockType();

		vecPos.push_back({ (float)pos.x, (float)pos.y, (float)pos.z });
		vecType.push_back(eType);

		for (int f = 0; f < CBatchBuffer::FACE_END; ++f)
		{
			BlockPos neighbor = {
				pos.x + s_NeighborDir[f].x,
				pos.y + s_NeighborDir[f].y,
				pos.z + s_NeighborDir[f].z
			};
			vecFaceVisible.push_back(!HasBlock(neighbor));
		}
	}

	m_pBatchBuffer->Rebuild(vecPos, vecType, vecFaceVisible);
}

void CBlockMgr::AddBlock(const _vec3& vPos, eBlockType eType)
{
	// 수정 - vPos.y 그대로 사용
	BlockPos tPos = ToPos(vPos);

	if (m_mapBlocks.find(tPos) != m_mapBlocks.end())
		return;

	CBlock* pBlock = CBlock::Create(m_pGraphicDev, vPos, eType);
	if (!pBlock)
		return;

	m_mapBlocks.insert({ tPos, pBlock });
}

void CBlockMgr::AddBlock(int x, int y, int z, eBlockType eType)
{
	BlockPos tPos = { x, y, z };

	if (m_mapBlocks.find(tPos) != m_mapBlocks.end())
		return;

	_vec3 vPos = { (float)x, (float)y, (float)z };
	CBlock* pBlock = CBlock::Create(m_pGraphicDev, vPos, eType);
	if (!pBlock)
		return;

	m_mapBlocks.insert({ tPos, pBlock });

	m_bDirty = true;
}

void CBlockMgr::RemoveBlock(const _vec3& vPos)
{
	BlockPos tPos = ToPos(vPos);

	auto iter = m_mapBlocks.find(tPos);
	if (iter == m_mapBlocks.end())
		return;

	Safe_Release(iter->second);
	m_mapBlocks.erase(iter);
}

void CBlockMgr::RemoveBlockByPos(const BlockPos& pos)
{
	auto iter = m_mapBlocks.find(pos);
	if (iter == m_mapBlocks.end())
		return;

	Safe_Release(iter->second);
	m_mapBlocks.erase(iter);

	m_bDirty = true;
}

void CBlockMgr::ClearBlocks()
{
	for_each(m_mapBlocks.begin(), m_mapBlocks.end(), [](auto& pair)
		{
			if (pair.second)
				Safe_Release(pair.second);
		});
	m_mapBlocks.clear();

	m_bDirty = true;
}

HRESULT CBlockMgr::SaveBlocks(FILE* pFile)
{
	int iCount = (int)m_mapBlocks.size();
	fwrite(&iCount, sizeof(int), 1, pFile);

	for (auto& pair : m_mapBlocks)
	{
		BlockData tData;
		tData.x = pair.first.x;
		tData.y = pair.first.y;
		tData.z = pair.first.z;
		tData.eType = (int)pair.second->GetBlockType();
		fwrite(&tData, sizeof(BlockData), 1, pFile);
	}
	return S_OK;
}

HRESULT CBlockMgr::LoadBlocks(FILE* pFile)
{
	for_each(m_mapBlocks.begin(), m_mapBlocks.end(), [](auto& pair)
		{
			if (pair.second) Safe_Release(pair.second);
		});
	m_mapBlocks.clear();

	int iCount = 0;
	fread(&iCount, sizeof(int), 1, pFile);

	for (int i = 0; i < iCount; ++i)
	{
		BlockData tData;
		fread(&tData, sizeof(tData), 1, pFile);

		_vec3 vPos = {
			static_cast<float>(tData.x),
			static_cast<float>(tData.y),
			static_cast<float>(tData.z)
		};
		BlockPos tPos = ToPos(vPos);

		if (m_mapBlocks.find(tPos) == m_mapBlocks.end())
		{
			CBlock* pBlock = CBlock::Create(m_pGraphicDev, vPos,
				static_cast<eBlockType>(tData.eType));
			if (pBlock)
				m_mapBlocks.insert({ tPos, pBlock });
		}
	}

	if (m_eRenderMode == RENDER_BATCH)
		RebuildBatchMesh();
	else if (m_eRenderMode == RENDER_QUADTREE)
		BuildQuadTree();

	return S_OK;
}

// 기존 - 단순 충돌 체크 (선택 모드, 몬스터 배치 등에서 사용)
bool CBlockMgr::RayAABBIntersect(const _vec3& vRayPos, const _vec3& vRayDir,
	BlockPos* pOutBlockPos, float* pOutT)
{
	float fMinT = FLT_MAX;
	bool  bHit = false;

	for (auto& pair : m_mapBlocks)
	{
		AABB  tAABB = Get_BlockAABB(pair.first);
		float tMin = 0.f, tMax = FLT_MAX;

		// X축
		float t1 = (tAABB.vMin.x - vRayPos.x) / vRayDir.x;
		float t2 = (tAABB.vMax.x - vRayPos.x) / vRayDir.x;
		if (t1 > t2) swap(t1, t2);
		tMin = max(tMin, t1);
		tMax = min(tMax, t2);
		if (tMin > tMax) continue;

		// Y축
		t1 = (tAABB.vMin.y - vRayPos.y) / vRayDir.y;
		t2 = (tAABB.vMax.y - vRayPos.y) / vRayDir.y;
		if (t1 > t2) swap(t1, t2);
		tMin = max(tMin, t1);
		tMax = min(tMax, t2);
		if (tMin > tMax) continue;

		// Z축
		t1 = (tAABB.vMin.z - vRayPos.z) / vRayDir.z;
		t2 = (tAABB.vMax.z - vRayPos.z) / vRayDir.z;
		if (t1 > t2) swap(t1, t2);
		tMin = max(tMin, t1);
		tMax = min(tMax, t2);
		if (tMin > tMax) continue;

		if (tMin < fMinT)
		{
			fMinT = tMin;
			*pOutBlockPos = pair.first;
			*pOutT = tMin;
			bHit = true;
		}
	}

	return bHit;
}

// 추가 - 법선 포함 충돌 체크 (블록 배치 미리보기, 옆면 배치에 사용)
bool CBlockMgr::RayAABBIntersectWithNormal(const _vec3& vRayPos, const _vec3& vRayDir,
	BlockPos* pOutBlockPos, float* pOutT, BlockPos* pOutNormal)
{
	float fMinT = FLT_MAX;
	bool  bHit = false;

	for (auto& pair : m_mapBlocks)
	{
		AABB  tAABB = Get_BlockAABB(pair.first);
		float tMin = 0.f, tMax = FLT_MAX;
		int   iHitAxis = -1;
		bool  bNegative = false;

		// X축
		if (fabsf(vRayDir.x) > 0.0001f)
		{
			float t1 = (tAABB.vMin.x - vRayPos.x) / vRayDir.x;
			float t2 = (tAABB.vMax.x - vRayPos.x) / vRayDir.x;
			if (t1 > t2) swap(t1, t2);
			if (t1 > tMin) { tMin = t1; iHitAxis = 0; bNegative = vRayDir.x > 0; }
			tMax = min(tMax, t2);
			if (tMin > tMax) continue;
		}

		// Y축
		if (fabsf(vRayDir.y) > 0.0001f)
		{
			float t1 = (tAABB.vMin.y - vRayPos.y) / vRayDir.y;
			float t2 = (tAABB.vMax.y - vRayPos.y) / vRayDir.y;
			if (t1 > t2) swap(t1, t2);
			if (t1 > tMin) { tMin = t1; iHitAxis = 1; bNegative = vRayDir.y > 0; }
			tMax = min(tMax, t2);
			if (tMin > tMax) continue;
		}

		// Z축
		if (fabsf(vRayDir.z) > 0.0001f)
		{
			float t1 = (tAABB.vMin.z - vRayPos.z) / vRayDir.z;
			float t2 = (tAABB.vMax.z - vRayPos.z) / vRayDir.z;
			if (t1 > t2) swap(t1, t2);
			if (t1 > tMin) { tMin = t1; iHitAxis = 2; bNegative = vRayDir.z > 0; }
			tMax = min(tMax, t2);
			if (tMin > tMax) continue;
		}

		if (tMin < fMinT)
		{
			fMinT = tMin;
			*pOutBlockPos = pair.first;
			*pOutT = tMin;

			// 법선 계산 - 충돌한 면의 반대 방향
			pOutNormal->x = pOutNormal->y = pOutNormal->z = 0;
			if (iHitAxis == 0) pOutNormal->x = bNegative ? -1 : 1;
			else if (iHitAxis == 1) pOutNormal->y = bNegative ? -1 : 1;
			else if (iHitAxis == 2) pOutNormal->z = bNegative ? -1 : 1;

			bHit = true;
		}
	}

	return bHit;
}

AABB CBlockMgr::Get_BlockAABB(const BlockPos& tPos)
{
	AABB tAABB;
	tAABB.vMax = { tPos.x + 0.5f, tPos.y + 0.5f, tPos.z + 0.5f };
	tAABB.vMin = { tPos.x - 0.5f, tPos.y - 0.5f, tPos.z - 0.5f };
	return tAABB;
}

bool CBlockMgr::HasBlock(const BlockPos& tPos)
{
	return m_mapBlocks.find(tPos) != m_mapBlocks.end();
}

BlockPos CBlockMgr::ToPos(const _vec3& vPos)
{
	return { (int)ceilf(vPos.x), (int)ceilf(vPos.y), (int)ceilf(vPos.z) };
}

void CBlockMgr::FillBlocks(BlockPos pos1, BlockPos pos2, eBlockType eType)
{
	int iMinX = min(pos1.x, pos2.x), iMaxX = max(pos1.x, pos2.x);
	int iMinY = min(pos1.y, pos2.y), iMaxY = max(pos1.y, pos2.y);
	int iMinZ = min(pos1.z, pos2.z), iMaxZ = max(pos1.z, pos2.z);

	for (int x = iMinX; x <= iMaxX; ++x)
		for (int y = iMinY; y <= iMaxY; ++y)
			for (int z = iMinZ; z <= iMaxZ; ++z)
				AddBlock(x, y, z, eType);
}

void CBlockMgr::EraseBlocks(BlockPos pos1, BlockPos pos2)
{
	int iMinX = min(pos1.x, pos2.x), iMaxX = max(pos1.x, pos2.x);
	int iMinY = min(pos1.y, pos2.y), iMaxY = max(pos1.y, pos2.y);
	int iMinZ = min(pos1.z, pos2.z), iMaxZ = max(pos1.z, pos2.z);

	for (int x = iMinX; x <= iMaxX; ++x)
		for (int y = iMinY; y <= iMaxY; ++y)
			for (int z = iMinZ; z <= iMaxZ; ++z)
				RemoveBlockByPos({ x, y, z });
}

vector<BlockData> CBlockMgr::GetBlocksInRange(BlockPos pos1, BlockPos pos2)
{
	vector<BlockData> vecResult;

	int iMinX = min(pos1.x, pos2.x), iMaxX = max(pos1.x, pos2.x);
	int iMinY = min(pos1.y, pos2.y), iMaxY = max(pos1.y, pos2.y);
	int iMinZ = min(pos1.z, pos2.z), iMaxZ = max(pos1.z, pos2.z);

	for (auto& pair : m_mapBlocks)
	{
		const BlockPos& p = pair.first;
		if (p.x >= iMinX && p.x <= iMaxX &&
			p.y >= iMinY && p.y <= iMaxY &&
			p.z >= iMinZ && p.z <= iMaxZ)
		{
			BlockData tData;
			tData.x = p.x;
			tData.y = p.y;
			tData.z = p.z;
			tData.eType = (int)pair.second->GetBlockType();
			vecResult.push_back(tData);
		}
	}
	return vecResult;
}

void CBlockMgr::Free()
{
	Destroy_Node(m_pQuadRoot);
	m_pQuadRoot = nullptr;

	ClearBlocks();
	Safe_Release(m_pBatchBuffer);
	Safe_Release(m_pTexture);
	Safe_Release(m_pGraphicDev);
}