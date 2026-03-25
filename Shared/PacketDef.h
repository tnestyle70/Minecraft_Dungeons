#pragma once

#include <winsock2.h>
#include <windows.h>

#define PROTOCOL_VERSION 1
#define MAX_PLAYERS 50
#define SESSION_TIMEOUT_MS 15000  // 15s timeout (after C2S_Input 20TPS verified)

enum PACKET_TYPE : WORD
{
    PKT_NONE = 0,

    C2S_LOGIN = 1,
    C2S_INPUT = 2,
    C2S_ATTACK = 4,   // Day 9: attack event (arrow)
    C2S_DAMAGE = 5,   // Day 9: PVP damage notify

    S2C_LOGIN_ACK = 101,
    S2C_SPAWN = 102,
    S2C_STATE_SNAPSHOT = 103,
    S2C_DESPAWN = 104,
    S2C_ATTACK = 106, // Day 9: attack relay
    S2C_DAMAGE = 107, // Day 9: damage relay
};

#pragma pack(push, 1)

struct PKT_HEADER
{
    WORD wSize;
    WORD wType;
};

struct PKT_C2S_Login
{
    PKT_HEADER header;
    char szNickname[32];
    int iVersion;
};

struct PKT_S2C_LoginAck
{
    PKT_HEADER header;
    int iPlayerId;
    int iStageId;
    bool bSuccess;
    char szMessage[64];
};

struct PlayerSpawnInfo
{
    int iPlayerId;
    float fX, fY, fZ;
    float fRotY;
    char szNickname[32];
};

struct PKT_S2C_Spawn
{
    PKT_HEADER header;
    int iMyPlayerId;
    int iPlayerCount;
    PlayerSpawnInfo players[MAX_PLAYERS];
};

struct PKT_C2S_Input
{
    PKT_HEADER header;
    int iSequence;
    float fDirX;
    float fDirZ;
    float fRotY;
    bool bMoving;
    DWORD dwTimestamp;
    float fX;   // client position (server correction)
    float fY;
    float fZ;
    bool  bOnDragon;    // on-dragon flag
    int   iDragonIdx;   // dragon index (0~3, -1 if none)
    float fDragonX, fDragonY, fDragonZ;  // Day 8: dragon root position
};

struct PlayerState
{
    int iPlayerId;
    float fX, fY, fZ;
    float fRotY;
    int  iState;
    int  iLastSequence;
    bool  bOnDragon;    // on-dragon flag
    int   iDragonIdx;   // dragon index (0~3, -1 if none)
    float fDragonX, fDragonY, fDragonZ;  // Day 8: dragon root position
};

struct PKT_S2C_StateSnapshot
{
    PKT_HEADER header;
    DWORD dwServerTick;
    int iPlayerCount;
    PlayerState players[MAX_PLAYERS];
};

struct PKT_S2C_Despawn
{
    PKT_HEADER header;
    int iPlayerId;
};

// Day 9: attack sync
struct PKT_C2S_Attack
{
    PKT_HEADER header;
    float fPosX, fPosY, fPosZ;   // 발사 위치
    float fDirX, fDirY, fDirZ;   // 정규화된 방향
    float fCharge;                // charge ratio (0.0~1.0)
    bool  bFirework;              // firework arrow flag
};

struct PKT_S2C_Attack
{
    PKT_HEADER header;
    int   iPlayerId;              // attacker id
    float fPosX, fPosY, fPosZ;
    float fDirX, fDirY, fDirZ;
    float fCharge;
    bool  bFirework;
};

// Day 9: PVP damage sync
struct PKT_C2S_Damage
{
    PKT_HEADER header;
    int   iTargetPlayerId;        // target id
    float fDamage;
};

struct PKT_S2C_Damage
{
    PKT_HEADER header;
    int   iTargetPlayerId;
    int   iAttackerPlayerId;
    float fDamage;
};

#pragma pack(pop)

template <typename T>
inline void FillHeader(T& pkt, PACKET_TYPE type)
{
    pkt.header.wSize = static_cast<WORD>(sizeof(T));
    pkt.header.wType = static_cast<WORD>(type);
}
