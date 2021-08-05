#include <iostream>
#include <exception>

#include <chrono>

#include <cmath>

#include <fstream>
#include <thread>
#include <mutex>
#include <list>
#include <vector>
#include <python3.9/Python.h>
#include <stdint.h>

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>

#include <cblas.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define VIRT_CUBE

#define DEBUG_MODE
#ifdef DEBUG_MODE
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

#ifdef VIRT_CUBE

extern "C"
{
#include <wsserver/ws.h>
}

#else

#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

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
    std::cout << premsg << ": (" << v[0] << ", " << v[1] << ", " << v[2] << ")" << std::endl;

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

#define EPSILON 0.00001
#define DEF_LINEW 0.5

struct Brush
{
    unsigned char g;
    unsigned char r;
    unsigned char b;
    double rr;
    double gg;
    double bb;
    double brigthness;
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
    CALL_POLYPYR_TYPE = 2,
    CALL_LINE_TYPE = 3,
    CALL_CIRCLE_TYPE = 4,
    CALL_FCIRCLE_TYPE = 5,
    CALL_SPHERE_TYPE = 6,
    CALL_FSPHERE_TYPE = 7,
};

struct DrawCall
{
    int type;
    Pixel color;
    float data[16];
};

struct Transform
{
    double translation[16] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
    double rotation[16] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
    double scale[16] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
    double final[16] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
    double rx = 0.0;
    double ry = 0.0;
    double rz = 0.0;
    bool need_recalc = false;
};

class CubeDrawer
{
private:
    Brush cur_brush;
    Pixel back_buf[4096];
    long min_show_delay = 0;
    std::vector<double> cur_parsed_args;
    std::vector<Transform *> transform_list;

    long prev_show_time;
    long min_frame_delay;
    static std::mutex mutex_;

#ifndef VIRT_CUBE
    ShmBuf *shm_buf;
#endif

    int parse_num_input(PyObject *input, int req_len = 0);
    ParseFuncs parse_funcs[2] = {(ParseFuncs){PyTuple_Size, PyTuple_GetItem}, (ParseFuncs){PyList_Size, PyList_GetItem}};
    PyObject *py_exception = PyErr_NewException("ledcd.CubeDrawer", NULL, NULL);
    void apply_transforms(double *cur_vec);

    //opengl
    GLuint vbo, vao, dc_vbo; // main vbo/vao and draw calls data vbo
    GLuint main_prog;
    GLuint pix_buf;

    GLfloat gl_vertices[12288]; // static vertices
    std::vector<DrawCall> draw_calls_arr;

    GLFWwindow *context;

    void init_gl();
    bool check_compile(GLuint obj);
    void render_texture();
    void pool_events();
    //

    CubeDrawer(double brightness = 1.0, bool sync = false, int fps_cap = 70);
    ~CubeDrawer(){};

public:
    static CubeDrawer &get_obj();

    CubeDrawer(CubeDrawer &other) = delete;
    void operator=(const CubeDrawer &) = delete;

#ifdef VIRT_CUBE
    std::list<int> virt_fds;
    int _get_virt_amount_();
    bool wait_cube = true;
#endif
    bool is_sync;
    double delta_time;

    PyObject *get_cur_color();

    void push_matrix();
    void pop_matrix();

    void translate(double x, double y, double z);
    void translate(PyObject *input);

    void rotate(double x, double y, double z);
    void rotate(PyObject *input);

    void scale(double x, double y, double z);
    void scale(PyObject *input);

    void clear(double r, double g, double b);
    void clear(double rgb = 0.0);
    void clear(PyObject *input);

    void show();

    void set_fps_cap(int fps);

    //
    void set_brigthness(double b);
    void set_brigthness(int b);

    void set_color(int r, int g, int b);
    void set_color(int rgb);

    void set_color(double r, double g, double b);
    void set_color(double rgb);

    void set_color(PyObject *input);
    //

    void point(double x, double y, double z, double line_width = DEF_LINEW - EPSILON);
    void line(double x1, double y1, double z1, double x2, double y2, double z2, double line_width = DEF_LINEW - EPSILON);
};