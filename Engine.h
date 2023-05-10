//Author: Hyosang Jung
#include<vector>
#include"Projects/Projects.h"

class Engine
{
public:
	Engine();
	~Engine();
	void Add(Projects* demo);
	void init();
	void Update();

	void Draw();
	bool ShouldClose();
	void ChangeProject();
	std::vector<Projects*> tests;
	int current;
};