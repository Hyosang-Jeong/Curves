//Author: Hyosang Jung
#include"Engine.h"
#include"projects/DeCasteljau_Polynomial.h"
#include"projects/DeCasteljau_BezierCurves.h"
#include"projects/InterpolatingPolynomials_NewtonForm.h"
#include"projects/InterpolatingCubicSplines.h"
#include"projects/DeBoorAlgorithm_BSplineCurve.h"
int main()
{

    Engine engine;
    DeCasteljau_Polynomial* project1 = new DeCasteljau_Polynomial();
    DeCasteljau_BezierCurves* project2 = new DeCasteljau_BezierCurves();
    Newton_Form* project3 = new Newton_Form();
    CubicSplines* project4 = new CubicSplines();
    DeBoor_BSpline* project5 = new DeBoor_BSpline();
    engine.Add(project1);
    engine.Add(project2);
    engine.Add(project3);
    engine.Add(project4);
    engine.Add(project5);

    engine.init();

    while (!engine.ShouldClose())
    {
        engine.Update();
        engine.Draw();
    }
}

