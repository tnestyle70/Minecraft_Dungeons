#include "CCalculator.h"
#include "CTerrainTex.h"
#include "CTransform.h"

CCalculator::CCalculator(LPDIRECT3DDEVICE9 pGraphicDev)
	: CComponent(pGraphicDev)
{
}

CCalculator::CCalculator(const CCalculator& rhs)
	: CComponent(rhs)
{
}

CCalculator::~CCalculator()
{
}

HRESULT CCalculator::Ready_Calculator()
{
	return S_OK;
}

_float CCalculator::Compute_HeightOnTerrain(const _vec3* pPos, 
											const _vec3* pTerrainVtxPos, 
											const _ulong& dwCntX, 
											const _ulong& dwCntZ,
											const _ulong& dwVtxItv)
{
	_ulong		dwIndex = _ulong(pPos->z / dwVtxItv) * dwCntX + _ulong(pPos->x / dwVtxItv);

	_float		fRatioX = (pPos->x - pTerrainVtxPos[dwIndex + dwCntX].x) / dwVtxItv;
	_float		fRatioZ = (pTerrainVtxPos[dwIndex + dwCntX].z - pPos->z) / dwVtxItv;

	D3DXPLANE		Plane;

	// 오른쪽 위
	if (fRatioX > fRatioZ)
	{
		D3DXPlaneFromPoints(&Plane, 
			&pTerrainVtxPos[dwIndex + dwCntX],
			&pTerrainVtxPos[dwIndex + dwCntX + 1],
			&pTerrainVtxPos[dwIndex + 1]);
	}

	// 왼쪽 아래
	else
	{
		D3DXPlaneFromPoints(&Plane,
			&pTerrainVtxPos[dwIndex + dwCntX],
			& pTerrainVtxPos[dwIndex + 1],
			& pTerrainVtxPos[dwIndex]);
	}

	// ax + by + cz + d
	// 
	// by = -ax - cz - d
	// 
	// y = (- ax - cz - d) / b

	return (-Plane.a * pPos->x - Plane.c * pPos->z - Plane.d) / Plane.b;
}

_vec3 CCalculator::Picking_OnTerrain(HWND hWnd,
	CTerrainTex* pTerrainBufferCom,
	CTransform* pTerrainTransformCom)
{
	POINT		ptMouse{};
	GetCursorPos(&ptMouse);
	ScreenToClient(hWnd, &ptMouse);

	_vec3			vMousePos;

	D3DVIEWPORT9	ViewPort;
	ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));

	m_pGraphicDev->GetViewport(&ViewPort);
	
	// 뷰 포트 -> Normalized Device Coordinate (0~800, 0~600)의 스크린 좌표를 -1 ~ 1사이로 변환,
	//다양한 해상도에 대응하기 위함
	vMousePos.x = ptMouse.x / (ViewPort.Width * 0.5f) - 1.f;
	vMousePos.y = ptMouse.y / -(ViewPort.Height * 0.5f) + 1.f;
	vMousePos.z = 0.f;

	// 투영 -> 뷰 스페이스
	D3DXMATRIX	matProj;
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);
	D3DXMatrixInverse(&matProj, 0, &matProj);
	D3DXVec3TransformCoord(&vMousePos, &vMousePos, &matProj);

	// 뷰 스페이스 -> 월드
	D3DXMATRIX	matView;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixInverse(&matView, 0, &matView);

	_vec3	vRayPos{ 0.f, 0.f, 0.f };
	_vec3	vRayDir = vMousePos - vRayPos;

	D3DXVec3TransformCoord(&vRayPos, &vRayPos, &matView);
	D3DXVec3TransformNormal(&vRayDir, &vRayDir, &matView);

	// 월드 -> 로컬
	D3DXMATRIX	matWorld = *pTerrainTransformCom->Get_World();
	D3DXMatrixInverse(&matWorld, 0, &matWorld);

	D3DXVec3TransformCoord(&vRayPos, &vRayPos, &matWorld);
	D3DXVec3TransformNormal(&vRayDir, &vRayDir, &matWorld);

	const _vec3*		pTerrainVtxPos = pTerrainBufferCom->Get_VtxPos();

	_ulong	dwVtxNumber[3]{};
	_float fU(0.f), fV(0.f), fDist(0.f);

	for (_ulong i = 0; i < VTXCNTZ - 1; ++i)
	{
		for (_ulong j = 0; j < VTXCNTX - 1; ++j)
		{
			_ulong dwIndex = i * VTXCNTX + j;

			// 오른쪽 위
			dwVtxNumber[0] = dwIndex + VTXCNTX;
			dwVtxNumber[1] = dwIndex + VTXCNTX + 1;
			dwVtxNumber[2] = dwIndex + 1;

			if (D3DXIntersectTri(&pTerrainVtxPos[dwVtxNumber[1]], 
				&pTerrainVtxPos[dwVtxNumber[0]],
				&pTerrainVtxPos[dwVtxNumber[2]],
				&vRayPos, &vRayDir, 
				&fU, &fV, &fDist))
			{
				return _vec3(pTerrainVtxPos[dwVtxNumber[1]].x + fU * (pTerrainVtxPos[dwVtxNumber[0]].x - pTerrainVtxPos[dwVtxNumber[1]].x),
							 0.f, 
							 pTerrainVtxPos[dwVtxNumber[1]].z + fV * (pTerrainVtxPos[dwVtxNumber[2]].z - pTerrainVtxPos[dwVtxNumber[1]].z));
			}

			// 왼쪽 아래
			dwVtxNumber[0] = dwIndex + VTXCNTX;
			dwVtxNumber[1] = dwIndex + 1;
			dwVtxNumber[2] = dwIndex;

			//V1 + U(V2 - V1) + V(V3 - V1)
			if (D3DXIntersectTri(&pTerrainVtxPos[dwVtxNumber[2]],
								 &pTerrainVtxPos[dwVtxNumber[1]],
								 &pTerrainVtxPos[dwVtxNumber[0]],
								 &vRayPos, &vRayDir,
								 &fU, &fV, &fDist))
			{
				return _vec3(pTerrainVtxPos[dwVtxNumber[2]].x + fU * (pTerrainVtxPos[dwVtxNumber[1]].x - pTerrainVtxPos[dwVtxNumber[2]].x),
					0.f,
					pTerrainVtxPos[dwVtxNumber[2]].z + fV * (pTerrainVtxPos[dwVtxNumber[0]].z - pTerrainVtxPos[dwVtxNumber[2]].z));
			}
		}
	}

	return _vec3(0.f, 0.f, 0.f);
}

void CCalculator::ComputePickRay(HWND hWnd, _vec3* pRayPos, _vec3* pRayDir)
{
	//마우스 피킹 레이 생성
	POINT ptMouse;
	GetCursorPos(&ptMouse);
	ScreenToClient(hWnd, &ptMouse);

	char szBuf[128];
	RECT rc;
	GetClientRect(hWnd, &rc);
	sprintf_s(szBuf, "ClientRect W:%d H:%d | Mouse x:%d y:%d\n",
		rc.right, rc.bottom, ptMouse.x, ptMouse.y);
	OutputDebugStringA(szBuf);
	
	//픽셀 스크린 -> Normalized Device Coordinates
	//(0~800, 0~600 -> ~1, 1  ~1, 1의 NDC로 변환) 다양한 모니터 해상도에 대응하기 위함
	_vec3 vMousePos;
	D3DVIEWPORT9 viewPort;
	ZeroMemory(&viewPort, sizeof(D3DVIEWPORT9));
	m_pGraphicDev->GetViewport(&viewPort);
	
	vMousePos.x = ptMouse.x / (viewPort.Width * 0.5f) - 1.f;
	vMousePos.y = ptMouse.y / -(viewPort.Height * 0.5f) + 1.f;
	vMousePos.z = 0.f;

	//NDC -> 투영(원근감 적용 Z 나누기)역행렬 적용해서 뷰 스페이스로 내리기
	_matrix matProj;
	//Camera에서 세팅한 Projection 행렬 설정
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matProj);
	//역행렬 적용
	D3DXMatrixInverse(&matProj, 0, &matProj);
	D3DXVec3TransformCoord(&vMousePos, &vMousePos, &matProj);
	
	//뷰 스페이스(월드를 카메라 시점으로 이동) -> 월드
	_matrix matView;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matView);
	D3DXMatrixInverse(&matView, 0, &matView);
	//뷰 공간, 카메라 공간의 정의 자체가 카메라를 원점으로 놓고 세계를 재배치한 공간을 의미
	*pRayPos = _vec3(0.f, 0.f, 0.f);
	*pRayDir = vMousePos - *pRayPos;
	//뷰 공간 -> 월드 공간으로 변환, 원점에 뷰의 역행렬을 적용하면 카메라 월드 위치가 됨
	D3DXVec3TransformCoord(pRayPos, pRayPos, &matView); //위치 벡터이므로 coord w = 1 위치 적용 O
	//방향 벡터에 월드의 역행렬을 적용해주면 카메라 기준 방향 벡터가 나옴
	D3DXVec3TransformNormal(pRayDir, pRayDir, &matView);//방향 벡터이므로 normal w = 0 위치 적용 X
	D3DXVec3Normalize(pRayDir, pRayDir);
}

CCalculator* CCalculator::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CCalculator* pCalculator = new CCalculator(pGraphicDev);

	if (FAILED(pCalculator->Ready_Calculator()))
	{
		Safe_Release(pCalculator);
		MSG_BOX("Calculator Create Failed");
		return nullptr;
	}

	return pCalculator;
}

CComponent* CCalculator::Clone()
{
	return new CCalculator(*this);
}

void CCalculator::Free()
{
	CComponent::Free();
}
