#pragma once

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include <glad/glad.h>

class Shader
{
private:
    GLuint id = 0;

public:
    Shader() = default;
    ~Shader();

    inline GLuint get_id() {
        return id;
    }

    /* Returns 0 on success, otherwise 1 */
    int load_shader(GLenum type, std::string file_path);
};
