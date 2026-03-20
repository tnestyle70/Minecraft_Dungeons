#include "pch.h"
#include "CBlockPlacer.h"
#include "CBlockMgr.h"
#include "CBlockPreset.h"

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
		return 0;

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

	bool bQBtn = (GetAsyncKeyState('Q') & 0x8000) != 0;
	bool bEBtn = (GetAsyncKeyState('E') & 0x8000) != 0;

	if (!bQBtn && !bEBtn)
	{
		if ((!bLBtn || m_bLBtnPrev) && (!bRBtn || m_bRBtnPrev))
		{
			m_bLBtnPrev = bLBtn;
			m_bRBtnPrev = bRBtn;
			return 0;
		}
	}

	_vec3 vRayPos, vRayDir;
	Compute_Ray(&vRayPos, &vRayDir);

	BlockPos tHitPos, tNormal;
	float fT = 0.f;

	// 수정 - 법선 기반 충돌 체크
	bool bBlockHit = CBlockMgr::GetInstance()->RayAABBIntersectWithNormal(
		vRayPos, vRayDir, &tHitPos, &fT, &tNormal);

	// Q - 연속 배치
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		if (bBlockHit)
		{
			// 수정 - 법선 방향으로 배치
			_vec3 vPlacePos = {
				(float)(tHitPos.x + tNormal.x),
				(float)(tHitPos.y + tNormal.y),
				(float)(tHitPos.z + tNormal.z)
			};
			CBlockMgr::GetInstance()->AddBlock(vPlacePos, eType);
			m_undoStack.push(CBlockMgr::GetInstance()->ToPos(vPlacePos));
		}
		else
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

	// E - 연속 삭제
	if (GetAsyncKeyState('E') & 0x8000)
	{
		if (bBlockHit)
			CBlockMgr::GetInstance()->RemoveBlockByPos(tHitPos);
	}

	// 좌클릭 - 단일 배치
	if (bLBtn && !m_bLBtnPrev)
	{
		if (bBlockHit)
		{
			// 수정 - 법선 방향으로 배치
			_vec3 vPlacePos = {
				(float)(tHitPos.x + tNormal.x),
				(float)(tHitPos.y + tNormal.y),
				(float)(tHitPos.z + tNormal.z)
			};

			if (m_bPresetMode)
			{
				switch (m_iSelectedPreset)
				{
				case 0: CBlockPreset::Place_OakTree((int)vPlacePos.x, (int)vPlacePos.y, (int)vPlacePos.z);    break;
				case 1: CBlockPreset::Place_CherryTree((int)vPlacePos.x, (int)vPlacePos.y, (int)vPlacePos.z); break;
				case 2: CBlockPreset::Place_Dragon((int)vPlacePos.x, (int)vPlacePos.y, (int)vPlacePos.z);     break;
				}
			}
			else
			{
				CBlockMgr::GetInstance()->AddBlock(vPlacePos, eType);
				m_undoStack.push(CBlockMgr::GetInstance()->ToPos(vPlacePos));
			}
		}
		else
		{
			_vec3 vHit;
			if (RayOnGround(&vRayPos, &vRayDir, &vHit))
			{
				if (m_bPresetMode)
				{
					_vec3 vSnap = SnapToGrid(&vHit, BLOCK_GRASS);
					switch (m_iSelectedPreset)
					{
					case 0: CBlockPreset::Place_OakTree((int)vSnap.x, (int)vSnap.y, (int)vSnap.z);    break;
					case 1: CBlockPreset::Place_CherryTree((int)vSnap.x, (int)vSnap.y, (int)vSnap.z); break;
					case 2: CBlockPreset::Place_Dragon((int)vSnap.x, (int)vSnap.y, (int)vSnap.z);     break;
					}
				}
				else
				{
					_vec3 vPlacePos = SnapToGrid(&vHit, eType);
					CBlockMgr::GetInstance()->AddBlock(vPlacePos, eType);
					m_undoStack.push(CBlockMgr::GetInstance()->ToPos(vPlacePos));
				}
			}
		}
	}

	// 우클릭 - 삭제
	if (bRBtn && !m_bRBtnPrev)
	{
		if (bBlockHit)
		{
			CBlockMgr::GetInstance()->RemoveBlockByPos(tHitPos);
			while (!m_undoStack.empty())
				m_undoStack.pop();
		}
	}

	m_bLBtnPrev = bLBtn;
	m_bRBtnPrev = bRBtn;

	return 0;
}

// 추가 - 현재 배치될 위치 계산
BlockPos CBlockPlacer::GetPlacePos(eBlockType eType)
{
	_vec3 vRayPos, vRayDir;
	Compute_Ray(&vRayPos, &vRayDir);

	BlockPos tHitPos, tNormal;
	float fT = 0.f;

	if (CBlockMgr::GetInstance()->RayAABBIntersectWithNormal(
		vRayPos, vRayDir, &tHitPos, &fT, &tNormal))
	{
		// 법선 방향으로 배치될 위치
		return { tHitPos.x + tNormal.x,
				 tHitPos.y + tNormal.y,
				 tHitPos.z + tNormal.z };
	}

	// 바닥 평면
	_vec3 vGroundHit;
	if (RayOnGround(&vRayPos, &vRayDir, &vGroundHit))
	{
		_vec3 vSnap = SnapToGrid(&vGroundHit, eType);
		return CBlockMgr::GetInstance()->ToPos(vSnap);
	}

	return { 0, 0, 0 };
}

// 추가 - 배치 위치 미리보기 (흰색 와이어프레임 박스)
void CBlockPlacer::Render_Preview(eBlockType eType)
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	BlockPos tPlacePos = GetPlacePos(eType);

	// 이미 블록 있으면 미리보기 안 보여줌
	if (CBlockMgr::GetInstance()->HasBlock(tPlacePos)) return;

	float fX = (float)tPlacePos.x;
	float fY = (float)tPlacePos.y;
	float fZ = (float)tPlacePos.z;

	float fMinX = fX - 0.5f, fMaxX = fX + 0.5f;
	float fMinY = fY - 0.5f, fMaxY = fY + 0.5f;
	float fMinZ = fZ - 0.5f, fMaxZ = fZ + 0.5f;

	DWORD dwColor = 0xAAFFFFFF; // 반투명 흰색

	struct LineVertex { float x, y, z; DWORD color; };
	LineVertex verts[24] =
	{
		// 아래 면
		{fMinX, fMinY, fMinZ, dwColor}, {fMaxX, fMinY, fMinZ, dwColor},
		{fMaxX, fMinY, fMinZ, dwColor}, {fMaxX, fMinY, fMaxZ, dwColor},
		{fMaxX, fMinY, fMaxZ, dwColor}, {fMinX, fMinY, fMaxZ, dwColor},
		{fMinX, fMinY, fMaxZ, dwColor}, {fMinX, fMinY, fMinZ, dwColor},
		// 위 면
		{fMinX, fMaxY, fMinZ, dwColor}, {fMaxX, fMaxY, fMinZ, dwColor},
		{fMaxX, fMaxY, fMinZ, dwColor}, {fMaxX, fMaxY, fMaxZ, dwColor},
		{fMaxX, fMaxY, fMaxZ, dwColor}, {fMinX, fMaxY, fMaxZ, dwColor},
		{fMinX, fMaxY, fMaxZ, dwColor}, {fMinX, fMaxY, fMinZ, dwColor},
		// 기둥
		{fMinX, fMinY, fMinZ, dwColor}, {fMinX, fMaxY, fMinZ, dwColor},
		{fMaxX, fMinY, fMinZ, dwColor}, {fMaxX, fMaxY, fMinZ, dwColor},
		{fMaxX, fMinY, fMaxZ, dwColor}, {fMaxX, fMaxY, fMaxZ, dwColor},
		{fMinX, fMinY, fMaxZ, dwColor}, {fMinX, fMaxY, fMaxZ, dwColor},
	};

	DWORD dwLighting;
	m_pGraphicDev->GetRenderState(D3DRS_LIGHTING, &dwLighting);

	_matrix matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matIdentity);
	m_pGraphicDev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	m_pGraphicDev->SetTexture(0, nullptr);
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pGraphicDev->DrawPrimitiveUP(D3DPT_LINELIST, 12, verts, sizeof(LineVertex));
	m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, dwLighting);
}

void CBlockPlacer::Compute_Ray(_vec3* pRayPos, _vec3* pRayDir)
{
	POINT ptMouse;
	GetCursorPos(&ptMouse);
	ScreenToClient(g_hWnd, &ptMouse);

	_vec3 vMousePos;
	D3DVIEWPORT9 ViewPort;
	ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));
	m_pGraphicDev->GetViewport(&ViewPort);

	vMousePos.x = ptMouse.x / (ViewPort.Width * 0.5f) - 1.f;
	vMousePos.y = ptMouse.y / -(ViewPort.Height * 0.5f) + 1.f;
	vMousePos.z = 0.f;

	_matrix matInvProj;
	m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matInvProj);
	D3DXMatrixInverse(&matInvProj, 0, &matInvProj);
	D3DXVec3TransformCoord(&vMousePos, &vMousePos, &matInvProj);

	_matrix matInvView;
	m_pGraphicDev->GetTransform(D3DTS_VIEW, &matInvView);
	D3DXMatrixInverse(&matInvView, 0, &matInvView);

	*pRayPos = _vec3(0.f, 0.f, 0.f);
	*pRayDir = vMousePos - *pRayPos;

	D3DXVec3TransformCoord(pRayPos, pRayPos, &matInvView);
	D3DXVec3TransformNormal(pRayDir, pRayDir, &matInvView);
	D3DXVec3Normalize(pRayDir, pRayDir);
}

bool CBlockPlacer::RayOnGround(_vec3* pRayPos, _vec3* pRayDir, _vec3* pHitOut)
{
	if (fabsf(pRayDir->y) < 0.0001f)
		return false;

	float t = -pRayPos->y / pRayDir->y;

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
	if (eType == eBlockType::BLOCK_IRONBAR)
		fBlockSize = 0.5f;

	_vec3 vSnapPos = _vec3(
		ceilf(pHit->x / fBlockSize) * fBlockSize,
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