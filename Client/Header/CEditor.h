#pragma once
#include "CBase.h"
#include "CScene.h"
#include "CProtoMgr.h"
#include "CBlockPlacer.h"

class CEditor : public CScene
{
protected:
	explicit CEditor(LPDIRECT3DDEVICE9 pGraphicDev);
	virtual ~CEditor();
public:
	virtual			HRESULT		Ready_Scene();
	virtual			_int		Update_Scene(const _float& fTimeDelta);
	virtual			void		LateUpdate_Scene(const _float& fTimeDelta);
	virtual			void		Render_Scene();
private:
	HRESULT Ready_Environment_Layer(const _tchar* pLayerTag);
public:
	bool IsEditorMode() { return m_bEditorMode; }
	void SetEditorMode(bool editorMode);
private:
	void Render_MenuBar();
	void Render_Hierarchy();
	void Render_Inspector();
	void Render_Viewport();
private:
	CBlockPlacer* m_pBlockPlacer = nullptr;
	bool m_bEditorMode;
	//Selected Block
	eBlockType m_eSelectedBlock = BLOCK_GRASS;
public:
	static CEditor* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	virtual void Free();
};