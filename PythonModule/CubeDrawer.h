#include <iostream>
#include <exception>

#include <chrono>

#include <cmath>

#include <vector>
#include <python3.9/Python.h>
#include <stdint.h>

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>

#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "cblas.h"

#define THROW_EXP(msg, ret)                         \
    PyErr_SetString(CubeDrawer::py_exception, msg); \
    return ret;

#define CHECK_IN_BOX(x, y, z) \
    (x >= 0 && x <= 15 && y >= 0 && y <= 15 && z >= 0 && z <= 15)

#define CHECK_P_IN_BOX(p) \
    CHECK_IN_BOX(p[0], p[1], p[2])

#define EPSILON 0.00001

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
    struct Pixel buf[4096];
    struct ShmFlags flags;
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
    struct Brush cur_brush;
    struct Pixel back_buf[4096];
    struct ShmBuf *shm_buf;

    std::vector<double> cur_parsed_args;

    std::vector<struct Transform *> transform_list;
    int parse_num_input(PyObject *input, int req_len = 0);

    static const struct ParseFuncs parse_funcs[];
    static PyObject *py_exception;
    void calc_transform(double *cur_vec);

    bool line_box_intr(double *p1, double *p2, bool opt_both_points);
    bool optimise_line_points(double *p1, double *p2);

    double line_res = 0.5;
    // 0, -1, 0 / -1, 0, 0, /
    double normal_list[18] = {-1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    double coord_list[6] = {0.0, 0.0, 0.0, 15.0, 15.0, 15.0};
    bool is_sync;

    std::chrono::time_point<std::chrono::system_clock> prev_show_t;

public:
    double delta_time;
    CubeDrawer(double brightness = 0.1, bool sync = true, int mat_thread_num = 4);
    void set_brigthness(double b);

    void set_color(double r, double g, double b);
    void set_color(double rgb);
    void set_color(PyObject *input);

    PyObject *get_cur_color();

    void set_pixel(double x, double y, double z);
    void set_pixel(PyObject *input);

    void set_pixel_nt(double x, double y, double z); // put pixel without transformation
    void set_pixel_nt(PyObject *input);

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

    void line(double x1, double y1, double z1, double x2, double y2, double z2);
    void line(PyObject *input1, PyObject *input2);
    void line(PyObject *input);
};