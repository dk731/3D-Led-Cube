

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstddef>

#include <stdlib.h>
#include <string.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void check_compile(GLuint obj);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void update_draw_calls();

enum draw_call_types
{
  CALL_POINT_TYPE = 0,
  CALL_POLYGON_TYPE = 1,
  CALL_POLYPYR_TYPE = 2,
  CALL_LINE_TYPE = 3,
  CALL_CIRCLE_TYPE = 4,
  CALL_FCIRCLE_TYPE = 5,
  CALL_SPHERE_TYPE = 6,
  CALL_FSPHERE_TYPE = 7,
};

struct draw_call
{
  int type;
  unsigned char color[3];
  float trans_mat[16];
  float data[16];
};

unsigned char b = 0;

std::vector<draw_call> draw_calls_arr;
unsigned int VBO, VAO, dc_vbo;

int main()
{
  // for (int z = 0; z < 16; z++)
  //   for (int y = 0; y < 16; y++)
  //     for (int x = 0; x < 16; x++)
  //     {
  //       draw_call *new_point_call = new draw_call({
  //           .type = CALL_POINT_TYPE,
  //           .color = {x * 16, y * 16, z * 16},
  //           .trans_mat = {
  //               1.0f, 0.0f, 0.0f, 0.0f,
  //               0.0f, 1.0f, 0.0f, 0.0f,
  //               0.0f, 0.0f, 1.0f, 0.0f,
  //               0.0f, 0.0f, 0.0f, 1.0f},
  //           .data = {x, y, z, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
  //       });
  //       draw_calls_arr.push_back(*new_point_call);
  //     }

  draw_call *new_point_call = new draw_call({
      .type = CALL_SPHERE_TYPE,
      .color = {255, 0, 0},
      .trans_mat = {
          1.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 1.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 1.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 1.0f},
      .data = {7.5f, 7.5f, 7.5f, 1.0f, 7.0f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
  });
  draw_calls_arr.push_back(*new_point_call);

  srand(time(NULL));

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(16, 256, "", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetKeyCallback(window, key_callback);

  GLenum err = glewInit();
  if (err != GLEW_OK)
    exit(1);

  std::ifstream in("./shaders/main.vert");
  std::string tmp_vert = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  const char *vert_str = tmp_vert.c_str();
  in.close();

  in = std::ifstream("./shaders/main.frag");
  std::string tmp_frag = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  const char *frag_str = tmp_frag.c_str();
  in.close();

  GLuint vert_shade = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_shade, 1, &vert_str, NULL);
  glCompileShader(vert_shade);
  check_compile(vert_shade);

  GLuint frag_shade = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_shade, 1, &frag_str, NULL);
  glCompileShader(frag_shade);
  check_compile(frag_shade);

  GLuint main_prog = glCreateProgram();

  glAttachShader(main_prog, vert_shade);
  glAttachShader(main_prog, frag_shade);

  int success;
  char infoLog[512];
  glLinkProgram(main_prog);
  glGetProgramiv(main_prog, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(main_prog, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }
  glUseProgram(main_prog);

  GLfloat vertices[12288];
  for (int z = 0; z < 16; z++)
    for (int y = 0; y < 16; y++)
      for (int x = 0; x < 16; x++)
      {
        int ind = (x + (y + z * 16) * 16) * 3;
        vertices[ind] = x;
        vertices[ind + 1] = y;
        vertices[ind + 2] = z;
      }

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &dc_vbo);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, dc_vbo);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(draw_call) * draw_calls_arr.size(), &draw_calls_arr[0], GL_DYNAMIC_DRAW);

  for (int i = 1; i < 11; i++)
    glEnableVertexAttribArray(i);

  int dc_str_size = sizeof(draw_call);

  glVertexAttribIPointer(1, 1, GL_INT, dc_str_size, (GLvoid *)offsetof(draw_call, type));
  glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, dc_str_size, (GLvoid *)offsetof(draw_call, color));
  for (int i = 0; i < 4; i++)
    glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, dc_str_size, (GLvoid *)(offsetof(draw_call, trans_mat) + sizeof(float) * 4 * i));
  for (int i = 0; i < 4; i++)
    glVertexAttribPointer(7 + i, 4, GL_FLOAT, GL_FALSE, dc_str_size, (GLvoid *)(offsetof(draw_call, data) + sizeof(float) * 4 * i));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  for (int i = 1; i < 11; i++)
    glVertexAttribDivisor(i, 1);

  glBindVertexArray(0);

  // GLuint frameb;
  // glGenFramebuffers(1, &frameb);
  // glBindFramebuffer(GL_FRAMEBUFFER, frameb);
  // GLuint text;
  // glGenTextures(1, &text);
  // glBindTexture(GL_TEXTURE_2D, text);

  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, text, 0);

  // GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  // glDrawBuffers(1, DrawBuffers);

  // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  //   exit(1);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  while (!glfwWindowShouldClose(window))
  {
    processInput(window);
    if (draw_calls_arr.size())
    {
      update_draw_calls();
      glClear(GL_DEPTH_BUFFER_BIT);

      glUseProgram(main_prog);
      glBindVertexArray(VAO);

      GLuint tmp_val = glGetUniformLocation(main_prog, "prim_calls_sum");

      glUniform1i(tmp_val, draw_calls_arr.size());

      glDrawArraysInstanced(GL_POINTS, 0, 4096, draw_calls_arr.size());

      draw_calls_arr.clear();
      glfwSwapBuffers(window);
    }
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // unsigned char *a = (unsigned char *)malloc(12288);
    // glReadPixels(0, 0, 16, 256, GL_RGB, GL_UNSIGNED_BYTE, a);

    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(main_prog);

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void check_compile(GLuint obj)
{
  GLint status;
  glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
  {
    GLint length;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(length);
    glGetShaderInfoLog(obj, length, NULL, &log[0]);
    std::cerr << &log[0];
    throw std::runtime_error("Compilation error");
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_M && action == GLFW_PRESS)
  {
    float tmpp[3] = {rand() % 150 / 10.0f, rand() % 150 / 10.0f, rand() % 150 / 10.0f};
    std::cout << "Adding new point at: (" << tmpp[0] << ", " << tmpp[1] << ", " << tmpp[2] << ")" << std::endl;

    draw_call *new_point_call = new draw_call({
        .type = CALL_POINT_TYPE,
        .color = {rand() % 255, rand() % 255, rand() % 255},
        .trans_mat = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f},
        .data = {tmpp[0], tmpp[1], tmpp[2], 1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    });
    draw_calls_arr.push_back(*new_point_call);
  }
  else if (key == GLFW_KEY_N && action == GLFW_PRESS)
  {
    float tmpp[9];
    for (int i = 0; i < 9; i++)
      tmpp[i] = rand() % 1500 / 100.0f;

    std::cout << "Adding new triangle at: ";
    for (int i = 0; i < 3; i++)
    {
      std::cout << "(" << tmpp[i * 3] << ", " << tmpp[i * 3 + 1] << ", " << tmpp[i * 3 + 2] << ") ";
    }
    std::cout << std::endl;

    draw_call *new_point_call = new draw_call({
        .type = CALL_POLYGON_TYPE,
        .color = {rand() % 255, rand() % 255, rand() % 255},
        .trans_mat = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f},
        .data = {tmpp[0], tmpp[1], tmpp[2], 1.0f, tmpp[3], tmpp[4], tmpp[5], 1.0f, tmpp[6], tmpp[7], tmpp[8], 1.0f, 0.5f, 0.0f, 0.0f, 0.0f},
    });
    draw_calls_arr.push_back(*new_point_call);
  }
  else if (key == GLFW_KEY_P && action == GLFW_PRESS)
  {
    float tmpp[12];
    for (int i = 0; i < 12; i++)
      tmpp[i] = rand() % 1500 / 100.0f;

    std::cout << "Adding new pyramid at: ";
    for (int i = 0; i < 4; i++)
    {
      std::cout << "(" << tmpp[i * 3] << ", " << tmpp[i * 3 + 1] << ", " << tmpp[i * 3 + 2] << ") ";
    }
    std::cout << std::endl;

    draw_call *new_point_call = new draw_call({
        .type = CALL_POLYPYR_TYPE,
        .color = {rand() % 255, rand() % 255, rand() % 255},
        .trans_mat = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f},
        .data = {tmpp[0], tmpp[1], tmpp[2], 1.0f, tmpp[3], tmpp[4], tmpp[5], 1.0f, tmpp[6], tmpp[7], tmpp[8], 1.0f, tmpp[9], tmpp[10], tmpp[11], 1.0f},
    });
    draw_calls_arr.push_back(*new_point_call);
  }
  else if (key == GLFW_KEY_C && action == GLFW_PRESS)
  {
    glClear(GL_COLOR_BUFFER_BIT);
  }
}

void update_draw_calls()
{
  glBindBuffer(GL_ARRAY_BUFFER, dc_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(draw_call) * draw_calls_arr.size(), &draw_calls_arr[0], GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, dc_vbo);
}