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

	//// 배치버퍼만 생성 (프로토타입 접근 없음)
	//map<eBlockType, const _tchar*> tTypeToProto =
	//{
	//	{BLOCK_GRASS, L"Proto_GrassTexture"},
	//	{BLOCK_DIRT,  L"Proto_DirtTexture"},
	//	{BLOCK_ROCK,  L"Proto_RockTexture"},
	//	{BLOCK_SAND,  L"Proto_SandTexture"},
	//};

	//for (auto& pair : tTypeToProto)
	//{
	//	CBatchBuffer* pBatch = CBatchBuffer::Create(m_pGraphicDev);
	//	if (!pBatch) return E_FAIL;
	//	m_mapBatchBuffers.insert({ pair.first, pBatch });
	//}

	return S_OK;
}

HRESULT CBlockMgr::Ready_Textures()
{
	map<eBlockType, const _tchar*> tTypeToProto =
	{
		{BLOCK_GRASS, L"Proto_GrassTexture"},
		{BLOCK_DIRT,  L"Proto_DirtTexture"},
		{BLOCK_ROCK,  L"Proto_RockTexture"},
		{BLOCK_SAND,  L"Proto_SandTexture"},
		{BLOCK_BEDROCK,  L"Proto_BedrockTexture"},
		{BLOCK_OBSIDIAN,  L"Proto_ObsidianTexture"},
		{BLOCK_STONEBRICK,  L"Proto_StoneBrickTexture"},
		{BLOCK_IRONBAR, L"Proto_RockTexture"}
	};

	for (auto& pair : tTypeToProto)
	{
		CTexture* pTexture = dynamic_cast<CTexture*>(
			CProtoMgr::GetInstance()->Clone_Prototype(pair.second));
		if (!pTexture)
		{
			MSG_BOX("Block Texture Clone Failed");
			return E_FAIL;
		}
		m_mapTextures.insert({ pair.first, pTexture });
	}

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
	for (auto& pair : m_mapBlocks)
		pair.second->Render_GameObject();

	//if (m_mapBatchBuffers.empty())
	//	return;
	//
	////정정이 이미 월드 좌표로 bake되어있으므로 단위 행렬을 설정
	//D3DXMATRIX matIdentity;
	//D3DXMatrixIdentity(&matIdentity);
	//m_pGraphicDev->SetTransform(D3DTS_WORLD, &matIdentity);

	////타입별 텍스쳐 바인딩 후 렌더링
	//for (auto& pair : m_mapBatchBuffers)
	//{
	//	//해당 타입 텍스쳐 바인딩
	//	auto itTex = m_mapTextures.find(pair.first);
	//	if (itTex != m_mapTextures.end())
	//	{
	//		itTex->second->Set_Texture(0);
	//	}
	//	pair.second->Render_Buffer();
	//}
}

void CBlockMgr::RebuildBatchMesh()
{
	//타입별 위치 목록 분리
	map<eBlockType, vector<_vec3>> mapByType;

	for (const auto& pair : m_mapBlocks)
	{
		eBlockType eType = pair.second->GetBlockType();
		mapByType[eType].push_back({
			static_cast<float>(pair.first.x),
			static_cast<float>(pair.first.y),
			static_cast<float>(pair.first.z)
			});
	}

	//타입별로 각 배치버퍼 rebuild
	for (auto& pair : m_mapBatchBuffers)
	{
		auto it = mapByType.find(pair.first);
		if (it != mapByType.end())
		{
			pair.second->Rebuild(it->second);
		}
		else//해당 타입의 블럭이 존재하지 않을 경우 블록 비움
		{
			pair.second->Rebuild({});
		}
	}
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

	//블럭이 갱신될 때마다 배치 매쉬 갱신
	RebuildBatchMesh();
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

	//블럭이 제거될 때마다 배치 매쉬 갱신
	RebuildBatchMesh();
}

void CBlockMgr::RemoveBlockByPos(const BlockPos& pos)
{
	auto iter = m_mapBlocks.find(pos);
	if (iter == m_mapBlocks.end())
		return;
	Safe_Release(iter->second);
	m_mapBlocks.erase(iter);
	RebuildBatchMesh();
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

	//배치 매쉬 갱신 - 비어있으므로 버퍼가 0으로 리셋
	RebuildBatchMesh();
}

HRESULT CBlockMgr::SaveBlocks(const _tchar* pFilePath)
{
	//파일에 블럭들의 위치, 타입 정보 저장
	FILE* filePath = nullptr;
	_wfopen_s(&filePath, pFilePath, L"wb");
	if (!filePath)
	{
		MSG_BOX("File Open Failed");
		return E_FAIL;
	}
	//블럭 개수, 블럭 데이터 저장
	int iCount = m_mapBlocks.size();
	fwrite(&iCount, sizeof(int), 1, filePath);
	
	for (auto& pair : m_mapBlocks)
	{
		BlockData tData;
		tData.x = pair.first.x;
		tData.y = pair.first.y;
		tData.z = pair.first.z;
		tData.eType = pair.second->GetBlockType();
		fwrite(&tData, sizeof(BlockData), 1, filePath);
	}
	fclose(filePath);
	return S_OK;
}

HRESULT CBlockMgr::LoadBlocks(const _tchar* pFilePath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, pFilePath, L"rb");
	if (!pFile)
	{
		MSG_BOX("Load File Failed");
		return E_FAIL;
	}

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

	fclose(pFile);

	// 전체 로드 완료 후 딱 1번만 Rebuild
	RebuildBatchMesh();

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

	for (auto& pair : m_mapBatchBuffers)
	{
		Safe_Release(pair.second);
	}
	m_mapBatchBuffers.clear();

	for (auto& pair : m_mapTextures)
	{
		Safe_Release(pair.second);
	}
	m_mapTextures.clear();

	Safe_Release(m_pGraphicDev);
}
