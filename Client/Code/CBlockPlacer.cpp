#include "pch.h"
#include "CBlockPlacer.h"
#include "CBlockMgr.h"

CBlockPlacer::CBlockPlacer(LPDIRECT3DDEVICE9 pGraphicDev)
	: m_pGraphicDev(pGraphicDev), m_fBlockSize(1.f),
	m_bLBtnPrev(false), m_bRBtnPrev(false), m_bZBtnPrev(false)
{
}

CBlockPlacer::~CBlockPlacer()
{
	Free();
}

_int CBlockPlacer::Update_Placer(eBlockType eType)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return 0;  // ImGui가 마우스 쓰고 있으면 피킹 안 함

	bool bLBtn = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool bRBtn = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	bool bZBtn = (GetAsyncKeyState('Z') & 0x8000) != 0;

	if (bZBtn && !m_bZBtnPrev)
	{
		if (!m_undoStack.empty())
		{
			CBlockMgr::GetInstance()->RemoveBlockByPos(m_undoStack.top());
			m_undoStack.pop();
		}
	}

	m_bZBtnPrev = bZBtn;

	// 클릭 없으면 레이 계산 안 함
	if ((!bLBtn || m_bLBtnPrev) && (!bRBtn || m_bRBtnPrev))
	{
		m_bLBtnPrev = bLBtn;
		m_bRBtnPrev = bRBtn;
		return 0;
	}

	_vec3 vRayPos, vRayDir;
	Compute_Ray(&vRayPos, &vRayDir);

	//기존 블럭에 레이 충돌 체크
	BlockPos tHitPos;
	float fT = 0.f;
	bool bBlockHit = CBlockMgr::GetInstance()->RayAABBIntersect(vRayPos,
		vRayDir, &tHitPos, &fT);
	
	if (bLBtn && !m_bLBtnPrev)
	{
		if (bBlockHit)
		{
			//히트된 블럭 위에 배치
			_vec3 vPlacePos = { (float)tHitPos.x, (float)tHitPos.y + 1.f,
			(float)tHitPos.z };
			CBlockMgr::GetInstance()->AddBlock(vPlacePos, eType);
			//실제 삽입된 PlacePos를 stack에 저장
			m_undoStack.push(CBlockMgr::GetInstance()->ToPos(vPlacePos));
		}
		else //바닥일 경우 그냥 배치
		{
			_vec3 vHit;
			if (RayOnGround(&vRayPos, &vRayDir, &vHit))
			{
				_vec3 vPlacePos = SnapToGrid(&vHit, eType);
				CBlockMgr::GetInstance()->AddBlock(vPlacePos, eType);
				m_undoStack.push(CBlockMgr::GetInstance()->ToPos(vPlacePos));
			}
		}
	}

	if (bRBtn && !m_bRBtnPrev)
	{
		if (bBlockHit)
		{
			CBlockMgr::GetInstance()->RemoveBlockByPos(tHitPos);
			//스택 클리어 해주기
			while (!m_undoStack.empty())
			{
				m_undoStack.pop();
			}
		}
	}

	m_bLBtnPrev = bLBtn;
	m_bRBtnPrev = bRBtn;

	return 0;

	/*
	_vec3 vHit;

	if (!RayOnGround(&vRayPos, &vRayDir, &vHit))
	{
		m_bLBtnPrev = bLBtn;
		m_bRBtnPrev = bRBtn;
		return 0;
	}

	_vec3 vPlacePos = SnapToGrid(&vHit);

	char szBuf[256];
	sprintf_s(szBuf, 256, "Hit: %.2f %.2f %.2f | Snap: %.2f %.2f %.2f\n",
		vHit.x, vHit.y, vHit.z,
		vPlacePos.x, vPlacePos.y, vPlacePos.z);
	OutputDebugStringA(szBuf);

	if (bLBtn && !m_bLBtnPrev)
	{
		CBlockMgr::GetInstance()->AddBlock(vPlacePos, eType);
		m_undoStack.push(CBlockMgr::GetInstance()->ToPos(vPlacePos));
	}
	
	if (bRBtn && !m_bRBtnPrev)
	{
		CBlockMgr::GetInstance()->RemoveBlock(vPlacePos);
	}
	
	m_bLBtnPrev = bLBtn;
	m_bRBtnPrev = bRBtn;

	return 0;
	*/
}

void CBlockPlacer::Compute_Ray(_vec3* pRayPos, _vec3* pRayDir)
{
	//모니터 -> 클라이언트(뷰포트) -> 투영 -> 뷰 -> 월드 -> 로컬 

	//마우스 전체 모니터 화면 스크린 좌표
	POINT ptMouse;
	GetCursorPos(&ptMouse);
	//모니터 화면 좌표 -> exe 클라이언트 좌표로 변환
	ScreenToClient(g_hWnd, &ptMouse);

	//뷰포트 -> (Normalized Device Coordinated -1 ~ 1사이)
	_vec3 vMousePos;
	D3DVIEWPORT9 ViewPort;
	ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));
	m_pGraphicDev->GetViewport(&ViewPort);

	vMousePos.x = ptMouse.x / (ViewPort.Width * 0.5f) - 1.f;
	vMousePos.y = ptMouse.y / -(ViewPort.Height * 0.5f) + 1.f;
	vMousePos.z = 0.f;
	
	//NDC 투영 역행렬 적용 후 뷰로 변환
	_matrix matInvProj;
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matInvProj);
	D3DXMatrixInverse(&matInvProj, 0, &matInvProj);
	D3DXVec3TransformCoord(&vMousePos, &vMousePos, &matInvProj);
	//뷰 -> 월드
	_matrix matInvView;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matInvView);
	D3DXMatrixInverse(&matInvView, 0, &matInvView);
	//원점에서 카메라의 역행렬을 곱해서 카메라 좌표를 얻어낸다
	*pRayPos = _vec3(0.f, 0.f, 0.f);
	*pRayDir = vMousePos - *pRayPos;
	
	D3DXVec3TransformCoord(pRayPos, pRayPos, &matInvView);
	D3DXVec3TransformNormal(pRayDir, pRayDir, &matInvView);
	D3DXVec3Normalize(pRayDir, pRayDir);

	return;
}

bool CBlockPlacer::RayOnGround(_vec3* pRayPos, _vec3* pRayDir, _vec3* pHitOut)
{
	//y = 0 평면 : t = -raypos.y / raydir.y
	if (fabsf(pRayDir->y) < 0.0001f)
		return false;

	float t = -pRayPos->y / pRayDir->y;

	//카메라 뒤쪽 
	if (t < 0.f)
		return false;

	pHitOut->x = pRayPos->x + pRayDir->x * t;
	pHitOut->y = 0.f;
	pHitOut->z = pRayPos->z + pRayDir->z * t;

	return true;
}

_vec3 CBlockPlacer::SnapToGrid(_vec3* pHit, eBlockType eType)
{
	float fBlockSize = m_fBlockSize;
	//창살일 경우 사이즈 0.5로 줄이기
	if (eType == eBlockType::BLOCK_IRONBAR)
	{
		fBlockSize = 0.5f;
	}

	//ceilf로 블럭 크기 단위에 맞춰서 스냅 시키기
	_vec3 vSnapPos = _vec3(ceilf(pHit->x / fBlockSize) * fBlockSize,
		ceilf(pHit->y / fBlockSize) * fBlockSize,
		ceilf(pHit->z / fBlockSize) * fBlockSize);

	return vSnapPos;
}

void CBlockPlacer::Undo()
{
}

void CBlockPlacer::Free()
{
	Safe_Release(m_pGraphicDev);
}
