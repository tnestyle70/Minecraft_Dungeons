#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

enum REDSTONEGOLEM_PART
{
	GOLEM_BODY,

	GOLEM_HEAD,
	GOLEM_CORE,
	GOLEM_LSHOULDER,
	GOLEM_RSHOULDER,
	GOLEM_HIP,

	GOLEM_LARM,
	GOLEM_RARM,
	GOLEM_LLEG,
	GOLEM_RLEG,

	GOLEM_END
};

class CRedStoneGolemPart : public CGameObject
{
private:
	explicit CRedStoneGolemPart(LPDIRECT3DDEVICE9 pGrpahicDev, REDSTONEGOLEM_PART ePart);
	explicit CRedStoneGolemPart(const CRedStoneGolemPart& rhs);
	virtual ~CRedStoneGolemPart();

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
	REDSTONEGOLEM_PART m_ePart;

	CVIBuffer* m_pBufferCom;
	CTransform* m_pTransformCom;

	_vec3 m_vLocalOffset;

public:
	static CRedStoneGolemPart* Create(LPDIRECT3DDEVICE9 pGraphicDev, REDSTONEGOLEM_PART ePart);
	
private:
	virtual void Free();
};