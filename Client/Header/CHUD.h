#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

//HUD 플레이어의 생성과 동기화를 시켜서 체력 정보를 업데이트하는 것이 핵심

class CHUD : public CGameObject
{
private:
	explicit CHUD(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CHUD(const CGameObject& rhs);
	virtual ~CHUD();

public:
	virtual			HRESULT		Ready_GameObject();
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT			Add_Component();

private:
	Engine::CRcTex* m_pBufferCom;
	Engine::CTexture* m_pTextureCom;

public:
	static CHUD* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free();

};

