#include "CTestStage.h"
#include "CSession.h"
#include "CSessionMgr.h"
#include "ServerLog.h"
#include "../Shared/PacketDef.h"
#include <cstring>
#include <vector>

// =====================================================================
//  스폰 포인트 좌표 (서버 Y=0 기준 고정)
// =====================================================================
const SpawnPoint CTestStage::SPAWN_POINTS[CTestStage::MAX_SPAWN] =
{
    // 그리드: X(-45..45 step 10) × Z(25..-15 step -10) = 10열 × 5행 = 50개
    // 행 0 (Z=25)
    { -45.f, 0.f,  25.f }, { -35.f, 0.f,  25.f }, { -25.f, 0.f,  25.f }, { -15.f, 0.f,  25.f }, {  -5.f, 0.f,  25.f },
    {   5.f, 0.f,  25.f }, {  15.f, 0.f,  25.f }, {  25.f, 0.f,  25.f }, {  35.f, 0.f,  25.f }, {  45.f, 0.f,  25.f },
    // 행 1 (Z=15)
    { -45.f, 0.f,  15.f }, { -35.f, 0.f,  15.f }, { -25.f, 0.f,  15.f }, { -15.f, 0.f,  15.f }, {  -5.f, 0.f,  15.f },
    {   5.f, 0.f,  15.f }, {  15.f, 0.f,  15.f }, {  25.f, 0.f,  15.f }, {  35.f, 0.f,  15.f }, {  45.f, 0.f,  15.f },
    // 행 2 (Z=5)
    { -45.f, 0.f,   5.f }, { -35.f, 0.f,   5.f }, { -25.f, 0.f,   5.f }, { -15.f, 0.f,   5.f }, {  -5.f, 0.f,   5.f },
    {   5.f, 0.f,   5.f }, {  15.f, 0.f,   5.f }, {  25.f, 0.f,   5.f }, {  35.f, 0.f,   5.f }, {  45.f, 0.f,   5.f },
    // 행 3 (Z=-5)
    { -45.f, 0.f,  -5.f }, { -35.f, 0.f,  -5.f }, { -25.f, 0.f,  -5.f }, { -15.f, 0.f,  -5.f }, {  -5.f, 0.f,  -5.f },
    {   5.f, 0.f,  -5.f }, {  15.f, 0.f,  -5.f }, {  25.f, 0.f,  -5.f }, {  35.f, 0.f,  -5.f }, {  45.f, 0.f,  -5.f },
    // 행 4 (Z=-15)
    { -45.f, 0.f, -15.f }, { -35.f, 0.f, -15.f }, { -25.f, 0.f, -15.f }, { -15.f, 0.f, -15.f }, {  -5.f, 0.f, -15.f },
    {   5.f, 0.f, -15.f }, {  15.f, 0.f, -15.f }, {  25.f, 0.f, -15.f }, {  35.f, 0.f, -15.f }, {  45.f, 0.f, -15.f }
};

CTestStage::CTestStage()
{
    for (int i = 0; i < MAX_SPAWN; ++i)
        m_bOccupied[i] = false;

    LOG_INFO("TestStage initialized — %d spawn points ready", MAX_SPAWN);
}

// =====================================================================
//  AssignSpawnPoint
//  빈 슬롯을 찾아 세션에 좌표를 설정하고 슬롯 인덱스를 반환
// =====================================================================
int CTestStage::AssignSpawnPoint(CSession* pSession)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (int i = 0; i < MAX_SPAWN; ++i)
    {
        if (!m_bOccupied[i])
        {
            m_bOccupied[i] = true;
            m_SpawnMap[pSession->GetSessionId()] = i;

            const SpawnPoint& sp = SPAWN_POINTS[i];
            pSession->SetPosition(sp.fX, sp.fY, sp.fZ);
            pSession->SetRotY(0.f);

            LOG_NET("Player %d assigned spawn slot %d (%.1f, %.1f, %.1f)",
                pSession->GetPlayerId(), i, sp.fX, sp.fY, sp.fZ);
            return i;
        }
    }

    LOG_WARN("No spawn slot available for session %d", pSession->GetSessionId());
    return -1;
}

// =====================================================================
//  ReleaseSpawnPoint
//  세션이 퇴장할 때 슬롯 해제
// =====================================================================
void CTestStage::ReleaseSpawnPoint(int iSessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_SpawnMap.find(iSessionId);
    if (it == m_SpawnMap.end())
        return;

    int iSlot = it->second;
    m_bOccupied[iSlot] = false;
    m_SpawnMap.erase(it);

    LOG_NET("Spawn slot %d released (session %d)", iSlot, iSessionId);
}

// =====================================================================
//  SendSpawnToNew
//  신규 플레이어에게 현재 스테이지의 전체 플레이어 목록 전송
//  (자신 포함, 로그인 완료된 세션만)
// =====================================================================
void CTestStage::SendSpawnToNew(CSession* pNewSession)
{
    std::vector<int> ids = CSessionMgr::GetInstance()->GetSessionIds();

    PKT_S2C_Spawn pkt = {};
    FillHeader(pkt, S2C_SPAWN);
    pkt.iMyPlayerId  = pNewSession->GetPlayerId();
    pkt.iPlayerCount = 0;

    for (int id : ids)
    {
        CSession* pS = CSessionMgr::GetInstance()->Find(id);
        if (!pS || !pS->IsLoggedIn())
            continue;
        if (pkt.iPlayerCount >= MAX_PLAYERS)
            break;

        int idx = pkt.iPlayerCount++;
        pkt.players[idx].iPlayerId = pS->GetPlayerId();
        pkt.players[idx].fX       = pS->GetX();
        pkt.players[idx].fY       = pS->GetY();
        pkt.players[idx].fZ       = pS->GetZ();
        pkt.players[idx].fRotY    = pS->GetRotY();
        strncpy_s(pkt.players[idx].szNickname, pS->GetNickname(), _TRUNCATE);
    }

    pNewSession->Send(&pkt, sizeof(pkt));

    LOG_NET("Sent spawn list (%d players) to player %d ('%s')",
        pkt.iPlayerCount, pNewSession->GetPlayerId(), pNewSession->GetNickname());
}

// =====================================================================
//  BroadcastNewPlayer
//  기존 로그인 플레이어들에게 신규 플레이어 스폰 알림
//  수신자마다 iMyPlayerId(자신의 ID)를 개별 세팅하여 전송
// =====================================================================
void CTestStage::BroadcastNewPlayer(CSession* pNewSession)
{
    std::vector<int> ids = CSessionMgr::GetInstance()->GetSessionIds();
    int iNotifyCount = 0;

    for (int id : ids)
    {
        CSession* pS = CSessionMgr::GetInstance()->Find(id);
        if (!pS || !pS->IsLoggedIn())
            continue;
        if (pS->GetSessionId() == pNewSession->GetSessionId())
            continue;   // 신규 본인 제외

        // 수신자 기준 패킷 구성
        PKT_S2C_Spawn pkt = {};
        FillHeader(pkt, S2C_SPAWN);
        pkt.iMyPlayerId  = pS->GetPlayerId();     // 수신자 자신의 ID
        pkt.iPlayerCount = 1;

        pkt.players[0].iPlayerId = pNewSession->GetPlayerId();
        pkt.players[0].fX        = pNewSession->GetX();
        pkt.players[0].fY        = pNewSession->GetY();
        pkt.players[0].fZ        = pNewSession->GetZ();
        pkt.players[0].fRotY     = pNewSession->GetRotY();
        strncpy_s(pkt.players[0].szNickname, pNewSession->GetNickname(), _TRUNCATE);

        pS->Send(&pkt, sizeof(pkt));
        ++iNotifyCount;
    }

    LOG_NET("Broadcast new player %d ('%s') → notified %d existing player(s)",
        pNewSession->GetPlayerId(), pNewSession->GetNickname(), iNotifyCount);
}
