#pragma once
#include "CBase.h"
#include "CScene.h"
#include "CProtoMgr.h"
#include "CBlockPlacer.h"
#include "CMonster.h"
#include "CTriggerBox.h"
#include "CIronBar.h"
#include "StageData.h"
#include "CParticleMgr.h"
#include "CDragon.h"

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
	MODE_TRIGGERBOX,
	MODE_PARTICLE
};

enum eMonsterType
{
	MONSTER_ZOMBIE,
	MONSTER_SKELETON,
	MONSTER_CREEPER,
	MONSTER_SPIDER,
	MONSTER_REDSTONE,
	MONSTER_GUARDIAN,
	MONSTER_DRAGON
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
	virtual void Render_UI() override;

private:
	HRESULT Ready_Environment_Layer(const _tchar* pLayerTag);
	HRESULT Ready_ProtoType();

public:
	bool IsEditorMode() { return m_bEditorMode; }
	void SetEditorMode(bool editorMode);
	//현재 선택된 스테이지의 .dat 경로 반환
	const _tchar* GetStagePath(eStageType eStage) const;
	//스테이지 전환 : 현재 스테이지 저장 -> 블럭 클리어 -> 새 스테이지 로드
	void SwitchStage(eStageType eNewStage);
private:
	void Render_MenuBar();
	void Render_Hierarchy();
	void Render_Inspector();
	void Render_Viewport();

	void Render_StageSelector();
	
	void Render_BlockPalette();
	void Render_MonsterPalette();
	void Render_IronBarPalette();
	void Render_TriggerPalette();
	//Particle
	void Render_ParticleEditor();
	void Respawn_PreviewParticle();
private:
	void UpdateMonsterMode();
	void UpdateIronBarMode();
	void UpdateTriggerBoxMode();
	void UpdateParticleMode();
private:
	void Load_ParticlePreset(eParticlePreset eType, ParticleDesc& outDesc);
public:
	HRESULT SaveStageData(const _tchar* szPath);
	HRESULT LoadStageData(const _tchar* szPath);
private:
	eStageType m_eCurrentStage = STAGE_SQUIDCOAST;

	//Dragon 
	CDragon* m_pDragon = nullptr;

	//Perset
	bool m_bPresetMode = false;
	int m_eSelectedBlockPreset = 0;

	//Particle
	ParticleDesc m_tEditDesc;
	LPDIRECT3DTEXTURE9 m_pPreviewTexture = nullptr;
	eParticlePreset m_eSelectedPreset = PARTICLE_FOOTSTEP;
	
	//Monster, IronBar ,TriggerBox Instance Data 
	map<MonsterData, CMonster*> m_mapMonsters;
	map<IronBarData, CIronBar*> m_mapIronBars;
	map<TriggerBoxData, CTriggerBox*> m_mapTriggerBoxes;

	CBlockPlacer* m_pBlockPlacer = nullptr;
	bool m_bEditorMode = true;
	eEditMode m_eEditMode = MODE_BLOCK;
	eBlockType m_eSelectedBlock = BLOCK_GRASS;
	int m_iSelectedMonster = 0;

	//Monster TriggerID 
	int m_iMonsterTriggerID = 0;

	//Trigger Box Settings
	int m_iTriggerWidth = 1;
	int m_iTriggerHeight = 1;
	int m_iTriggerDepth = 1;
	int m_iCurTriggerID = 0;
	int m_iTriggerType = TRIGGER_IRONBAR;
	bool m_bLBtnPrev = false;
	bool m_bRBtnPrev = false;
public:
	static CEditor* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
	virtual void Free();
};