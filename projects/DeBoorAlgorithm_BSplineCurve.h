#pragma once

#include"projects.h"
#include <GL/glew.h> 
#include"../glslshader.h"
#include"../Font.h"

class DeBoor_BSpline : public Projects
{
public:
	DeBoor_BSpline() = default;
	void init() override;
	void Update(float deltaTime)  override;
	void Draw() override;
	void UnLoad() override;
	void OnImGuiRender() override;

	//helper functions
	void Calculate();


public:
	void clear();
	void setup_matrix();
	void init_vertices();
	void init_curves();
	void draw_points();
	void draw_curve();
	void update_vertice();
	void contorlPointsCurveUpdate();
	glm::dvec2 DeBoorRecursion(double t, int d, int j);
	bool in_mouse(double mouse_pos_x, double mouse_pos_y, glm::vec2 pos);
private:
	GLSLShader Prog;

	//Control points
	GLuint VAO{ 0 };
	GLuint VBO{ 0 };

	//Curve
	GLuint Curve_VAO{ 0 };
	GLuint Curve_VBO{ 0 };

	//Curve for control points
	GLuint Control_Curve_VAO{ 0 };
	GLuint Control_Curve_VBO{ 0 };

	std::vector<glm::dvec2> control_points;
	std::vector<glm::dvec2> vertices;
	std::vector<glm::dvec2> control_pointsCurve;

	glm::mat4 matrix;

	int degree{ 0 };
	std::vector<double> knot_sequence;
};