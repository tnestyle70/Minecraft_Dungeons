#pragma once

// winsock2.h는 windows.h보다 먼저 — framework.h에 WIN32_LEAN_AND_MEAN이 있어서 충돌 없음
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <map>
#include <vector>
#include "../../Shared/PacketDef.h"

class CRemotePlayer;
class CPlayerArrow;   // Day 9: 네트워크 화살 시각 객체
class CNetworkPlayer; // Day 9: 로컬 플레이어 HP 갱신용

// =====================================================================
//  CNetworkMgr  —  네트워크 싱글턴 매니저
//  - 서버 Connect / Disconnect
//  - 매 프레임 Update() 에서 패킷 수신 + 파싱
//  - CRemotePlayer 생성 / 제거 / 갱신
//  - CNetworkStage 에서만 Update / Render 호출
// =====================================================================
class CNetworkMgr
{
private:
    CNetworkMgr() = default;
    ~CNetworkMgr();
    CNetworkMgr(const CNetworkMgr&) = delete;
    CNetworkMgr& operator=(const CNetworkMgr&) = delete;

    static CNetworkMgr* m_pInstance;

public:
    static CNetworkMgr* GetInstance();
    static void         DestroyInstance();

    // ── 초기화 / 해제 ──────────────────────────────────────────────────
    // pGraphicDev : CRemotePlayer 생성 시 필요
    bool Connect(LPDIRECT3DDEVICE9 pGraphicDev,
        const char* szIP, int iPort,
        const char* szNickname);
    void Disconnect();
    bool IsConnected() const { return m_bConnected; }

    // ── 프레임 루프 ────────────────────────────────────────────────────
    void Update(float fTimeDelta);  // 패킷 수신 + RemotePlayer Update
    void LateUpdate(float fTimeDelta);  // RemotePlayer LateUpdate
    void Render();                  // RemotePlayer Render

    // ── 송신 ──────────────────────────────────────────────────────────
    // Day 3 에서 CPlayer 입력 연동 시 호출
    void SendInput(float fDirX, float fDirZ, float fRotY, bool bMoving,
        float fX, float fY, float fZ,
        bool bOnDragon = false, int iDragonIdx = -1,
        float fDragonX = 0.f, float fDragonY = 0.f, float fDragonZ = 0.f);

    // Day 9: 공격 / 피해 전송
    void SendAttack(float fPosX, float fPosY, float fPosZ,
        float fDirX, float fDirY, float fDirZ,
        float fCharge, bool bFirework);
    void SendDamage(int iTargetPlayerId, float fDamage);

    // ── 상태 조회 ─────────────────────────────────────────────────────
    int  GetMyPlayerId()  const { return m_iMyPlayerId; }
    bool IsLoggedIn()     const { return m_iMyPlayerId != -1; }

    // ImGui 디버그 패널용
    int    GetRemoteCount()   const { return static_cast<int>(m_mapRemote.size()); }
    const std::map<int, CRemotePlayer*>& GetRemotePlayers() const { return m_mapRemote; }

    // Day 8: 드래곤 위치 동기화용 — CNetworkStage에서 dragon 제어에 사용
    const std::map<int, CRemotePlayer*>& GetRemoteMap() const { return m_mapRemote; }

    // Day 9: 로컬 플레이어 등록 (피격 HP 갱신용)
    void SetLocalPlayer(CNetworkPlayer* pPlayer) { m_pLocalPlayer = pPlayer; }

private:
    // ── 수신 / 파싱 ───────────────────────────────────────────────────
    void RecvAndProcess();
    void ProcessPacket(const PKT_HEADER* pHdr);

    void On_LoginAck(const PKT_S2C_LoginAck* pPkt);
    void On_Spawn(const PKT_S2C_Spawn* pPkt);
    void On_Despawn(const PKT_S2C_Despawn* pPkt);
    void On_Snapshot(const PKT_S2C_StateSnapshot* pPkt);
    void On_Attack(const PKT_S2C_Attack* pPkt);   // Day 9
    void On_Damage(const PKT_S2C_Damage* pPkt);   // Day 9

    // ── 소켓 / 버퍼 ───────────────────────────────────────────────────
    bool Send(const void* pData, int iSize);

    SOCKET  m_hSocket = INVALID_SOCKET;
    bool    m_bConnected = false;

    static constexpr int RECV_BUF_SIZE = 8192;
    char    m_recvBuf[RECV_BUF_SIZE] = {};
    int     m_iRecvLen = 0;

    // ── 게임 상태 ─────────────────────────────────────────────────────
    int     m_iMyPlayerId = -1;
    int     m_iSequence = 0;

    LPDIRECT3DDEVICE9             m_pGraphicDev = nullptr;
    std::map<int, CRemotePlayer*> m_mapRemote;  // playerId → CRemotePlayer*

    // Day 9: 네트워크 화살 (원격 플레이어가 발사한 시각 전용 화살)
    std::vector<CPlayerArrow*>    m_vecNetArrows;
    CNetworkPlayer* m_pLocalPlayer = nullptr;  // 피격 HP 갱신용
};

