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
    lstrcpy(m_szLoading, L"Buffer Loading.....................................");

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

    lstrcpy(m_szLoading, L"Texture Loading.....................................");

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TerrainTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Terrain/Grass_%d.tga", 2))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SkyBoxTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/SkyBox/burger%d.dds", 4))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_EffectTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Explosion/Explosion%d.png", 90))))
        return E_FAIL;

    // BOSS
    // RedStoneGolem
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneGolemTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Boss/T_RedStone_Golem.png"))))
        return E_FAIL;

    //오징어 해안 로딩 텍스쳐`
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SquidCoastLoadingTexture",
    //    Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Loading_Screen_Squid_Coast.png"))))
    //    return E_FAIL;

    ////캠프 로딩 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CampLoadingTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Loading_Screen_Lobby.png"))))
        return E_FAIL;

    //레드 스톤 로딩 텍스쳐
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneLoadingTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Fiery_Forge.png"))))
        return E_FAIL;

    //옵시디언 로딩 텍스쳐
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


    //플레이어 아머 텍스쳐

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ArmorTexture",
        CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Player/T_BardsGarb.png"))))
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
    //stonebrick
    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_StoneBrickTexture",
        CTexture::Create(m_pGraphicDev, TEX_CUBE, L"../Bin/Resource/Texture/blocks/StoneBrickTexture.dds"))))
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

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_HotbarBackgroundLeft",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackgroundLeft.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_HotbarBackgroundMiddle",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackgroundMiddle.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_HotbarBackgroundMiddleIndent",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackgroundMiddleIndent.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_HotbarBackgroundRight",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/hotbarBackgroundRight.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_IndentSlotLeft",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotIndent/hotbar_indentslot_left.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_IndentSlotCenter",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotIndent/hotbar_indentslot_center.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_IndentSlotRight",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotIndent/hotbar_indentslot_right.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RollingIcon",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotIndent/rolling_icon.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SmallSlot",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotSmall/smallslot.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_IconInventory",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/Icons/v2_icon_inventory.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_IconMap",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/Icons/v2_icon_map.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_SquareFrame",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/square_frame.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Rocket",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/rocket.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_PotionEmpty",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/potion_empty.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ArrowSlot",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/SlotArrow/arrow_slot.png"))))
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

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_IconEmerald",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar2/Emeralds/icon_emerald.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_HeartFrame",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/heart_frame.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_HeartColor",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/HotBar/heart_color.png"))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_TestHotbarTexture",
        Engine::CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/UI/Materials/hotbar.png"))))
        return E_FAIL;

    lstrcpy(m_szLoading, L"Etc Loading.....................................");

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Transform", Engine::CTransform::Create(m_pGraphicDev))))
        return E_FAIL;

    if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_Calculator", Engine::CCalculator::Create(m_pGraphicDev))))
        return E_FAIL;

    lstrcpy(m_szLoading, L"Loading Complete !!!!");

    m_bFinish = true;

    return 0;
}

_uint CLoading::Loading_Camp()
{
    m_bFinish = false;

    lstrcpy(m_szLoading, L"Camp Texture Loading.......");

    //캠프 로딩씬
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_CampTexture",
    //    CTexture::Create(m_pGraphicDev, TEX_NORMAL, 
    //        L"../Bin/Resource/Texture/Logo/Loading_Screen_Lobby.png")))); 
    // 스카이박스 텍스처 등록
   

    lstrcpy(m_szLoading, L"Camp Loading Complete");

    m_bFinish = true;

    return 0;
}

_uint CLoading::Loading_RedStone()
{
    lstrcpy(m_szLoading, L"RedStone Texture Loading.......");

    //레드스톤 로딩씬
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_RedStoneTexture",
    //    CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Fiery_Forge.png"))));

    lstrcpy(m_szLoading, L"Loading Complete");

    m_bFinish = true;

    return 0;
}

_uint CLoading::Loading_Obsidian()
{
    lstrcpy(m_szLoading, L"Obsidian Texture Loading.......");

    //옵시디언 로딩씬
    //if (FAILED(CProtoMgr::GetInstance()->Ready_Prototype(L"Proto_ObsidianTexture",
    //    CTexture::Create(m_pGraphicDev, TEX_NORMAL, L"../Bin/Resource/Texture/Logo/Obsidian_Pinnacle.png"))))

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