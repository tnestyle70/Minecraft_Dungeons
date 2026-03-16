#pragma once

#include "CUI.h"

class CHotbar : public CUI
{
private:
	explicit CHotbar(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CHotbar(const CHotbar& rhs);
	virtual ~CHotbar();

public:
	virtual HRESULT Ready_GameObject() override;
	virtual _int Update_GameObject(const _float& fTimeDelta) override;
	virtual void LateUpdate_GameObject(const _float& fTimeDelta) override;
	virtual void Render_GameObject() override;

private:
	HRESULT Add_Children();

public:
	static CHotbar* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free() override;
};
