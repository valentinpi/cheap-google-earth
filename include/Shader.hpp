#pragma once

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include <glad/glad.h>

#include "Constants.hpp"

static const int LOAD_SHADER_SUCCESS = 0;
static const int LOAD_SHADER_FAILURE = 1;

class Shader
{
private:
    GLenum type = 0;
    GLuint shader = 0;
public:
    Shader(GLenum type);
    ~Shader();

    inline GLuint get_id() {
        return shader;
    }

    int load_shader(std::string file_path);
};
