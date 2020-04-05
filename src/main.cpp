#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

GLFWwindow *window = nullptr;
const int window_width = 750, window_height = 750;

/* CALLBACK */
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

bool load_shader(const char *file_path, GLenum type, GLuint *id);

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    if (!glfwInit()) {
        const char *error = nullptr;
        glfwGetError(&error);
        std::cout << "GLFW initialization failed!\n" << error << std::endl;
        return 1;
    }

    window = glfwCreateWindow(
        window_width,
        window_height,
        "Cheap Google Earth",
        nullptr,
        nullptr);
    glfwSetWindowSizeLimits(window, 200, 200, 2000, 2000);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        std::cout << "glad initialization failed!" << std::endl;

        glfwDestroyWindow(window);
        
        return 1;
    }

    // TEST: Render triangle
    // This data, after being processed by the vertex shader, will be in NDC (normalized device coordinates)
    // Right now, we will use NDC at start
    double vertices[] = {
        -0.5, -0.5, 0.0,  0.0,  0.0,  1.0,
         0.0,  0.5, 0.0,  1.0,  0.0,  0.0,
         0.5, -0.5, 0.0,  0.0,  1.0,  0.0,
    };

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vertex_shader = 0, fragment_shader = 0;
    bool vertex_shader_load_successful   = load_shader("../src/vertex_shader.vert",   GL_VERTEX_SHADER,   &vertex_shader);
    bool fragment_shader_load_successful = load_shader("../src/fragment_shader.frag", GL_FRAGMENT_SHADER, &fragment_shader);

    if (!vertex_shader_load_successful || !fragment_shader_load_successful) {
        return 1;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[513];
        std::memset(log, 0, 513);
        glGetProgramInfoLog(program, 512, NULL, log);
        std::cout << "Program linking failed!\n" << log << std::endl;
        return 1;
    }

    glUseProgram(program);

    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 6 * sizeof(double), nullptr);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, 6 * sizeof(double), (void*) (3 * sizeof(double)));
    glEnableVertexAttribArray(1);

    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // Disable wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    bool running = true;
    while (running) {
        glfwPollEvents();

        if (glfwWindowShouldClose(window)) {
            running = false;
            break;
        }

        //double mouse_x = 0, mouse_y = 0;
        //glfwGetCursorPos(window, &mouse_x, &mouse_y);
        //std::cout << "(" << mouse_x << ", " << mouse_y << ")" << std::endl;

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    glDeleteBuffers(1, &vbo);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void) window;

    glViewport(0, 0, width, height);
}

bool load_shader(const char *file_path, GLenum type, GLuint *id)
{
    std::ifstream file(file_path);

    if (!file) {
        std::cout << "Could not open " << file_path << "!" << std::endl;
        return false;
    }

    file.seekg(0, std::ios_base::end);
    uint64_t file_size = file.tellg();
    file.seekg(0, std::ios_base::beg);

    char shader_src[file_size + 1];
    std::memset(shader_src, 0, file_size);
    file.read(shader_src, file_size);

    file.close();
    
    GLuint shader = glCreateShader(type);
    const char *shader_src_ptr = shader_src;
    glShaderSource(shader, 1, &shader_src_ptr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char log[513];
        std::memset(log, 0, 513);
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cout << "Compilation of " << file_path << " failed!\n" << log << std::endl;
        return false;
    }

    *id = shader;
    return true;
}
