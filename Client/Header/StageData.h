#pragma once

struct MonsterData
{
	int x, y, z;
	int iMonsterType;
	int iTriggerID = -1; //-1이면 바로 스폰, 0이상이면 트리거 대기 후 트리거 박스 ID에 따라서 다르게 스폰
	bool operator<(const MonsterData& other) const
	{
		if (x != other.x) return x < other.x;
		if (y != other.y) return y < other.y;
		return z < other.z;
	}
};

struct TriggerBoxData
{
	int x, y, z;
	int width, height, depth;
	int iTriggerID; //박스의 고유 아이디 무조건 아이디 하나씩 증가
	int iTriggerBoxType; //박스의 고유 타입
	bool operator<(const TriggerBoxData& other) const
	{
		if (x != other.x) return x < other.x;
		if (y != other.y) return y < other.y;
		return z < other.z;
	}
};

struct IronBarData
{
	int x, y, z;
	int iTriggerID;

	bool operator<(const IronBarData& other) const
	{
		if (x != other.x) return x < other.x;
		if (y != other.y) return y < other.y;
		return z < other.z;
	}
};