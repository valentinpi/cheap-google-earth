#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"

static const double PI = 3.14159265358979323846264338327950288;

GLFWwindow *window = nullptr;
const int window_width = 750, window_height = 750;

// TEST: Render triangle
// This data, after being processed by the vertex shader, will be in NDC (normalized device coordinates)
// Right now, we will use NDC at start
double vertices[] = {
    -0.5, -0.5, 0.0,  0.0,  0.0,  1.0,
     0.0,  0.5, 0.0,  1.0,  0.0,  0.0,
     0.5, -0.5, 0.0,  0.0,  1.0,  0.0,
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void) window;

    glViewport(0, 0, width, height);
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    if (!glfwInit()) {
        const char *error = nullptr;
        glfwGetError(&error);
        std::cout << "GLFW initialization failed!\n" << error << std::endl;

        return EXIT_FAILURE;
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

        return EXIT_FAILURE;
    }

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    Shader vertex_shader, fragment_shader;
    if (vertex_shader.load_shader(GL_VERTEX_SHADER, "shaders/vertex_shader.vert")) {
        return EXIT_FAILURE;
    }
    if (fragment_shader.load_shader(GL_FRAGMENT_SHADER, "shaders/fragment_shader.frag")) {
        return EXIT_FAILURE;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader.get_id());
    glAttachShader(program, fragment_shader.get_id());
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[513];
        std::memset(log, 0, 513);
        glGetProgramInfoLog(program, 512, NULL, log);
        std::cout << "Program linking failed!\n" << log << std::endl;

        return EXIT_FAILURE;
    }

    glUseProgram(program);

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

    glm::mat4 transform = glm::mat4(1.0);

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

        //transform = glm::translate(transform, glm::vec3(1.0, 0.0, 0.0));
        transform = glm::rotate(transform, glm::radians(5.0f), glm::vec3(0.0, 0.0, 1.0));
        GLint transformUniform = glGetUniformLocation(program, "transform");
        glUniformMatrix4fv(transformUniform, 1, false, glm::value_ptr(transform));

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    glDeleteBuffers(1, &vbo);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
