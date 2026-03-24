#include "CSession.h"
#include "ServerLog.h"
#include <cstring>

CSession::CSession(int iSessionId, SOCKET hSocket)
    : m_iSessionId(iSessionId), m_hSocket(hSocket), m_bConnected(true), m_dwLastRecvTime(GetTickCount()) {}

CSession::~CSession()
{
    Disconnect();
}

bool CSession::RecvData()
{
    if (!m_bConnected)
        return false;

    int iBufRemain = RECV_BUF_SIZE - m_iRecvLen;
    if (iBufRemain <= 0)
    {
        LOG_WARN("Session %d recv buffer full — disconnecting", m_iSessionId);
        Disconnect();
        return false;
    }

    int iRecv = recv(m_hSocket, m_recvBuf + m_iRecvLen, iBufRemain, 0);

    if (iRecv == 0)
    {
        LOG_NET("Session %d disconnected (graceful)", m_iSessionId);
        Disconnect();
        return false;
    }
    if (iRecv == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return true;

        LOG_WARN("Session %d recv error: %d", m_iSessionId, err);
        Disconnect();
        return false;
    }

    m_iRecvLen += iRecv;
    m_dwLastRecvTime = GetTickCount();
    return true;
}

const PKT_HEADER* CSession::PeekPacket() const
{
    if (m_iRecvLen < static_cast<int>(sizeof(PKT_HEADER)))
        return nullptr;

    const PKT_HEADER* pHdr = reinterpret_cast<const PKT_HEADER*>(m_recvBuf);
    if (m_iRecvLen < static_cast<int>(pHdr->wSize))
        return nullptr;

    return pHdr;
}

void CSession::PopPacket()
{
    if (m_iRecvLen < static_cast<int>(sizeof(PKT_HEADER)))
        return;

    const PKT_HEADER* pHdr = reinterpret_cast<const PKT_HEADER*>(m_recvBuf);
    int iPktSize = static_cast<int>(pHdr->wSize);

    if (m_iRecvLen < iPktSize)
        return;

    memmove(m_recvBuf, m_recvBuf + iPktSize, m_iRecvLen - iPktSize);
    m_iRecvLen -= iPktSize;
}

bool CSession::Send(const void* pData, int iSize)
{
    if (!m_bConnected || m_hSocket == INVALID_SOCKET)
        return false;

    int iSent = 0;
    const char* pBuf = static_cast<const char*>(pData);

    while (iSent < iSize)
    {
        int iRet = send(m_hSocket, pBuf + iSent, iSize - iSent, 0);
        if (iRet == SOCKET_ERROR)
        {
            LOG_WARN("Session %d send error: %d", m_iSessionId, WSAGetLastError());
            Disconnect();
            return false;
        }
        iSent += iRet;
    }
    return true;
}

void CSession::SetNickname(const char* sz)
{
    strncpy_s(m_szNickname, sz, _TRUNCATE);
}

void CSession::Disconnect()
{
    if (!m_bConnected)
        return;

    m_bConnected = false;
    if (m_hSocket != INVALID_SOCKET)
    {
        shutdown(m_hSocket, SD_BOTH);
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }
}
