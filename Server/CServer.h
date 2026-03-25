#pragma once

#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <atomic>
#include "../Shared/PacketDef.h"
#include "CGameLoop.h"

class CSession;

class CServer
{
public:
    CServer();
    ~CServer();

    bool Init(int iPort);
    void Run();
    void Stop() { m_bRunning = false; }
    bool IsRunning() const { return m_bRunning; }

private:
    void AcceptThread();
    void RecvThread();
    void HandleLogin (CSession* pSession, const PKT_C2S_Login*  pPkt);
    void HandleInput (CSession* pSession, const PKT_C2S_Input*  pPkt);
    void HandleAttack(CSession* pSession, const PKT_C2S_Attack* pPkt); // Day 9
    void HandleDamage(CSession* pSession, const PKT_C2S_Damage* pPkt); // Day 9

    // 세션 종료 공통 처리: 스폰 해제 + Despawn 브로드캐스트 + OnDisconnect
    void HandleDisconnect(int iSessId, int iPlayerId, const char* szNickname);

    SOCKET m_hListenSocket = INVALID_SOCKET;
    std::atomic<bool> m_bRunning = false;
    std::thread m_acceptThread;
    std::thread m_recvThread;
    DWORD m_dwLastStatusPrint = 0;
    DWORD m_dwLastTimeoutCheck = 0;

    CGameLoop m_gameLoop;
};
