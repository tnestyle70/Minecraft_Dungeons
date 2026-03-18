#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

enum BOX_PART
{
	BOX_BOTTOM,
	BOX_TOP,

	BOX_END
};

class CBoxPart : public CGameObject
{
private:
	explicit CBoxPart(LPDIRECT3DDEVICE9 pGrpahicDev, BOX_PART ePart);
	explicit CBoxPart(const CBoxPart& rhs);
	virtual ~CBoxPart();

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
	BOX_PART m_ePart;

	CVIBuffer* m_pBufferCom;
	CTransform* m_pTransformCom;

	_vec3 m_vLocalOffset;

public:
	static CBoxPart* Create(LPDIRECT3DDEVICE9 pGraphicDev, BOX_PART ePart);

private:
	virtual void Free();
};

