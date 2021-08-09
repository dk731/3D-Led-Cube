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

#define EPSILON 0.00001f
#define DEF_LINEW 0.5f
#define DEF_ZHEIGHT 0.5f

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
    float translation[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float rotation[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float scale[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float local_final[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float final[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    float rx = 0.0f;
    float ry = 0.0f;
    float rz = 0.0f;

    bool local_recalc = false;
    bool recalc = true;

    void update_local()
    {
        float temp_mat[16];
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 4, 4, 4, 1.0f, translation, 4, rotation, 4, 0.0f, temp_mat, 4);
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 4, 4, 4, 1.0f, temp_mat, 4, scale, 4, 0.0f, local_final, 4);
        local_recalc = false;
        recalc = true;
    }

    void update_global(Transform *prev)
    {
        if (local_recalc)
            update_local();

        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 4, 4, 4, 1.0f, local_final, 4, prev->local_final, 4, 0.0f, final, 4);
        recalc = false;
    }
};

class CubeDrawer
{
private:
    Brush cur_brush;
    Pixel back_buf[4096];
    long min_show_delay = 0;
    std::vector<float> cur_parsed_args;
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
    void apply_transforms(float *cur_vec);

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
    void clear_draw_call_buf();
    //

    CubeDrawer(float brightness = 1.0, bool sync = false, int fps_cap = 70);
    ~CubeDrawer(){};

    // OpenGL Renderer API Binds
    void apoint(float x, float y, float z, float line_width = DEF_LINEW);
    void apoly(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float z_height = DEF_ZHEIGHT);
    void apoly_pyr(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
    void aline(float x1, float y1, float z1, float x2, float y2, float z2, float line_width = DEF_LINEW);
    void acircle(float *model_mat, float rx, float ry, bool filled = true, float z_height = DEF_ZHEIGHT, float line_width = DEF_LINEW);
    void asphere(float *model_mat, float rx, float ry, float rz, bool filled = true, float line_width = DEF_LINEW);
    // \OpenGL Renderer API Binds

    static void err_clb(int i);

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
    float delta_time;

    PyObject *get_cur_color();

    void push_matrix();
    void pop_matrix();

    void translate(float x, float y, float z);
    void translate(PyObject *input);

    void rotate(float x, float y, float z);
    void rotate(PyObject *input);

    void scale(float x, float y, float z);
    void scale(PyObject *input);

    void clear(float r, float g, float b);
    void clear(float rgb = 0.0);
    void clear(PyObject *input);

    void show();

    void set_fps_cap(int fps);

    //
    void set_brigthness(float b);
    void set_brigthness(int b);

    void set_color(int r, int g, int b);
    void set_color(int rgb);

    void set_color(float r, float g, float b);
    void set_color(float rgb);

    void set_color(PyObject *input);
    //

    //// User friendly API Calls overloads
    // CALL_POINT_TYPE
    void point(float x, float y, float z);
    void point(PyObject *p); // tuple with 3 values
    void filled_sphere(float x, float y, float z, float r);
    void filled_sphere(PyObject *p, float r);

    // CALL_POLYGON_TYPE
    void poly(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float height = DEF_ZHEIGHT);
    void poly(PyObject *p1, PyObject *p2, PyObject *p3, float height = DEF_ZHEIGHT);

    // CALL_POLYPYR_TYPE
    void poly_pyr(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
    void poly_pyr(PyObject *p1, PyObject *p2, PyObject *p3, PyObject *p4);

    // CALL_LINE_TYPE
    void line(float x1, float y1, float z1, float x2, float y2, float z2, float line_width = DEF_LINEW);
    void line(PyObject *p1, PyObject *p2, float line_width = DEF_LINEW);
    void cylinder(float x, float y, float z, float r, float height);
    void cylinder(PyObject *p1, PyObject *p2, float r, float height);
    void filled_circle(float x, float y, float z, float r);
    void filled_circle(PyObject *p, float r);

    // CALL_CIRCLE_TYPE
    void circle(float x, float y, float z, float r, float line_width = DEF_LINEW);
    void circle(PyObject *p, float r, float line_width = DEF_LINEW);
    // void circle(float x, float y, float z, float rx, float ry, float line_width = DEF_LINEW);
    void circle(PyObject *p, PyObject *r, float line_width = DEF_LINEW);

    // CALL_FCIRCLE_TYPE
    void filled_circle(float x, float y, float z, float rx, float ry);
    void filled_circle(PyObject *p, PyObject *r);
    void cylinder(float x, float y, float z, float rx, float ry, float height);
    void cylinder(PyObject *p, PyObject *r, float height);

    // CALL_FCIRCLE_TYPE
    void sphere(float x, float y, float z, float r, float line_width = DEF_LINEW);
    void sphere(float x, float y, float z, float rx, float ry, float rz, float line_width = DEF_LINEW);
    void sphere(PyObject *p, PyObject *r, float line_width = DEF_LINEW);

    // CALL_FSPHERE_TYPE
    void filled_sphere(float x, float y, float z, float rx, float ry, float rz);
    void filled_sphere(PyObject *p, PyObject *r);
    //// \User friendly API Calls overloads
};