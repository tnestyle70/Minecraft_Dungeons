#pragma once
#include "CScene.h"

class CCamp : public CScene
{
protected:
	explicit CCamp(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CCamp();
public:
	virtual HRESULT Ready_Scene();
	virtual _int Update_Scene(const _float& fTimeDelta);
	virtual void LateUpdate_Scene(const _float& fTimeDelta);
	virtual void Render_Scene();
private:
	HRESULT Ready_Environment_Layer(const _tchar* pLayerTag);
	HRESULT Ready_GameLogic_Layer(const _tchar* pLayerTag);
	HRESULT Ready_UI_Layer(const _tchar* pLayerTag);
	HRESULT Ready_Light();
public:
	static CCamp* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	virtual void Free();
};