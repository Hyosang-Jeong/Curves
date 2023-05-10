//Author: Hyosang Jung
#include "InterpolatingPolynomials_NewtonForm.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include<vector>
#include"../glhelper.h"
#include <iomanip>
#include <sstream>
#include<algorithm>
using namespace glm;

void Newton_Form::init()
{
    setup_matrix();

    init_curves();
    init_vertices();

    //shader
    std::vector<std::pair<GLenum, std::string>> shdr_files;
    shdr_files.push_back(std::make_pair(GL_VERTEX_SHADER, "shaders/curve.vert"));
    shdr_files.push_back(std::make_pair(GL_FRAGMENT_SHADER, "shaders/curve.frag"));

    Prog.CompileLinkValidate(shdr_files);
    fonts.init();
}


void Newton_Form::Update(float)
{
    update_vertice();
}

void Newton_Form::Draw()
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

void Newton_Form::UnLoad()
{
}

void Newton_Form::OnImGuiRender()
{
    if (ImGui::Button("Clear"))
    {
        clear();
    }
}

void Newton_Form::NewtonFormCalculate()
{
    vertices.clear();

    double t_diff = 0.0005;
    double t = 0;
    while (t <= degree)
    {
        double x{ 0 };
        double y{ 0 };
        for (int i = 0; i <= degree; i++)
        {
            double dd_x =  tableX[i][0];
            double dd_y =  tableY[i][0];

            double tval = 1.f;
            for (int j = 0; j < i; j++)
            {
                tval *= (t - (double)j);
            }
            dd_x *= tval;
            dd_y *= tval;
            x += dd_x;
            y += dd_y;
        }
         vertices.push_back({ x,y });

        t += t_diff;
    }
    vertices.push_back(control_points[degree]);
    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * vertices.size() , &vertices[0].x, GL_STATIC_DRAW);
}

void Newton_Form::SetupTable()
{
    tableX.clear();
    tableY.clear();

    for (unsigned i = 0; i <= degree; i++)
    {
        tableX.push_back(std::vector<double>());
        tableY.push_back(std::vector<double>());
        tableX[i].resize(degree + 1 - i);
        tableY[i].resize(degree + 1 - i);
    }

    for (unsigned i = 0; i <= degree; i++)
    {
        tableX[0][i]=(control_points[i].x);
        tableY[0][i]=(control_points[i].y);
    }

    double x = DividedDifferenceRecursion(tableX,0, degree, degree);
    double y = DividedDifferenceRecursion(tableY,0, degree, degree);

    tableX[degree].push_back(x);
    tableY[degree].push_back(y);

}

double Newton_Form::DividedDifferenceRecursion(std::vector<std::vector<double>>& table, int start, int end, int level)
{
    //base case
    if (start == end)
        return table[0][start];

    double value = DividedDifferenceRecursion(table, start + 1, end, level - 1) - DividedDifferenceRecursion(table, start, end - 1, level - 1);
    value = value / (static_cast<double>(end - start));
    table[level][start] = value;

    return value;
}

void Newton_Form::clear()
{
    degree = -1;
    control_points.clear();
    vertices.clear();
    control_pointsCurve.clear();
}

void Newton_Form::setup_matrix()
{
    matrix = glm::mat4(1.0f);
}



void Newton_Form::init_vertices()
{

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // position attribute
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);

}

void Newton_Form::init_curves()
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



void Newton_Form::draw_points()
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

void Newton_Form::draw_curve()
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



void Newton_Form::update_vertice()
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
        if (control_points.size() < 20)
        {
            degree++;
            left_pressed = true;
            control_points.push_back({ static_cast<float>(mouse_pos_x) ,static_cast<float>(mouse_pos_y) });
            contorlPointsCurveUpdate();
            SetupTable();
            if(degree > 0)
                NewtonFormCalculate();
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * control_points.size() * 3, &control_points[0].x, GL_STATIC_DRAW); 
        }
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
         SetupTable();
         NewtonFormCalculate();
    }

}



void Newton_Form::contorlPointsCurveUpdate()
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

bool Newton_Form::in_mouse(double mouse_pos_x, double mouse_pos_y, glm::vec2 pos)
{
    float offset = 0.04f;
    if (mouse_pos_x >= static_cast<double>(pos.x) - static_cast<double>(offset) && mouse_pos_x <= static_cast<double>(pos.x) + static_cast<double>(offset)
        && mouse_pos_y >= static_cast<double>(pos.y) - static_cast<double>(offset) && mouse_pos_y <= static_cast<double>(pos.y) + static_cast<double>(offset))
    {
        return true;
    }
    return false;
}




