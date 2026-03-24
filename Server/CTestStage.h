#pragma once

#include <map>
#include <mutex>

class CSession;

// =====================================================================
//  SpawnPoint  —  서버는 DX9를 사용하지 않으므로 단순 float 구조체 사용
// =====================================================================
struct SpawnPoint
{
    float fX, fY, fZ;
};

// =====================================================================
//  CTestStage
//  - 스폰 포인트 4개 관리 (배정 / 해제)
//  - 신규 접속자에게 전체 플레이어 목록 전송
//  - 기존 플레이어들에게 신규 접속 알림
// =====================================================================
class CTestStage
{
public:
    CTestStage();
    ~CTestStage() = default;

    // 빈 슬롯 배정 → 세션에 위치 설정 → 슬롯 인덱스 반환 (-1: 슬롯 없음)
    int  AssignSpawnPoint(CSession* pSession);

    // 퇴장 시 슬롯 해제
    void ReleaseSpawnPoint(int iSessionId);

    // 신규 플레이어에게 현재 전체 플레이어 목록 전송 (자신 포함)
    void SendSpawnToNew(CSession* pNewSession);

    // 기존 로그인 플레이어들에게 신규 플레이어 스폰 알림
    void BroadcastNewPlayer(CSession* pNewSession);

    int  GetStageId() const { return m_iStageId; }

private:
    static constexpr int MAX_SPAWN = 4;

    // 빈 맵 기준 고정 좌표 — 서로 10유닛 간격
    //
    //    Z+
    //  [-5, 5]   [5, 5]
    //      |
    //  ----+----  X
    //      |
    //  [-5,-5]   [5,-5]
    //    Z-
    static const SpawnPoint SPAWN_POINTS[MAX_SPAWN];

    int                m_iStageId = 1;
    bool               m_bOccupied[MAX_SPAWN] = {};
    std::map<int, int> m_SpawnMap;   // sessionId → slotIndex
    std::mutex         m_mutex;
};
