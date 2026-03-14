#pragma once
#include "CScene.h"

class CSquidCoast : public CScene
{
protected:
	explicit CSquidCoast(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CSquidCoast();
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
	HRESULT Ready_StageData(const _tchar* szPath);
private:
	bool m_bActionCamFinished = false;
public:
	static CSquidCoast* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	virtual void Free();
};