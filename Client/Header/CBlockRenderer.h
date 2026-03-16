#pragma once

#include "CGameObject.h"

class CBlockRenderer : public Engine::CGameObject
{
private:
	explicit CBlockRenderer(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CBlockRenderer();

public:
	virtual HRESULT Ready_GameObject() override;
	virtual _int Update_GameObject(const _float& fTimeDelta) override;
	virtual void LateUpdate_GameObject(const _float& fTimeDelta) override;
	virtual void Render_GameObject() override;

public:
	static CBlockRenderer* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void Free() override;
};
