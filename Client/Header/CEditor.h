#pragma once
#include "CBase.h"
#include "CScene.h"
#include "CProtoMgr.h"
#include "CBlockPlacer.h"
#include "CMonster.h"
#include "CTriggerBox.h"
#include "CIronBar.h"
#include "CJumpingTrap.h"
#include "StageData.h"
#include "CParticleMgr.h"
#include "CDragon.h"
#include <deque> 
#include "CBlockMgr.h"
#include "CPlayer.h"


enum eStageType
{
    STAGE_SQUIDCOAST,
    STAGE_CAMP,
    STAGE_REDSTONE,
    STAGE_OBSIDIAN,
    STAGE_JS,
    STAGE_TG,
    STAGE_CY,
    STAGE_GB,
    STAGE_END
};

enum eEditMode
{
    MODE_BLOCK,
    MODE_MONSTER,
    MODE_IRONBAR,
    MODE_TRIGGERBOX,
    MODE_JUMPINGTRAP,
    MODE_PARTICLE
};

enum eMonsterType
{
    MONSTER_ZOMBIE,
    MONSTER_SKELETON,
    MONSTER_CREEPER,
    MONSTER_SPIDER
};

// 추가 - 복사/붙여넣기용 상대좌표 블록
struct ClipboardBlock
{
    int rx, ry, rz;     // 기준점으로부터 상대 좌표
    eBlockType eType;   // 블록 타입
};

// 추가 - Undo/Redo용 작업 단위
struct EditorAction
{
    vector<BlockData> vecAdded;   // 추가된 블록들 (Undo 시 제거)
    vector<BlockData> vecRemoved; // 제거된 블록들 (Undo 시 복원)
};

class CEditor : public CScene
{
protected:
    explicit CEditor(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CEditor();
public:
    virtual HRESULT Ready_Scene();
    virtual _int    Update_Scene(const _float& fTimeDelta);
    virtual void    LateUpdate_Scene(const _float& fTimeDelta);
    virtual void    Render_Scene();
    virtual void    Render_UI() override;
private:
    HRESULT Ready_Environment_Layer(const _tchar* pLayerTag);
    HRESULT Ready_ProtoType();
public:
    bool IsEditorMode() { return m_bEditorMode; }
    void SetEditorMode(bool editorMode);
    const _tchar* GetStagePath(eStageType eStage) const;
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
    void Render_JumpingTrapPalette();
    void Render_ParticleEditor();
    void Respawn_PreviewParticle();
    void Render_SelectionBox(); // 추가 - 선택 영역 와이어프레임
private:
    void UpdateBlockMode(const _float& fTimeDelta); // 추가 - 블록 모드 통합 업데이트
    void UpdateSelectionMode();                      // 추가 - Shift 드래그 선택
    void UpdateMonsterMode();
    void UpdateIronBarMode();
    void UpdateTriggerBoxMode();
    void UpdateJumpingTrapMode();
    void UpdateParticleMode();
private:
    // 추가 - 선택 영역 조작
    void CopySelection();   // Ctrl+C
    void PasteClipboard();  // Ctrl+V
    void EraseSelection();  // Delete
    void FillSelection();   // Inspector 버튼
    // 추가 - Undo/Redo
    void Push_UndoAction(const EditorAction& action);
    void Undo(); // Ctrl+Z
    void Redo(); // Ctrl+Y
private:
    void Load_ParticlePreset(eParticlePreset eType, ParticleDesc& outDesc);
public:
    HRESULT SaveStageData(const _tchar* szPath);
    HRESULT LoadStageData(const _tchar* szPath);
private:
    eStageType m_eCurrentStage = STAGE_SQUIDCOAST;
    CDragon* m_pDragon = nullptr;
    CPlayer* m_pPlayer = nullptr;
    bool m_bPresetMode = false;
    int  m_eSelectedBlockPreset = 0;
    ParticleDesc m_tEditDesc;
    LPDIRECT3DTEXTURE9 m_pPreviewTexture = nullptr;
    eParticlePreset m_eSelectedPreset = PARTICLE_FOOTSTEP;
    map<MonsterData, CMonster*>     m_mapMonsters;
    map<IronBarData, CIronBar*>     m_mapIronBars;
    map<TriggerBoxData, CTriggerBox*> m_mapTriggerBoxes;
    map<JumpingTrapData, CJumpingTrap*> m_mapJumpingTraps;
    CBlockPlacer* m_pBlockPlacer = nullptr;
    bool m_bEditorMode = true;
    eEditMode  m_eEditMode = MODE_BLOCK;
    eBlockType m_eSelectedBlock = BLOCK_GRASS;
    int  m_iSelectedMonster = 0;
    int  m_iMonsterTriggerID = 0;
    int  m_iTriggerWidth = 1;
    int  m_iTriggerHeight = 1;
    int  m_iTriggerDepth = 1;
    int  m_iCurTriggerID = 0;
    int  m_iTriggerType = TRIGGER_IRONBAR;
    bool m_bLBtnPrev = false;
    bool m_bRBtnPrev = false;

    int m_iRangeMinX = 0, m_iRangeMaxX = 0;
    int m_iRangeMinY = 0, m_iRangeMaxY = 0;
    int m_iRangeMinZ = 0, m_iRangeMaxZ = 0;

    // 추가 - 영역 선택
    BlockPos m_tSelPos1 = {};
    BlockPos m_tSelPos2 = {};
    bool     m_bHasSel = false; // 선택 완료 여부
    bool     m_bSelecting = false; // 드래그 중 여부

    // 추가 - 클립보드
    vector<ClipboardBlock> m_vecClipboard;

    // 추가 - Undo/Redo
    deque<EditorAction> m_undoStack;
    deque<EditorAction> m_redoStack;
    static const int MAX_UNDO = 20;

    // 추가 - 키 상태
    bool m_bZKeyPrev = false;
    bool m_bYKeyPrev = false;
    bool m_bCKeyPrev = false;
    bool m_bVKeyPrev = false;
    bool m_bDelKeyPrev = false;

public:
    static CEditor* Create(LPDIRECT3DDEVICE9 pGraphicDev);
private:
    virtual void Free();
};