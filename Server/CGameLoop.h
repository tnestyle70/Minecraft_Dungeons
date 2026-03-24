#pragma once

#include <thread>
#include <atomic>
#include "../Shared/PacketDef.h"

// =====================================================================
//  CGameLoop  —  서버 게임 틱 루프
//  - 20 TPS (50ms / tick)
//  - 매 틱: 입력 수집 → 위치 계산 → S2C_StateSnapshot 브로드캐스트
//  - CServer::Run() 에서 별도 스레드로 실행
// =====================================================================
class CGameLoop
{
public:
    CGameLoop()  = default;
    ~CGameLoop() { Stop(); }

    void Start();   // 틱 스레드 시작
    void Stop();    // 틱 스레드 종료 (Run() 반환 대기)

    bool IsRunning() const { return m_bRunning; }

private:
    void Run();             // 틱 루프 본체 (스레드)
    void Tick(float fDt);   // 단일 틱 처리

    void UpdateWorld  (float fDt);      // 입력 → 위치 계산
    void BroadcastSnapshot(DWORD dwTick); // S2C_StateSnapshot 전송

    // 플레이어 이동 속도 — CPlayer::m_fMoveSpeed 와 동일
    static constexpr float PLAYER_SPEED = 10.f;
    static constexpr int   TICK_RATE    = 20;       // TPS
    static constexpr int   TICK_MS      = 1000 / TICK_RATE; // 50ms

    std::thread       m_thread;
    std::atomic<bool> m_bRunning = false;
};
