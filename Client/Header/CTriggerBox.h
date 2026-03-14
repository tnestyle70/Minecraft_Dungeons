#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

//Trigger Box 종류
enum eTriggerBoxType
{
	TRIGGER_IRONBAR = 0,
	TRIGGER_MONSTER,
	TRIGGER_SCENECHANGE,
	TRIGGER_END
};

class CTriggerBox : public CGameObject
{
private:
	explicit CTriggerBox(LPDIRECT3DDEVICE9 pGarphicDev);
	explicit CTriggerBox(const CGameObject& rhs);
	virtual ~CTriggerBox();
public: //Initialize pos
	virtual HRESULT Ready_GameObject(const _vec3& vPos, _int iTriggerID,
		eTriggerBoxType& triggerType);
	virtual _int Update_GameObject(const _float& fTimeDelta);
	virtual void LateUpdate_GameObject(const _float& fTimeDelta);
	virtual void Render_GameObject();
public:
	bool IsTriggered() { return m_bTriggered; }
	void SetTrigger(bool bTriggered) { m_bTriggered = bTriggered; }
	void CheckCollide(CCollider* pCollider);
private:
	HRESULT Add_Component();
private:
	void Trigger_Ironbar();
	void Trigger_Monster();
	void Trigger_SceneChange();
private:
	CTransform* m_pTransformCom = nullptr;
	CCollider* m_pColliderCom = nullptr;

	_int m_iTriggerID = 0;

	eTriggerBoxType m_eTrigger = TRIGGER_END;

	bool m_bTriggered = false;

	_vec3 m_vPos = { 0.f, 0.f, 0.f };
public://Decide box type, when it created, so that TriggerBoxMgr only 
	//turn on trigger
	static CTriggerBox* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _vec3& vPos, _int iTriggerID = 0 ,eTriggerBoxType triggerType = TRIGGER_END);
private:
	virtual void Free();
};