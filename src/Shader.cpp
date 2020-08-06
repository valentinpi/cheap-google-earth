#include "Shader.hpp"

Shader::~Shader()
{
    glDeleteShader(id);
}

int Shader::load_shader(GLenum type, std::string file_path)
{
    std::ifstream file(file_path);
    if (!file) {
        std::cout << "Could not open " << file_path << "!" << std::endl;
        return 1;
    }

    std::stringstream src;
    src << file.rdbuf();
    file.close();
    
    GLuint shader = glCreateShader(type);
    std::string shader_src = src.str();
    const char *shader_src_ptr = shader_src.c_str();
    glShaderSource(shader, 1, &shader_src_ptr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char log[512];
        std::memset(log, 0, 512);
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cout << "Compilation of " << file_path << " failed!\n" << log << std::endl;
        return 1;
    }

    id = shader;
    return 0;
}
