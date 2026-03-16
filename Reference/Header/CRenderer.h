#pragma once
#include "CBase.h"
#include "Engine_Define.h"
#include "CGameObject.h"
#include <functional>

BEGIN(Engine)

class ENGINE_DLL CRenderer :  public CBase
{
	DECLARE_SINGLETON(CRenderer)

private:
	explicit CRenderer();
	virtual ~CRenderer();

public:
	void		Add_RenderGroup(RENDERID eID, CGameObject* pGameObject);
	void		Render_GameObject(LPDIRECT3DDEVICE9& pGraphicDev);
	void		Clear_RenderGroup();

public:
	void		Render_Priority(LPDIRECT3DDEVICE9& pGraphicDev);
	void		Render_NonAlpha(LPDIRECT3DDEVICE9& pGraphicDev);
	void		Render_Alpha(LPDIRECT3DDEVICE9& pGraphicDev);
	void		Render_UI(LPDIRECT3DDEVICE9& pGraphicDev);
public:
	void Set_BlockRenderCallback(function<void()>callback)
	{
		m_BlockRenderCallback = callback;
	}
	void Set_ParticleRenderCallback(function<void()>callback)
	{
		m_ParticleRenderCallback = callback;
	}
private:
	function<void()> m_BlockRenderCallback = nullptr;
	function<void()> m_ParticleRenderCallback = nullptr;
private:
	list<CGameObject*>		m_RenderGroup[RENDER_END];

private:
	virtual void		Free();
};

END