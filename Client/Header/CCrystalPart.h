#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

enum CRYSTAL_PART
{
	CRYSTAL_8x20,
	CRYSTAL_5x14,
	CRYSTAL_4x11,
	CRYSTAL_3x15,
	CRYSTAL_2x13,
	CRYSTAL_3x5,

	CRYSTAL_END
};

class CCrystalPart : public CGameObject
{
private:
	explicit CCrystalPart(LPDIRECT3DDEVICE9 pGrpahicDev, CRYSTAL_PART ePart);
	virtual ~CCrystalPart();

public:
	virtual HRESULT Ready_GameObject();
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual	void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();

public:
	void Set_LocalOffset(_vec3 vOffset)
	{
		m_vLocalOffset = vOffset;

		m_pTransformCom->Set_Pos(
			vOffset.x,
			vOffset.y,
			vOffset.z
		);
	}

	_vec3 Get_LocalOffset() { return m_vLocalOffset; }
	CTransform* Get_Transform() { return m_pTransformCom; }

private:
	HRESULT Add_Component();

private:
	CRYSTAL_PART m_ePart;

	CVIBuffer* m_pBufferCom;
	CTransform* m_pTransformCom;

	_vec3 m_vLocalOffset;

public:
	static CCrystalPart* Create(LPDIRECT3DDEVICE9 pGraphicDev, CRYSTAL_PART ePart);

private:
	virtual void Free();
};

