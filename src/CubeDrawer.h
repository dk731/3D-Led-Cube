#include <iostream>
#include <exception>

#include <chrono>

#include <cmath>
#include <mutex>

#include <fstream>
#include <thread>
#include <list>
#include <vector>
#include <map>
#include <Python.h>
#include <stdint.h>

#include <time.h>
#include <string.h>
#include <thread>
#include <stdio.h>
#include <signal.h>

#include <glm/gtx/io.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef DYNAMIC_SHADER_INCLUDE
#include "shaders.h"
#endif

#if defined(VIRTUAL_RENDER)

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#elif defined(RASPI_RENDER)

#include <fcntl.h>
#include <unistd.h>

#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#elif defined(REMOTE_RENDER)

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") //Winsock Library

// #else

// #error "Rendering Option was not defined!"

#endif

#define THROW_EXP(msg, ret)                 \
    {                                       \
        PyErr_SetString(py_exception, msg); \
        return ret;                         \
    }

#define CHECK_IN_BOX(x, y, z) \
    (x >= 0 && x <= 15 && y >= 0 && y <= 15 && z >= 0 && z <= 15)

#define CHECK_P_IN_BOX(p) \
    CHECK_IN_BOX(p[0], p[1], p[2])

#define COUT_VECTOR(premsg, v) \
    std::cout << premsg << ": (" << v[0] << ", " << v[1] << ", " << v[2] << ")" << std::endl

#define COUT_MAT(premsg, m)            \
    std::cout << premsg << std::endl;  \
    for (int i = 0; i < 4; i++)        \
    {                                  \
        for (int j = 0; j < 4; j++)    \
            std::cout << m[i * 4 + j]; \
        std::cout << std::endl;        \
    }

#define GET_MICROS() std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define SLEEP_MICROS(val) std::this_thread::sleep_for(std::chrono::microseconds(val))

#define EPSILON 0.00001f
#define DEF_LINEW1 (0.5f - EPSILON)
#define DEF_LINEW2 (1.0f - EPSILON)
#define DEF_ZHEIGHT (0.5f - EPSILON)

const int MAT4_SIZE = sizeof(float) * 16;

struct Brush
{
    unsigned char g;
    unsigned char r;
    unsigned char b;
    float rr;
    float gg;
    float bb;
    float brigthness;
};

struct Pixel
{
    unsigned char g;
    unsigned char r;
    unsigned char b;
};

struct ShmFlags
{
    uint8_t frame_shown : 1, lock : 1, other : 6;
};

struct ShmBuf
{
    Pixel buf[4096];
    ShmFlags flags;
};

struct ParseFuncs
{
    Py_ssize_t (*get_size)(PyObject *);
    PyObject *(*get_item)(PyObject *, Py_ssize_t);
};

enum ParseFuncType
{
    PY_TUPLE_PARSE,
    PY_LIST_PARSE
};

enum DrawCallTypes
{
    CALL_POINT_TYPE = 0,
    CALL_POLYGON_TYPE = 1,
    CALL_TETR_TYPE = 2,
    CALL_LINE_TYPE = 3,
    CALL_CIRCLE_TYPE = 4,
    CALL_FCIRCLE_TYPE = 5,
    CALL_SPHERE_TYPE = 6,
    CALL_FSPHERE_TYPE = 7,
    CALL_CLEAR_TYPE = 8
};

struct DrawCall
{
    int type;
    Pixel color;
    float data[16];
};

struct Transform
{
    glm::mat4 translation = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    glm::mat4 rotation = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    glm::mat4 scale = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    glm::mat4 local_final = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    glm::mat4 final = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    float rx = 0.0f;
    float ry = 0.0f;
    float rz = 0.0f;

    bool local_recalc = false;
    bool recalc = true;

    void update_local()
    {
        local_final = translation * rotation * scale;
        local_recalc = false;
        recalc = true;
    }

    void update_global(Transform *prev)
    {
        if (local_recalc)
            update_local();

        if (recalc)
            final = prev->final * local_final;

        local_recalc = false;
        recalc = false;
    }
};

class CubeDrawer
{
private:
    bool init_suc;
    bool cleaned;

    Brush cur_brush;
    Pixel back_buf[4096];
    long min_show_delay = 0;
    std::vector<float> cur_parsed_args;
    std::vector<Transform *> transform_list;

    long long prev_show_time;
    long long min_frame_delay;
    static std::mutex mutex_;

#if defined(VIRTUAL_RENDER)
    websocketpp::server<websocketpp::config::asio> ws_server;
#elif defined(REMOTE_RENDER)
    SOCKET raspi_soc;
    struct sockaddr_in server;
#endif

    int parse_num_input(PyObject *input, int req_len = 0);
    ParseFuncs parse_funcs[2];
    PyObject *py_exception = PyErr_NewException("ledcd.CubeDrawer", NULL, NULL);
    void apply_transforms(glm::vec4 &cur_vec);

    //opengl
    GLuint vbo, vao, dc_vbo; // main vbo/vao and draw calls data vbo
    GLuint main_prog;
    GLuint pix_buf;

    GLfloat gl_vertices[12288]; // static vertices
    std::vector<DrawCall> draw_calls_arr;

    GLFWwindow *context;

    void stop_cube();

    void init_gl();
    bool check_compile(GLuint obj);
    void render_texture();
    void pool_events();
    void clear_draw_call_buf();
    void update_matrix();
    //

    CubeDrawer(float brightness = 1.0, int fps_cap = 70);
    ~CubeDrawer();

    // OpenGL Renderer API Binds
    void apoint(float *p, float line_width);
    void apoly(float *p1, float *p2, float *p3, float z_height);
    void atetr(float *p1, float *p2, float *p3, float *p4);
    void aline(float *p1, float *p2, float line_width);
    void acircle(float *model_mat, float *r, bool filled, float z_height, float line_width);
    void asphere(float *model_mat, float *r, bool filled, float line_width);

    void get_mat_offset(float *mat, float x, float y, float z);
    // \OpenGL Renderer API Binds

    static void err_clb(int i);
    bool draw_immediate = false;

public:
    static CubeDrawer &get_obj();

    CubeDrawer(CubeDrawer &other) = delete;
    void operator=(const CubeDrawer &) = delete;
    void _clean_obj();

#ifdef VIRTUAL_RENDER
    int get_virt_amount();
    bool wait_cube = true;
    void set_wait_cube(bool v);
    std::list<websocketpp::connection_hdl> virt_hdls;
#endif

    float delta_time;

    PyObject *get_cur_color();

    void push_matrix();
    void pop_matrix();
    void pop_all_matrix();

    void translate(float x, float y, float z);
    void translate(PyObject *input);

    void rotate(float x, float y, float z);
    void rotate(PyObject *input);

    void scale(float x, float y, float z);
    void scale(PyObject *input);

    void clear(float r, float g, float b);
    void clear(float rgb = 0.0f);
    void clear(int rgb);
    void clear(int r, int g, int b);
    void clear(PyObject *input);

    void show();

    void set_fps_cap(float fps);

    void set_brigthness(float b);
    void set_brigthness(int b);

    void set_color(int r, int g, int b);
    void set_color(int rgb);

    void set_color(float r, float g, float b);
    void set_color(float rgb);

    void set_color(PyObject *input);

    void set_immediate(bool v);

    //// User API Calls overloads

    void point(float x, float y, float z);
    void point(PyObject *p);

    void line(float x1, float y1, float z1, float x2, float y2, float z2, float line_width = DEF_LINEW1);
    void line(PyObject *p1, PyObject *p2, float line_width = DEF_LINEW1);

    void circle(float x, float y, float z, float r, float line_width = DEF_LINEW1, float thickness = DEF_ZHEIGHT);
    void circle(PyObject *p, float r, float line_width = DEF_LINEW1, float thickness = DEF_ZHEIGHT);

    void filled_circle(float x, float y, float z, float r, float thickness = DEF_LINEW1);
    void filled_circle(PyObject *p, float r, float thickness = DEF_LINEW1);

    void ellipse(float x, float y, float z, float rx, float ry, float line_width = DEF_LINEW2, float thickness = DEF_ZHEIGHT);
    void ellipse(PyObject *p, PyObject *r, float line_width = DEF_LINEW2, float thickness = DEF_ZHEIGHT);

    void filled_ellipse(float x, float y, float z, float rx, float ry, float thickness = DEF_ZHEIGHT);
    void filled_ellipse(PyObject *p, PyObject *r, float thickness = DEF_ZHEIGHT);

    void sphere(float x, float y, float z, float r, float line_width = DEF_LINEW2);
    void sphere(PyObject *p, float r, float line_width = DEF_LINEW2);

    void filled_sphere(float x, float y, float z, float r);
    void filled_sphere(PyObject *p, float r);

    void ellipsoid(float x, float y, float z, float rx, float ry, float rz, float line_width = DEF_LINEW2);
    void ellipsoid(PyObject *p, PyObject *r, float line_width = DEF_LINEW2);

    void filled_ellipsoid(float x, float y, float z, float rx, float ry, float rz);
    void filled_ellipsoid(PyObject *p, PyObject *r);

    void poly(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float height = DEF_ZHEIGHT);
    void poly(PyObject *p1, PyObject *p2, PyObject *p3, float height = DEF_ZHEIGHT);

    void tetr(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
    void tetr(PyObject *p1, PyObject *p2, PyObject *p3, PyObject *p4);
};