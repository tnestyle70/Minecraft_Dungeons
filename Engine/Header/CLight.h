#pragma once
#include "CBase.h"
#include "Engine_Define.h"

BEGIN(Engine)

class ENGINE_DLL CLight :  public CBase
{
private:
	explicit CLight(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CLight();

public:
	HRESULT		Ready_Light(const D3DLIGHT9* pLightInfo, const _uint& iIndex);

private:
	LPDIRECT3DDEVICE9		m_pGraphicDev;
	_uint					m_iIndex;
	D3DLIGHT9				m_tLight;

public:
	static CLight* Create(LPDIRECT3DDEVICE9 pGraphicDev,
							const D3DLIGHT9* pLightInfo,
							const _uint& iIndex);
private:
	virtual void	Free();
};

END