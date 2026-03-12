#include "pch.h"
#include "CBlockMgr.h"

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
	//이미 graphicDev가 설정되어있을 경우 return
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
	{
		pair.second->Update_GameObject(fTimeDelta);
	}
}

void CBlockMgr::Render()
{
	if (m_bEditorMode)
	{
		Render_Editor();
	}
	else
	{
		Render_Stage();
	}
}

void CBlockMgr::Render_Editor()
{
	//기존 최적화 X 렌더링
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

	if (m_pTexture)
		m_pTexture->Set_Texture(0);

	m_pBatchBuffer->Render_Buffer();   // 드로우콜 1번

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void CBlockMgr::SetEditorMode(bool bEditor)
{
	m_bEditorMode = bEditor;
	if (!bEditor)
		RebuildBatchMesh();
}

void CBlockMgr::RebuildBatchMesh()
{
	//면컬링용 인접 방향 오프셋
	//BatchBuffer::EFace 순서와 일치 시키기
	//인접한 면에 블럭 면이 있는지 판단해서 있으면 그리지 않기!
	static const BlockPos s_NeighborDir[CBatchBuffer::FACE_END] =
	{
		{0, 1, 0}, //FACE_TOP
		{0, -1, 0}, //FACE_BOTTOM
		{1, 0, 0}, //FACE_RIGHT
		{-1, 0, 0}, //FACE_LEFT
		{0, 0, 1}, //FACE_FRONT
		{0, 0, -1} //FACE_BACK
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

	vector<_vec3> vecPos;
	vector<eBlockType> vecType;
	vector<bool> vecFaceVisible;

	for (const auto& pair : m_mapBlocks)
	{
		const BlockPos& pos = pair.first;
		eBlockType eType = pair.second->GetBlockType();

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
	//vecType 쪽 분리
	m_pBatchBuffer->Rebuild(vecPos, vecType, vecFaceVisible);
}

void CBlockMgr::AddBlock(const _vec3& vPos, eBlockType eType)
{
	//입력 좌표 변환값을 기준으로 계산
	BlockPos tInputPos = ToPos(vPos);

	//해당 xz에서 가장 높은 Y 찾아서 배치
	int iTopY = 0;

	for (auto& pair : m_mapBlocks)
	{
		//같은 위치일 경우 y 값보다 더 위에 배치
		if (pair.first.x == tInputPos.x &&
			pair.first.z == tInputPos.z)
		{
			if (pair.first.y >= iTopY)
			{
				iTopY = pair.first.y + 1;
			}
		}
	}

	_vec3 vStackedPos = { vPos.x, (float)iTopY, vPos.z };
	BlockPos tPos = ToPos(vStackedPos);
	//해당 위치에 이미 블럭이 존재할 경우 return 
	if (m_mapBlocks.find(tPos) != m_mapBlocks.end())
	{
		return;
	}

	CBlock* pBlock = CBlock::Create(m_pGraphicDev, vStackedPos, eType);

	if (!pBlock)
		return;

	m_mapBlocks.insert({ tPos, pBlock });

	if (!m_bEditorMode)
	{
		//블럭이 갱신될 때마다 배치 매쉬 갱신
		RebuildBatchMesh();
	}
}

void CBlockMgr::RemoveBlock(const _vec3& vPos)
{
	//스크린 -> exe 클라 화면(뷰포트) -> 투영 -> 뷰스페이스 -> 월드 -> 로컬
	BlockPos tPos = ToPos(vPos);

	auto iter = m_mapBlocks.find(tPos);
	if (iter == m_mapBlocks.end())
		return;
	Safe_Release(iter->second);
	m_mapBlocks.erase(iter);

	if (!m_bEditorMode)
	{
		//블럭이 갱신될 때마다 배치 매쉬 갱신
		RebuildBatchMesh();
	}
}

void CBlockMgr::RemoveBlockByPos(const BlockPos& pos)
{
	auto iter = m_mapBlocks.find(pos);
	if (iter == m_mapBlocks.end())
		return;
	Safe_Release(iter->second);
	m_mapBlocks.erase(iter);
	if (!m_bEditorMode)
	{
		//블럭이 갱신될 때마다 배치 매쉬 갱신
		RebuildBatchMesh();
	}
}

void CBlockMgr::ClearBlocks()
{
	//map에 존재하는 블럭들 비워주기
	for_each(m_mapBlocks.begin(), m_mapBlocks.end(), [](auto& pair)
		{
			if (pair.second)
			{
				Safe_Release(pair.second);
			}
		});
	m_mapBlocks.clear();

	if (!m_bEditorMode)
	{
		//블럭이 갱신될 때마다 배치 매쉬 갱신
		RebuildBatchMesh();
	}
}

HRESULT CBlockMgr::SaveBlocks(FILE* pFile)
{
	//매개 변수로 넘겨 받은 파일에 정보 저장
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
	// 기존 블럭 전부 제거 (내부에서 Rebuild 1번 호출되지만 무시)
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

		_vec3    vPos = { static_cast<float>(tData.x),
						  static_cast<float>(tData.y),
						  static_cast<float>(tData.z) };
		BlockPos tPos = ToPos(vPos);

		// 중복 방지 후 직접 삽입 (Rebuild 없음)
		if (m_mapBlocks.find(tPos) == m_mapBlocks.end())
		{
			CBlock* pBlock = CBlock::Create(m_pGraphicDev, vPos,
				static_cast<eBlockType>(tData.eType));
			if (pBlock)
				m_mapBlocks.insert({ tPos, pBlock });
		}
	}

	if (!m_bEditorMode)
	{
		// 전체 로드 완료 후 딱 1번만 Rebuild
		RebuildBatchMesh();
	}

	return S_OK;
}

bool CBlockMgr::RayAABBIntersect(const _vec3& vRayPos, const _vec3& vRayDir, 
	BlockPos* pOutBlockPos, float* pOutT)
{
	//Calculator에서 계산한 Ray를 바탕으로 AABB 충돌 처리 후 
	//충돌한 블럭 반환
	float fMinT = FLT_MAX;
	bool bHit = false;
	//모든 블럭들과 충돌 처리
	for (auto& pair : m_mapBlocks)
	{
		AABB tAABB = Get_BlockAABB(pair.first);
		float fT = 0.f;
		//X축
		float tMin = 0.f;
		float tMax = FLT_MAX;

		float t1 = (tAABB.vMin.x - vRayPos.x) / vRayDir.x;
		float t2 = (tAABB.vMax.x - vRayPos.x) / vRayDir.x;
		if (t1 > t2)
			swap(t1, t2);

		tMin = max(tMin, t1);
		tMax = min(tMax, t2);

		if (tMin > tMax)
			continue;

		//Y축
		t1 = (tAABB.vMin.y - vRayPos.y) / vRayDir.y;
		t2 = (tAABB.vMax.y - vRayPos.y) / vRayDir.y;
		if (t1 > t2)
			swap(t1, t2);
		tMin = max(tMin, t1);
		tMax = min(tMax, t2);
		if (tMin > tMax)
			continue;

		//Z축
		t1 = (tAABB.vMin.z - vRayPos.z) / vRayDir.z;
		t2 = (tAABB.vMax.z - vRayPos.z) / vRayDir.z;
		if (t1 > t2)
			swap(t1, t2);
		tMin = max(tMin, t1);
		tMax = min(tMax, t2);
		if (tMin > tMax)
			continue;

		//가장 가까운 블럭 갱신
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

AABB CBlockMgr::Get_BlockAABB(const BlockPos& tPos)
{
	AABB tAABB;
	//tPos 기준 aabb 값 설정해서 return
	tAABB.vMax = { tPos.x + 0.5f, tPos.y + 0.5f, tPos.z + 0.5f };
	tAABB.vMin = { tPos.x - 0.5f, tPos.y - 0.5f, tPos.z - 0.5f };
	return tAABB;
}

bool CBlockMgr::HasBlock(const BlockPos& tPos)
{
	//헤당 tPos에 블럭이 존재하는지 판단
	return m_mapBlocks.find(tPos) != m_mapBlocks.end();
}

BlockPos CBlockMgr::ToPos(const _vec3& vPos)
{
	//float로 들어온 값을 int로 변환해서 저장
	return {(int)ceilf(vPos.x), (int)ceilf(vPos.y), (int)ceilf(vPos.z)};
}

void CBlockMgr::Free()
{
	ClearBlocks();
	Safe_Release(m_pBatchBuffer);
	Safe_Release(m_pTexture);
	Safe_Release(m_pGraphicDev);
}
