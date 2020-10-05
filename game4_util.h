#pragma once

#include <glm/glm.hpp>

#include <vector>

//taken from base0 NotPong code
struct Vertex {
	Vertex(glm::vec3 const& Position_, glm::u8vec4 const& Color_, glm::vec2 const& TexCoord_) :
		Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
	glm::vec3 Position;
	glm::u8vec4 Color;
	glm::vec2 TexCoord;
};
static_assert(sizeof(Vertex) == 4 * 3 + 1 * 4 + 4 * 2, "Vertex should be packed");

typedef struct {
	std::vector<Vertex> vertices;
	unsigned int textureId;
} VMesh;