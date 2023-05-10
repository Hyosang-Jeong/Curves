#pragma once

#include"projects.h"
#include <GL/glew.h> 
#include"../glslshader.h"
#include"../Font.h"

enum class Project2_Mode
{
	NLI,
	BB,
	MID_POINT
};

class DeCasteljau_BezierCurves : public Projects
{
public:
	DeCasteljau_BezierCurves() = default;
	void init() override;
	void Update(float deltaTime)  override;
	void Draw() override;
	void UnLoad() override;
	void OnImGuiRender() override;


	//Calculate curve using Nested Linear Interpolation
	void NLI_begin();
	glm::vec2 NLI_Calculate(float t, int i, int j);


	//Calculate curve using BB-form
	void BB_form();
	int   Pascal_recursion(int first, int second);

	//Calculate Midpoint subdivision
	void MidPoint_subdivision_Begin();
	void MidPoint_subdivision_Calculate(std::vector<glm::vec2> controlPoints, int num);
	//helper functions
public:
	void clear();

	void setup_matrix();
	void init_vertices();
	void init_curves();
	void draw_points();
	void draw_curve();
	void update_vertice();
	void set_pascalValue();
	void contorlPointsCurveUpdate();
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

	//Curve for NLI step
	GLuint NLI_VAO{ 0 };
	GLuint NLI_VBO{ 0 };


	Fonts fonts;

	std::vector<glm::vec2> control_points;
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> control_pointsCurve;
	std::vector<glm::vec2> NLI_step_vertices;

	std::vector<int>pascal_value;
	glm::mat4 matrix;

	int k = 5;
	float tval{ 0.5f };

	Project2_Mode mode{ Project2_Mode::NLI };
};