#include "CubeDrawer.h"

std::mutex CubeDrawer::mutex_;

CubeDrawer &CubeDrawer::get_obj()
{
    CubeDrawer::mutex_.lock();
    static CubeDrawer instance;
    CubeDrawer::mutex_.unlock();
    return instance;
}

#ifdef VIRT_CUBE
void onopen(int fd)
{
    CubeDrawer::get_obj().virt_fds.push_back(fd);
    std::cout << "Virtual cube:" << fd << " connected" << std::endl;
}

void onclose(int fd)
{
    CubeDrawer::get_obj().virt_fds.remove(fd);
    std::cout << "Virtual cube: " << fd << " disconnected" << std::endl;
}

int CubeDrawer::_get_virt_amount_()
{
    return virt_fds.size();
}
#endif

CubeDrawer::CubeDrawer(double brightness, bool sync, int fps) : prev_show_time(0), is_sync(sync), delta_time(0)
{
    init_gl();

    transform_list.push_back(new struct Transform);
#ifdef VIRT_CUBE
    struct ws_events evs;
    evs.onopen = &onopen;
    evs.onclose = &onclose;
    std::cout << "Oppening socket" << std::endl;
    ws_socket(&evs, 8080, 100);
#else
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
#endif

    set_fps_cap(fps);

    memset(back_buf, 0, 12288);
    cur_brush.brigthness = brightness;
    set_color(255.0);
}

int CubeDrawer::parse_num_input(PyObject *input, int req_len)
{
    const struct ParseFuncs *cur_funcs;
    if (PyTuple_Check(input))
        cur_funcs = &parse_funcs[PY_TUPLE_PARSE];
    else if (PyList_Check(input))
        cur_funcs = &parse_funcs[PY_LIST_PARSE];
    else
        THROW_EXP("Invalid input, was expecting tuple or list", -1)

    cur_parsed_args.clear();

    Py_ssize_t inp_len = cur_funcs->get_size(input);
    if (req_len == 0 ? 0 : inp_len != req_len)
    {
        char tmp_str[86];
        sprintf(tmp_str, "Invalid input, was expecting object with size: %d, but %ld elements were given", req_len, inp_len);
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

PyObject *CubeDrawer::get_cur_color()
{

    return PyTuple_Pack(3,
                        PyLong_FromLongLong(cur_brush.r),
                        PyLong_FromLongLong(cur_brush.g),
                        PyLong_FromLongLong(cur_brush.b));
}

void CubeDrawer::apply_transforms(double *cur_vec)
{
    double tmp_vec[4];
    for (int i = transform_list.size() - 1; i >= 0; i--)
    {
        Transform *cur_trans = transform_list[i];
        double temp_mat[16];

        if (cur_trans->need_recalc)
        {
            cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 4, 4, 4, 1.0, cur_trans->translation, 4, cur_trans->rotation, 4, 0.0, temp_mat, 4);
            cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 4, 4, 4, 1.0, temp_mat, 4, cur_trans->scale, 4, 0.0, cur_trans->final, 4);
            cur_trans->need_recalc = false;
        }

        cblas_dgemv(CblasRowMajor, CblasNoTrans, 4, 4, 1.0, cur_trans->final, 4, cur_vec, 1, 0.0, tmp_vec, 1);
        memcpy(cur_vec, tmp_vec, sizeof(double) << 2);
    }
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
    Transform *trans_obj = transform_list.back();
    double *cur_trans = trans_obj->rotation;
    trans_obj->need_recalc = true;

    trans_obj->rx += x;
    trans_obj->ry += y;
    trans_obj->rz += z;

    cur_trans[0] = cos(trans_obj->rz) * cos(trans_obj->ry);
    cur_trans[1] = cos(trans_obj->rz) * sin(trans_obj->ry) * sin(trans_obj->rx) - sin(trans_obj->rz) * cos(trans_obj->rx);
    cur_trans[2] = cos(trans_obj->rz) * sin(trans_obj->ry) * cos(trans_obj->rx) + sin(trans_obj->rz) * sin(trans_obj->rx);
    cur_trans[4] = sin(trans_obj->rz) * cos(trans_obj->ry);
    cur_trans[5] = sin(trans_obj->rz) * sin(trans_obj->ry) * sin(trans_obj->rx) + cos(trans_obj->rz) * cos(trans_obj->rx);
    cur_trans[6] = sin(trans_obj->rz) * sin(trans_obj->ry) * cos(trans_obj->rx) + cos(trans_obj->rz) * sin(trans_obj->rx);
    cur_trans[8] = -sin(trans_obj->ry);
    cur_trans[9] = cos(trans_obj->ry) * sin(trans_obj->rx);
    cur_trans[10] = cos(trans_obj->ry) * cos(trans_obj->rx);
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
    render_texture();
    long delta = min_frame_delay - (GET_MICROS() - prev_show_time);
    if (delta > 0)
        SLEEP_MICROS(delta);

#ifdef VIRT_CUBE
    if (wait_cube)
        while (!virt_fds.size())
        {
            SLEEP_MICROS(100000);
            prev_show_time = GET_MICROS();
        }

    // Send back_buf to all oppened sockets
    for (auto t = virt_fds.begin(); t != virt_fds.end(); t++)
        ws_sendframe(*t, (char *)back_buf, 12288, false, WS_FR_OP_BIN);

#else
    while (shm_buf->flags.lock || (is_sync && !shm_buf->flags.frame_shown))
        usleep(100);
    // {
    //     std::cout << "Lock: " << shm_buf->flags.lock << " shown: " << shm_buf->flags.frame_shown << std::endl;
    // }

    shm_buf->flags.lock = 1;
    memcpy(shm_buf->buf, back_buf, 12288);
    shm_buf->flags.frame_shown = 0;
    shm_buf->flags.lock = 0;
#endif

    delta_time = (GET_MICROS() - prev_show_time) / 1000000.0;
    prev_show_time = GET_MICROS();
}

void CubeDrawer::set_fps_cap(int fps)
{
    if (!fps)
        min_frame_delay = 0;
    else
        min_frame_delay = (1.0 / (double)fps) * 1000000.0;

    std::cout << "Changed fps cap to be: " << fps << " fps or " << min_frame_delay << " us or " << min_frame_delay / 1000.0 << " ms" << std::endl;
}

////////// Color
void CubeDrawer::set_brigthness(double b)
{
    if (b < 0 || b > 1)
        THROW_EXP("Invalid input, values only in range [0, 1] are allowed", )

    cur_brush.brigthness = b;
    set_color(cur_brush.r, cur_brush.g, cur_brush.b);
}

void CubeDrawer::set_brigthness(int b)
{
    if (b < 0 || b > 255)
        THROW_EXP("Invalid input, values only in range [0, 255] are allowed", )

    cur_brush.brigthness = b / 255.0;
    set_color(cur_brush.r, cur_brush.g, cur_brush.b);
}

void CubeDrawer::set_color(int r, int g, int b)
{
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
        THROW_EXP("Invalid input, values must be in range [0, 255]", )

    cur_brush.r = (unsigned char)round(r * cur_brush.brigthness);
    cur_brush.g = (unsigned char)round(g * cur_brush.brigthness);
    cur_brush.b = (unsigned char)round(b * cur_brush.brigthness);

    cur_brush.r = r && !cur_brush.r ? 1 : cur_brush.r;
    cur_brush.g = g && !cur_brush.g ? 1 : cur_brush.g;
    cur_brush.b = b && !cur_brush.b ? 1 : cur_brush.b;
}

void CubeDrawer::set_color(int rgb)
{
    set_color(rgb, rgb, rgb);
}

void CubeDrawer::set_color(double r, double g, double b)
{
    if (r < 0.0 || r > 1.0 || g < 0.0 || g > 1.0 || b < 0.0 || b > 1.0)
        THROW_EXP("Invalid input, values must be in range [0.0, 1.0]", )

    set_color((int)r * 255, (int)g * 255, (int)b * 255);
}

void CubeDrawer::set_color(double rgb)
{
    set_color(rgb, rgb, rgb);
}

void CubeDrawer::set_color(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;
    if (PyLong_Check(PyTuple_GetItem(input, 0)))
        set_color(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
    else
        set_color((int)cur_parsed_args[0], (int)cur_parsed_args[1], (int)cur_parsed_args[2]);
}

////////// \Color

void CubeDrawer::point(double x, double y, double z, double line_width)
{
}

void CubeDrawer::line(double x1, double y1, double z1, double x2, double y2, double z2, double line_width)
{
}

////////// OpenGL
void CubeDrawer::init_gl()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    context = glfwCreateWindow(16, 256, "", NULL, NULL);
    if (context == NULL)
    {
        std::cout << "Failed to create GLFW context" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(context);

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cout << "Failed initializing GLEW, error code: " << err << std::endl;
        return;
    }

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

    main_prog = glCreateProgram();

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

    glDeleteShader(vert_shade);
    glDeleteShader(frag_shade);

    for (int z = 0; z < 16; z++)
        for (int y = 0; y < 16; y++)
            for (int x = 0; x < 16; x++)
            {
                int ind = (x + (y + z * 16) * 16) * 3;
                gl_vertices[ind] = x;
                gl_vertices[ind + 1] = y;
                gl_vertices[ind + 2] = z;
            }

    // Init in/out data
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &dc_vbo);

    glBindVertexArray(vao);
    //// Init vertices data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12288, gl_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
    ////

    //// Init draw calls data
    glBindBuffer(GL_ARRAY_BUFFER, dc_vbo);

    for (int i = 1; i < 7; i++)
        glEnableVertexAttribArray(i);

    int dc_str_size = sizeof(DrawCall);

    glVertexAttribIPointer(1, 1, GL_INT, dc_str_size, (GLvoid *)offsetof(DrawCall, type));
    glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, GL_FALSE, dc_str_size, (GLvoid *)offsetof(DrawCall, color));

    for (int i = 0; i < 4; i++)
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, dc_str_size, (GLvoid *)(offsetof(DrawCall, data) + sizeof(float) * 4 * i));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    for (int i = 1; i < 7; i++)
        glVertexAttribDivisor(i, 1);

    glBindVertexArray(0);
    ////

    //// Init output buffer (texture)

    glGenFramebuffers(1, &pix_buf);
    glBindFramebuffer(GL_FRAMEBUFFER, pix_buf);
    GLuint text;
    glGenTextures(1, &text);
    glBindTexture(GL_TEXTURE_2D, text);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, text, 0);

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer was not initialized" << std::endl;
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    std::thread tmp_t(&CubeDrawer::pool_events, this);
    tmp_t.detach();

    DrawCall *new_point_call = new DrawCall({
        .type = CALL_SPHERE_TYPE,
        .color = {255, 0, 0},
        .data = {7.5f, 7.5f, 7.5f, 1.0f, 7.0f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    });

    draw_calls_arr.push_back(*new_point_call);
}

bool CubeDrawer::check_compile(GLuint obj)
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
        return false;
    }
    return true;
}

void CubeDrawer::render_texture()
{
    if (draw_calls_arr.size())
    {
        // Update draw calls data
        glBindBuffer(GL_ARRAY_BUFFER, dc_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DrawCall) * draw_calls_arr.size(), &draw_calls_arr[0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, dc_vbo);

        // Clear deph buffer
        glClear(GL_DEPTH_BUFFER_BIT);

        glUseProgram(main_prog);
        glBindVertexArray(vao);
        glBindFramebuffer(GL_FRAMEBUFFER, pix_buf);
        // glViewport(0, 0, 16, 768);

        // Setup uniforms
        GLuint tmp_val = glGetUniformLocation(main_prog, "prim_calls_sum");
        glUniform1i(tmp_val, draw_calls_arr.size());

        // Render
        glDrawArraysInstanced(GL_POINTS, 0, 4096, draw_calls_arr.size());

        draw_calls_arr.clear();
        // glfwSwapBuffers(context);

        glBindFramebuffer(GL_FRAMEBUFFER, pix_buf);
        glReadPixels(0, 0, 16, 256, GL_RGB, GL_UNSIGNED_BYTE, back_buf);
    }
}

void CubeDrawer::pool_events()
{
    while (1)
    {
        glfwPollEvents();
        SLEEP_MICROS(500000);
    }
}
////////// \Opengl