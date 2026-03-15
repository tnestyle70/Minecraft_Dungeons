#pragma once

#include "CGameObject.h"

BEGIN(Engine)
class CRcTex;
class CTexture;
class CTransform;
END

class CUI : public Engine::CGameObject
{
protected:
	explicit CUI(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CUI(const CUI& rhs);
	virtual ~CUI();

public:
	void Set_Pos(_float fX, _float fY) { m_fX = fX; m_fY = fY; }
	void Set_Scale(_float fScale) { m_fUIScale = fScale; }
	void Set_Visible(_bool bVisible) { m_bVisible = bVisible; }
	_float Get_SizeX() { return m_fSizeX * m_fUIScale; }
	_float Get_SizeY() { return m_fSizeY * m_fUIScale; }
	_bool Get_Visible() { return m_bVisible; }

	void Add_Child(CUI* pChild);
	void Set_Parent(CUI* pParent) { m_pParent = pParent; }

public:
	virtual HRESULT Ready_GameObject() override;
	virtual _int Update_GameObject(const _float& fTimeDelta) override;
	virtual void LateUpdate_GameObject(const _float& fTimeDelta) override;
	virtual void Render_GameObject() override;

protected:
	HRESULT Add_Component();

protected:
	Engine::CRcTex*		m_pBufferCom = nullptr;
	Engine::CTexture*	m_pTextureCom = nullptr;
	Engine::CTransform* m_pTransformCom = nullptr;

	_float m_fX, m_fY;
	_float m_fSizeX, m_fSizeY;
	_float m_fUIScale = 1.f;
	_bool m_bVisible = true;

	CUI* m_pParent = nullptr;
	vector<CUI*> m_vecChildren;

	_vec2 m_vUVScale = { 1.f, 1.f };
	_vec2 m_vUVOffset = { 0.f, 0.f };

	_matrix m_matProj;

protected:
	virtual void Free() override;
};
