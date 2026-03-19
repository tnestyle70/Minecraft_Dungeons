#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"

class CBackGround : public CGameObject
{
private:
	explicit CBackGround(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CBackGround(const CGameObject& rhs);
	virtual ~CBackGround();

public:
	virtual			HRESULT		Ready_GameObject(const _tchar* pPath);
	virtual			_int		Update_GameObject(const _float& fTimeDelta);
	virtual			void		LateUpdate_GameObject(const _float& fTimeDelta);
	virtual			void		Render_GameObject();

private:
	HRESULT			Add_Component(const _tchar* pProtoPath);

private:
	Engine::CRcTex*		m_pBufferCom;
	Engine::CTexture*	m_pTextureCom;
	
public:
	static CBackGround* Create(LPDIRECT3DDEVICE9 pGraphicDev,
		const _tchar* pProtoPath);

private:
	virtual void Free();

};

