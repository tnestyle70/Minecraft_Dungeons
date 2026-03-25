#include "CTerrainTex.h"

CTerrainTex::CTerrainTex() : m_pPos(nullptr)
{
}

CTerrainTex::CTerrainTex(LPDIRECT3DDEVICE9 pGraphicDev)
	: CVIBuffer(pGraphicDev), m_hFile(nullptr), m_pPos(nullptr)
{
}

CTerrainTex::CTerrainTex(const CTerrainTex& rhs)
	: CVIBuffer(rhs)
	, m_hFile(rhs.m_hFile)
	, m_fH(rhs.m_fH)
	, m_iH(rhs.m_iH)
	, m_pPos(rhs.m_pPos)
{
}

CTerrainTex::~CTerrainTex()
{
}

HRESULT CTerrainTex::Ready_Buffer(const _ulong& dwVtxCntX,
								const _ulong& dwVtxCntZ,
								const _ulong& dwVtxItv)
{
	m_dwVtxSize = sizeof(VTXTEX);
	m_dwVtxCnt = dwVtxCntX * dwVtxCntZ;
	m_dwTriCnt = (dwVtxCntX - 1) * (dwVtxCntZ - 1) * 2;
	m_dwFVF = FVF_TEX;

	m_dwIdxSize = sizeof(INDEX32);
	m_IdxFmt = D3DFMT_INDEX32;

	m_pPos = new _vec3[m_dwVtxCnt];

	if (FAILED(CVIBuffer::Ready_Buffer()))
		return E_FAIL;

	m_hFile = CreateFile(L"../Bin/Resource/Texture/Terrain/Height.bmp", 
						GENERIC_READ, 
						FILE_SHARE_READ,
						0,
						OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL, 
						0);

	if (INVALID_HANDLE_VALUE == m_hFile)
		return E_FAIL;

	_ulong	dwByte(0);

	ReadFile(m_hFile, &m_fH, sizeof(BITMAPFILEHEADER), &dwByte, NULL);
	ReadFile(m_hFile, &m_iH, sizeof(BITMAPINFOHEADER), &dwByte, NULL);

	_ulong* pPixel = new _ulong[m_iH.biWidth * m_iH.biHeight];

	ReadFile(m_hFile, pPixel, sizeof(_ulong) * m_iH.biWidth * m_iH.biHeight, &dwByte, NULL);


	VTXTEX* pVertex = NULL;
	_ulong	dwIndex(0);

	// &pVertex : 버텍스 버퍼에 저장된 버텍스 중 첫번째 버텍스의 주소를 얻어옴
	m_pVB->Lock(0, 0, (void**)&pVertex, 0);

	for (_ulong i = 0; i < dwVtxCntZ; ++i)
	{
		for (_ulong j = 0; j < dwVtxCntX; ++j)
		{
			dwIndex = i * dwVtxCntX + j;

			pVertex[dwIndex].vPosition = { _float(j * dwVtxItv), 
											_float(pPixel[dwIndex] & 0x000000ff) / 20.f,
											_float(i * dwVtxItv) };
		
			pVertex[dwIndex].vTexUV	   = { float(j) / (dwVtxCntX - 1) * 20.f, 
										   float(i) / (dwVtxCntZ - 1) * 20.f};

			m_pPos[dwIndex] = pVertex[dwIndex].vPosition;
		}
	}

	Safe_Delete_Array(pPixel);

	INDEX32* pIndex = nullptr;

	_vec3 vDst, vSrc, vNormal;

	_ulong dwTriCnt(0);

	m_pIB->Lock(0, 0, (void**)&pIndex, 0);

	for (_ulong i = 0; i < dwVtxCntZ - 1; ++i)
	{
		for (_ulong j = 0; j < dwVtxCntX - 1; ++j)
		{
			dwIndex = i * dwVtxCntX + j;

			// 오른쪽 위
			pIndex[dwTriCnt]._0 = dwIndex + dwVtxCntX;
			pIndex[dwTriCnt]._1 = dwIndex + dwVtxCntX + 1;
			pIndex[dwTriCnt]._2 = dwIndex + 1;

			//법선 계산하기(시계방향으로 외적 순서 정해서 넣기)
			vDst = pVertex[pIndex[dwTriCnt]._1].vPosition - pVertex[pIndex[dwTriCnt]._0].vPosition;
			vSrc = pVertex[pIndex[dwTriCnt]._2].vPosition - pVertex[pIndex[dwTriCnt]._1].vPosition;
			//0->1로 향하는 벡터와 1->2로 향하는 벡터를 외적해서 삼각형의 법선 벡터 구하기
			D3DXVec3Cross(&vNormal, &vDst, &vSrc);
			//삼각형의 세 정점에 normal 법선 값 설정해주기
			pVertex[pIndex[dwTriCnt]._0].vNormal += vNormal;
			pVertex[pIndex[dwTriCnt]._1].vNormal += vNormal;
			pVertex[pIndex[dwTriCnt]._2].vNormal += vNormal;

			dwTriCnt++;

			// 왼쪽 아래
			pIndex[dwTriCnt]._0 = dwIndex + dwVtxCntX;
			pIndex[dwTriCnt]._1 = dwIndex + 1;
			pIndex[dwTriCnt]._2 = dwIndex;

			vDst = pVertex[pIndex[dwTriCnt]._1].vPosition - pVertex[pIndex[dwTriCnt]._0].vPosition;
			vSrc = pVertex[pIndex[dwTriCnt]._2].vPosition - pVertex[pIndex[dwTriCnt]._1].vPosition;

			D3DXVec3Cross(&vNormal, &vDst, &vSrc);

			pVertex[pIndex[dwTriCnt]._0].vNormal += vNormal;
			pVertex[pIndex[dwTriCnt]._1].vNormal += vNormal;
			pVertex[pIndex[dwTriCnt]._2].vNormal += vNormal;

			dwTriCnt++;
		}
	}
	//정규화 시켜주기
	for (int i = 0; i < m_dwVtxCnt; ++i)
	{
		D3DXVec3Normalize(&pVertex[i].vNormal, &pVertex[i].vNormal);
	}

	//vertex buffer를 나중에 해제해서 해당 vertex buffer의 정보를 가지고 normal 값을 계산한다
	m_pVB->Unlock();
	m_pIB->Unlock();

	return S_OK;
}

void CTerrainTex::Render_Buffer()
{
	CVIBuffer::Render_Buffer();
}

CTerrainTex* CTerrainTex::Create(LPDIRECT3DDEVICE9 pGraphicDev, 
								const _ulong& dwVtxCntX,
								const _ulong& dwVtxCntZ,
								const _ulong& dwVtxItv)
{
	CTerrainTex* pTerrainTex = new CTerrainTex(pGraphicDev);

	if (FAILED(pTerrainTex->Ready_Buffer(dwVtxCntX, dwVtxCntZ, dwVtxItv)))
	{
		Safe_Release(pTerrainTex);
		MSG_BOX("pTerrainTex Create Failed");
		return nullptr;
	}

	return pTerrainTex;
}

CComponent* CTerrainTex::Clone()
{
	return new CTerrainTex(*this);
}

void CTerrainTex::Free()
{
	if (false == m_bClone)
	{
		Safe_Delete_Array(m_pPos);
	}

	CVIBuffer::Free();
}
