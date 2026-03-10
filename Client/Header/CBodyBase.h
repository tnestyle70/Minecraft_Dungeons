#pragma once
#include "CBase.h"
#include "CCubeBodyTex.h"
#include "CProtoMgr.h"

//파트 인덱스는 개별 클래스에서 정의하기
using EBodyPart = int;

//파트 런타임 데이터
struct BodyPartData
{
	CCubeBodyTex* pBuffer = nullptr;
    //몸통 기준 로컬 위치
	_vec3 vAnchorPos = { 0.f, 0.f, 0.f };
    //회전 중심으로 이동할 오프셋
	_vec3 vPivotOffset = { 0.f, 0.f, 0.f };
};

//파트 등록
struct BodyPart
{
    const _tchar* szProtoName; //Proto에 등록한 이름
    _vec3 vAnchorPos; //부모 기준 파츠 로컬 좌표 위치
    //애니메이션 재생시 이동할 오프셋 
    _vec3 vPivotOffset; 
};

//포즈 데이터 - 파트 데이터 기반
struct BodyPose
{
	vector<_vec3> vecPartRot;
    float fBodySwayY = 0.f;
    
    void Initialize(int iPartCount)
    {
        vecPartRot.assign(iPartCount, { 0.f, 0.f, 0.f });
    }
    void SetRot(int iPartIndex, float fX, float fY, float fZ)
    {
        if (iPartIndex < (int)vecPartRot.size())
        {
            vecPartRot[iPartIndex] = { fX, fY, fZ };
        }
    }
    _vec3 GetRot(int iPartIndex) const
    {
        if (iPartIndex < (int)vecPartRot.size())
            return vecPartRot[iPartIndex];
        return { 0.f, 0.f, 0.f };
    }
};

// 애니메이션 추상 인터페이스
class CBodyAnim
{
public:
    virtual ~CBodyAnim() {}
    virtual void             Update(const _float& fTimeDelta, bool bMoving, bool bAttack) PURE;
    virtual const BodyPose& Get_Pose() PURE;
};

// CBodyBase
class CBodyBase : public CBase
{
protected:
    explicit CBodyBase(LPDIRECT3DDEVICE9 pGraphicDev);
    virtual ~CBodyBase();

public:
    virtual HRESULT         Ready_Body() PURE;
    virtual _int            Update_Body(const _float& fTimeDelta, bool bMoving, bool bAttack);
    virtual void            Render_Body(const _matrix* pParentWorld, Engine::CTexture* pTexture);

    CBodyAnim* Get_Anim() { return m_pAnim; }
    int                     Get_PartCount() { return (int)m_vecParts.size(); }

protected:
    HRESULT                 Register_Part(EBodyPart iPartIndex, const BodyPart& part);
    void                    Render_Part(EBodyPart iPartIndex, const _matrix& matParent,
        float fRotX, float fRotY, float fRotZ);
    _matrix                 Calc_PartMatrix(EBodyPart iPartIndex, const _matrix& matParent,
        float fRotX, float fRotY, float fRotZ);

protected:
    LPDIRECT3DDEVICE9       m_pGraphicDev;
    vector<BodyPartData>    m_vecParts;
    CBodyAnim* m_pAnim;

protected:
    virtual void            Free() override;
};