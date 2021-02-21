#include "CubeDrawer.h"

CubeDrawer::CubeDrawer(float brightness, bool sync) : py_exception(PyErr_NewException("ledcd.CubeDrawer", NULL, NULL))
{
    // int fd = shm_open("VirtualCubeSHMemmory", O_RDWR, 0);
    // if (fd < 0)
    //     throw std::runtime_error("Was not able to open Shared");

    // shm_buf = (struct ShmBuf *)mmap(NULL, sizeof(struct ShmBuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // if (shm_buf == MAP_FAILED)
    //     throw std::runtime_error("Was not able to mmap shared memmory");
    shm_buf = new (struct ShmBuf);
    shm_buf->flags.frame_ready = 0;
    shm_buf->flags.lock = 0;
    shm_buf->flags.sync = sync;

    cur_brush.brigthness = brightness;
}

void CubeDrawer::set_brigthness(float b)
{
    if (b < 0 || b > 1)
    {
        THROW_EXP(py_exception, "Invalid input, values only in range [0, 1] are allowed")
    }
    cur_brush.brigthness = b;
    set_color(cur_brush.r, cur_brush.g, cur_brush.b);
}

void CubeDrawer::set_color(int r, int g, int b)
{
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
    {
        THROW_EXP(py_exception, "Invalid input, values must be in range [0, 255]")
    }

    cur_brush.r = r * cur_brush.brigthness;
    cur_brush.g = g * cur_brush.brigthness;
    cur_brush.b = b * cur_brush.brigthness;
}

void CubeDrawer::set_color(PyObject *incoming)
{
    if (PyTuple_Check(incoming))
    {
        if (PyTuple_Size(incoming) != 3)
        {
            THROW_EXP(py_exception, "Invalid input, only tuple with size 3 are allowed")
        }

        for (int i = 0; i < 3; i++)
            if (!PyLong_CheckExact(PyTuple_GetItem(incoming, i)))
            {
                THROW_EXP(py_exception, "Invalid input, only integer values are accepted")
            }

        set_color(
            (int)PyLong_AsLong(PyTuple_GetItem(incoming, 0)),
            (int)PyLong_AsLong(PyTuple_GetItem(incoming, 1)),
            (int)PyLong_AsLong(PyTuple_GetItem(incoming, 2)));
    }
    else if (PyList_Check(incoming))
    {
        if (PyList_Size(incoming) != 3)
        {
            THROW_EXP(py_exception, "Invalid input, only integer values are accepted")
        }

        for (int i = 0; i < 3; i++)
            if (!PyLong_CheckExact(PyList_GetItem(incoming, i)))
            {
                THROW_EXP(py_exception, "Invalid input, only integer values are accepted")
            }

        set_color(
            (int)PyLong_AsLong(PyList_GetItem(incoming, 0)),
            (int)PyLong_AsLong(PyList_GetItem(incoming, 1)),
            (int)PyLong_AsLong(PyList_GetItem(incoming, 2)));
    }
    else
    {
        THROW_EXP(py_exception, "Invalid input, only tuple or list with lenght 3 are allowed")
    }
}

int CubeDrawer::foo(int b)
{
    return b;
}