#include "CSessionMgr.h"
#include "ServerLog.h"
#include <cstring>
#include <vector>

IMPLEMENT_SINGLETON_SERVER(CSessionMgr)

CSessionMgr::~CSessionMgr()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [id, pSession] : m_mapSessions)
        delete pSession;
    m_mapSessions.clear();
}
// =====================================================================
//  OnConnect
//  새 클라이언트 소켓을 받아 CSession 생성
// =====================================================================
CSession* CSessionMgr::OnConnect(SOCKET hSocket)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (static_cast<int>(m_mapSessions.size()) >= MAX_PLAYERS)
    {
        LOG_WARN("Server full (%d/%d) — rejecting new connection", (int)m_mapSessions.size(), MAX_PLAYERS);
        closesocket(hSocket);
        return nullptr;
    }

    int iId = m_iNextSessionId++;
    CSession* pSession = new CSession(iId, hSocket);
    m_mapSessions[iId] = pSession;

    LOG_NET("Session %d connected (total: %d)", iId, (int)m_mapSessions.size());
    return pSession;
}

// =====================================================================
//  OnDisconnect
// =====================================================================
void CSessionMgr::OnDisconnect(int iSessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_mapSessions.find(iSessionId);
    if (it == m_mapSessions.end())
        return;

    CSession* pSession = it->second;
    LOG_NET("Session %d (%s) disconnected (total after: %d)",
        iSessionId,
        pSession->GetNickname()[0] ? pSession->GetNickname() : "?",
        (int)m_mapSessions.size() - 1);

    pSession->Disconnect();
    delete pSession;
    m_mapSessions.erase(it);
}

// =====================================================================
//  CheckTimeout
//  마지막 수신으로부터 SESSION_TIMEOUT_MS 초과한 세션 제거
// =====================================================================
std::vector<CSessionMgr::TimedOutInfo> CSessionMgr::CheckTimeout()
{
    // 1. 락 안에서 타임아웃 세션 정보 수집 + 세션 삭제
    std::vector<CSessionMgr::TimedOutInfo> result;
    std::vector<int> toRemove;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        DWORD dwNow = GetTickCount();
        for (auto& [id, pSession] : m_mapSessions)
        {
            if (dwNow - pSession->GetLastRecvTime() > SESSION_TIMEOUT_MS)
            {
                LOG_WARN("Session %d ('%s') timed out",
                    id, pSession->GetNickname()[0] ? pSession->GetNickname() : "?");

                CSessionMgr::TimedOutInfo info = {};
                info.iSessionId = pSession->GetSessionId();
                info.iPlayerId  = pSession->GetPlayerId();
                strncpy_s(info.szNickname, pSession->GetNickname(), _TRUNCATE);
                result.push_back(info);
                toRemove.push_back(id);
            }
        }
    }

    // 2. 락 밖에서 OnDisconnect (내부에서 다시 락을 잡음)
    for (int id : toRemove)
        OnDisconnect(id);

    return result;  // CServer가 Despawn 브로드캐스트 + 스폰 해제에 사용
}

// =====================================================================
//  Find
// =====================================================================
CSession* CSessionMgr::Find(int iSessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_mapSessions.find(iSessionId);
    return (it != m_mapSessions.end()) ? it->second : nullptr;
}

// =====================================================================
//  FindByNickname  —  닉네임 중복 체크용
// =====================================================================
CSession* CSessionMgr::FindByNickname(const char* szNickname)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [id, pSession] : m_mapSessions)
    {
        if (pSession->IsLoggedIn() &&
            _stricmp(pSession->GetNickname(), szNickname) == 0)
            return pSession;
    }
    return nullptr;
}

// =====================================================================
//  GetCount
// =====================================================================
int CSessionMgr::GetCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<int>(m_mapSessions.size());
}

std::vector<int> CSessionMgr::GetSessionIds() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<int> ids;
    ids.reserve(m_mapSessions.size());
    for (auto& [id, _] : m_mapSessions)
        ids.push_back(id);
    return ids;
}

// =====================================================================
//  IssuePlayerId  —  순차 발급
// =====================================================================
int CSessionMgr::IssuePlayerId()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_iNextPlayerId++;
}

// =====================================================================
//  Broadcast  —  모든 세션(또는 excludeSessionId 제외)에 전송
// =====================================================================
void CSessionMgr::Broadcast(const void* pData, int iSize, int excludeSessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [id, pSession] : m_mapSessions)
    {
        if (id == excludeSessionId)
            continue;
        if (pSession->IsConnected())
            pSession->Send(pData, iSize);
    }
    ++m_iSendCount;
}

// =====================================================================
//  BroadcastToLoggedIn  —  로그인 완료된 세션에만 전송
// =====================================================================
void CSessionMgr::BroadcastToLoggedIn(const void* pData, int iSize, int excludeSessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [id, pSession] : m_mapSessions)
    {
        if (id == excludeSessionId)
            continue;
        if (pSession->IsConnected() && pSession->IsLoggedIn())
            pSession->Send(pData, iSize);
    }
    ++m_iSendCount;
}

// =====================================================================
//  PrintStatus  —  디버그 상태 출력
// =====================================================================
void CSessionMgr::PrintStatus() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    printf("=== Server Status ===  Players: %d  recv:%d/s  send:%d/s\n",
        (int)m_mapSessions.size(), m_iRecvCount, m_iSendCount);
    for (auto& [id, pSession] : m_mapSessions)
    {
        printf("  [%d] %-16s  x=%.1f y=%.1f z=%.1f  state=%s\n",
            pSession->GetPlayerId(),
            pSession->GetNickname()[0] ? pSession->GetNickname() : "(login...)",
            pSession->GetX(), pSession->GetY(), pSession->GetZ(),
            pSession->GetState() == 1 ? "run" : "idle");
    }
}
