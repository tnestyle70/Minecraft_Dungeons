#pragma once
#include "CGameObject.h"
#include "CProtoMgr.h"
#include "CCollider.h"
// CArrow와 동일한 구조 - 보스 전용 빔 투사체
// CAncientGuardian, CVengefulHeartOfEnder 공용으로 사용
class CBeam : public CGameObject
{
private:
    explicit CBeam(LPDIRECT3DDEVICE9 pGraphicDev);
    explicit CBeam(const CGameObject& rhs);
    virtual ~CBeam();
public:
    virtual HRESULT Ready_GameObject();
    virtual _int    Update_GameObject(const _float fTimeDelta); // CDLCBoss와 동일하게 레퍼런스 없음
    virtual void    LateUpdate_GameObject(const _float& fTimeDelta); // 빌보드 행렬 계산
    virtual void    Render_GameObject();                             // 알파블렌딩 + Z쓰기 OFF 후 렌더
private:
    HRESULT         Add_Component();
private:
    Engine::CTransform* m_pTransformCom = nullptr; // 위치, 회전, 크기
    Engine::CCollider* m_pColliderCom = nullptr; // 플레이어 피격 판정용 AABB
    Engine::CTexture* m_pTextureCom = nullptr; // AG 텍스처 재사용
    CCubeBodyTex* m_pBufferCom = nullptr; // 수정 - 다시 CCubeBodyTex로 (AG_Spike 큐브 사용)
    _vec3   m_vDir = { 0.f, 0.f, 1.f }; // 날아갈 방향
    float   m_fSpeed = 15.f;               // 이동 속도
    float   m_fLifeTime = 0.f;                // 현재 생존 시간
    float   m_fMaxLifeTime = 4.f;                // 최대 생존 시간
    bool    m_bDead = false;              // 삭제 플래그
public:
    void    Set_Direction(const _vec3& vDir) { m_vDir = vDir; } // 발사 방향 설정 - Create 직후 호출
    bool    Is_Dead() const { return m_bDead; }                 // Update_Beams에서 삭제 여부 판단
    int     m_iDamage = 15;                                     // 플레이어 팀원이 Get해서 Hit에 사용
    Engine::CCollider* Get_Collider() { return m_pColliderCom; } // 플레이어 충돌 체크용
public:
    static CBeam* Create(LPDIRECT3DDEVICE9 pGraphicDev,
        const _vec3& vStartPos, const _vec3& vDir);
private:
    virtual void Free();
};