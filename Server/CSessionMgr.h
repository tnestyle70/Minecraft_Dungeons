#pragma once

#include <map>
#include <mutex>
#include <vector>
#include "CSession.h"
#include "../Shared/PacketDef.h"
#define DECLARE_SINGLETON_SERVER(CLASSNAME)         \
    private:                                        \
        CLASSNAME() = default;                      \
        ~CLASSNAME();                               \
        CLASSNAME(const CLASSNAME&) = delete;       \
        CLASSNAME& operator=(const CLASSNAME&) = delete; \
        static CLASSNAME* m_pInstance;              \
    public:                                         \
        static CLASSNAME* GetInstance();            \
        static void DestroyInstance();

#define IMPLEMENT_SINGLETON_SERVER(CLASSNAME)       \
    CLASSNAME* CLASSNAME::m_pInstance = nullptr;    \
    CLASSNAME* CLASSNAME::GetInstance() {           \
        if (!m_pInstance) m_pInstance = new CLASSNAME(); \
        return m_pInstance;                         \
    }                                               \
    void CLASSNAME::DestroyInstance() {             \
        delete m_pInstance;                         \
        m_pInstance = nullptr;                      \
    }


class CSessionMgr
{
    DECLARE_SINGLETON_SERVER(CSessionMgr)

// CheckTimeout 반환용 — 세션 삭제 전에 CServer가 Despawn/스폰 해제에 쓸 정보
struct TimedOutInfo
{
    int  iSessionId;
    int  iPlayerId;
    char szNickname[32];
};

public:
    CSession* OnConnect(SOCKET hSocket);
    void OnDisconnect(int iSessionId);
    std::vector<TimedOutInfo> CheckTimeout(); // 타임아웃 세션 정리 후 정보 반환

    CSession* Find(int iSessionId);
    CSession* FindByNickname(const char* szNickname);
    int GetCount() const;
    // 현재 세션 ID 목록을 복사해서 반환 (RecvThread에서 포인터 대신 ID만 가져갈 때 사용)
    std::vector<int> GetSessionIds() const;
    int IssuePlayerId();

    void Broadcast(const void* pData, int iSize, int excludeSessionId = -1);
    void BroadcastToLoggedIn(const void* pData, int iSize, int excludeSessionId = -1);

    void PrintStatus() const;
    void AddRecvCount() { ++m_iRecvCount; }
    void AddSendCount() { ++m_iSendCount; }
    void ResetCounters() { m_iRecvCount = 0; m_iSendCount = 0; }
    int GetRecvCount() const { return m_iRecvCount; }
    int GetSendCount() const { return m_iSendCount; }

private:
    std::map<int, CSession*> m_mapSessions;
    mutable std::mutex m_mutex;
    int m_iNextSessionId = 1;
    int m_iNextPlayerId = 1;
    int m_iRecvCount = 0;
    int m_iSendCount = 0;
};
