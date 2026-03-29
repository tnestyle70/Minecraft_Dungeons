#include "pch.h"
#include "CLoading.h"
#include "CProtoMgr.h"
#include "CMonsterUV.h"
#include "CAncientGuardianUV.h"

CLoading::CLoading(LPDIRECT3DDEVICE9 pGraphicDev)
    : m_pGraphicDev(pGraphicDev), m_bFinish(false), m_eLoadingID(LOADING_END)
{
    ZeroMemory(m_szLoading, sizeof(m_szLoading));
    m_pGraphicDev->AddRef();
}

CLoading::~CLoading()
{
}

HRESULT CLoading::Ready_Loading(LOADINGID eID)
{
    InitializeCriticalSection(&m_Crt);

    m_eLoadingID = eID;

    m_hThread = (HANDLE)_beginthreadex(NULL, // 보안 속성(핸들의 상속 여부, NULL인 경우 상속에서 제외)
        0,  // 디폴트 스택 사이즈(1 바이트)
        Thread_Main, // 구동할 쓰레드 함수
        this,          // 3번 매개 변수 함수를 통해 가공할 데이터 주소
        0,             // 쓰레드 생성 및 실행을 조정하기 위한 옵션
        NULL);         // 쓰레드 ID
    return S_OK;
}

_uint CLoading::Loading_SquidCoast()
{
    lstrcpy(m_szLoading, L"버퍼 로딩중");

    if (nullptr == CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_TriCol"))
    {
        if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TriCol", Engine::CTriCol::Create(m_pGraphicDev))))
            return E_FAIL;
    }

    if (nullptr == CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcCol"))
    {
        if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RcCol", Engine::CRcCol::Create(m_pGraphicDev))))
            return E_FAIL;
    }

    if (nullptr == CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_TerrainTex"))
    {
        if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TerrainTex", Engine::CTerrainTex::Create(m_pGraphicDev))))
            return E_FAIL;
    }

    if (nullptr == CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CubeTex"))
    {
        if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CubeTex", Engine::CCubeTex::Create(m_pGraphicDev))))
            return E_FAIL;
    }

    if (nullptr == CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"))
    {
        if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RcTex", Engine::CRcTex::Create(m_pGraphicDev))))
            return E_FAIL;
    }

    //UI - Inventory
    //Slot Frame
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_FrameTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/gear_main_slot.png"))))
        return E_FAIL;
    //Slot Hover Frame
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_HoverFrameTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/hover_frame.png"))))
        return E_FAIL;
    //Slot Click slot
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ClickedFrameTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/click_frame.png"))))
        return E_FAIL;
    //small unique slot
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SmallUniqueSlotHoverTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/small_unique_slot.png"))))
        return E_FAIL;
    //Inventory Background
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_InventoryBackgroundTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/Inventory_Background.png"))))
        return E_FAIL;
    //Bow
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ArrowLoadedBowTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/Arrow_Loaded_Crossbow.png"))))
        return E_FAIL;
    //Sword
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_IronSwordTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/Iron_Sword.png"))))
        return E_FAIL;
    //Player Rob
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RobeTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/Robe.png"))))
        return E_FAIL;
    //Upgrade Lock
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_UpgradeTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/asset_mapnode_locked_top_hover.png"))))
        return E_FAIL;
    //Ocean Diffuse
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_OceanTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/ocean_diffuse.png"))))
        return E_FAIL;
    //Ocean Normal
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_OceanNormalTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/ocean_normal.png"))))
        return E_FAIL;

    // RedStoneGolem
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemBodyTex", Engine::CRedStoneGolemBodyTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemHeadTex", Engine::CRedStoneGolemHeadTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemShoulderTex", Engine::CRedStoneGolemShoulderTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemHipTex", Engine::CRedStoneGolemHipTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemCoreTex", Engine::CRedStoneGolemCoreTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemArmTex", Engine::CRedStoneGolemArmTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemLegTex", Engine::CRedStoneGolemLegTex::Create(m_pGraphicDev))))
        return E_FAIL;

    // Box
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BoxBottomTex", Engine::CBoxBottomTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BoxTopTex", Engine::CBoxTopTex::Create(m_pGraphicDev))))
        return E_FAIL;

    // Lamp
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LampBodyTex", Engine::CLampBodyTex::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LampHeadTex", Engine::CLampHeadTex::Create(m_pGraphicDev))))
        return E_FAIL;

    lstrcpy(m_szLoading, L"텍스쳐 로딩중");

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TerrainTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Terrain/Grass_%d.tga", 2))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SkyBoxTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/SkyBox/sky.dds"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_EffectTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Explosion/Explosion%d.png", 90))))
        return E_FAIL;

    // BOSS
    // RedStoneGolem
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Boss/T_RedStone_Golem.png"))))
        return E_FAIL;

    // Object
    // Box
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BoxTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Object/T_LargeBoxChest.png"))))
        return E_FAIL;

    // Lamp
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LampTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Object/T_Lamp.png"))))
        return E_FAIL;

    // Emerald
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_EmeraldTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Object/chest_emerald_pop.png"))))
        return E_FAIL;

    //오징어 해안 로딩 텍스쳐`
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SquidCoastLoadingTexture",
    //    Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Loading_Screen_Squid_Coast.png"))))
    //    return E_FAIL;

    ////캠프 로딩 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CampLoadingTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Loading_Screen_Lobby.png"))))
        return E_FAIL;

    ////레드 스톤 로딩 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneLoadingTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Fiery_Forge.png"))))
        return E_FAIL;

    ////옵시디언 로딩 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianLoadingTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Obsidian_Pinnacle.png"))))
        return E_FAIL;

    // 플레이어 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_PlayerTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/steve_real.png"))))
        return E_FAIL;

    //칼 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SwordTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Player/iron_sword.png"))))
        return E_FAIL;

    //활 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowStandby",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Player/bow_standby.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowPulling0",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Player/bow_pulling_0.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowPulling1",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Player/bow_pulling_1.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowPulling2",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Player/bow_pulling_2.png"))))
        return E_FAIL;
    //NPC
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_NPCTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/NPC/T_Knight_Skin.png"))))
        return E_FAIL;
    //대화창
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_DialogueBoxTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/NPC/NPCDialogue.png"))))
        return E_FAIL;

    //=======UI=========//
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_HUDTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/hotbar.png"))))
        return E_FAIL;
    //Boss Health Bar
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BossHealthBarTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/Boss_HealthBar.png"))))
        return E_FAIL;
    //Boss Health Bar Empty
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_EmptyBossHealthBarTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/Boss_HealthBar_Empty.png"))))
        return E_FAIL;

    //UI - 하트 
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_FilledHeart",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/filled_heart.png"))))
        return E_FAIL;
    //Gray Heart
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_EmptyHeart",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/gray_heart.png"))))
        return E_FAIL;
    //Inventory sword tab on
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SwordTabOff",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/gear_icon.png"))))
        return E_FAIL;
    //Inventory sword tab off
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SwordTabOn",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/gear_icon_bright.png"))))
        return E_FAIL;
    //Inventory armor tab on
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ArmorTabOn",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/armour_select.png"))))
        return E_FAIL;
    //Inventory armor tab off
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ArmorTabOff",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/armour_default.png"))))
        return E_FAIL;
    //Inventory bow tab on
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowTabOn",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/bow_icon_bright.png"))))
        return E_FAIL;
    //Inventory bow tab off
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowTabOff",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI_0/bow_icon.png"))))
        return E_FAIL;

    //플레이어 아머 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ArmorTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Player/T_EndRobes.png"))))
        return E_FAIL;

    //TNT
     if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TNT_Top",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/blocks/T_TNTCart.png"))))
        return E_FAIL;

    // 닭 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ChickenTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/chicken.png"))))
        return E_FAIL;

    //블럭 텍스쳐
    //잔디 
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_GrassTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/GrassSideTexture.dds"))))
        return E_FAIL;
    //흙
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_DirtTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/DirtTexture.dds"))))
        return E_FAIL;
    //모래
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SandTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/SandTexture.dds"))))
        return E_FAIL;
    //돌
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RockTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/RockTexture.dds"))))
        return E_FAIL;
    //bedrock
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BedrockTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/BedrockTexture.dds"))))
        return E_FAIL;
    //obsidian
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/ObsidianTexture.dds"))))
        return E_FAIL;
    //obsidian png
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianPngTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/blocks/obsidian.png"))))
        return E_FAIL;
    //stonebrick
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_StoneBrickTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/StoneBrickTexture.dds"))))
        return E_FAIL;
    //oak
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_OakTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/OakTexture.dds"))))
        return E_FAIL;
    //oak leaves
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_OakLeavesTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/OakLeaves.dds"))))
        return E_FAIL;
    //cherry leaves
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CherryLeavesTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/CherryLeaves.dds"))))
        return E_FAIL;
    //lava
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_LavaTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/lava.dds"))))
        return E_FAIL;
    //planks acacia
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_PlankAcaciaTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/planks_acacia.dds"))))
        return E_FAIL;
    //planks spruce
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_PlankSpruceTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/planks_spruce.dds"))))
        return E_FAIL;
    //RedStone
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/RedStone_Block.dds"))))
        return E_FAIL;
    //OakWood
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_OakWoodTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/Oak_Wood.dds"))))
        return E_FAIL;
    //블럭 텍스쳐 아틀라스
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BlockAtlasTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/blocks/minecraft_block_atlas_4x4.png"))))
        return E_FAIL;

    //=========Effect=========//
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_FootPrintTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Effect/FootPrint.png"))))
        return E_FAIL;

    // Zobie
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ZombieTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/zombie.png"))))
        return E_FAIL;

    // 좀비 파츠 버퍼
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_Head",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::HEAD))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_Body",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::BODY))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_RArm",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::R_ARM))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_LArm",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::L_ARM))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_RLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::R_LEG))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Zombie_LLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, ZombieUV::L_LEG))))
        return E_FAIL;
    //Skeleton
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SkeletonTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/skeleton.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_Head",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::HEAD))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_Body",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::BODY))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_RArm",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::R_ARM))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_LArm",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::L_ARM))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_RLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::R_LEG))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Skeleton_LLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SkeletonUV::L_LEG))))
        return E_FAIL;
    //arrow
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowStandbyTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/bow_standby.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_BowPullingTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/bow_pulling_0.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ArrowTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/arrow.png"))))
        return E_FAIL;

    // Creeper
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeperTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/creeper.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_Head",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::HEAD))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_Body",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::BODY))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_RFLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::LEG))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_LFLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::LEG))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_RBLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::LEG))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_creeper_LBLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, CreeperUV::LEG))))
        return E_FAIL; 

    // Spider
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SpiderTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/mob/T_Spider_Skin.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_Head",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::HEAD))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_Body",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::BODY))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_RFLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::LEG))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_LFLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::LEG))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_RBLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::LEG))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Spider_LBLeg",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, SpiderUV::LEG))))
        return E_FAIL;

    //가디언
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_AncientGuardianTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/DLC boss/Ancient_Guardian.png"))))
        return E_FAIL; 
    // 추가 - 파츠 버퍼
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_AG_Head",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, AncientGuardianUV::HEAD))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_AG_Tail1",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, AncientGuardianUV::TAIL1))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_AG_Tail2",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, AncientGuardianUV::TAIL2))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_AG_Tail3",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, AncientGuardianUV::TAIL3))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_AG_Spike",
        Engine::CCubeBodyTex::Create(m_pGraphicDev, AncientGuardianUV::SPIKE))))
        return E_FAIL;


    // Hotbar UI Textures

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Rocket",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/rocket.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_PotionEmpty",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/potion_empty.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ArrowEmpty",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/arrows_empty.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Arrow",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/arrow.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_IconTNTHUD",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/icon_TNT_HUD.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_HeartMain",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/Hotbar2/Heart/heart_main.png"))) )
        return E_FAIL;


    lstrcpy(m_szLoading, L"기타 등등 로딩");

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Transform", Engine::CTransform::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Calculator", Engine::CCalculator::Create(m_pGraphicDev))))
        return E_FAIL;

    lstrcpy(m_szLoading, L"Loading Complete!!!!");

    m_bFinish = true;

    return 0;
}

_uint CLoading::Loading_Camp()
{
    m_bFinish = false;

    lstrcpy(m_szLoading, L"Camp Texture Loading.......");

    //캠프 로딩씬
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CampTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, 
            L"../Bin/Resource/Texture/Logo/Loading_Screen_Lobby.png")))); 
    // 스카이박스 텍스처 등록
   

    lstrcpy(m_szLoading, L"Camp Loading Complete");

    m_bFinish = true;

    return 0;
}

_uint CLoading::Loading_RedStone()
{
    lstrcpy(m_szLoading, L"RedStone Texture Loading.......");

    //레드스톤 로딩씬
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Fiery_Forge.png"))));

    lstrcpy(m_szLoading, L"Loading Complete");

    m_bFinish = true;

    return 0;
}

_uint CLoading::Loading_Obsidian()
{
    lstrcpy(m_szLoading, L"Obsidian Texture Loading.......");

    //옵시디언 로딩씬
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Obsidian_Pinnacle.png"))))

    lstrcpy(m_szLoading, L"Loading Complete");

    m_bFinish = true;

    return 0;
}

unsigned int CLoading::Thread_Main(void* pArg)
{
    OutputDebugString(L"Thread_Main 시작\n");
    CLoading* pLoading = reinterpret_cast<CLoading*>(pArg);

    int iFlag(0);

    EnterCriticalSection(pLoading->Get_Crt());

    switch (pLoading->Get_LoadingID())
    {
    case LOADIND_SQUIDCOAST:
        iFlag = pLoading->Loading_SquidCoast();
        break;
    case LOADING_CAMP:
        iFlag = pLoading->Loading_Camp();
        break;
    case LOADING_REDSTONE:
        iFlag = pLoading->Loading_RedStone();
        break;
    case LOADING_OBSIDIAN:
        iFlag = pLoading->Loading_Obsidian();
        break;
    }

    LeaveCriticalSection(pLoading->Get_Crt());

    //_endthreadex(0);

    return iFlag;       // 0 리턴 시, _endthreadex가 자동 호출
}

CLoading* CLoading::Create(LPDIRECT3DDEVICE9 pGraphicDev, LOADINGID eID)
{
    CLoading* pLoading = new CLoading(pGraphicDev);

    if (FAILED(pLoading->Ready_Loading(eID)))
    {
        Safe_Release(pLoading);

        MSG_BOX("CLoading Create Failed");
        return nullptr;
    }

    return pLoading;
}

void CLoading::Free()
{
    WaitForSingleObject(m_hThread, INFINITE);

    CloseHandle(m_hThread);

    DeleteCriticalSection(&m_Crt);

    Safe_Release(m_pGraphicDev);


}