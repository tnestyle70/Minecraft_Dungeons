#pragma once
#include "CScene.h"
#include "CProtoMgr.h"

class CObjectEditor : public CScene
{
protected:
	explicit CObjectEditor(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CObjectEditor();

public:
	virtual			HRESULT		Ready_Scene();
	virtual			_int		Update_Scene(const _float& fTimeDelta);
	virtual			void		LateUpdate_Scene(const _float& fTimeDelta);
	virtual			void		Render_Scene();

private:
	HRESULT			Ready_Environment_Layer(const _tchar* pLayerTag);
	HRESULT			Ready_Prototype();

private:
	void Editor_Input();
	_vec3 Get_MouseWorldPos();
	void Get_MouseRay(_vec3& vOrigin, _vec3& vDir);
	bool IntersectRayAABB(const _vec3& rayOrigin, const _vec3& rayDir, const _vec3& min, const _vec3& max, _float& t);

private:
	CGameObject* m_pSelectedObject = nullptr;
	map<wstring, CGameObject*> m_mapEditObject;

public:
	static CObjectEditor* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void	Free();
};