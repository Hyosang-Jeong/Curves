//Author: Hyosang Jung
#include "DeCasteljau_Polynomial.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include<vector>
#include"../glhelper.h"
#include <iomanip>
#include <sstream>
using namespace glm;

void DeCasteljau_Polynomial::init()
{
    degree = 3;
    setup_matrix();
    init_grid();
    init_vertices();
    NLI_begin();
    init_curves();
    set_pascalValue();
    //shader
    std::vector<std::pair<GLenum, std::string>> shdr_files;
    shdr_files.push_back(std::make_pair(GL_VERTEX_SHADER, "shaders/curve.vert"));
    shdr_files.push_back(std::make_pair(GL_FRAGMENT_SHADER, "shaders/curve.frag"));

    Prog.CompileLinkValidate(shdr_files);
    fonts.init();
}


void DeCasteljau_Polynomial::Update(float )
{
    update_vertice();
}

void DeCasteljau_Polynomial::Draw()
{
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Prog.Use();
    glLineWidth(1.f);

    draw_grid();
    draw_curve();
    draw_points();
    glLineWidth(1.f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    OnImGuiRender();

}

void DeCasteljau_Polynomial::UnLoad()
{
}

void DeCasteljau_Polynomial::OnImGuiRender()
{

    if( ImGui::InputInt("Degree", &degree))
    {
        change_degree();
    }

    static const char* Method[] =
    { "NLI",
       "BB-form" 
    };
    static  const char* current_method = Method[0];
    static int current = 0;
    if (ImGui::BeginCombo("Calculation Method", current_method)) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < 2; n++)
        {
            bool is_selected = (current_method == Method[n]);
            if (ImGui::Selectable(Method[n], is_selected))
            {
                current_method = Method[n];
                current = n;
                mode = Project1_Mode(n);
                if (mode == Project1_Mode::NLI)
                    NLI_begin();
                else
                    BB_form();
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void DeCasteljau_Polynomial::setup_matrix()
{
    glm::mat4 trans = {
            1,0,0,0,0,1,0,0,0,0,1,0,-0.8f,0,0,1
    };
    glm::mat4 scale = {
             1.6f,0,0,0,
             0,0.8f/3.f,0,
             0,0,0,1,
             0,0,0,0,1
    };
    matrix = trans* scale;
}

void DeCasteljau_Polynomial::init_grid()
{
    glm::vec2 start = { 0,-3.f };
    int stack = 12;
    for (int i = 0; i <= stack; i++)
    {
        grid_vertices.push_back(start);
        grid_vertices.push_back({ start.x+1.f,start.y });
        start.y += 0.5f;
    }
    start.y -= 0.5;
    grid_vertices.push_back(start);
    start = { 0,-3.f };
    grid_vertices.push_back(start);

    glGenVertexArrays(1, &Grid_VAO);
    glGenBuffers(1, &Grid_VBO);
    glBindVertexArray(Grid_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Grid_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * grid_vertices.size() * 3, &grid_vertices[0].x, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

}

void DeCasteljau_Polynomial::init_vertices()
{
    glm::vec2 start{ 0,1.f };
    float diff = 1.f / (float)degree;
    for (int i = 0; i <= degree; i++)
    {
        control_points.push_back(start);
        start.x += diff;
    }


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * control_points.size() * 3, &control_points[0].x, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

}

void DeCasteljau_Polynomial::init_curves()
{
    glGenVertexArrays(1, &Curve_VAO);
    glGenBuffers(1, &Curve_VBO);
    glBindVertexArray(Curve_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size() * 3, &vertices[0].x, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void DeCasteljau_Polynomial::draw_grid()
{
    Prog.Use();
    glBindVertexArray(Grid_VAO);
    glVertexAttrib3f(1, 0.f,0.f, 0.f); // white color for line
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(grid_vertices.size()));
    glm::vec2 start = { -0.8,-0.8 };
    int stack = 12;
    double val = -3.0;
   
    glm::vec2 pos = { 10,59 };
    for (int i = 0; i <= stack; i++)
    {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << val;
        fonts.RenderText(stream.str(), pos.x,pos.y, 0.2f, glm::vec3(0));
        val += 0.5;
        pos.y += 39.5;
    }
    Prog.UnUse();
}

void DeCasteljau_Polynomial::draw_points()
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

void DeCasteljau_Polynomial::draw_curve()
{
    Prog.Use();
    unsigned int loc = glGetUniformLocation(Prog.GetHandle(), "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &matrix[0].x);
    glBindVertexArray(Curve_VAO);

    glVertexAttrib3f(1, 0.f, 1.0f, 0.f); // for points
    glLineWidth(3);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(vertices.size()));
    Prog.UnUse();
}

void DeCasteljau_Polynomial::NLI_begin()
{
    vertices.clear();

    float t_diff = 0.01f;
    float t = 0;
    while (t < 1)
    {
        vertices.push_back(NLI_Calculate(t,degree,0));
        t += t_diff;
    }

    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size() * 3, &vertices[0].x, GL_STATIC_DRAW);

}

glm::vec2 DeCasteljau_Polynomial::NLI_Calculate(float t, int i, int j)
{
    std::vector<float> tmp;
    for (int i = 0; i <= degree; i++)
    {
        tmp.push_back(control_points[i].y);
    }
    std::vector<float>tmp2;
    for (int k = 1; k <= degree; k++)
    {
         tmp2.clear();
        for (int j = 0; j <= degree  - k; j++)
        {
                tmp2.push_back((1 - t) * tmp[j] + (t)*tmp[j+1]);
        }
         tmp = tmp2;
    }

    return { t,tmp2.front()};
}

void DeCasteljau_Polynomial::BB_form()
{
    vertices.clear();
    float t_diff = 0.01f;
    float t = 0;

    while (t < 1)
    {
        glm::vec2 val{ 0 };
        for (int i = 0; i <= degree; i++)
        {
            val.x += pascal_value[i]* pow(1 - t, degree - i)* pow(t, i)* control_points[i].x;
            val.y += pascal_value[i] * pow(1 - t, degree - i) * pow(t, i) * control_points[i].y;
        }
        vertices.push_back(val);
        t += t_diff;
    }

    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size() * 3, &vertices[0].x, GL_STATIC_DRAW);
}

int DeCasteljau_Polynomial::Pascal_recursion(int first, int second)
{
    //Base Case
    // nC1 OR nCn
    if (first == second || second == 0)
        return 1;


    return Pascal_recursion(first-1,second-1) + Pascal_recursion(first-1,second);
}

void DeCasteljau_Polynomial::update_vertice()
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
    static bool pressed = false;
    static int current = -1;
    if (GLHelper::leftmouse_pressed == true)
    {
        if (current == -1)
        {
            for (int i = 0; i < control_points.size(); i++)
            {
                if (in_mouse(mouse_pos_x, mouse_pos_y, control_points[i]) == true)
                {
                    current = i;
                    pressed = true;
                    break;
                }
            }
        }
    }
    else 
    {
        pressed = false;
        current = -1;
    }
    if (current != -1)
    {
        control_points[current].y = static_cast<float>(mouse_pos_y);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * control_points.size() * 3, &control_points[0].x, GL_STATIC_DRAW);
        if (mode == Project1_Mode::NLI)
            NLI_begin();
        else
            BB_form();
    }
}

void DeCasteljau_Polynomial::set_pascalValue()
{
    pascal_value.clear();
    for (int i = 0; i <= degree; i++)
    {
        pascal_value.push_back(Pascal_recursion(degree, i));
    }
}

bool DeCasteljau_Polynomial::in_mouse(double mouse_pos_x, double mouse_pos_y, glm::vec2 pos)
{
    float offset = 0.02f;
    if (mouse_pos_x >= static_cast<double>(pos.x) - static_cast<double>(offset) && mouse_pos_x <= static_cast<double>(pos.x) + static_cast<double>(offset)
        && mouse_pos_y >= static_cast<double>(pos.y) - static_cast<double>(offset) && mouse_pos_y <= static_cast<double>(pos.y) + static_cast<double>(offset))
    {
        return true;
    }
    return false;
}

void DeCasteljau_Polynomial::change_degree()
{
    if (degree == 0)
    {
        degree = 1;
        return;
    }
    if (degree == 21)
    {
        degree = 20;
        return;
    }
    set_pascalValue();
    control_points.clear();
    glm::vec2 start{ 0,1.f };
    float diff = 1.f / (float)degree;
    for (int i = 0; i <= degree; i++)
    {
        control_points.push_back(start);
        start.x += diff;
    }
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * control_points.size() * 3, &control_points[0].x, GL_STATIC_DRAW);
    if (mode == Project1_Mode::NLI)
        NLI_begin();
    else
        BB_form();
}


