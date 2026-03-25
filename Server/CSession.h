#pragma once

#include <winsock2.h>
#include <windows.h>
#include "../Shared/PacketDef.h"

class CSession
{
public:
    explicit CSession(int iSessionId, SOCKET hSocket);
    ~CSession();

    bool RecvData();
    const PKT_HEADER* PeekPacket() const;
    void PopPacket();
    bool Send(const void* pData, int iSize);

    int GetSessionId() const { return m_iSessionId; }
    SOCKET GetSocket() const { return m_hSocket; }
    bool IsConnected() const { return m_bConnected; }
    bool IsLoggedIn() const { return m_bLoggedIn; }
    DWORD GetLastRecvTime() const { return m_dwLastRecvTime; }

    const char* GetNickname() const { return m_szNickname; }
    int GetPlayerId() const { return m_iPlayerId; }

    float GetX() const { return m_fX; }
    float GetY() const { return m_fY; }
    float GetZ() const { return m_fZ; }
    float GetRotY() const { return m_fRotY; }
    int GetState() const { return m_iState; }
    int GetLastSeq() const { return m_iLastSequence; }

    // Last input direction from C2S_INPUT.
    float GetDirX() const { return m_fDirX; }
    float GetDirZ() const { return m_fDirZ; }

    void SetLoggedIn(bool b) { m_bLoggedIn = b; }
    void SetNickname(const char* sz);
    void SetPlayerId(int id) { m_iPlayerId = id; }
    void SetPosition(float x, float y, float z) { m_fX = x; m_fY = y; m_fZ = z; }
    void SetRotY(float r) { m_fRotY = r; }
    void SetState(int s) { m_iState = s; }
    void SetLastSeq(int seq) { m_iLastSequence = seq; }
    void SetInput(float fDirX, float fDirZ, float fRotY)
    {
        m_fDirX = fDirX; m_fDirZ = fDirZ; m_fRotY = fRotY;
    }
    bool GetOnDragon()  const { return m_bOnDragon; }
    int  GetDragonIdx() const { return m_iDragonIdx; }
    void SetOnDragon(bool b, int idx) { m_bOnDragon = b; m_iDragonIdx = idx; }
    void Disconnect();

private:
    int m_iSessionId = -1;
    SOCKET m_hSocket = INVALID_SOCKET;
    bool m_bConnected = false;
    bool m_bLoggedIn = false;
    DWORD m_dwLastRecvTime = 0;

    static constexpr int RECV_BUF_SIZE = 8192;
    char m_recvBuf[RECV_BUF_SIZE] = {};
    int m_iRecvLen = 0;

    char m_szNickname[32] = {};
    int m_iPlayerId = -1;
    float m_fX = 0.f, m_fY = 0.f, m_fZ = 0.f;
    float m_fRotY = 0.f;
    int m_iState = 0;
    int m_iLastSequence = 0;

    // Cached last input direction.
    float m_fDirX = 0.f;
    float m_fDirZ = 0.f;

    // 드래곤 탑승 상태
    bool  m_bOnDragon = false;
    int   m_iDragonIdx = -1;
};
