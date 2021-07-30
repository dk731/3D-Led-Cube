
#include <iostream>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ShaderDrawer.h"

#define CHECK_GL_ERROR(msg)                                              \
  {                                                                      \
    GLuint i = glGetError();                                             \
    if (i != GL_NO_ERROR)                                                \
      std::cout << msg << "Error occured with code: " << i << std::endl; \
  }
void err_callb(int error, const char *description)
{
  std::cout << "Error occured with code: " << error << " ->\n"
            << description << std::endl;
}

int main()
{
  if (glfwInit() == GLFW_FALSE)
  {
    std::cout << "Erorr occured during GLFW initialization" << std::endl;
    return -1;
  }
  glfwSetErrorCallback(err_callb);

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

  GLFWwindow *window = glfwCreateWindow(10, 10, "", NULL, NULL);
  if (window == NULL)
    return -1;
  glfwMakeContextCurrent(window);

  glewExperimental = true;

  GLenum err = glewInit();
  if (err != GLEW_OK)
  {
    std::cout << "glewInit failed: " << glewGetErrorString(err) << std::endl;
    return -1;
  }

  std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;

  ShaderDrawer shader_draw;
  shader_draw.calculate_points();

  unsigned char *data = (unsigned char *)malloc(12288);
  shader_draw.get_buf(data);
  for (int i = 0; i < 12288; i++)
    std::cout << (int)data[i];
  std::cout << std::endl;
  return 0;
}