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

using std::chrono::high_resolution_clock;

#define CLEAR_COLOR 0.2f, 0.3f, 0.3f, 1.0f

static const float FOV = 90.0f;
static const float ROTATION_SPEED = 5.0f;

GLFWwindow *window = nullptr;
const int window_width = 750, window_height = 750;
bool mouse_pressed = false;

GLuint vbo = 0;
Shader vertex_shader, fragment_shader;
GLuint program = 0;
GLuint vao = 0;

double xpos_prev = 0, ypos_prev = 0;
double dx = 0, dy = 0;

// TEST: Render triangle
// This data, after being processed by the vertex shader, will be in NDC (normalized device coordinates)
// Right now, we will use NDC at start
double vertices[] = {
    -0.5, -0.5, 0.0,  1.0,  0.0,  0.0,
     0.0,  0.5, 0.0,  0.0,  1.0,  0.0,
     0.5, -0.5, 0.0,  0.0,  0.0,  1.0,
};

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void) window;

    glViewport(0, 0, width, height);
}

static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
    (void) window;

    dx = xpos - xpos_prev;
    dy = ypos - ypos_prev;
    xpos_prev = xpos;
    ypos_prev = ypos;
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    (void) window;
    (void) mods;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mouse_pressed = (action == GLFW_PRESS);
    }
}

int main()
{
    if (!glfwInit()) {
        const char *error = nullptr;
        glfwGetError(&error);
        std::cerr << "GLFW initialization failed!\n" << error << std::endl;
        
        return EXIT_FAILURE;
    }

    window = glfwCreateWindow(
        window_width,
        window_height,
        "Cheap Google Earth",
        nullptr,
        nullptr);
    if (window == NULL) {
        const char *error = nullptr;
        glfwGetError(&error);
        std::cerr << "GLFW window creation failed!\n" << error << std::endl;

        return EXIT_FAILURE;
    }
    glfwSetWindowSizeLimits(window, 200, 200, 2000, 2000);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        std::cerr << "glad initialization failed!" << std::endl;

        return EXIT_FAILURE;
    }

    // Create VBO and load vertex data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Load and compile shaders
    if (vertex_shader.load_shader(GL_VERTEX_SHADER, "shaders/vertex_shader.vert") != 0) {
        return EXIT_FAILURE;
    }
    if (fragment_shader.load_shader(GL_FRAGMENT_SHADER, "shaders/fragment_shader.frag") != 0) {
        return EXIT_FAILURE;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex_shader.get_id());
    glAttachShader(program, fragment_shader.get_id());
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        
        char *log_ptr = (char*) calloc(len, 1);
        std::unique_ptr<char> log(log_ptr);
        glGetProgramInfoLog(program, len, nullptr, log.get());

        std::cerr << "Program linking failed!\n" << log.get() << std::endl;

        return EXIT_FAILURE;
    }

    glUseProgram(program);

    // Set VAO and vertex attributes
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 6 * sizeof(double), nullptr);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, 6 * sizeof(double), (void*) (3 * sizeof(double)));
    glEnableVertexAttribArray(1);

    GLint model = glGetUniformLocation(program, "model");
    GLint view = glGetUniformLocation(program, "view");
    GLint proj = glGetUniformLocation(program, "proj");
    
    glm::mat4 model_transform = glm::mat4(1.0f);
    glm::mat4 view_transform = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 proj_transform = glm::perspective(glm::radians(FOV), (float) window_width / (float) window_height, 0.0f, 3.0f);

    glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(view_transform));
    glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(proj_transform));

    glfwGetCursorPos(window, &xpos_prev, &ypos_prev);
    auto begin = high_resolution_clock::now();
    while (true) {
        glfwPollEvents();

        if (glfwWindowShouldClose(window)) {
            break;
        }

        if (mouse_pressed && dx != 0 && dy != 0) {
            glm::vec3 rot(0.0f);
            if (abs(dx) >= abs(dy)) {
                rot.y = dx;
            }
            else {
                rot.x = dy;
            }
            model_transform = glm::rotate(model_transform, glm::radians(ROTATION_SPEED), glm::normalize(rot));
            dx = 0;
            dy = 0;
        }

        glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(model_transform));

        glClearColor(CLEAR_COLOR);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);

        auto end = high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(16 - elapsed));
        begin = high_resolution_clock::now();
    }

    glDeleteBuffers(1, &vbo);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
