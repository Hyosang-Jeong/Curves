//Author: Hyosang Jung
#ifndef FONT_H
#define FONT_H

#include<map>
#include<GL/glew.h>
#include<ft2build.h>
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include"glslshader.h"
#include FT_FREETYPE_H

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::vec2   Size;      // Size of glyph
    glm::vec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};


class Fonts
{
public:
    FT_Library ft;
    FT_Face face;
    GLSLShader Prog;
    std::map<GLchar, Character> Characters;
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

    unsigned VAO;
    unsigned VBO;
public:
    void init();
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
};
#endif // !FONT_H