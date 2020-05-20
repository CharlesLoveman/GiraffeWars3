#ifndef _MOVESET_H_
#define _MOVESET_H_

#include "Collider.h"
#include <vector>
#include <array>

constexpr int NUM_MOVES = 24;
constexpr int NUM_POINTS = 38;

struct MoveSet {
public:
	std::array<std::vector<std::vector<HitCollider>>, NUM_MOVES> Hitboxes;
	std::array<std::vector<std::array<HurtCollider, 6>>, NUM_MOVES> Hurtboxes;
	std::array<std::vector<std::array<Vec2, NUM_POINTS>>, NUM_MOVES> SkelPoints;
	std::array<int, 10> LandingLag;
	MoveSet();
private:
	static int index;
};

#endif // !_MOVESET_H_
