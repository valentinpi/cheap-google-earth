#include "Shader.hpp"

Shader::Shader(GLenum type) : type(type) {}

Shader::~Shader()
{
    glDeleteShader(shader);
}

int Shader::load_shader(std::string file_path)
{
    shader = glCreateShader(type);

    std::ifstream file(file_path);
    if (!file) {
        std::cerr << "Could not open " << file_path << "!" << std::endl;
        return LOAD_SHADER_FAILURE;
    }

    // Use read buffer to read entire file into stringstream
    std::stringstream src;
    src << file.rdbuf();
    file.close();

    std::string shader_src = src.str();
    const char *shader_src_ptr = shader_src.c_str();
    glShaderSource(shader, 1, &shader_src_ptr, nullptr);

    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

        char *log_ptr = (char*) calloc(len, 1);
        std::unique_ptr<char> log(log_ptr);
        glGetShaderInfoLog(shader, len, nullptr, log.get());
        
        std::cerr << "Compilation of " << file_path << " failed!\n" << log.get() << std::endl;

        return LOAD_SHADER_FAILURE;
    }

    return LOAD_SHADER_SUCCESS;
}
