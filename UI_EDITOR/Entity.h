// Entity.h
#pragma once
#include <string>
#include <cstdint>

struct Entity {
	int id = -1;
	std::string name;

	float x = 0.F;
	float y = 0.F;

	float width = 0.F;
	float height = 0.F;

	uint32_t color = 0xFFFFFFFFU; // RGBA

	void* texture = nullptr;
	float uv_u = 1.0f;
	float uv_v = 1.0f;

	bool visible = true;
	bool locked = false;

	int parentId = -1;

	// Pivot & Anchors (Stage 3)
	float pivotX = 0.5f;
	float pivotY = 0.5f;
	float anchorMinX = 0.0f;
	float anchorMinY = 0.0f;
	float anchorMaxX = 0.0f;
	float anchorMaxY = 0.0f;
};