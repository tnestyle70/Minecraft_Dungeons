#include "CGameLoop.h"
#include "CSessionMgr.h"
#include "ServerLog.h"
#include <vector>
#include <cmath>

// =====================================================================
//  Start / Stop
// =====================================================================
void CGameLoop::Start()
{
    if (m_bRunning) return;
    m_bRunning = true;
    m_thread   = std::thread(&CGameLoop::Run, this);
    LOG_INFO("GameLoop started (%d TPS, %dms/tick)", TICK_RATE, TICK_MS);
}

void CGameLoop::Stop()
{
    m_bRunning = false;
    if (m_thread.joinable())
        m_thread.join();
}

// =====================================================================
//  Run  —  틱 루프 (드리프트 보정 포함)
// =====================================================================
void CGameLoop::Run()
{
    DWORD dwTick      = 0;
    DWORD dwLastTime  = GetTickCount();

    while (m_bRunning)
    {
        DWORD dwNow     = GetTickCount();
        DWORD dwElapsed = dwNow - dwLastTime;
        dwLastTime      = dwNow;

        float fDt = static_cast<float>(dwElapsed) / 1000.f;
        // fDt 상한: 프레임 드롭 시 위치 튐 방지
        if (fDt > 0.1f) fDt = 0.1f;

        Tick(fDt);
        ++dwTick;

        // 드리프트 보정 슬립
        DWORD dwProcess = GetTickCount() - dwNow;
        if (dwProcess < static_cast<DWORD>(TICK_MS))
            Sleep(TICK_MS - dwProcess);
    }
}

// =====================================================================
//  Tick
// =====================================================================
void CGameLoop::Tick(float fDt)
{
    UpdateWorld(fDt);

    static DWORD s_dwTick = 0;
    BroadcastSnapshot(++s_dwTick);
}

// =====================================================================
//  UpdateWorld  —  C2S_Input 입력값으로 위치 계산 (서버 권위)
// =====================================================================
void CGameLoop::UpdateWorld(float fDt)
{
    std::vector<int> ids = CSessionMgr::GetInstance()->GetSessionIds();

    for (int id : ids)
    {
        CSession* pS = CSessionMgr::GetInstance()->Find(id);
        if (!pS || !pS->IsLoggedIn()) continue;
        //드래곤 탑승 중일 경우 서버단에서의 적분 생략
        if (pS->GetOnDragon()) continue;

        // 마지막으로 받은 입력 방향으로 위치 계산
        float fDirX  = pS->GetDirX();
        float fDirZ  = pS->GetDirZ();
        bool  bMove  = (fDirX != 0.f || fDirZ != 0.f);

        if (bMove)
        {
            // 방향 정규화
            float fLen = sqrtf(fDirX * fDirX + fDirZ * fDirZ);
            if (fLen > 0.f) { fDirX /= fLen; fDirZ /= fLen; }

            float fNewX = pS->GetX() + fDirX * PLAYER_SPEED * fDt;
            float fNewZ = pS->GetZ() + fDirZ * PLAYER_SPEED * fDt;
            pS->SetPosition(fNewX, pS->GetY(), fNewZ);
            pS->SetState(1); // run
        }
        else
        {
            pS->SetState(0); // idle
        }
    }
}

// =====================================================================
//  BroadcastSnapshot  —  모든 로그인 세션에 상태 전송
// =====================================================================
void CGameLoop::BroadcastSnapshot(DWORD dwTick)
{
    std::vector<int> ids = CSessionMgr::GetInstance()->GetSessionIds();

    PKT_S2C_StateSnapshot pkt = {};
    FillHeader(pkt, S2C_STATE_SNAPSHOT);
    pkt.dwServerTick  = dwTick;
    pkt.iPlayerCount  = 0;

    for (int id : ids)
    {
        CSession* pS = CSessionMgr::GetInstance()->Find(id);
        if (!pS || !pS->IsLoggedIn()) continue;
        if (pkt.iPlayerCount >= MAX_PLAYERS) break;

        int idx = pkt.iPlayerCount++;
        pkt.players[idx].iPlayerId    = pS->GetPlayerId();
        pkt.players[idx].fX           = pS->GetX();
        pkt.players[idx].fY           = pS->GetY();
        pkt.players[idx].fZ           = pS->GetZ();
        pkt.players[idx].fRotY        = pS->GetRotY();
        pkt.players[idx].iState       = pS->GetState();
        pkt.players[idx].iLastSequence = pS->GetLastSeq();
        pkt.players[idx].bOnDragon = pS->GetOnDragon();
        pkt.players[idx].iDragonIdx = pS->GetDragonIdx();
    }

    if (pkt.iPlayerCount > 0)
    {
        CSessionMgr::GetInstance()->BroadcastToLoggedIn(&pkt, sizeof(pkt));
    }
}
