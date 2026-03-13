#pragma once
#include "CComponent.h"

BEGIN(Engine)

class ENGINE_DLL CTransform :  public CComponent
{
private :
	explicit CTransform();
	explicit CTransform(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CTransform(const CTransform& rhs);
	virtual ~CTransform();

public:
	void	Move_Pos(const _vec3* pDir, const _float& fSpeed, const _float& fTimeDelta)
	{
		m_vInfo[INFO_POS] += *pDir * fSpeed * fTimeDelta;
	}

	void	Rotation(ROTATION eType, const _float& fAngle)
	{
		*(((_float*)&m_vAngle) + eType) += fAngle;
	}

	_matrix* Get_World() { return &m_matWorld; }

	void Set_World(_matrix* matWorld) { m_matWorld = *matWorld; }

	void	Get_Info(INFO eType, _vec3* pInfo)
	{
		memcpy(pInfo, &m_matWorld.m[eType][0], sizeof(_vec3));
	}
	void		Set_Pos(_float fX, _float fY, _float fZ)
	{
		m_vInfo[INFO_POS] = { fX, fY, fZ };
	}

	void Set_Scale(_float fScale)
	{
		m_vScale *= fScale;
	}

public:
	HRESULT		 Ready_Transform();
	virtual _int Update_Component(const _float& fTimeDelta);
	virtual void LateUpdate_Component();

	void	   	 Chase_Target(const _vec3* pPos, const _float& fSpeed, const _float& fTimeDelta);
	_matrix*	 Compute_Lookattarget(const _vec3* pPos);
public:
	_vec3		m_vInfo[INFO_END];
	_vec3		m_vScale;
	_vec3		m_vAngle;

	_matrix		m_matWorld;

public:
	static CTransform* Create(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual CComponent* Clone();

private:
	virtual void	Free();
};

END