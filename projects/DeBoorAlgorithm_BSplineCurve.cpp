//Author: Hyosang Jung
#include "DeBoorAlgorithm_BSplineCurve.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include<vector>
#include"../glhelper.h"
#include <iomanip>
#include <sstream>
#include<algorithm>
#include<fstream>
using namespace glm;

using namespace std;


void DeBoor_BSpline::init()
{
    setup_matrix();

    init_curves();
    init_vertices();

    //shader
    std::vector<std::pair<GLenum, std::string>> shdr_files;
    shdr_files.push_back(std::make_pair(GL_VERTEX_SHADER, "shaders/curve.vert"));
    shdr_files.push_back(std::make_pair(GL_FRAGMENT_SHADER, "shaders/curve.frag"));

    Prog.CompileLinkValidate(shdr_files);

    degree = 3;
    knot_sequence = std::vector<double>{ 0.0, 1.0, 2.0, 3.0 };
}


void DeBoor_BSpline::Update(float)
{
    update_vertice();
}

void DeBoor_BSpline::Draw()
{
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Prog.Use();
    glLineWidth(1.f);

    draw_curve();
    draw_points();

    glLineWidth(1.f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    OnImGuiRender();

}

void DeBoor_BSpline::UnLoad()
{
}

void DeBoor_BSpline::OnImGuiRender()
{
    if (ImGui::Button("Clear"))
    {
        clear();
    }
    if (ImGui::InputInt("Degree", &degree))
    {
        clear();
        knot_sequence.clear();
        for (int i = 0; i <= degree; i++)
            knot_sequence.push_back((double)i);
        Calculate();
    }
    if (ImGui::Button("Show vertices"))
    {
        std::ofstream ofs("control.txt", std::ios::out | std::ios::app);
        for (int i = 0; i < control_points.size(); i++)
        {
            ofs << "Point " << i << " :" << control_points[i].x << "    " << control_points[i].y << std::endl;
        }
    }
}

void DeBoor_BSpline::Calculate()
{
    vertices.clear();
    double t = knot_sequence[degree];

    while (t < knot_sequence.back() - degree)
    {
        int J = 0;
        for (int i = 0; i < knot_sequence.size() - 1; i++)
        {
            if (t >= knot_sequence[i] && t < knot_sequence[i + 1])
            {
                J = i;
                break;
            }
        }

        vertices.push_back(DeBoorRecursion(t, degree, J));
        t += 0.05;
    }

    if (vertices.size() != 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * vertices.size(), &vertices[0].x, GL_STATIC_DRAW);
    }
}



void DeBoor_BSpline::clear()
{

    knot_sequence = std::vector<double>{ 0.0, 1.0, 2.0, 3.0 };
    control_points.clear();
    vertices.clear();
    control_pointsCurve.clear();
}

void DeBoor_BSpline::setup_matrix()
{
    matrix = glm::mat4(1.0f);
}



void DeBoor_BSpline::init_vertices()
{

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // position attribute
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);

}

void DeBoor_BSpline::init_curves()
{
    glGenVertexArrays(1, &Curve_VAO);
    glGenBuffers(1, &Curve_VBO);
    glBindVertexArray(Curve_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    // position attribute
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);


    glGenVertexArrays(1, &Control_Curve_VAO);
    glGenBuffers(1, &Control_Curve_VBO);
    glBindVertexArray(Control_Curve_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Control_Curve_VBO);
    // position attribute
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);
}



void DeBoor_BSpline::draw_points()
{
    Prog.Use();
    unsigned int loc = glGetUniformLocation(Prog.GetHandle(), "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &matrix[0].x);
    glBindVertexArray(VAO);

    glVertexAttrib3f(1, 1.f, 0.0f, 0.f); // for points
    glPointSize(10.f);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(control_points.size()));

    Prog.UnUse();
}

void DeBoor_BSpline::draw_curve()
{
    Prog.Use();
    unsigned int loc = glGetUniformLocation(Prog.GetHandle(), "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &matrix[0].x);

    glBindVertexArray(Curve_VAO);
    glVertexAttrib3f(1, 0.f, 1.0f, 0.f); // COLOR
    glLineWidth(3);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(vertices.size()));

    glBindVertexArray(Control_Curve_VAO);
    glVertexAttrib3f(1, 1.f, 0.0f, 0.f); // COLOR
    glLineWidth(1);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(control_pointsCurve.size()));

    Prog.UnUse();
}



void DeBoor_BSpline::update_vertice()
{
    double mouse_pos_x;
    double mouse_pos_y;

    //upper left is 0,0
    //bottom right is width, height
    glfwGetCursorPos(GLHelper::ptr_window, &mouse_pos_x, &mouse_pos_y);

    //conver to ndc coord
    mouse_pos_x = ((mouse_pos_x / static_cast<double>(GLHelper::width)) * 2) - 1;
    mouse_pos_y = ((mouse_pos_y / static_cast<double>(GLHelper::height)) * -2) + 1;

    vec4 tmp = { mouse_pos_x,mouse_pos_y,0,1 };
    tmp = glm::inverse(matrix) * tmp;
    mouse_pos_x = tmp.x;
    mouse_pos_y = tmp.y;
    static bool left_pressed = false;
    if (left_pressed == false && GLHelper::leftmouse_pressed == true)
    {
            knot_sequence.push_back(knot_sequence.back() + 1.0);
            left_pressed = true;
            control_points.push_back({ static_cast<float>(mouse_pos_x) ,static_cast<float>(mouse_pos_y) });
            contorlPointsCurveUpdate();
            if (control_points.size() > degree)
            {
                Calculate();
            }
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * control_points.size() * 3, &control_points[0].x, GL_STATIC_DRAW);
        
    }
    if (GLHelper::leftmouse_released == true)
        left_pressed = false;


    static bool right_pressed = false;
    static int current = -1;
    if (GLHelper::rightmouse_pressed == true)
    {
        if (current == -1)
        {
            for (int i = 0; i < control_points.size(); i++)
            {
                if (in_mouse(mouse_pos_x, mouse_pos_y, control_points[i]) == true)
                {
                    current = i;
                    right_pressed = true;
                    break;
                }
            }
        }
    }
    else
    {
        right_pressed = false;
        current = -1;
    }
    if (current != -1)
    {

        control_points[current].x = static_cast<float>(mouse_pos_x);
        control_points[current].y = static_cast<float>(mouse_pos_y);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * control_points.size() * 3, &control_points[0].x, GL_STATIC_DRAW);
        contorlPointsCurveUpdate();
        if (degree > 1)
            Calculate();
    }

}



void DeBoor_BSpline::contorlPointsCurveUpdate()
{
    control_pointsCurve.clear();
    double t_diff = 0.05f;
    double t = 0;

    for (int i = 0; i < control_points.size() - 1; i++)
    {
        while (t < 1)
        {
            glm::dvec2 val;
            val.x = (1 - t) * control_points[i].x + t * (control_points[i + 1].x);
            val.y = (1 - t) * control_points[i].y + t * (control_points[i + 1].y);
            control_pointsCurve.push_back(val);
            t += t_diff;
        }
        t = 0;
    }
    if (!control_pointsCurve.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, Control_Curve_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * control_pointsCurve.size(), &control_pointsCurve[0].x, GL_STATIC_DRAW);
    }
}

glm::dvec2 DeBoor_BSpline::DeBoorRecursion(double t, int p, int i)
{
    //base case
    if (p == 0)
    {
        return control_points[i];
    }

    double left = (t - knot_sequence[i]) / (knot_sequence[i + degree - (p - 1)] - knot_sequence[i]);
    double right = (knot_sequence[i + degree - (p - 1)] - t) / (knot_sequence[i + degree - (p - 1)] - knot_sequence[i]);


    return left * DeBoorRecursion(t,p-1,i) + right* DeBoorRecursion(t, p - 1, i-1);
}

bool DeBoor_BSpline::in_mouse(double mouse_pos_x, double mouse_pos_y, glm::vec2 pos)
{
    float offset = 0.04f;
    if (mouse_pos_x >= static_cast<double>(pos.x) - static_cast<double>(offset) && mouse_pos_x <= static_cast<double>(pos.x) + static_cast<double>(offset)
        && mouse_pos_y >= static_cast<double>(pos.y) - static_cast<double>(offset) && mouse_pos_y <= static_cast<double>(pos.y) + static_cast<double>(offset))
    {
        return true;
    }
    return false;
}




