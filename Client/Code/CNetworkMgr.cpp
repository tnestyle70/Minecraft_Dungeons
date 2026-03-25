#include "pch.h"
#include "CNetworkMgr.h"
#include "CRemotePlayer.h"
#include "CPlayerArrow.h"   // Day 9: 네트워크 화살 시각 객체
#include "CNetworkPlayer.h" // Day 9: 로컬 플레이어 HP 갱신
#include <cstring>
#include <cstdio>
#include <algorithm>

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

    // Day 9: 네트워크 화살 전체 해제
    for (auto* p : m_vecNetArrows)
        if (p) p->Release();
    m_vecNetArrows.clear();
    m_pLocalPlayer = nullptr;

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

    // Day 9: 네트워크 화살 업데이트 + Dead 정리
    for (auto* p : m_vecNetArrows)
        if (p) p->Update_GameObject(fTimeDelta);

    m_vecNetArrows.erase(
        std::remove_if(m_vecNetArrows.begin(), m_vecNetArrows.end(),
            [](CPlayerArrow* p) {
                if (p && p->Is_Dead() && !p->Is_Exploding()) { p->Release(); return true; }
                return false;
            }),
        m_vecNetArrows.end());
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

    // Day 9: 원격 발사 화살 시각 렌더
    for (auto* pArrow : m_vecNetArrows)
    {
        if (pArrow)
            pArrow->Render_GameObject();
    }
}

// =====================================================================
//  SendInput  —  Day 3 에서 CPlayer 입력 연동 시 호출
// =====================================================================
void CNetworkMgr::SendInput(float fDirX, float fDirZ, float fRotY, bool bMoving,
    float fX, float fY, float fZ, bool bOnDragon, int iDragonIdx,
    float fDragonX, float fDragonY, float fDragonZ)
{
    if (!m_bConnected || m_iMyPlayerId == -1)
        return;

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
    pkt.fDragonX = fDragonX;
    pkt.fDragonY = fDragonY;
    pkt.fDragonZ = fDragonZ;

    // #region agent log
    if (bOnDragon)
    {
        FILE* fp = nullptr;
        if (_wfopen_s(&fp, L"debug-9b3cff.log", L"a") == 0 && fp)
        {
            fprintf(fp, "{\"sessionId\":\"9b3cff\",\"runId\":\"pre-fix\",\"hypothesisId\":\"H2\",\"location\":\"Client/Code/CNetworkMgr.cpp:173\",\"message\":\"send_input_on_dragon\",\"data\":{\"seq\":%d,\"dragonIdx\":%d,\"x\":%.3f,\"y\":%.3f,\"z\":%.3f},\"timestamp\":%llu}\n",
                pkt.iSequence, pkt.iDragonIdx, pkt.fX, pkt.fY, pkt.fZ, (unsigned long long)GetTickCount64());
            fclose(fp);
        }
    }
    // #endregion

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
    case S2C_ATTACK:
        On_Attack(reinterpret_cast<const PKT_S2C_Attack*>(pHdr));
        break;
    case S2C_DAMAGE:
        On_Damage(reinterpret_cast<const PKT_S2C_Damage*>(pHdr));
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
                st.iState, st.iLastSequence, st.bOnDragon,
                st.iDragonIdx, st.fDragonX, st.fDragonY, st.fDragonZ);
        }
    }
}

// =====================================================================
//  Send  —  내부 전송 헬퍼
// =====================================================================
// =====================================================================
//  SendAttack  —  Day 9: 화살 발사 이벤트 전송
// =====================================================================
void CNetworkMgr::SendAttack(float fPosX, float fPosY, float fPosZ,
    float fDirX, float fDirY, float fDirZ, float fCharge, bool bFirework)
{
    if (!m_bConnected || m_iMyPlayerId == -1)
        return;

    PKT_C2S_Attack pkt = {};
    FillHeader(pkt, C2S_ATTACK);
    pkt.fPosX = fPosX;  pkt.fPosY = fPosY;  pkt.fPosZ = fPosZ;
    pkt.fDirX = fDirX;  pkt.fDirY = fDirY;  pkt.fDirZ = fDirZ;
    pkt.fCharge = fCharge;
    pkt.bFirework = bFirework;

    // #region agent log
    {
        FILE* fp = nullptr;
        if (_wfopen_s(&fp, L"debug-9b3cff.log", L"a") == 0 && fp)
        {
            fprintf(fp, "{\"sessionId\":\"9b3cff\",\"runId\":\"pre-fix\",\"hypothesisId\":\"H1\",\"location\":\"Client/Code/CNetworkMgr.cpp:430\",\"message\":\"send_attack\",\"data\":{\"pos\":[%.3f,%.3f,%.3f],\"dir\":[%.3f,%.3f,%.3f],\"charge\":%.3f,\"firework\":%d},\"timestamp\":%llu}\n",
                pkt.fPosX, pkt.fPosY, pkt.fPosZ, pkt.fDirX, pkt.fDirY, pkt.fDirZ, pkt.fCharge, pkt.bFirework ? 1 : 0, (unsigned long long)GetTickCount64());
            fclose(fp);
        }
    }
    // #endregion

    Send(&pkt, sizeof(pkt));
}

// =====================================================================
//  SendDamage  —  Day 9: PVP 피해 통보 전송
// =====================================================================
void CNetworkMgr::SendDamage(int iTargetPlayerId, float fDamage)
{
    if (!m_bConnected || m_iMyPlayerId == -1)
        return;

    PKT_C2S_Damage pkt = {};
    FillHeader(pkt, C2S_DAMAGE);
    pkt.iTargetPlayerId = iTargetPlayerId;
    pkt.fDamage = fDamage;

    Send(&pkt, sizeof(pkt));
}

// =====================================================================
//  On_Attack  —  Day 9: 원격 화살 시각 객체 생성
// =====================================================================
void CNetworkMgr::On_Attack(const PKT_S2C_Attack* pPkt)
{
    // 자신의 공격은 이미 로컬에서 생성됨 → 스킵
    if (pPkt->iPlayerId == m_iMyPlayerId) return;

    _vec3 vPos = { pPkt->fPosX, pPkt->fPosY, pPkt->fPosZ };
    _vec3 vDir = { pPkt->fDirX, pPkt->fDirY, pPkt->fDirZ };

    CPlayerArrow* pArrow = CPlayerArrow::Create(m_pGraphicDev, vPos, vDir, pPkt->fCharge);
    if (pArrow)
    {
        pArrow->Set_Firework(pPkt->bFirework);
        m_vecNetArrows.push_back(pArrow);

        // #region agent log
        {
            FILE* fp = nullptr;
            if (_wfopen_s(&fp, L"debug-9b3cff.log", L"a") == 0 && fp)
            {
                fprintf(fp, "{\"sessionId\":\"9b3cff\",\"runId\":\"pre-fix\",\"hypothesisId\":\"H3\",\"location\":\"Client/Code/CNetworkMgr.cpp:468\",\"message\":\"recv_attack_spawn_arrow\",\"data\":{\"attacker\":%d,\"charge\":%.3f,\"firework\":%d,\"netArrowCount\":%d},\"timestamp\":%llu}\n",
                    pPkt->iPlayerId, pPkt->fCharge, pPkt->bFirework ? 1 : 0, (int)m_vecNetArrows.size(), (unsigned long long)GetTickCount64());
                fclose(fp);
            }
        }
        // #endregion
    }
}

// =====================================================================
//  On_Damage  —  Day 9: 내가 피격됐을 때 HP 갱신
// =====================================================================
void CNetworkMgr::On_Damage(const PKT_S2C_Damage* pPkt)
{
    if (pPkt->iTargetPlayerId == m_iMyPlayerId)
    {
        // ── 내가 피격됨 → HP 갱신 + 로컬 피격 이펙트 ──────────────────
        if (!m_pLocalPlayer) return;

        m_pLocalPlayer->Hit(pPkt->fDamage);
        float fNewHp = m_pLocalPlayer->Get_Hp();

        printf("[NET] PVP damage received: %.1f HP  (attacker=%d, remaining=%.1f)\n",
            pPkt->fDamage, pPkt->iAttackerPlayerId, fNewHp);

        // #region agent log
        {
            FILE* fp = nullptr;
            if (_wfopen_s(&fp, L"debug-9b3cff.log", L"a") == 0 && fp)
            {
                fprintf(fp, "{\"sessionId\":\"9b3cff\",\"runId\":\"pre-fix\",\"hypothesisId\":\"H5\",\"location\":\"Client/Code/CNetworkMgr.cpp:486\",\"message\":\"apply_damage\",\"data\":{\"attacker\":%d,\"damage\":%.3f,\"remainingHp\":%.3f},\"timestamp\":%llu}\n",
                    pPkt->iAttackerPlayerId, pPkt->fDamage, fNewHp, (unsigned long long)GetTickCount64());
                fclose(fp);
            }
        }
        // #endregion
    }
    else
    {
        // ── Day 10: 원격 플레이어 피격 → 빨간 플래시 이펙트 ───────────
        auto it = m_mapRemote.find(pPkt->iTargetPlayerId);
        if (it != m_mapRemote.end() && it->second)
            it->second->Set_Hit();
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
