#pragma once

#include <winsock2.h>
#include <windows.h>

#define PROTOCOL_VERSION 1
#define MAX_PLAYERS 10
#define SESSION_TIMEOUT_MS 15000  // 15초 타임아웃 (C2S_Input 20TPS 정상 동작 확인 후 복구)

enum PACKET_TYPE : WORD
{
    PKT_NONE = 0,

    C2S_LOGIN = 1,
    C2S_INPUT = 2,

    S2C_LOGIN_ACK = 101,
    S2C_SPAWN = 102,
    S2C_STATE_SNAPSHOT = 103,
    S2C_DESPAWN = 104,
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
    float fX;   // 클라이언트 실제 위치 (서버 위치 보정용)
    float fY;
    float fZ;
    bool  bOnDragon;    // 드래곤 탑승 여부
    int   iDragonIdx;   // 탑승한 드래곤 인덱스 (0~3, 미탑승 시 -1)
};

struct PlayerState
{
    int iPlayerId;
    float fX, fY, fZ;
    float fRotY;
    int  iState;
    int  iLastSequence;
    bool bOnDragon;     // 드래곤 탑승 여부
    int  iDragonIdx;    // 탑승한 드래곤 인덱스 (0~3, 미탑승 시 -1)
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

#pragma pack(pop)

template <typename T>
inline void FillHeader(T& pkt, PACKET_TYPE type)
{
    pkt.header.wSize = static_cast<WORD>(sizeof(T));
    pkt.header.wType = static_cast<WORD>(type);
}
