#include "CSoundMgr.h"

IMPLEMENT_SINGLETON(CSoundMgr)

CSoundMgr::CSoundMgr()
{
}

CSoundMgr::~CSoundMgr()
{
    Free();
}

void CSoundMgr::Initialize()
{
    FMOD::System_Create(&m_pSystem);
    m_pSystem->init(1024, FMOD_INIT_NORMAL, nullptr);
    LoadSoundFile();
}

void CSoundMgr::PlaySound(const TCHAR* pSoundKey, CHANNELID eID, float fVolume)
{
    auto iter = find_if(m_mapSound.begin(), m_mapSound.end(),
        [&](auto& pair)->bool
        {
            return !lstrcmp(pSoundKey, pair.first);
        });

    if (iter == m_mapSound.end())
        return;

    // 해당 채널이 재생 중이면 정지
    bool bPlaying = false;
    if (m_pChannelArr[eID])
        m_pChannelArr[eID]->isPlaying(&bPlaying);

    if (bPlaying)
        m_pChannelArr[eID]->stop();

    // nullptr = 기본 채널 그룹, false = 즉시 재생
    m_pSystem->playSound(iter->second, nullptr, false, &m_pChannelArr[eID]);
    m_pChannelArr[eID]->setVolume(fVolume);
    m_pSystem->update();
}

void CSoundMgr::PlayEffect(const TCHAR* pSoundKey, float fVolume)
{
    auto iter = find_if(m_mapSound.begin(), m_mapSound.end(),
        [&](auto& pair)->bool
        {
            return !lstrcmp(pSoundKey, pair.first);
        });

    if (iter == m_mapSound.end())
        return;

    // 빈 채널 자동 할당 (nullptr로 받으면 채널 관리 안 함)
    FMOD::Channel* pChannel = nullptr;
    m_pSystem->playSound(iter->second, nullptr, false, &pChannel);

    if (pChannel)
        pChannel->setVolume(fVolume);

    m_pSystem->update();
}

void CSoundMgr::PlayBGM(const TCHAR* pSoundKey, float fVolume)
{
    auto iter = find_if(m_mapSound.begin(), m_mapSound.end(),
        [&](auto& pair) { return !lstrcmp(pSoundKey, pair.first); });

    if (iter == m_mapSound.end())
        return;

    // FMOD_CHANNEL_FREE 대신 nullptr
    m_pSystem->playSound(iter->second, nullptr, false, &m_pChannelArr[SOUND_BGM]);
    m_pChannelArr[SOUND_BGM]->setMode(FMOD_LOOP_NORMAL);
    m_pChannelArr[SOUND_BGM]->setVolume(fVolume);
    m_pSystem->update();
}

void CSoundMgr::StopSound(CHANNELID eID)
{
    if (m_pChannelArr[eID])
        m_pChannelArr[eID]->stop();
}

void CSoundMgr::StopAll()
{
    for (int i = 0; i < MAXCHANNEL; ++i)
    {
        if (m_pChannelArr[i])
            m_pChannelArr[i]->stop();
    }
}

void CSoundMgr::SetChannelVolume(CHANNELID eID, float fVolume)
{
    if (m_pChannelArr[eID])
        m_pChannelArr[eID]->setVolume(fVolume);

    m_pSystem->update();
}

void CSoundMgr::LoadSoundFile()
{
    LoadSoundFileRecursive("../Bin/Resource/Sound/", "");
}

void CSoundMgr::LoadSoundFileRecursive(const char* szFolderPath, const char* szRelativePath)
{
    char szFullPath[MAX_PATH];
    sprintf_s(szFullPath, "%s%s*", szFolderPath, szRelativePath);

    OutputDebugStringA(szFullPath);
    OutputDebugStringA("\n");

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(szFullPath, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do
    {
        if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
            continue;

        char szNextRel[MAX_PATH];
        sprintf_s(szNextRel, "%s%s", szRelativePath, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            char szSubRel[MAX_PATH];
            sprintf_s(szSubRel, "%s/", szNextRel);
            LoadSoundFileRecursive(szFolderPath, szSubRel);
        }
        else
        {
            char szFilePath[MAX_PATH];
            sprintf_s(szFilePath, "%s%s", szFolderPath, szNextRel);

            FMOD::Sound* pSound = nullptr;
            m_pSystem->createSound(szFilePath, FMOD_DEFAULT, nullptr, &pSound);

            if (pSound)
            {
                // key를 "BGM/Title.wav" 형태로 저장
                TCHAR* pKey = new TCHAR[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, szNextRel, -1, pKey, MAX_PATH);
                m_mapSound.insert({ pKey, pSound });
            }
        }

    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
}

void CSoundMgr::Free()
{
    // Sound 맵 해제
    for (auto& pair : m_mapSound)
    {
        delete[] pair.first;		// TCHAR* 키 메모리 해제
        pair.second->release();		// FMOD Sound 해제
    }
    m_mapSound.clear(); 

    // 시스템 해제 (close → release 순서 중요)
    if (m_pSystem)
    {
        m_pSystem->close();
        m_pSystem->release();
        m_pSystem = nullptr;
    }
}
