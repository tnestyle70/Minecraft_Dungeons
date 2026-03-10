#pragma once
#include "CBase.h"
#include "CScene.h"
#include "CProtoMgr.h"
#include "CBlockPlacer.h"

enum eStageType
{
	STAGE_SQUIDCOAST,
	STAGE_CAMP,
	STAGE_REDSTONE,
	STAGE_OBSIDIAN,
	STAGE_END
};

enum eEditMode
{
	MODE_BLOCK, 
	MODE_MONSTER,
	MODE_IRONBAR,
	MODE_TRIGGERBOX
};

enum eMonsterType
{
	MONSTER_ZOMBIE,
	MONSTER_SKELETON,
	MONSTER_CREEPER
};

struct MonsterData
{
	int x, y, z;
	int iMonsterType;
};

struct TriggerBoxData
{
	int x, y, z;
	int width, height, depth;
};

struct IronBarsData
{
	int x, y, z;
	int iTriggerID;
};

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
	
	void Render_BlockPalette();
	void Render_MonsterPalette();
	void Reneder_IronBarPalette();
	void Render_TriggerPalette();
private:
	void UpdateMonsterMode();
	void UpdateTriggerBoxMode();
private:
	HRESULT SaveStageData(const _tchar* szPath);
private:
	eStageType m_eCurrentStage = STAGE_SQUIDCOAST;
	//스테이지별 데이터
	vector<MonsterData> m_vecMonsterData;
	vector<IronBarsData> m_vecIronBarsData;
	vector<TriggerBoxData> m_vecTriggerBoxData;

	CBlockPlacer* m_pBlockPlacer = nullptr;
	bool m_bEditorMode;
	eEditMode m_eEditMode = MODE_BLOCK;
	eBlockType m_eSelectedBlock = BLOCK_GRASS;
	int m_iSelectedMonster = 0;
	//Trigger Box Settings
	int m_iTriggerWidth = 1;
	int m_iTriggerHeight = 1;
	int m_iTriggerDepth = 1;
	bool m_bLBtnPrev = false;
	bool m_bRBtnPrev = false;
public:
	static CEditor* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	virtual void Free();
};