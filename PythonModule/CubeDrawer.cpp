#include "CubeDrawer.h"

const struct ParseFuncs CubeDrawer::parse_funcs[] = {(struct ParseFuncs){PyTuple_Size, PyTuple_GetItem},
                                                     (struct ParseFuncs){PyList_Size, PyList_GetItem}};

PyObject *CubeDrawer::py_exception = PyErr_NewException("ledcd.CubeDrawer", NULL, NULL);

CubeDrawer::CubeDrawer(double brightness, bool sync, int mat_thread_num) : is_sync(sync)
{
    transform_list.push_back(new struct Transform);

    int fd = shm_open("VirtualCubeSHMemmory", O_RDWR, 0);
    if (fd < 0)
    {
        THROW_EXP("Was not able to open shared memmory object", )
    }

    shm_buf = (struct ShmBuf *)mmap(NULL, sizeof(struct ShmBuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shm_buf == MAP_FAILED)
    {
        THROW_EXP("Invalid input, values only in range [0, 1] are allowed", )
    }

    std::cout << "Successfuly oppened shared memmory object" << std::endl;

    shm_buf->flags.lock = 0;
    shm_buf->flags.frame_shown = 1;

    cur_brush.brigthness = brightness;
    set_color(255.0);

    openblas_set_num_threads(mat_thread_num);
    std::cout << "Open Blass is running in: " << openblas_get_num_threads() << " threads." << std::endl;
}

int CubeDrawer::parse_num_input(PyObject *input, int req_len)
{
    const struct ParseFuncs *cur_funcs;
    if (PyTuple_Check(input))
        cur_funcs = &parse_funcs[PY_TUPLE_PARSE];
    else if (PyList_Check(input))
        cur_funcs = &parse_funcs[PY_LIST_PARSE];
    else
    {
        THROW_EXP("Invalid input, was expecting tuple or list", -1)
    }

    cur_parsed_args.clear();

    Py_ssize_t inp_len = cur_funcs->get_size(input);
    if (req_len == 0 ? 0 : inp_len != req_len)
    {
        char tmp_str[86];
        sprintf(tmp_str, "Invalid input, was expecting object with size: %d, but %d elements were given", req_len, inp_len);
        THROW_EXP(tmp_str, -1)
    }

    for (int i = 0; i < inp_len; i++)
    {
        PyObject *tmp_obj = cur_funcs->get_item(input, i);
        if (PyLong_Check(tmp_obj))
            cur_parsed_args.push_back((double)PyLong_AsLong(tmp_obj));
        else if (PyFloat_Check(tmp_obj))
            cur_parsed_args.push_back(PyFloat_AsDouble(tmp_obj));
        else
        {
            THROW_EXP("Invalid input, was expecting tuple with numbers", -1)
        }
    }
    return inp_len;
}

void CubeDrawer::set_brigthness(double b)
{
    if (b < 0 || b > 1)
    {
        THROW_EXP("Invalid input, values only in range [0, 1] are allowed", )
    }
    cur_brush.brigthness = b;
    set_color(cur_brush.rr, cur_brush.gg, cur_brush.bb);
}

void CubeDrawer::set_color(double r, double g, double b)
{
    if (r < 0.0 || r > 255.0 || g < 0.0 || g > 255.0 || b < 0.0 || b > 255.0)
    {
        THROW_EXP("Invalid input, values must be in range [0, 255]", )
    }

    cur_brush.rr = r;
    cur_brush.gg = g;
    cur_brush.bb = b;

    cur_brush.r = (unsigned char)round(cur_brush.rr * cur_brush.brigthness);
    cur_brush.g = (unsigned char)round(cur_brush.gg * cur_brush.brigthness);
    cur_brush.b = (unsigned char)round(cur_brush.bb * cur_brush.brigthness);

    cur_brush.r = cur_brush.rr && !cur_brush.r ? 1 : cur_brush.r;
    cur_brush.g = cur_brush.gg && !cur_brush.g ? 1 : cur_brush.g;
    cur_brush.b = cur_brush.bb && !cur_brush.b ? 1 : cur_brush.b;
}

void CubeDrawer::set_color(double rgb)
{
    set_color(rgb, rgb, rgb);
}

void CubeDrawer::set_color(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;

    set_color(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

PyObject *CubeDrawer::get_cur_color()
{

    return PyTuple_Pack(3,
                        PyLong_FromLongLong(cur_brush.r),
                        PyLong_FromLongLong(cur_brush.g),
                        PyLong_FromLongLong(cur_brush.b));
}

void CubeDrawer::calc_transform(double *cur_vec)
{
    double tmp_vec[4];
    // std::cout << "Input vector: (" << cur_vec[0] << ", " << cur_vec[1] << ", " << cur_vec[2] << ")" << std::endl;
    for (int i = transform_list.size() - 1; i >= 0; i--)
    {
        Transform *cur_trans = transform_list[i];
        double temp_mat[16];
        if (cur_trans->need_recalc)
        {
            cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 4, 4, 4, 1.0, cur_trans->translation, 4, cur_trans->rotation, 4, 0.0, temp_mat, 4);
            cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 4, 4, 4, 1.0, temp_mat, 4, cur_trans->rotation, 4, 0.0, cur_trans->final, 4);
        }

        cblas_dgemv(CblasRowMajor, CblasNoTrans, 4, 4, 1.0, cur_trans->final, 4, cur_vec, 1.0, 0.0, tmp_vec, 1.0);
        memcpy(cur_vec, tmp_vec, sizeof(double) << 2);
    }
    // std::cout << "Output vector: (" << cur_vec[0] << ", " << cur_vec[1] << ", " << cur_vec[2] << ")" << std::endl;
}

void CubeDrawer::set_pixel(double x, double y, double z)
{
    double cur_vec[4] = {x, y, z, 1.0};
    calc_transform(cur_vec);

    set_pixel_nt(cur_vec[0], cur_vec[1], cur_vec[2]);
}

void CubeDrawer::set_pixel(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;
    set_pixel(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::set_pixel_nt(double x, double y, double z)
{
    // std::cout << "Set pixel at: (" << x << ", " << y << ", " << z << ")" << std::endl;
    int rx = round(x), ry = round(y), rz = round(z);

    if (!CHECK_IN_BOX(rx, ry, rz))
        return;

    memcpy(&back_buf[(((rz << 4) + ry) << 4) + rx].g, &cur_brush.g, 3);
}

void CubeDrawer::set_pixel_nt(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;
    set_pixel_nt(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::push_matrix()
{
    transform_list.push_back(new struct Transform);
}

void CubeDrawer::pop_matrix()
{
    if (transform_list.size() == 1) // clear first matrix to default values
    {
        Transform *cur_trans = transform_list[0];
        for (int i = 0; i < 16; i++)
            cur_trans->translation[i] = i % 5 ? 0.0 : 1.0;

        for (int i = 0; i < 16; i++)
            cur_trans->rotation[i] = i % 5 ? 0.0 : 1.0;

        for (int i = 0; i < 16; i++)
            cur_trans->scale[i] = i % 5 ? 0.0 : 1.0;

        for (int i = 0; i < 16; i++)
            cur_trans->final[i] = i % 5 ? 0.0 : 1.0;

        cur_trans->rx = 0.0;
        cur_trans->ry = 0.0;
        cur_trans->rz = 0.0;
        cur_trans->need_recalc = false;
    }
    else
    {
        delete transform_list.back();
        transform_list.pop_back();
    }
}

void CubeDrawer::translate(double x, double y, double z)
{
    double *cur_trans = transform_list.back()->translation;

    cur_trans[3] += x;
    cur_trans[7] += y;
    cur_trans[11] += z;

    transform_list.back()->need_recalc = true;
}

void CubeDrawer::translate(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;

    translate(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::rotate(double x, double y, double z)
{
    double *cur_trans = transform_list.back()->rotation;
    Transform *trans_obj = transform_list.back();

    trans_obj->rx += x;
    trans_obj->ry += y;
    trans_obj->rz += z;

    cur_trans[0] = cos(trans_obj->rz) * cos(trans_obj->ry);
    cur_trans[1] = cos(trans_obj->rz) * sin(trans_obj->ry) * sin(trans_obj->rx) - sin(trans_obj->rz) * cos(trans_obj->rx);
    cur_trans[2] = cos(trans_obj->rz) * sin(trans_obj->ry) * cos(trans_obj->rx) + sin(trans_obj->rz) * sin(trans_obj->rx);
    cur_trans[3] = sin(trans_obj->rz) * cos(trans_obj->ry);
    cur_trans[4] = sin(trans_obj->rz) * sin(trans_obj->ry) * sin(trans_obj->rx) + cos(trans_obj->rz) * cos(trans_obj->rx);
    cur_trans[5] = sin(trans_obj->rz) * sin(trans_obj->ry) * cos(trans_obj->rx) + cos(trans_obj->rz) * sin(trans_obj->rx);
    cur_trans[6] = -sin(trans_obj->ry);
    cur_trans[7] = cos(trans_obj->ry) * sin(trans_obj->rx);
    cur_trans[8] = cos(trans_obj->ry) * cos(trans_obj->rx);

    trans_obj->need_recalc = true;
}
void CubeDrawer::rotate(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;

    rotate(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::scale(double x, double y, double z)
{
    double *cur_trans = transform_list.back()->scale;

    cur_trans[0] += x;
    cur_trans[5] += y;
    cur_trans[10] += z;

    transform_list.back()->need_recalc = true;
}

void CubeDrawer::scale(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;

    scale(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::clear(double r, double g, double b)
{
    if (r < 0.0 || r > 255.0 || g < 0.0 || g > 255.0 || b < 0.0 || b > 255.0)
    {
        THROW_EXP("Invalid input, values must be in range [0, 255]", )
    }

    unsigned char tmp_col_arr[3] = {(unsigned char)round(g * cur_brush.brigthness),
                                    (unsigned char)round(r * cur_brush.brigthness),
                                    (unsigned char)round(b * cur_brush.brigthness)};

    for (int i = 0; i < 4096; i++)
        memcpy(&back_buf[i], tmp_col_arr, 3);
}

void CubeDrawer::clear(double rgb)
{
    clear(rgb, rgb, rgb);
}

void CubeDrawer::clear(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;

    clear(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::show()
{
    while (shm_buf->flags.lock || (is_sync && !shm_buf->flags.frame_shown))
        ;

    shm_buf->flags.lock = 1;
    memcpy(shm_buf->buf, back_buf, 12288);
    shm_buf->flags.frame_shown = 0;
    shm_buf->flags.lock = 0;
}

bool CubeDrawer::line_box_intr(double *p1, double *p2)
{
    double line[3] = {p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2]};
    // COUT_VECTOR("Line", line)

    for (int i = 0; i < 6; i++)
    {
        double *normal = &normal_list[i * 3];

        // COUT_VECTOR("Current normal", normal)
        // COUT_VECTOR("Current coord", (&coord_list[i < 3 ? 0 : 3]))

        double nor_l = cblas_ddot(3, normal, 0, line, 0);

        if (abs(nor_l) < EPSILON)
            continue;
        // double len = sqrt(line[0] * line[0] + line[1] * line[1] + line[2] * line[2]);

        double t = (cblas_ddot(3, normal, 0, &coord_list[i < 3 ? 0 : 3], 0) - cblas_ddot(3, normal, 0, p1, 0)) / nor_l;

        double tmp[3];

        memcpy(tmp, p1, sizeof(double) * 3);

        tmp[0] += line[0] * t;
        tmp[1] += line[1] * t;
        tmp[2] += line[2] * t;

        if (CHECK_P_IN_BOX(tmp))
        {
            memcpy(p1, tmp, sizeof(double) * 3);
            // COUT_VECTOR("New point", p1)
            return true;
        }
    }

    return false;
}

bool CubeDrawer::optimise_line_points(double *p1, double *p2)
{
    bool p1_in = CHECK_P_IN_BOX(p1), p2_in = CHECK_P_IN_BOX(p2);

    if (p1_in && p2_in)
        return true;
    else if (!p1_in && !p2_in)
    {
        if (!line_box_intr(p1, p2))
            return false;
        line_box_intr(p2, p1);
    }
    else
    {
        if (p1_in)
        {
            double *tmp;
            tmp = p2;
            p2 = p1;
            p1 = tmp;
        }
        line_box_intr(p1, p2);
    }
    return true;
}

void CubeDrawer::line(double x1, double y1, double z1, double x2, double y2, double z2)
{
    double p1[3] = {x1, y1, z1};
    double p2[3] = {x2, y2, z2};

    calc_transform(p1);
    calc_transform(p2);

    if (!optimise_line_points(p1, p2))
        return;

    double line[3] = {p2[0] - p1[0],
                      p2[1] - p1[1],
                      p2[2] - p1[2]};
    double len = sqrt(line[0] * line[0] + line[1] * line[1] + line[2] * line[2]);

    if (len < EPSILON)
    {
        set_pixel(p1[0], p1[1], p1[2]);
        return;
    }

    double cur_point = 0.0;

    while (cur_point <= len)
    {
        double t = cur_point / len;
        set_pixel_nt(p1[0] + t * line[0], p1[1] + t * line[1], p1[2] + t * line[2]);
        cur_point += line_res;
    }
    set_pixel_nt(p2[0], p2[1], p2[2]);
}

void CubeDrawer::line(PyObject *input1, PyObject *input2)
{
    if (parse_num_input(input1, 3) < 0)
        return;
    double tmp[3];
    memcpy(tmp, &cur_parsed_args[0], sizeof(double) * 3);

    if (parse_num_input(input2, 3) < 0)
        return;

    line(tmp[0], tmp[1], tmp[2], cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::line(PyObject *input)
{
    if (parse_num_input(input, 6) < 0)
        return;

    line(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2], cur_parsed_args[3], cur_parsed_args[4], cur_parsed_args[5]);
}