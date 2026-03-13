#pragma once

struct MonsterData
{
	int x, y, z;
	int iMonsterType;
	int iTriggerID = -1; //-1이면 바로 스폰, 0이상이면 트리거 대기 후 스폰
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
	int iTriggerID; //몇 번째 배치된 트리거박스인지 저장
	int iTriggerBoxType;
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