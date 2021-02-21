#include <iostream>
#include <exception>

#include <cmath>

#include <vector>
#include <python3.7/Python.h>
#include <stdint.h>

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

// #include "../../OpenBLAS/cblas.h"

#define THROW_EXP(a, b)    \
    PyErr_SetString(a, b); \
    return;

struct Brush
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
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
    uint8_t frame_ready : 1, lock : 1, sync : 1, other : 5;
};

struct ShmBuf
{
    struct Pixel buf[4096];
    struct ShmFlags flags;
};

class CubeDrawer
{
private:
    struct Brush cur_brush;
    struct Pixel back_buf[4096];
    struct ShmBuf *shm_buf;

    PyObject *py_exception;

    bool _is_sync;

public:
    CubeDrawer(float brightness = 0.1f, bool sync = true);
    void set_brigthness(float b);

    void set_color(int r, int g, int b);
    void set_color(PyObject *args);

    void set_pixel(int x, int y, int z);
    void set_pixel(PyObject *args);

    void set_pixel_nt(int x, int y, int z); // put pixel without transformation
    void set_pixel_nt(PyObject *args);

    void push_matrix();
    void pop_matrix();

    void translate(float x, float y, float z);
    void translate(PyObject *args);

    void rotate(float x, float y, float z);
    void rotate(PyObject *args);

    void scale(float x, float y, float z);
    void scale(PyObject *args);

    void clear();
    void show();
};