#pragma once
#include "CScene.h"
#include "CProtoMgr.h"

enum OBJECT_TYPE
{
	OBJECT_BOX,
	OBJECT_LAMP,

	OBJECT_END
};

struct OBJECT_DATA
{
	OBJECT_TYPE eType;
	_vec3 vPos;
	_vec3 vRot;
};

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
	virtual void Render_UI() override;

public:
	HRESULT SaveObjectData(const char* pFileName);
	HRESULT LoadObjectData(const char* pFileName);
	HRESULT LoadStageData(const char* pFileName);

private:
	HRESULT			Ready_Environment_Layer(const _tchar* pLayerTag);
	HRESULT			Ready_Prototype();

private:
	void Render_CreateUI();
	void Render_Inspector();
	void Render_SaveLoad();
	void Create_Object(const wstring& type);
	void Start_CreateMode(const wstring& type);

private:
	void Editor_Input();
	_vec3 Get_MouseWorldPos();
	void Get_MouseRay(_vec3& vOrigin, _vec3& vDir);
	bool IntersectRayAABB(const _vec3& rayOrigin, const _vec3& rayDir, const _vec3& min, const _vec3& max, _float& t);

private:
	CGameObject* m_pSelectedObject = nullptr;
	CGameObject* m_pPreviewObject = nullptr;
	map<wstring, CGameObject*> m_mapEditObject;

	_bool m_bLButtonPrev = false;
	_bool m_bRButtonPrev = false;

	wstring m_wstrCreateType;

public:
	static CObjectEditor* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
	virtual void	Free();
};