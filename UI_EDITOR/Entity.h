// Entity.h
#pragma once
#include <string>
#include <cstdint>

struct Entity {
	std::string name;

	float x = 0.F;
	float y = 0.F;

	float width = 0.F;
	float height = 0.F;

	uint32_t color = 0xFFFFFFFFU; // RGBA

	void* texture = nullptr;
	float uv_u = 1.0f;
	float uv_v = 1.0f;
};