#pragma once
#include "CDLCBoss.h"
#include "CBeam.h"

// 지네 보스 행동 상태
enum class ECentipedeState
{
    IDLE,       // 대기
    RUSH,       // 플레이어를 향한 돌진
    VOID_SPEW,  // 회전하며 빔 난사
};

class CVengefulHeartOfEnder : public CDLCBoss
{
private:
    explicit CVengefulHeartOfEnder(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CVengefulHeartOfEnder();

public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float fTimeDelta); // CDLCBoss와 동일하게 레퍼런스 없음
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);

protected:
    virtual HRESULT Add_Component()                     override;
    virtual void    Update_AI(const _float& fTimeDelta) override;
    virtual void    Render_GameObject()                 override;

private:
    void Update_Segments();                          // 앞 마디 위치를 뒷 마디가 따라가는 체인
    void Update_Rush(const _float& fTimeDelta);      // 돌진 로직 - 가속 → 직진 → 감속
    void Update_VoidSpew(const _float& fTimeDelta);  // 회전하며 빔 발사 로직
    void Fire_Beam();                                // 빔 생성 및 발사
    void Update_Beams(const _float& fTimeDelta);     // 빔 갱신 + 수명 끝난 것 삭제

private:
    // 분절 마디
    vector<_vec3>   m_vecSegmentPos;
    int             m_iSegmentCount = 8;

    // 돌진
    _vec3   m_vRushTarget = {};
    float   m_fCurSpeed = 0.f;
    float   m_fAccel = 12.f;
    float   m_fBrake = 9.f;

    // 빔 난사
    float   m_fSpewAngle = 0.f;
    float   m_fSpewTimer = 0.f;
    float   m_fRotThreshold = D3DXToRadian(30.f); // 30도마다 빔 1발

    // 현재 상태
    ECentipedeState m_eState = ECentipedeState::IDLE;

    // 빔 목록
    vector<CBeam*>  m_vecBeams;

public:
    static CVengefulHeartOfEnder* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);

protected:
    virtual void Free() override;
};