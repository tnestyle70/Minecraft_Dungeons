#pragma once

#include "CUI.h"

class CSampleUI : public CUI
{
private:
	explicit CSampleUI(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CSampleUI(const CSampleUI& rhs);
	virtual ~CSampleUI();

public:
	virtual HRESULT Ready_GameObject() override;
	virtual _int Update_GameObject(const _float& fTimeDelta) override;
	virtual void LateUpdate_GameObject(const _float& fTimeDelta) override;
	virtual void Render_GameObject() override;

private:
	HRESULT Add_Component();

public:
	static CSampleUI* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free() override;
};
