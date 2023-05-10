//Author: Hyosang Jung
#include "InterpolatingCubicSplines.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include<vector>
#include"../glhelper.h"
#include <iomanip>
#include <sstream>
#include<algorithm>
using namespace glm;

using namespace std;

typedef vector<double> Row;
typedef vector<Row> Matrix;

void swap_rows(Matrix& A, int i, int j) {
    Row temp = A[i];
    A[i] = A[j];
    A[j] = temp;
}


vector<double> PerformOperation(std::vector<vector<double>>a, int n)
{
    int i, j, k = 0, c, flag = 0, m = 0;
    double pro = 0;

    // Performing elementary operations
    for (i = 0; i < n; i++)
    {
        if (a[i][i] == 0)
        {
            c = 1;
            while ((i + c) < n && a[i + c][i] == 0)
                c++;
            if ((i + c) == n) {
                flag = 1;
                break;
            }
            for (j = i, k = 0; k <= n; k++)
                swap(a[j][k], a[j + c][k]);
        }

        for (j = 0; j < n; j++) {

            // Excluding all i == j
            if (i != j) {

                // Converting Matrix to reduced row
                // echelon form(diagonal matrix)
                pro = a[j][i] / a[i][i];

                for (k = 0; k <= n; k++)
                    a[j][k] = a[j][k] - (a[i][k]) * pro;
            }
        }
    }

    vector<double> tmp;
    for (int i = 0; i < n; i++)
        tmp.push_back(a[i][n] / a[i][i] );
    return tmp;
}



void CubicSplines::init()
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


void CubicSplines::Update(float)
{
    update_vertice();
}

void CubicSplines::Draw()
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

void CubicSplines::UnLoad()
{
}

void CubicSplines::OnImGuiRender()
{
    if (ImGui::Button("Clear"))
    {
        clear();
    }
}

void CubicSplines::Calculate()
{
    vertices.clear();
    Matrix xMat;
    Matrix yMat;
    for (unsigned i = 0; i < degree; i++)
    {
        xMat.push_back(Row({ (double)1,(double)i,pow(i,2),pow(i,3)}));
        yMat.push_back(Row({ (double)1,(double)i,pow(i,2),pow(i,3)}));
        for (unsigned j = 1; j < degree-1; j++)
        {
            double val;
            if (i >= j)
            {
                val = pow(double(i) - double(j), 3);
            }
            else
            {
                val = 0;
            }
            xMat[i].push_back(val);
            yMat[i].push_back(val);
        }
        xMat[i].push_back(control_points[i].x);
        yMat[i].push_back(control_points[i].y);
    }

    //f''(0)
    xMat.push_back(Row({ 0,0,2,0}));
    yMat.push_back(Row({ 0,0,2,0}));
    for (auto i = 0; i < degree-2; i++)
    {
        xMat[degree].push_back(0);
        yMat[degree].push_back(0);
    }
    //output
    xMat[degree].push_back(0);
    yMat[degree].push_back(0);

    //f''(k)
    xMat.push_back(Row({ 0,0,2, 6.0 * double(degree-1)}));
    yMat.push_back(Row({ 0,0,2, 6.0 * double(degree - 1) }));
    for (auto i = 1; i <= degree - 2; i++)
    {
        xMat[degree+1].push_back(6*(degree-1-i));
        yMat[degree+1].push_back(6 * (degree - 1 - i));
    }
    xMat[degree+1].push_back(0);
    yMat[degree+1].push_back(0);


    std::vector<double> x_coeffi = PerformOperation(xMat, xMat.size());
    std::vector<double> y_coeffi = PerformOperation(yMat, yMat.size());

    double t_diff = 0.0005;
    double t = 0;
    double k = degree - 1;
    while (t <= k)
    {
        double x{ x_coeffi[0]};
        double y{ y_coeffi[0]};
        unsigned i = 1;
        for (; i < 4; i++)
        {
            x += x_coeffi[i] * pow(t, i);
            y += y_coeffi[i] * pow(t, i);
        }

        for (unsigned j = 1; j < k; j++,i++)
        {
            double val;
            if (t >= j)
            {
                val = pow(t - double(j), 3);
            }
            else
            {
                val = 0;
            }
            x += x_coeffi[i] * val;
            y += y_coeffi[i] * val;
        }


        vertices.push_back({ x,y });

        t += t_diff;
    }





    glBindBuffer(GL_ARRAY_BUFFER, Curve_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * vertices.size(), &vertices[0].x, GL_STATIC_DRAW);
}



void CubicSplines::clear()
{
    degree =0;
    control_points.clear();
    vertices.clear();
    control_pointsCurve.clear();
}

void CubicSplines::setup_matrix()
{
    matrix = glm::mat4(1.0f);
}



void CubicSplines::init_vertices()
{

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // position attribute
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);

}

void CubicSplines::init_curves()
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



void CubicSplines::draw_points()
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

void CubicSplines::draw_curve()
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



void CubicSplines::update_vertice()
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
            if (degree > 1)
                Calculate();
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
        if (degree > 1)
            Calculate();
    }

}



void CubicSplines::contorlPointsCurveUpdate()
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

bool CubicSplines::in_mouse(double mouse_pos_x, double mouse_pos_y, glm::vec2 pos)
{
    float offset = 0.04f;
    if (mouse_pos_x >= static_cast<double>(pos.x) - static_cast<double>(offset) && mouse_pos_x <= static_cast<double>(pos.x) + static_cast<double>(offset)
        && mouse_pos_y >= static_cast<double>(pos.y) - static_cast<double>(offset) && mouse_pos_y <= static_cast<double>(pos.y) + static_cast<double>(offset))
    {
        return true;
    }
    return false;
}




