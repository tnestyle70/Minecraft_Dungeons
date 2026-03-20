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
	for (auto& pair : m_mapBlocks)
		pair.second->Update_GameObject(fTimeDelta);
}

void CBlockMgr::Render()
{
	switch (m_eRenderMode)
	{
	case RENDER_EDITOR:   Render_Editor();    break;
	case RENDER_BATCH:    Render_Stage();     break;
	case RENDER_QUADTREE: Rendre_QuadTree(); break;
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

void CBlockMgr::Rendre_QuadTree()
{
}

void CBlockMgr::SetRenderMode(eRenderMode eMode)
{
	m_eRenderMode = eMode;

	if (eMode == RENDER_BATCH)
		RebuildBatchMesh();
	else if (eMode == RENDER_QUADTREE)
		BuildQuadTree();
}

void CBlockMgr::BuildQuadTree()
{
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
}

void CBlockMgr::ClearBlocks()
{
	for_each(m_mapBlocks.begin(), m_mapBlocks.end(), [](auto& pair)
		{
			if (pair.second)
				Safe_Release(pair.second);
		});
	m_mapBlocks.clear();
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
	ClearBlocks();
	Safe_Release(m_pBatchBuffer);
	Safe_Release(m_pTexture);
	Safe_Release(m_pGraphicDev);
}