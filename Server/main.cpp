#include <winsock2.h>
#include <windows.h>
#include <csignal>
#include <cstdio>
#include "CServer.h"
#include "CSessionMgr.h"
#include "ServerLog.h"

static constexpr int SERVER_PORT = 9000;
static CServer* g_pServer = nullptr;

static void OnSignal(int sig)
{
    if (sig == SIGINT && g_pServer)
    {
        LOG_INFO("Shutdown signal received. Stopping server...");
        g_pServer->Stop();
    }
}

int main()
{
    SetConsoleTitleA("Minecraft Dungeons Server");

    LOG_INFO("=== Minecraft Dungeons Server ===");
    LOG_INFO("Protocol version: %d", PROTOCOL_VERSION);
    LOG_INFO("Max players: %d", MAX_PLAYERS);

    CServer server;
    g_pServer = &server;
    signal(SIGINT, OnSignal);

    if (!server.Init(SERVER_PORT))
    {
        LOG_WARN("Server initialization failed. Exiting.");
        return 1;
    }

    server.Run();

    CSessionMgr::DestroyInstance();
    LOG_INFO("Server exited cleanly.");
    return 0;
}
