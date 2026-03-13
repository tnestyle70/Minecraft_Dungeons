#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CRedStoneGolem : public CGameObject
{
private:
	explicit CRedStoneGolem(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CRedStoneGolem(const CRedStoneGolem& rhs);
	virtual ~CRedStoneGolem();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT			Add_Component();
	void Set_PartsPos();
	void Set_DefaultScale();
	
private:
	// Buffer
	Engine::CRedStoneGolemHeadTex* m_pHeadBufferCom;
	Engine::CRedStoneGolemBodyTex* m_pBodyBufferCom;
	Engine::CRedStoneGolemShoulderTex* m_pLeftShoulderBufferCom;
	Engine::CRedStoneGolemShoulderTex* m_pRightShoulderBufferCom;
	Engine::CRedStoneGolemHipTex* m_pHipBufferCom;
	Engine::CRedStoneGolemCoreTex* m_pCoreBufferCom;
	Engine::CRedStoneGolemArmTex* m_pLeftArmBufferCom;
	Engine::CRedStoneGolemArmTex* m_pRightArmBufferCom;
	Engine::CRedStoneGolemLegTex* m_pLeftLegBufferCom;
	Engine::CRedStoneGolemLegTex* m_pRightLegBufferCom;

	// Transform
	Engine::CTransform* m_pHeadTransformCom;
	Engine::CTransform* m_pBodyTransformCom;
	Engine::CTransform* m_pLeftShoulderTransformCom;
	Engine::CTransform* m_pRightShoulderTransformCom;
	Engine::CTransform* m_pHipTransformCom;
	Engine::CTransform* m_pCoreTransformCom;
	Engine::CTransform* m_pLeftArmTransformCom;
	Engine::CTransform* m_pRightArmTransformCom;
	Engine::CTransform* m_pLeftLegTransformCom;
	Engine::CTransform* m_pRightLegTransformCom;

	// Texture
	Engine::CTexture* m_pTextureCom;

public:
	static CRedStoneGolem* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();
};

