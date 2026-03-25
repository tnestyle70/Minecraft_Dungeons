#include "pch.h"
#include "CNetworkMgr.h"
#include "CRemotePlayer.h"
#include <cstring>
#include <cstdio>

CNetworkMgr* CNetworkMgr::m_pInstance = nullptr;

// =====================================================================
CNetworkMgr* CNetworkMgr::GetInstance()
{
    if (!m_pInstance)
        m_pInstance = new CNetworkMgr();
    return m_pInstance;
}

void CNetworkMgr::DestroyInstance()
{
    if (m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
}

CNetworkMgr::~CNetworkMgr()
{
    Disconnect();
}

// =====================================================================
//  Connect
//  Winsock 초기화 → 소켓 생성 → 서버 연결 → 논블로킹 전환 → C2S_Login 전송
// =====================================================================
bool CNetworkMgr::Connect(LPDIRECT3DDEVICE9 pGraphicDev,
    const char* szIP, int iPort,
    const char* szNickname)
{
    m_pGraphicDev = pGraphicDev;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("[NET] WSAStartup failed: %d\n", WSAGetLastError());
        return false;
    }

    m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_hSocket == INVALID_SOCKET)
    {
        printf("[NET] socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return false;
    }

    // TCP_NODELAY — 소량 패킷 지연 방지
    int iOpt = 1;
    setsockopt(m_hSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&iOpt, sizeof(iOpt));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(iPort));
    inet_pton(AF_INET, szIP, &addr.sin_addr);

    if (connect(m_hSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        printf("[NET] connect() failed: %d\n", WSAGetLastError());
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    // Non-blocking recv (FIONBIO)
    unsigned long nonBlocking = 1;
    ioctlsocket(m_hSocket, FIONBIO, &nonBlocking);

    m_bConnected = true;
    printf("[NET] Connected to %s:%d\n", szIP, iPort);

    // ── 로그인 패킷 전송 ──────────────────────────────────────────────
    PKT_C2S_Login pkt = {};
    FillHeader(pkt, C2S_LOGIN);
    strncpy_s(pkt.szNickname, szNickname, _TRUNCATE);
    pkt.iVersion = PROTOCOL_VERSION;
    Send(&pkt, sizeof(pkt));

    printf("[NET] Sent C2S_Login: nickname='%s'\n", szNickname);
    return true;
}

// =====================================================================
//  Disconnect
// =====================================================================
void CNetworkMgr::Disconnect()
{
    if (m_hSocket != INVALID_SOCKET)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }
    m_bConnected = false;
    m_iMyPlayerId = -1;

    // 원격 플레이어 전체 해제
    for (auto& [id, pRemote] : m_mapRemote)
    {
        if (pRemote)
            pRemote->Release();
    }
    m_mapRemote.clear();

    WSACleanup();
    printf("[NET] Disconnected\n");
}

// =====================================================================
//  Update  —  매 프레임 호출 (CNetworkStage::Update_Scene)
// =====================================================================
void CNetworkMgr::Update(float fTimeDelta)
{
    if (!m_bConnected)
        return;

    RecvAndProcess();

    // 원격 플레이어 업데이트
    for (auto& [id, pRemote] : m_mapRemote)
    {
        if (pRemote)
            pRemote->Update_GameObject(fTimeDelta);
    }
}

void CNetworkMgr::LateUpdate(float fTimeDelta)
{
    for (auto& [id, pRemote] : m_mapRemote)
    {
        if (pRemote)
            pRemote->LateUpdate_GameObject(fTimeDelta);
    }
}

void CNetworkMgr::Render()
{
    for (auto& [id, pRemote] : m_mapRemote)
    {
        if (pRemote)
            pRemote->Render_GameObject();
    }
}

// =====================================================================
//  SendInput  —  Day 3 에서 CPlayer 입력 연동 시 호출
// =====================================================================
void CNetworkMgr::SendInput(float fDirX, float fDirZ, float fRotY, bool bMoving,
    float fX, float fY, float fZ, bool bOnDragon, int iDragonIdx)
{
    if (!m_bConnected || m_iMyPlayerId == -1)
        return;
    //전송할 패킷 구조체의 정보
    PKT_C2S_Input pkt = {};
    FillHeader(pkt, C2S_INPUT);
    pkt.iSequence = ++m_iSequence;
    pkt.fDirX = fDirX;
    pkt.fDirZ = fDirZ;
    pkt.fRotY = fRotY;
    pkt.bMoving = bMoving;
    pkt.dwTimestamp = GetTickCount();
    pkt.fX = fX;
    pkt.fY = fY;
    pkt.fZ = fZ;
    pkt.bOnDragon = bOnDragon;
    pkt.iDragonIdx = iDragonIdx;

    Send(&pkt, sizeof(pkt));
}

// =====================================================================
//  RecvAndProcess  —  논블로킹 recv → 버퍼 누적 → 패킷 단위 처리
// =====================================================================
void CNetworkMgr::RecvAndProcess()
{
    // 남은 버퍼 공간만큼 수신 시도
    int iBufLeft = RECV_BUF_SIZE - m_iRecvLen;
    if (iBufLeft <= 0)
    {
        printf("[NET] WARN: recv buffer full, clearing\n");
        m_iRecvLen = 0;
        return;
    }

    int iRet = recv(m_hSocket, m_recvBuf + m_iRecvLen, iBufLeft, 0);
    if (iRet > 0)
    {
        m_iRecvLen += iRet;
    }
    else if (iRet == 0)
    {
        printf("[NET] Server closed connection\n");
        m_bConnected = false;
        return;
    }
    else
    {
        int iErr = WSAGetLastError();
        // WSAEWOULDBLOCK = 수신 데이터 없음 (논블로킹 정상)
        if (iErr != WSAEWOULDBLOCK)
        {
            printf("[NET] recv error: %d\n", iErr);
            m_bConnected = false;
        }
        return;
    }

    const int kHeaderBytes = static_cast<int>(sizeof(PKT_HEADER));
    while (m_iRecvLen >= kHeaderBytes)
    {
        const PKT_HEADER* pHdr = reinterpret_cast<const PKT_HEADER*>(m_recvBuf);
        const int pktSize = static_cast<int>(pHdr->wSize);
        if (m_iRecvLen < pktSize)
            break;

        ProcessPacket(pHdr);

        const int consumed = pktSize;
        m_iRecvLen -= consumed;
        if (m_iRecvLen > 0)
            memmove(m_recvBuf, m_recvBuf + consumed, static_cast<size_t>(m_iRecvLen));
    }
}

// =====================================================================
//  ProcessPacket  —  패킷 타입별 분기
// =====================================================================
void CNetworkMgr::ProcessPacket(const PKT_HEADER* pHdr)
{
    switch (pHdr->wType)
    {
    case S2C_LOGIN_ACK:
        On_LoginAck(reinterpret_cast<const PKT_S2C_LoginAck*>(pHdr));
        break;
    case S2C_SPAWN:
        On_Spawn(reinterpret_cast<const PKT_S2C_Spawn*>(pHdr));
        break;
    case S2C_DESPAWN:
        On_Despawn(reinterpret_cast<const PKT_S2C_Despawn*>(pHdr));
        break;
    case S2C_STATE_SNAPSHOT:
        On_Snapshot(reinterpret_cast<const PKT_S2C_StateSnapshot*>(pHdr));
        break;
    default:
        printf("[NET] Unknown packet type: %d\n", pHdr->wType);
        break;
    }
}

// =====================================================================
//  On_LoginAck
// =====================================================================
void CNetworkMgr::On_LoginAck(const PKT_S2C_LoginAck* pPkt)
{
    if (pPkt->bSuccess)
    {
        m_iMyPlayerId = pPkt->iPlayerId;
        printf("[NET] LoginAck OK — playerId=%d stageId=%d\n",
            pPkt->iPlayerId, pPkt->iStageId);
    }
    else
    {
        printf("[NET] LoginAck FAILED — %s\n", pPkt->szMessage);
        m_bConnected = false;
    }
}

// =====================================================================
//  On_Spawn
//  iMyPlayerId = 수신자 자신의 ID
//  players[]   = 스폰할 플레이어 목록
//    - 자신(iMyPlayerId)은 위치만 반영, CRemotePlayer 생성 안 함
//    - 타인은 CRemotePlayer 생성 (이미 있으면 위치 갱신)
// =====================================================================
void CNetworkMgr::On_Spawn(const PKT_S2C_Spawn* pPkt)
{
    printf("[NET] On_Spawn — myId=%d, count=%d\n",
        pPkt->iMyPlayerId, pPkt->iPlayerCount);

    for (int i = 0; i < pPkt->iPlayerCount; ++i)
    {
        const PlayerSpawnInfo& info = pPkt->players[i];

        // 자신이면 스킵 (로컬 플레이어는 CPlayer가 담당)
        if (info.iPlayerId == m_iMyPlayerId)
        {
            printf("[NET]   (me) playerId=%d pos=(%.1f,%.1f,%.1f)\n",
                info.iPlayerId, info.fX, info.fY, info.fZ);
            continue;
        }

        // 이미 존재하면 위치만 갱신 (iSequence=-1: sequence 체크 없이 강제 갱신)
        auto it = m_mapRemote.find(info.iPlayerId);
        if (it != m_mapRemote.end())
        {
            it->second->SetTargetState(info.fX, info.fY, info.fZ, info.fRotY, 0, -1);
            printf("[NET]   (update) playerId=%d pos=(%.1f,%.1f,%.1f)\n",
                info.iPlayerId, info.fX, info.fY, info.fZ);
            continue;
        }

        // 신규 원격 플레이어 생성
        CRemotePlayer* pRemote = CRemotePlayer::Create(m_pGraphicDev);
        if (!pRemote)
        {
            printf("[NET] WARN: CRemotePlayer::Create failed for id=%d\n", info.iPlayerId);
            continue;
        }

        pRemote->InitSpawn(info.iPlayerId, info.fX, info.fY, info.fZ,
            info.fRotY, info.szNickname);

        m_mapRemote[info.iPlayerId] = pRemote;

        printf("[NET]   (new) playerId=%d nick='%s' pos=(%.1f,%.1f,%.1f)\n",
            info.iPlayerId, info.szNickname, info.fX, info.fY, info.fZ);
    }
}

// =====================================================================
//  On_Despawn
// =====================================================================
void CNetworkMgr::On_Despawn(const PKT_S2C_Despawn* pPkt)
{
    printf("[NET] On_Despawn — playerId=%d\n", pPkt->iPlayerId);

    auto it = m_mapRemote.find(pPkt->iPlayerId);
    if (it == m_mapRemote.end())
        return;

    if (it->second)
        it->second->Release();

    m_mapRemote.erase(it);
    printf("[NET] Remote player %d removed\n", pPkt->iPlayerId);
}

// =====================================================================
//  On_Snapshot  —  Day 3 구현 예정
// =====================================================================
void CNetworkMgr::On_Snapshot(const PKT_S2C_StateSnapshot* pPkt)
{
    for (int i = 0; i < pPkt->iPlayerCount; ++i)
    {
        const PlayerState& st = pPkt->players[i];

        // 자신은 스킵 (Day 3 에서 서버 보정 추가 예정)
        if (st.iPlayerId == m_iMyPlayerId)
            continue;

        auto it = m_mapRemote.find(st.iPlayerId);
        if (it != m_mapRemote.end())
        {
            // iLastSequence 전달 → 역전된 오래된 스냅샷 자동 무시
            it->second->SetTargetState(st.fX, st.fY, st.fZ, st.fRotY,
                st.iState, st.iLastSequence, st.bOnDragon);
        }
    }
}

// =====================================================================
//  Send  —  내부 전송 헬퍼
// =====================================================================
bool CNetworkMgr::Send(const void* pData, int iSize)
{
    if (m_hSocket == INVALID_SOCKET)
        return false;

    int iSent = 0;
    const char* pBuf = reinterpret_cast<const char*>(pData);
    while (iSent < iSize)
    {
        int iRet = send(m_hSocket, pBuf + iSent, iSize - iSent, 0);
        if (iRet == SOCKET_ERROR)
        {
            printf("[NET] send error: %d\n", WSAGetLastError());
            return false;
        }
        iSent += iRet;
    }
    return true;
}
