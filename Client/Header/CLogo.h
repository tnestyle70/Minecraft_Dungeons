#pragma once
#include "CScene.h"
#include "CLoading.h"

class CEditor;
class CObjectEditor;

class CLogo : public CScene
{
protected:
	explicit CLogo(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CLogo();

public:
	virtual			HRESULT		Ready_Scene();
	virtual			_int		Update_Scene(const _float& fTimeDelta);
	virtual			void		LateUpdate_Scene(const _float& fTimeDelta);
	virtual			void		Render_Scene();
	virtual void Render_UI() override;

private:
	HRESULT			Ready_Environment_Layer(const _tchar* pLayerTag);
	HRESULT			Ready_Prototype();

private:
	CEditor* m_pEditor;
	CObjectEditor* m_pObjectEditor;
	bool m_bF1Toggle = false;
	bool m_bF2Toggle = false;
public:
	static CLogo* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void	Free();

};

