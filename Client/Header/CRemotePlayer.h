#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CPlayerBody.h"
#include "CTexture.h"

// Remote player: server-driven position, lerp in Update, CPlayerBody rendering.
class CRemotePlayer : public Engine::CGameObject
{
private:
    explicit CRemotePlayer(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CRemotePlayer(const Engine::CGameObject& rhs) = delete;
    virtual ~CRemotePlayer();

public:
    virtual HRESULT Ready_GameObject() override;
    virtual _int    Update_GameObject(const _float& fTimeDelta) override;
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta) override;
    virtual void    Render_GameObject() override;

    void InitSpawn(int iPlayerId, float fX, float fY, float fZ,
        float fRotY, const char* szNickname);

    // iSequence: 서버 PlayerState.iLastSequence (-1 이면 강제 갱신)
    void SetTargetState(float fX, float fY, float fZ,
        float fRotY, int iState, int iSequence = -1, bool bOnDragon = false);

    int         GetPlayerId()      const { return m_iPlayerId; }
    int         GetLastSequence()  const { return m_iLastSequence; }
    const char* GetNickname()  const { return m_szNickname; }
    float       GetX()         const { return m_fCurX; }
    float       GetY()         const { return m_fCurY; }
    float       GetZ()         const { return m_fCurZ; }

    static CRemotePlayer* Create(LPDIRECT3DDEVICE9 pGraphicDev);

private:
    virtual void Free() override;

    enum BODYPART_REMOTE
    {
        PART_HEAD = 0,
        PART_BODY,
        PART_LARM,
        PART_RARM,
        PART_LLEG,
        PART_RLEG,
        PART_END
    };

    void Render_Part(int ePart, float fAngleX, float fAngleY, float fAngleZ,
        const _matrix& matRootWorld);
    void Render_NameTag();

    static constexpr float LERP_SPEED = 10.f;

    // ── 보간 상태 ──────────────────────────────────────────────────────────
    float   m_fTargetX = 0.f;
    float   m_fTargetY = 0.f;
    float   m_fTargetZ = 0.f;
    float   m_fTargetRotY = 0.f;
    int     m_iTargetState = 0;

    float   m_fCurX = 0.f;
    float   m_fCurY = 0.f;
    float   m_fCurZ = 0.f;
    float   m_fCurRotY = 0.f;

    int     m_iPlayerId = -1;
    int     m_iLastSequence = -1;   // sequence 역전 무시용
    char    m_szNickname[32] = {};

    float   m_fWalkTime = 0.f;
    bool    m_bMoving = false;
    bool    m_bOnDragon = false;    // 탑승 여부 (렌더링 확장용)

    // ── 렌더링 컴포넌트 ────────────────────────────────────────────────────
    CPlayerBody* m_pBufferCom[PART_END] = {};
    Engine::CTexture* m_pTextureCom = nullptr;

    // ── 방어구(로브) 렌더링 ────────────────────────────────────────────────
    CPlayerBody* m_pArmorBufferCom[PART_END] = {};
    Engine::CTexture* m_pArmorTextureCom = nullptr;
    _matrix            m_matPartWorld[PART_END] = {};   // 방어구 2패스용 행렬 캐시

    _vec3   m_vPartScale[PART_END] = {};
    _vec3   m_vPartOffset[PART_END] = {};
};
