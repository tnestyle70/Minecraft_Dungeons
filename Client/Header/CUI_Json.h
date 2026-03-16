#pragma once

#include "CUI.h"

class CUI_Json : public CUI
{
protected:
	explicit CUI_Json(LPDIRECT3DDEVICE9 pGraphicDev);
	explicit CUI_Json(const CUI_Json& rhs);
	virtual ~CUI_Json();

public:
	virtual HRESULT Ready_GameObject(const _tchar* pJsonPath, const _tchar* pProtoTextureTag);
	virtual _int Update_GameObject(const _float& fTimeDelta) override;
	virtual void LateUpdate_GameObject(const _float& fTimeDelta) override;
	virtual void Render_GameObject() override;

private:
	HRESULT Load_Json(const _tchar* pJsonPath);
	HRESULT Add_Component(const _tchar* pProtoTextureTag);

private:
	_int m_iSizeX = 0;
	_int m_iSizeY = 0;

public:
	static CUI_Json* Create(LPDIRECT3DDEVICE9 pGraphicDev, const _tchar* pJsonPath, const _tchar* pProtoTextureTag);

private:
	virtual void Free() override;
};
