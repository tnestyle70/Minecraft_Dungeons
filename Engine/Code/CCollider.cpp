#include "CCollider.h"

CCollider::CCollider(LPDIRECT3DDEVICE9 pGraphicDev)
	: CComponent(pGraphicDev), 
	m_vScale(1.f, 1.f, 1.f),
	m_vOffset(0.f, 0.f, 0.f),
	m_pDebugMesh(nullptr),
	m_bColliding(false)
{
	ZeroMemory(&m_tAABB, sizeof(m_tAABB));
}

CCollider::CCollider(const CCollider& rhs)
	: CComponent(rhs),
	m_vScale(rhs.m_vScale),
	m_vOffset(rhs.m_vOffset),
	m_tAABB(rhs.m_tAABB),
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
	//m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, TRUE);
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
