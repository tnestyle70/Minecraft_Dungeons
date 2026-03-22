#include "CCollider.h"

CCollider::CCollider(LPDIRECT3DDEVICE9 pGraphicDev)
	: CComponent(pGraphicDev), 
	m_vScale(1.f, 1.f, 1.f),
	m_vOffset(0.f, 0.f, 0.f),
	m_pDebugMesh(nullptr),
	m_bColliding(false)
{
	ZeroMemory(&m_tAABB, sizeof(m_tAABB));
	ZeroMemory(&m_tOBB, sizeof(m_tOBB));
}

CCollider::CCollider(const CCollider& rhs)
	: CComponent(rhs),
	m_vScale(rhs.m_vScale),
	m_vOffset(rhs.m_vOffset),
	m_tAABB(rhs.m_tAABB),
	m_tOBB(rhs.m_tOBB),
	m_pDebugMesh(rhs.m_pDebugMesh),
	m_bColliding(rhs.m_bColliding)
{
}

CCollider::~CCollider()
{
}

HRESULT CCollider::Ready_Collider(const _vec3& vScale, const _vec3& vOffset)
{
	//offset과 size를 반영한 collider 설정
	m_vScale = vScale;
	m_vOffset = vOffset;
	m_vOriginalHalfSize = vScale * 0.5f;

	if (FAILED(D3DXCreateBox(m_pGraphicDev, vScale.x, vScale.y,
		vScale.z, &m_pDebugMesh, nullptr)))
	{
		MSG_BOX("Debug Mesh Create Failed");
		return E_FAIL;
	}

	return S_OK;
}

_int CCollider::Update_Component(const _float& fTimeDelta)
{
	return 0;
}

void CCollider::Update_AABB(const _vec3& vWorldPos)
{
	//월드 위치와 offset 기준으로 오프셋 계산
	_vec3 vCenter = vWorldPos + m_vOffset;
	//AABB 구조체의 정보 채워주기
	m_tAABB.vMax = vCenter + (m_vScale * 0.5f);
	m_tAABB.vMin = vCenter - (m_vScale * 0.5f);
}

void CCollider::Update_OBB(const _matrix& matWorld)
{
	// 1. 중심 (translation)
	_vec3 vOffsetWorld;

	// 회전까지 고려해서 offset 변환
	vOffsetWorld.x = m_vOffset.x * matWorld._11 + m_vOffset.y * matWorld._21 + m_vOffset.z * matWorld._31;
	vOffsetWorld.y = m_vOffset.x * matWorld._12 + m_vOffset.y * matWorld._22 + m_vOffset.z * matWorld._32;
	vOffsetWorld.z = m_vOffset.x * matWorld._13 + m_vOffset.y * matWorld._23 + m_vOffset.z * matWorld._33;

	m_tOBB.vCenter = D3DXVECTOR3(matWorld._41, matWorld._42, matWorld._43) + vOffsetWorld;

	// 2. 축 추출 (회전 포함됨)
	m_tOBB.vAxis[0] = D3DXVECTOR3(matWorld._11, matWorld._12, matWorld._13); // right
	m_tOBB.vAxis[1] = D3DXVECTOR3(matWorld._21, matWorld._22, matWorld._23); // up
	m_tOBB.vAxis[2] = D3DXVECTOR3(matWorld._31, matWorld._32, matWorld._33); // look

	// 3. 축 정규화 + 스케일 추출
	_float fScaleX = D3DXVec3Length(&m_tOBB.vAxis[0]);
	_float fScaleY = D3DXVec3Length(&m_tOBB.vAxis[1]);
	_float fScaleZ = D3DXVec3Length(&m_tOBB.vAxis[2]);

	D3DXVec3Normalize(&m_tOBB.vAxis[0], &m_tOBB.vAxis[0]);
	D3DXVec3Normalize(&m_tOBB.vAxis[1], &m_tOBB.vAxis[1]);
	D3DXVec3Normalize(&m_tOBB.vAxis[2], &m_tOBB.vAxis[2]);

	// 4. halfSize에 스케일 반영
	m_tOBB.vHalfSize.x = m_vOriginalHalfSize.x * fScaleX;
	m_tOBB.vHalfSize.y = m_vOriginalHalfSize.y * fScaleY;
	m_tOBB.vHalfSize.z = m_vOriginalHalfSize.z * fScaleZ;
}

bool CCollider::IsColliding(const AABB& other) const
{
	//AABB vs AABB - 모든 축이 min, max 내부에 들어올 경우에 충돌
	if (m_tAABB.vMax.x <= other.vMin.x || m_tAABB.vMin.x >= other.vMax.x)
		return false;
	if (m_tAABB.vMax.y <= other.vMin.y || m_tAABB.vMin.y >= other.vMax.y)
		return false;
	if (m_tAABB.vMax.z <= other.vMin.z || m_tAABB.vMin.z >= other.vMax.z)
		return false;	
	return true;
}

bool CCollider::IntersectRay(const _vec3& vRayOrigin, const _vec3& vRayDir)
{
	//AABB 기준으로 슬랩 테스트 - AABB의 각 축을 두 평면 한 쌍의 슬랩으로 판단
	float tMin = -FLT_MAX;
	float tMax = FLT_MAX;

	float* pOrigin[3] = { (float*)&vRayOrigin.x, (float*)&vRayOrigin.y, (float*)&vRayOrigin.z };
	float* pDir[3] = { (float*)&vRayDir.x, (float*)&vRayDir.y, (float*)&vRayDir.z };
	float* pMin[3] = { (float*)&m_tAABB.vMin.x, (float*)&m_tAABB.vMin.y, (float*)&m_tAABB.vMin.z };
	float* pMax[3] = { (float*)&m_tAABB.vMax.x, (float*)&m_tAABB.vMax.y, (float*)&m_tAABB.vMax.z };

	for (int i = 0; i < 3; ++i)
	{
		float fDir = *pDir[i];

		//예외처리 추가 - 평행일 경우 

		//Ray의 Origin과 AABB와의 충돌 처리
		float fT1 = (*pMin[i] - *pOrigin[i]) / fDir;
		float fT2 = (*pMax[i] - *pOrigin[i]) / fDir;
		
		if (fT1 > fT2)
		{
			swap(fT1, fT2);
		}
		//모든 AABB 축 방향에 대한 충돌 처리를 진행
		tMin = max(tMin, fT1);
		tMax = min(tMax, fT2);

		if (tMin > tMax)
			return false;
	}
	//tMax가 < 0일 경우 뒤쪽에 존재하는 상태
	return tMax >= 0;
}

_vec3 CCollider::Resolve(const AABB& other) const
{
	//충돌했을 경우 반대 방향으로 밀어주기

	//각 축별 겹침 깊이 계산
	float fOverlapX = min(m_tAABB.vMax.x, other.vMax.x) - max(m_tAABB.vMin.x, other.vMin.x);
	float fOverlapY = min(m_tAABB.vMax.y, other.vMax.y) - max(m_tAABB.vMin.y, other.vMin.y);
	float fOverlapZ = min(m_tAABB.vMax.z, other.vMax.z) - max(m_tAABB.vMin.z, other.vMin.z);

	//가장 작은 겹침 축으로만 밀어내기(최소 분리 벡터)
	_vec3 vResolve = { 0.f, 0.f, 0.f };

	//Y축 충돌(바닥/천장)
	if (fOverlapY < fOverlapX && fOverlapY < fOverlapZ)
	{
		//바닥이면 위로, 천장이면 아래로 resolve
		float fSign = (m_tAABB.vMin.y < other.vMin.y) ? -1.f : 1.f;
		vResolve.y = fOverlapY * fSign;
	}
	//X축 충돌(좌우 벽)
	else if (fOverlapX < fOverlapZ)
	{
		float fSign = (m_tAABB.vMin.x < other.vMin.x) ? -1.f : 1.f;
		vResolve.x = fOverlapX * fSign;
	}
	//Z축 충돌(앞뒤 벽)
	else 
	{
		float fSign = (m_tAABB.vMin.z < other.vMin.z) ? -1.f : 1.f;
		vResolve.z = fOverlapZ * fSign;
	}

	return vResolve;
}

void CCollider::Render_Collider()
{
	//디버깅용 콜라이더 렌더링
	if (!m_pDebugMesh)
	{
		return;
	}
	//  라이팅 OFF - 이거 없으면 material 색상이 광원에 묻혀버림
	//m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
	//  텍스처 OFF - 블럭 텍스처가 와이어프레임 위에 영향 줌
	m_pGraphicDev->SetTexture(0, nullptr);

	//현재 콜라이더 중심 위치로 월드 행렬 세팅
	_vec3 vCenter = (m_tAABB.vMin + m_tAABB.vMax) * 0.5f;
	_matrix matWorld;
	D3DXMatrixTranslation(&matWorld, vCenter.x, vCenter.y, vCenter.z);
	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);
	//와이어프레임 렌더링
	m_pGraphicDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	//충돌 중이면 빨강 아니면 초록 - material 값 설정
	D3DMATERIAL9 material;
	ZeroMemory(&material, sizeof(D3DMATERIAL9));
	if (m_bColliding)
	{
		material.Emissive = { 1.f, 0.f, 0.f, 1.f };
	}
	else
	{
		material.Emissive = { 0.f, 0.f, 0.f, 1.f };
	}
	m_pGraphicDev->SetMaterial(&material);
	m_pDebugMesh->DrawSubset(0);
	//렌더링 후 원상 복구
	m_pGraphicDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	//콜라이더 월드 행렬 원상 복구
	//_matrix matIdentity;
	//D3DXMatrixIdentity(&matIdentity);
	//m_pGraphicDev->SetTransform(D3DTS_WORLD, &matIdentity);
}

void CCollider::Render_OBB()
{
	if (!m_pDebugMesh)
		return;

	// 텍스처 끄기
	m_pGraphicDev->SetTexture(0, nullptr);

	// -----------------------------
	// 1. Scale 행렬
	// -----------------------------
	_matrix matScale;
	D3DXMatrixScaling(&matScale,
						1.f,
						1.f,
						1.f);

	// -----------------------------
	// 2. Rotation 행렬 (axis 기반)
	// -----------------------------
	_matrix matRot;

	matRot._11 = m_tOBB.vAxis[0].x;
	matRot._12 = m_tOBB.vAxis[0].y;
	matRot._13 = m_tOBB.vAxis[0].z;

	matRot._21 = m_tOBB.vAxis[1].x;
	matRot._22 = m_tOBB.vAxis[1].y;
	matRot._23 = m_tOBB.vAxis[1].z;

	matRot._31 = m_tOBB.vAxis[2].x;
	matRot._32 = m_tOBB.vAxis[2].y;
	matRot._33 = m_tOBB.vAxis[2].z;

	matRot._14 = matRot._24 = matRot._34 = 0.f;
	matRot._41 = matRot._42 = matRot._43 = 0.f;
	matRot._44 = 1.f;

	// -----------------------------
	// 3. Translation
	// -----------------------------
	_matrix matTrans;
	D3DXMatrixTranslation(&matTrans,
		m_tOBB.vCenter.x,
		m_tOBB.vCenter.y,
		m_tOBB.vCenter.z);

	// -----------------------------
	// 4. 최종 월드 행렬
	// -----------------------------
	_matrix matWorld = matScale * matRot * matTrans;

	m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

	// -----------------------------
	// 5. 렌더 상태
	// -----------------------------
	m_pGraphicDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	D3DMATERIAL9 material;
	ZeroMemory(&material, sizeof(material));

	if (m_bColliding)
		material.Emissive = { 1.f, 0.f, 0.f, 1.f };
	else
		material.Emissive = { 0.f, 1.f, 0.f, 1.f };

	m_pGraphicDev->SetMaterial(&material);

	m_pDebugMesh->DrawSubset(0);

	// 복구
	m_pGraphicDev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
}

CCollider* CCollider::Create(LPDIRECT3DDEVICE9 pGraphicDev, 
	const _vec3& vScale, const _vec3& vOffset)
{
	CCollider* pCollider = new CCollider(pGraphicDev);

	if (FAILED(pCollider->Ready_Collider(vScale, vOffset)))
	{
		Safe_Release(pCollider);
		MSG_BOX("Collider Create Failed");
		return nullptr;
	}

	return pCollider;
}

CComponent* CCollider::Clone()
{
	return new CCollider(*this);
}

void CCollider::Free()
{
	CComponent::Free();
	Safe_Release(m_pDebugMesh);
}
