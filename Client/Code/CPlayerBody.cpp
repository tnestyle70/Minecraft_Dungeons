#include "pch.h"
#include "CPlayerBody.h"

// 위치 + 2D UV 전용 버텍스 (라이팅 OFF이므로 노멀 불필요)
struct VTXPART
{
	D3DXVECTOR3 vPos;
	D3DXVECTOR2 vUV;
};
const DWORD FVF_PART = D3DFVF_XYZ | D3DFVF_TEX1;

CPlayerBody::CPlayerBody(LPDIRECT3DDEVICE9 pGraphicDev, const FACE_UV* pFaceUVs)
	: CVIBuffer(pGraphicDev)
{
	memcpy(m_FaceUVs, pFaceUVs, sizeof(FACE_UV) * 6);
}

CPlayerBody::~CPlayerBody()
{
}

HRESULT CPlayerBody::Ready_Buffer()
{
	m_dwVtxSize = sizeof(VTXPART);
	m_dwVtxCnt = 24;			// 면당 4버텍스 × 6면
	m_dwTriCnt = 12;			// 면당 2삼각형 × 6면
	m_dwFVF = FVF_PART;
	m_dwIdxSize = sizeof(INDEX16);
	m_IdxFmt = D3DFMT_INDEX16;

	if (FAILED(CVIBuffer::Ready_Buffer()))
		return E_FAIL;

	// ===== 버텍스 세팅 =====
	VTXPART* pVtx = nullptr;
	m_pVB->Lock(0, 0, (void**)&pVtx, 0);

	// verts[0]=TL, [1]=TR, [2]=BR, [3]=BL
	auto SetFace = [&](int base, D3DXVECTOR3 v0, D3DXVECTOR3 v1, D3DXVECTOR3 v2, D3DXVECTOR3 v3, const FACE_UV& uv)
		{
			pVtx[base + 0] = { v0, { uv.u0, uv.v0 } };
			pVtx[base + 1] = { v1, { uv.u1, uv.v0 } };
			pVtx[base + 2] = { v2, { uv.u1, uv.v1 } };
			pVtx[base + 3] = { v3, { uv.u0, uv.v1 } };
		};

	// [0] FRONT (Z=-1)
	SetFace(0,
		{ -1.f,  1.f, -1.f }, { 1.f,  1.f, -1.f },
		{ 1.f, -1.f, -1.f }, { -1.f, -1.f, -1.f },
		m_FaceUVs[0]);

	// [1] BACK (Z=+1) ? 바깥에서 보면 좌우 반전
	SetFace(4,
		{ 1.f,  1.f,  1.f }, { -1.f,  1.f,  1.f },
		{ -1.f, -1.f,  1.f }, { 1.f, -1.f,  1.f },
		m_FaceUVs[1]);

	// [2] TOP (Y=+1)
	SetFace(8,
		{ -1.f,  1.f,  1.f }, { 1.f,  1.f,  1.f },
		{ 1.f,  1.f, -1.f }, { -1.f,  1.f, -1.f },
		m_FaceUVs[2]);

	// [3] BOTTOM (Y=-1)
	SetFace(12,
		{ -1.f, -1.f, -1.f }, { 1.f, -1.f, -1.f },
		{ 1.f, -1.f,  1.f }, { -1.f, -1.f,  1.f },
		m_FaceUVs[3]);

	// [4] LEFT (X=-1)
	SetFace(16,
		{ -1.f,  1.f,  1.f }, { -1.f,  1.f, -1.f },
		{ -1.f, -1.f, -1.f }, { -1.f, -1.f,  1.f },
		m_FaceUVs[4]);

	// [5] RIGHT (X=+1)
	SetFace(20,
		{ 1.f,  1.f, -1.f }, { 1.f,  1.f,  1.f },
		{ 1.f, -1.f,  1.f }, { 1.f, -1.f, -1.f },
		m_FaceUVs[5]);

	m_pVB->Unlock();

	// ===== 인덱스 세팅 (면당 2삼각형, CCW) =====
	INDEX16* pIdx = nullptr;
	m_pIB->Lock(0, 0, (void**)&pIdx, 0);

	for (int i = 0; i < 6; ++i)
	{
		int b = i * 2;		// 삼각형 인덱스 base
		int v = i * 4;		// 버텍스 base
		pIdx[b + 0] = { (_ushort)(v + 0), (_ushort)(v + 1), (_ushort)(v + 2) };
		pIdx[b + 1] = { (_ushort)(v + 0), (_ushort)(v + 2), (_ushort)(v + 3) };
	}

	m_pIB->Unlock();

	return S_OK;
}

void CPlayerBody::Render_Buffer()
{
	CVIBuffer::Render_Buffer();
}

CPlayerBody* CPlayerBody::Create(LPDIRECT3DDEVICE9 pGraphicDev, const FACE_UV* pFaceUVs)
{
	CPlayerBody* pInstance = new CPlayerBody(pGraphicDev, pFaceUVs);
	if (FAILED(pInstance->Ready_Buffer()))
	{
		delete pInstance;
		MSG_BOX("CPlayerCube Create Failed");
		return nullptr;
	}
	return pInstance;
}

void CPlayerBody::Free()
{
	CVIBuffer::Free();
}