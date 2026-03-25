#include "CServer.h"
#include "CSession.h"
#include "CSessionMgr.h"
#include "CStageMgr.h"
#include "ServerLog.h"
#include <cstring>
#include <vector>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

CServer::CServer() = default;

CServer::~CServer()
{
    m_gameLoop.Stop();
    Stop();

    // listen 소켓을 먼저 닫아야 AcceptThread의 accept() 블로킹이 풀림
    if (m_hListenSocket != INVALID_SOCKET)
    {
        closesocket(m_hListenSocket);
        m_hListenSocket = INVALID_SOCKET;
    }

    if (m_acceptThread.joinable()) m_acceptThread.join();
    if (m_recvThread.joinable())   m_recvThread.join();

    WSACleanup();
    LOG_INFO("Server shutdown complete");
}

bool CServer::Init(int iPort)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOG_WARN("WSAStartup failed: %d", WSAGetLastError());
        return false;
    }

    m_hListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_hListenSocket == INVALID_SOCKET)
    {
        LOG_WARN("socket() failed: %d", WSAGetLastError());
        WSACleanup();
        return false;
    }

    int opt = 1;
    setsockopt(m_hListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    setsockopt(m_hListenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt));

    sockaddr_in addr = {};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<u_short>(iPort));

    if (bind(m_hListenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        LOG_WARN("bind() failed: %d", WSAGetLastError());
        closesocket(m_hListenSocket);
        WSACleanup();
        return false;
    }

    if (listen(m_hListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        LOG_WARN("listen() failed: %d", WSAGetLastError());
        closesocket(m_hListenSocket);
        WSACleanup();
        return false;
    }

    LOG_INFO("Server listening on port %d (max players: %d)", iPort, MAX_PLAYERS);
    return true;
}

void CServer::Run()
{
    m_bRunning = true;
    m_dwLastStatusPrint  = GetTickCount();
    m_dwLastTimeoutCheck = GetTickCount();

    m_acceptThread = std::thread(&CServer::AcceptThread, this);
    m_recvThread   = std::thread(&CServer::RecvThread,   this);
    m_gameLoop.Start();

    LOG_INFO("Server running. Press Ctrl+C to stop.");

    while (m_bRunning)
    {
        DWORD dwNow = GetTickCount();

        if (dwNow - m_dwLastTimeoutCheck >= 5000)
        {
            // CheckTimeout: 타임아웃 세션 삭제 후 정보 반환 → Despawn + 스폰 해제
            auto timedOut = CSessionMgr::GetInstance()->CheckTimeout();
            for (auto& info : timedOut)
                HandleDisconnect(info.iSessionId, info.iPlayerId, info.szNickname);
            m_dwLastTimeoutCheck = dwNow;
        }

        if (dwNow - m_dwLastStatusPrint >= 5000)
        {
            CSessionMgr::GetInstance()->PrintStatus();
            CSessionMgr::GetInstance()->ResetCounters();
            m_dwLastStatusPrint = dwNow;
        }

        Sleep(500);
    }
}

void CServer::AcceptThread()
{
    while (m_bRunning)
    {
        sockaddr_in clientAddr = {};
        int iAddrLen = sizeof(clientAddr);

        SOCKET hClient = accept(m_hListenSocket, (sockaddr*)&clientAddr, &iAddrLen);
        if (hClient == INVALID_SOCKET)
        {
            if (m_bRunning)
                LOG_WARN("accept() failed: %d", WSAGetLastError());
            continue;
        }

        char szIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, szIP, sizeof(szIP));
        LOG_NET("Incoming connection from %s:%d", szIP, ntohs(clientAddr.sin_port));

        u_long iMode = 1;
        ioctlsocket(hClient, FIONBIO, &iMode);

        CSessionMgr::GetInstance()->OnConnect(hClient);
    }
}

void CServer::RecvThread()
{
    while (m_bRunning)
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        SOCKET maxFd = 0;

        // ID 목록을 락 안에서 복사 후, 개별 Find로 포인터 획득
        std::vector<CSession*> sessions;
        for (int id : CSessionMgr::GetInstance()->GetSessionIds())
        {
            CSession* pS = CSessionMgr::GetInstance()->Find(id);
            if (pS && pS->IsConnected())
            {
                sessions.push_back(pS);
                FD_SET(pS->GetSocket(), &readSet);
                if (pS->GetSocket() > maxFd) maxFd = pS->GetSocket();
            }
        }

        if (sessions.empty())
        {
            Sleep(10);
            continue;
        }

        timeval tv = { 0, 10000 };
        int iRet = select(static_cast<int>(maxFd + 1), &readSet, nullptr, nullptr, &tv);
        if (iRet <= 0) continue;

        for (CSession* pSession : sessions)
        {
            if (!FD_ISSET(pSession->GetSocket(), &readSet))
                continue;

            bool bOk = pSession->RecvData();
            CSessionMgr::GetInstance()->AddRecvCount();

            if (!bOk)
            {
                HandleDisconnect(pSession->GetSessionId(),
                                 pSession->GetPlayerId(),
                                 pSession->GetNickname());
                continue;
            }

            while (true)
            {
                const PKT_HEADER* pHdr = pSession->PeekPacket();
                if (!pHdr) break;

                switch (pHdr->wType)
                {
                case C2S_LOGIN:
                    HandleLogin(pSession, reinterpret_cast<const PKT_C2S_Login*>(pHdr));
                    break;
                case C2S_INPUT:
                    HandleInput(pSession, reinterpret_cast<const PKT_C2S_Input*>(pHdr));
                    break;
                default:
                    LOG_WARN("Session %d unknown packet type: %d", pSession->GetSessionId(), pHdr->wType);
                    break;
                }

                pSession->PopPacket();
            }
        }
    }
}

void CServer::HandleLogin(CSession* pSession, const PKT_C2S_Login* pPkt)
{
    PKT_S2C_LoginAck ack = {};
    FillHeader(ack, S2C_LOGIN_ACK);

    if (pPkt->iVersion != PROTOCOL_VERSION)
    {
        ack.bSuccess  = false;
        ack.iPlayerId = -1;
        strncpy_s(ack.szMessage, "Version mismatch", _TRUNCATE);
        pSession->Send(&ack, sizeof(ack));
        LOG_WARN("Session %d login rejected: version %d (expected %d)",
            pSession->GetSessionId(), pPkt->iVersion, PROTOCOL_VERSION);
        return;
    }

    if (CSessionMgr::GetInstance()->FindByNickname(pPkt->szNickname))
    {
        ack.bSuccess  = false;
        ack.iPlayerId = -1;
        strncpy_s(ack.szMessage, "Nickname already in use", _TRUNCATE);
        pSession->Send(&ack, sizeof(ack));
        LOG_WARN("Session %d login rejected: duplicate nickname '%s'",
            pSession->GetSessionId(), pPkt->szNickname);
        return;
    }

    int iPlayerId = CSessionMgr::GetInstance()->IssuePlayerId();
    pSession->SetNickname(pPkt->szNickname);
    pSession->SetPlayerId(iPlayerId);
    pSession->SetLoggedIn(true);

    ack.bSuccess  = true;
    ack.iPlayerId = iPlayerId;
    ack.iStageId  = 1;
    strncpy_s(ack.szMessage, "Welcome!", _TRUNCATE);
    pSession->Send(&ack, sizeof(ack));

    LOG_NET("Player %d ('%s') logged in (session %d)",
        iPlayerId, pPkt->szNickname, pSession->GetSessionId());

    // ── Day 2: 스폰 포인트 배정 ───────────────────────────────────────
    CTestStage* pStage = CStageMgr::GetInstance()->GetTestStage();

    int iSlot = pStage->AssignSpawnPoint(pSession);
    if (iSlot < 0)
    {
        // 슬롯 없음 → 로그인 취소 (이미 LoginAck를 보냈으므로 연결만 끊음)
        LOG_WARN("Stage full — disconnecting player %d", iPlayerId);
        pSession->Disconnect();
        return;
    }

    // ── 신규 플레이어에게 전체 플레이어 목록 전송 ───────────────────────
    //    (자신 포함, 로그인된 모든 세션 포함)
    pStage->SendSpawnToNew(pSession);

    // ── 기존 플레이어들에게 신규 플레이어 스폰 알림 ─────────────────────
    pStage->BroadcastNewPlayer(pSession);
}

// =====================================================================
//  HandleInput  —  C2S_Input 수신 → 세션에 입력값 저장
//  실제 위치 계산은 CGameLoop::UpdateWorld() 에서 처리
// =====================================================================
void CServer::HandleInput(CSession* pSession, const PKT_C2S_Input* pPkt)
{
    if (!pSession->IsLoggedIn()) return;
    //클라이언트가 서버로 보낸 패킷의 정보를 확인해서 해당 정보를 신뢰 후 위치 반영
    if (pPkt->bOnDragon)
        pSession->SetPosition(pPkt->fX, pPkt->fY, pPkt->fZ);
    else
        pSession->SetPosition(pPkt->fX, pSession->GetY(), pPkt->fZ);

    pSession->SetInput(pPkt->fDirX, pPkt->fDirZ, pPkt->fRotY);
    pSession->SetLastSeq(pPkt->iSequence);
    pSession->SetOnDragon(pPkt->bOnDragon, pPkt->iDragonIdx);
    CSessionMgr::GetInstance()->AddRecvCount();
}

// =====================================================================
//  HandleDisconnect  —  세션 종료 공통 처리
//  RecvThread (recv 실패) 와 CheckTimeout (시간 초과) 양쪽에서 호출
// =====================================================================
void CServer::HandleDisconnect(int iSessId, int iPlayerId, const char* szNickname)
{
    // 1. 스폰 포인트 해제
    CStageMgr::GetInstance()->GetTestStage()->ReleaseSpawnPoint(iSessId);

    // 2. 로그인된 플레이어였으면 나머지 클라이언트에 Despawn 브로드캐스트
    if (iPlayerId != -1)
    {
        PKT_S2C_Despawn pkt = {};
        FillHeader(pkt, S2C_DESPAWN);
        pkt.iPlayerId = iPlayerId;
        CSessionMgr::GetInstance()->BroadcastToLoggedIn(&pkt, sizeof(pkt), iSessId);
        LOG_NET("Despawn broadcast: player %d ('%s')", iPlayerId, szNickname);
    }

    // 3. 세션 제거 (소켓 닫기 + map에서 삭제)
    CSessionMgr::GetInstance()->OnDisconnect(iSessId);
}
