//Author: Hyosang Jung
#pragma once

#include<glm/glm.hpp>
struct Point
{
	glm::vec2 pos;
	glm::vec2 tangent;
};


class Projects
{
public:
	Projects() {};
	virtual ~Projects() {};

	virtual void init() {}
	virtual void Update(float ) {}
	virtual void Draw() {}
	virtual void OnImGuiRender() {};
	virtual void UnLoad() {}
private:

};