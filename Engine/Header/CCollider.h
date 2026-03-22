#pragma once
#include "CComponent.h"

//AABB 구조체 - min x, y, z와 max x, y, z 사이에 있는 값 비교
struct AABB
{
	_vec3 vMin;
	_vec3 vMax;
};

struct OBB
{
	_vec3 vCenter;
	_vec3 vAxis[3];
	_vec3 vHalfSize;
};

BEGIN(Engine)

class ENGINE_DLL CCollider : public CComponent
{
public:
	explicit CCollider(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CCollider(const CCollider& rhs);
	virtual ~CCollider();
public:
	HRESULT Ready_Collider(const _vec3& vScale, const _vec3&
	vOffset = {0.f, 0.f, 0.f});
	virtual _int Update_Component(const _float& fTimeDelta) override;
public:
	AABB Get_AABB() const { return m_tAABB; }
	OBB Get_OBB() const { return m_tOBB; }
	//Transform 위치를 기준으로 AABB 업데이트
	void Update_AABB(const _vec3& vWorldPos);
	void Update_OBB(const _matrix& matWorld);
	bool IsColliding(const AABB& other) const;
	bool IntersectRay(const _vec3& vRayOrigin, const _vec3& vRayDir);
	_vec3 Resolve(const AABB& other) const;
	//디버그용 렌더링
	void Render_Collider();
	void Render_OBB();
private:
	_vec3 m_vScale;
	_vec3 m_vOffset;
	//AABB 구조체
	AABB m_tAABB;
	//OBB 구조체
	OBB m_tOBB;
	_vec3 m_vOriginalHalfSize;
	//디버그용
	LPD3DXMESH m_pDebugMesh;
	bool m_bColliding;
public:
	static CCollider* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _vec3& vScale, const _vec3& vOffset = { 0.f, 0.f, 0.f });
	virtual CComponent* Clone() override;
private:
	virtual void Free() override;
};

END