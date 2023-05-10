//Author: Hyosang Jung
#include "Engine.h"
#include "glhelper.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

enum class Demo
{
    Project_1,
	Project_2,
    Project_3,
    Project_4,
    Project_5
};

Engine::Engine()
{
	if (!GLHelper::init(1600, 1000, "Class Project")) 
    {
		std::cout << "Unable to create OpenGL context" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(GLHelper::ptr_window, true);
	const char* glsl_version = "#version 450";
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGui::StyleColorsDark();
    current = static_cast<int>(Demo::Project_1);
}

Engine::~Engine()
{
    GLHelper::cleanup();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (auto& i : tests)
    {
        delete i;
        i = nullptr;
    }
}

void Engine::Add(Projects* demo)
{
	tests.push_back(demo);
}

void Engine::init()
{
    for (auto& i : tests)
    {
        i->init();
    }
}

void Engine::Update()
{
	glfwPollEvents();
	double delta_time = GLHelper::update_time(1.0);
	tests[current]->Update(static_cast<float>(delta_time));
}

void Engine::Draw()
{
	tests[current]->Draw();
	ImGui::Begin("Demo");
	ChangeProject();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(GLHelper::ptr_window);
}

bool Engine::ShouldClose()
{
	return glfwWindowShouldClose(GLHelper::ptr_window);
}

void Engine::ChangeProject()
{
    static const char* Method[] =
    {   "Project1",
        "Project2",
        "Project3",
        "Project4",
        "Project5",
    };
    static  const char* current_method = Method[current];
    if (ImGui::BeginCombo("Projects", current_method)) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < tests.size(); n++)
        {
            bool is_selected = (current_method == Method[n]);
            if (ImGui::Selectable(Method[n], is_selected))
            {
                current_method = Method[n];
                current = n;           
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

