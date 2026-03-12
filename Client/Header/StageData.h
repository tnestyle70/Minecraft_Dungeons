#pragma once

struct MonsterData
{
	int x, y, z;
	int iMonsterType;
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