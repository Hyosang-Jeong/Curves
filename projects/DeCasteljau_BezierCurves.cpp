//Author: Hyosang Jung
#include "DeCasteljau_BezierCurves.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include<vector>
#include"../glhelper.h"
#include <iomanip>
#include <sstream>
#include<algorithm>
using namespace glm;


void DeCasteljau_BezierCurves::init()
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


void DeCasteljau_BezierCurves::Update(float)
{
    update_vertice();
}

void DeCasteljau_BezierCurves::Draw()
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

void DeCasteljau_BezierCurves::UnLoad()
{
}

void DeCasteljau_BezierCurves::OnImGuiRender()
{
    if (ImGui::Button("Clear"))
    {
        clear();
    }
    static const char* Method[] =
    {  "NLI",
       "BB-form",
        "MidPoint subdivision"
    };
    static  const char* current_method = Method[0];
    static int current = 0;
    if (ImGui::BeginCombo("Calculation Method", current_method)) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < 3; n++)
        {
            bool is_selected = (current_method == Method[n]);
            if (ImGui::Selectable(Method[n], is_selected))
            {
                current_method = Method[n];
                current = n;
                mode = Project2_Mode(n);
                if (mode == Project2_Mode::NLI)
                {
                    vertices.clear();
                    NLI_begin();
                }
                else if(mode == Project2_Mode::BB)
                {
                    vertices.clear();
                    NLI_step_vertices.clear();
                    BB_form();
                }
                else
                {
                    vertices.clear();             
                    NLI_step_vertices.clear();
                    MidPoint_subdivision_Begin();
                }
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    if(ImGui::SliderFloat("t value",&tval, 0, 1.f))
    {
        if(mode == Project2_Mode::NLI)
            NLI_begin();
    }
}

void DeCasteljau_BezierCurves::clear()
{
     control_points.clear();
     vertices.clear();
     control_pointsCurve.clear();
     NLI_step_vertices.clear();
}

void DeCasteljau_BezierCurves::setup_matrix()
{
    glm::mat4 trans = {
            1,0,0,0,0,1,0,0,0,0,1,0,-0.8f,0,0,1
    };
    glm::mat4 scale = {
             1.6f,0,0,0,
             0,0.8f / 3.f,0,
             0,0,0,1,
             0,0,0,0,1
    };
    matrix = trans * scale;
}



void DeCasteljau_BezierCurves::init_vertices()
{

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

}

void DeCasteljau_BezierCurves::init_curves()
{
    glGenVertexArrays(1, &Curve_VAO);
    glGenBuffers(1, &Curve_VBO);
    glBindVertexArray(Curve_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glGenVertexArrays(1, &Control_Curve_VAO);
    glGenBuffers(1, &Control_Curve_VBO);
    glBindVertexArray(Control_Curve_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Control_Curve_VBO);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glGenVertexArrays(1, &NLI_VAO);
    glGenBuffers(1, &NLI_VBO);
    glBindVertexArray(NLI_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, NLI_VBO);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


}



void DeCasteljau_BezierCurves::draw_points()
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

void DeCasteljau_BezierCurves::draw_curve()
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

    glBindVertexArray(NLI_VAO);
    glVertexAttrib3f(1, 1.f, 0.0f, 0.f); // COLOR
    glLineWidth(1);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(NLI_step_vertices.size()));
    glVertexAttrib3f(1, 0.f, 0.0f, 1.f); // COLOR
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(NLI_step_vertices.size()));

    Prog.UnUse();
}

void DeCasteljau_BezierCurves::NLI_begin()
{
    if (control_points.size() <= 2)
        return;
    vertices.clear();
    float t_diff = 0.01f;
    float t = 0;
    while (t < 1)
    {
        vertices.push_back(NLI_Calculate(t, control_points.size()-1, 0));
        t += t_diff;
    }

    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size() , &vertices[0].x, GL_STATIC_DRAW);

    NLI_step_vertices.clear();

    NLI_Calculate(tval, control_points.size() - 1, 0);
    NLI_step_vertices.push_back((tval) * *(NLI_step_vertices.end()-2) + (1-tval)* *(NLI_step_vertices.end() - 1));
    glBindBuffer(GL_ARRAY_BUFFER, NLI_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * NLI_step_vertices.size(), &NLI_step_vertices[0].x, GL_STATIC_DRAW);
}

glm::vec2 DeCasteljau_BezierCurves::NLI_Calculate(float t, int i, int j)
{
    std::vector<glm::vec2> tmp;
    for (int i = 0; i <= control_points.size() - 1; i++)
    {
        tmp.push_back(control_points[i]);
    }
    std::vector<glm::vec2>tmp2;
    for (int k = 1; k <= control_points.size() - 1; k++)
    {
        tmp2.clear();
        for (int j = 0; j <= control_points.size() - 1 - k; j++)
        {
            tmp2.push_back((1 - t) * tmp[j] + (t)*tmp[j + 1]);

            if (j != control_points.size() - 1 - k)
            {
                NLI_step_vertices.push_back((1 - t) * tmp[j + 1] + (t)*tmp[j + 2]);
                NLI_step_vertices.push_back((1 - t) * tmp[j] + (t)*tmp[j + 1]);
            }
        }
        tmp = tmp2;
    }

    return { tmp2.front() };
}

void DeCasteljau_BezierCurves::BB_form()
{
   if (control_points.size() <= 2)
         return;
    vertices.clear();
    float t_diff = 0.01f;
    float t = 0;

    while (t < 1)
    {
        glm::vec2 val{ 0 };
        for (int i = 0; i <= control_points.size()-1; i++)
        {
            val.x += pascal_value[i] * pow(1 - t, control_points.size() - 1 - i) * pow(t, i) * control_points[i].x;
            val.y += pascal_value[i] * pow(1 - t, control_points.size() - 1 - i) * pow(t, i) * control_points[i].y;

        }
        vertices.push_back(val);
        t += t_diff;
    }

    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size(), &vertices[0].x, GL_STATIC_DRAW);
}

int DeCasteljau_BezierCurves::Pascal_recursion(int first, int second)
{
    //Base Case
    // nC1 OR nCn
    if (first == second || second == 0)
        return 1;


    return Pascal_recursion(first - 1, second - 1) + Pascal_recursion(first - 1, second);
}

void DeCasteljau_BezierCurves::MidPoint_subdivision_Begin()
{
    if (control_points.size() <= 2)
        return;

    vertices.clear();
    std::vector<glm::vec2> left;
    std::vector<glm::vec2> right;
    std::vector<glm::vec2> tmp;
    for (int i = 0; i < control_points.size(); i++)
    {
        tmp.push_back(control_points[i]);
    }
    left.push_back(tmp.front());
    right.push_back(tmp.back());
    std::vector<glm::vec2>tmp2;
    for (int k = 1; k <= control_points.size() - 1; k++)
    {
        tmp2.clear();
        for (int j = 0; j <= control_points.size() - 1 - k; j++)
        {
            tmp2.push_back((0.5f) * tmp[j] + (0.5f) * tmp[j + 1]);
        }
        left.push_back(tmp2.front());
        right.push_back(tmp2.back());
        tmp = tmp2;
    }
    std::reverse(right.begin(), right.end());
    MidPoint_subdivision_Calculate(left, k);
    MidPoint_subdivision_Calculate(right, k);


    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size(), &vertices[0].x, GL_STATIC_DRAW);
}

void DeCasteljau_BezierCurves::MidPoint_subdivision_Calculate(std::vector<glm::vec2> controlPoints, int num)
{
    if (num == 0)
    {
        for (auto& i : controlPoints)
            vertices.push_back(i);
        return;
    }
    std::vector<glm::vec2> left;
    std::vector<glm::vec2> right;
    std::vector<glm::vec2> tmp;
    for (int i = 0; i < controlPoints.size() ; i++)
    {
        tmp.push_back(controlPoints[i]);
    }
    left.push_back(tmp.front());
    right.push_back(tmp.back());
    std::vector<glm::vec2>tmp2;
    for (int k = 1; k <= controlPoints.size() - 1; k++)
    {
        tmp2.clear();
        for (int j = 0; j <= controlPoints.size() - 1 - k; j++)
        {
            tmp2.push_back((0.5f) * tmp[j] + (0.5f)*tmp[j + 1]);
        }
        left.push_back(tmp2.front());
        right.push_back(tmp2.back());
        tmp = tmp2;
    }
    std::reverse(right.begin(), right.end());
    MidPoint_subdivision_Calculate(left, num-1);
    MidPoint_subdivision_Calculate(right, num-1);
}

void DeCasteljau_BezierCurves::update_vertice()
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
    if (left_pressed == false && GLHelper::leftmouse_pressed == true )
    {
        if (control_points.size() < 20)
        {
            left_pressed = true;
            control_points.push_back({ static_cast<float>(mouse_pos_x) ,static_cast<float>(mouse_pos_y) });
            contorlPointsCurveUpdate();
            set_pascalValue();
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * control_points.size() * 3, &control_points[0].x, GL_STATIC_DRAW);

            switch (mode)
            {
            case Project2_Mode::NLI:
                NLI_begin();
                break;
            case Project2_Mode::BB:
                BB_form();
                break;
            case Project2_Mode::MID_POINT:
                MidPoint_subdivision_Begin();
                break;
            default:
                break;
            }
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * control_points.size() * 3, &control_points[0].x, GL_STATIC_DRAW);
        contorlPointsCurveUpdate();
        if (mode == Project2_Mode::NLI)
        {
            NLI_step_vertices.clear();
            set_pascalValue();
            NLI_begin();
        }
        else if (mode == Project2_Mode::BB)
        {
            BB_form();

        }
        else
        {
            MidPoint_subdivision_Begin();
        }
    }

}

void DeCasteljau_BezierCurves::set_pascalValue()
{
    pascal_value.clear();
    for (int i = 0; i <= control_points.size()-1; i++)
    {
        pascal_value.push_back(Pascal_recursion(control_points.size()-1, i));
    }
}

void DeCasteljau_BezierCurves::contorlPointsCurveUpdate()
{
    control_pointsCurve.clear();
    float t_diff = 0.05f;
    float t = 0;

    for (int i = 0; i < control_points.size() - 1; i++)
    {
        while (t < 1)
        {
            glm::vec2 val;
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * control_pointsCurve.size() , &control_pointsCurve[0].x, GL_STATIC_DRAW);
    }
}

bool DeCasteljau_BezierCurves::in_mouse(double mouse_pos_x, double mouse_pos_y, glm::vec2 pos)
{
    float offset = 0.04f;
    if (mouse_pos_x >= static_cast<double>(pos.x) - static_cast<double>(offset) && mouse_pos_x <= static_cast<double>(pos.x) + static_cast<double>(offset)
        && mouse_pos_y >= static_cast<double>(pos.y) - static_cast<double>(offset) && mouse_pos_y <= static_cast<double>(pos.y) + static_cast<double>(offset))
    {
        return true;
    }
    return false;
}




