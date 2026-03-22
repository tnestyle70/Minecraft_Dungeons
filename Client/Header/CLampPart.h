#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

enum LAMP_PART
{
	LAMP_BODY,
	LAMP_HEAD,

	LAMP_END
};

class CLampPart : public CGameObject
{
private:
	explicit CLampPart(LPDIRECT3DDEVICE9 pGrpahicDev, LAMP_PART ePart);
	virtual ~CLampPart();

public:
	virtual	HRESULT Ready_GameObject();
	virtual	_int Update_GameObject(const _float& fTimeDelta);
	virtual	void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual	void Render_GameObject();

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
	HRESULT	Add_Component();

private:
	LAMP_PART m_ePart;

	CVIBuffer* m_pBufferCom;
	CTransform* m_pTransformCom;

	_vec3 m_vLocalOffset;

public:
	static CLampPart* Create(LPDIRECT3DDEVICE9 pGraphicDev, LAMP_PART ePart);

private:
	virtual void Free();
};

