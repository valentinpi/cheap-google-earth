#include "Shader.hpp"

Shader::~Shader()
{
    glDeleteShader(id);
}

int Shader::load_shader(GLenum type, std::string file_path)
{
    std::ifstream file(file_path);
    if (!file) {
        std::cerr << "Could not open " << file_path << "!" << std::endl;
        return 1;
    }

    // Use read buffer to read entire file into stringstream
    std::stringstream src;
    src << file.rdbuf();
    file.close();
    
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        std::cerr << "Shader creation failed!" << std::endl;
        return 1;
    }

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

        return 1;
    }

    id = shader;
    return 0;
}
