#pragma once

#include"projects.h"
#include <GL/glew.h> 
#include"../glslshader.h"
#include"../Font.h"
enum class Project1_Mode
{
	NLI,
	BB
};

class DeCasteljau_Polynomial : public Projects
{
public:
	DeCasteljau_Polynomial()  = default;
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


	//helper functions
public:
	void setup_matrix();
	void init_grid();
	void init_vertices();
	void init_curves();
	void draw_grid();
	void draw_points();
	void draw_curve();
	void change_degree();
	void update_vertice();
	void set_pascalValue();
	bool in_mouse(double mouse_pos_x, double mouse_pos_y, glm::vec2 pos);
private:
	GLSLShader Prog;

	GLuint VAO{ 0 };
	GLuint VBO{ 0 };

	GLuint Grid_VAO{ 0 };
	GLuint Grid_VBO{ 0 };

	GLuint Curve_VAO{ 0 };
	GLuint Curve_VBO{ 0 };

	Fonts fonts;


	std::vector<glm::vec2> grid_vertices;
	std::vector<glm::vec2> control_points;
	std::vector<glm::vec2> vertices;
	 std::vector<int>pascal_value;
	glm::mat4 matrix;


	int degree{ 3 };
	float t_min{ 0 };
	float t_max{ 0 };

	Project1_Mode mode{ Project1_Mode::NLI};
};