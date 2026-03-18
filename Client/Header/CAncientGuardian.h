#pragma once
#include "CDLCBoss.h" 
#include "CAGBody.h"
#include "CBeam.h"

enum class EPufferFishState
{
    IDLE,   // 대기 - 플레이어 직선 추적
    ORBIT,  // 플레이어 주변 선회 - 가시 연속 발사
    CHARGE, // 플레이어를 향해 돌진
};

class CAncientGuardian : public CDLCBoss
{
private:
    explicit CAncientGuardian(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CAncientGuardian();
public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float& fTimeDelta);
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta);
protected:
    virtual HRESULT Add_Component()                     override; // 텍스처, 트랜스폼, 바디 등록
    virtual void    Update_AI(const _float& fTimeDelta) override; // 상태머신 - IDLE/ORBIT/CHARGE 전환
    virtual void    Render_GameObject()                 override; // 바디 렌더링
private:
    void Update_Orbit(const _float& fTimeDelta);    // 선회 이동 - 플레이어 주변 원형 궤도
    void Update_Charge(const _float& fTimeDelta);   // 돌진 이동 - 가속/감속 후 목표 도달 시 ORBIT 복귀
    void Fire_Beam();                               // 빔 1발 생성 → m_vecBeams에 추가
    void Update_Beams(const _float& fTimeDelta);    // 빔 갱신 + 수명 끝난 것 삭제
    void Drop_Biomine();                            // 바이오마인 1개 생성 → m_vecBiomines에 추가
    void Update_Biomines(const _float& fTimeDelta); // 바이오마인 갱신 + 폭발 끝난 것 삭제
private:
    CAGBody* m_pBodyCom = nullptr; // CMonster 의 m_pBodyCom 과 동일한 역할
    Engine::CTexture* m_pTextureCom = nullptr; // AG 전용 텍스처

    // 선회
    float m_fOrbitRadius = 6.f;  // 플레이어 기준 선회 반지름
    float m_fOrbitAngle = 0.f;  // 현재 선회 각도 (라디안, 매 프레임 증가)
    float m_fOrbitSpeed = 1.f;  // 선회 속도 (라디안/초)
    float m_fOrbitHeight = 4.f;  // 플레이어 Y + 이 값 = 보스 높이

    // 돌진
    _vec3 m_vChargeTarget = {};  // 돌진 목표 위치 (돌진 시작 시점 플레이어 위치 저장)
    float m_fCurSpeed = 0.f; // 현재 속도 (가속/감속)
    float m_fAccel = 10.f; // 가속도 - 거리 5 이상일 때 적용
    float m_fBrake = 8.f;  // 감속도 - 거리 5 이하일 때 적용

    // 쿨타임
    float m_fChargeCooldown = 5.f; // 현재 쿨타임 - 매 프레임 감소, 0이 되면 CHARGE 진입
    float m_fChargeCoolMax = 5.f; // 5초마다 돌진

    // 현재 상태
    EPufferFishState m_eState = EPufferFishState::IDLE; // 현재 AI 상태
    float            m_fHoverTime = 0.f;                  // 헤엄 모션용 누적 시간

private:
    vector<CBeam*> m_vecBeams;       // 발사된 가시들 - Update/Render/삭제 관리
    float m_fFireTimer = 0.f;     // 연속 발사 타이머 - fFireInterval마다 Fire_Beam 호출
    float m_fFireInterval = 0.1f;    // 발사 간격 - 0.1초마다 1발
    int   m_iFireCount = 0;       // 현재 발사 횟수 - iFireMax 도달 시 리셋
    int   m_iFireMax = 10;      // 최대 연속 발사 횟수 - 10발 후 카운트 리셋
    bool  m_bFiring = false;   // 발사 중 여부 - ORBIT이면 true, CHARGE/IDLE이면 false

public:
    static CAncientGuardian* Create(LPDIRECT3DDEVICE9 pGraphicDev, _vec3 vPos);
protected:
    virtual void Free() override;
};