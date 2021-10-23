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
void onopen(websocketpp::connection_hdl hdl)
{
    SLEEP_MICROS(500000);
    CubeDrawer::get_obj().virt_hdls.push_back(hdl);
    std::cout << "Virtual cube connected" << std::endl;
}

void onclose(websocketpp::connection_hdl hdl)
{
    CubeDrawer::get_obj().virt_hdls.remove_if([hdl](std::weak_ptr<void> p)
                                              {
                                                  std::shared_ptr<void> swp = hdl.lock();
                                                  std::shared_ptr<void> sp = p.lock();
                                                  if (swp && sp)
                                                      return swp == sp;
                                                  return false;
                                              });

    std::cout << "Virtual cube disconnected" << std::endl;
}

int CubeDrawer::_get_virt_amount_()
{
    return virt_hdls.size();
}
#endif

CubeDrawer::CubeDrawer(float brightness, bool sync, int fps) : prev_show_time(GET_MICROS()), is_sync(sync), delta_time(0)
{
    init_gl();
    parse_funcs[0].get_size = PyTuple_Size;
    parse_funcs[0].get_item = PyTuple_GetItem;

    parse_funcs[1].get_size = PyList_Size;
    parse_funcs[1].get_item = PyList_GetItem;

    transform_list.push_back(new Transform()); // First matrix not editable
    transform_list[0]->recalc = false;
    transform_list.push_back(new Transform());
#ifdef VIRT_CUBE
    ws_server.clear_access_channels(websocketpp::log::alevel::all);
    ws_server.init_asio();

    ws_server.set_open_handler(&onopen);
    ws_server.set_close_handler(&onclose);

    ws_server.listen(8080);
    ws_server.start_accept();

    std::thread virt_server([]()
                            { CubeDrawer::get_obj().ws_server.run(); });
    virt_server.detach();

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
    cur_brush.brigthness = 1.0f;
    set_color(255);
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
            cur_parsed_args.push_back((float)PyLong_AsLong(tmp_obj));
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

void CubeDrawer::update_matrix()
{
    Transform *lt = transform_list.back();
    if (lt->local_recalc)
        lt->update_local();
    if (lt->recalc)
        lt->update_global(transform_list[transform_list.size() - 2]);
}

void CubeDrawer::apply_transforms(glm::vec4 &cur_vec)
{
    update_matrix();
    cur_vec = transform_list.back()->final * cur_vec;
}

void CubeDrawer::push_matrix()
{
    update_matrix();

    transform_list.push_back(new Transform);
}

void CubeDrawer::pop_matrix()
{
    if (transform_list.size() == 2) // clear first matrix to default values
    {
        Transform *cur_trans = transform_list[1];

        memcpy(&(cur_trans->translation[0])[0], &(transform_list[0]->final[0])[0], sizeof(float) * 16);
        memcpy(&(cur_trans->rotation[0])[0], &(transform_list[0]->final[0])[0], sizeof(float) * 16);
        memcpy(&(cur_trans->scale[0])[0], &(transform_list[0]->final[0])[0], sizeof(float) * 16);
        memcpy(&(cur_trans->final[0])[0], &(transform_list[0]->final[0])[0], sizeof(float) * 16);

        cur_trans->rx = 0.0f;
        cur_trans->ry = 0.0f;
        cur_trans->rz = 0.0f;

        cur_trans->local_recalc = false;
        cur_trans->recalc = false;
    }
    else
    {
        delete transform_list.back();
        transform_list.pop_back();
    }
}

void CubeDrawer::translate(float x, float y, float z)
{
    transform_list.back()->translation[3] += glm::vec4(x, y, z, 0.0f);

    transform_list.back()->local_recalc = true;
}

void CubeDrawer::translate(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;

    translate(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::rotate(float x, float y, float z)
{
    Transform *trans_obj = transform_list.back();
    trans_obj->local_recalc = true;

    trans_obj->rx += x;
    trans_obj->ry += y;
    trans_obj->rz += z;

    trans_obj->rotation[0].x = cos(trans_obj->rx) * cos(trans_obj->ry);
    trans_obj->rotation[0].y = sin(trans_obj->rx) * cos(trans_obj->ry);
    trans_obj->rotation[0].z = -sin(trans_obj->ry);

    trans_obj->rotation[1].x = cos(trans_obj->rx) * sin(trans_obj->ry) * sin(trans_obj->rz) - sin(trans_obj->rx) * cos(trans_obj->rz);
    trans_obj->rotation[1].y = sin(trans_obj->rx) * sin(trans_obj->ry) * sin(trans_obj->rz) + cos(trans_obj->rx) * cos(trans_obj->rz);
    trans_obj->rotation[1].z = cos(trans_obj->ry) * sin(trans_obj->rz);

    trans_obj->rotation[2].x = cos(trans_obj->rx) * sin(trans_obj->ry) * cos(trans_obj->rz) + sin(trans_obj->rx) * sin(trans_obj->rz);
    trans_obj->rotation[2].y = sin(trans_obj->rx) * sin(trans_obj->ry) * cos(trans_obj->rz) - cos(trans_obj->rx) * sin(trans_obj->rz);
    trans_obj->rotation[2].z = cos(trans_obj->ry) * cos(trans_obj->rz);
}
void CubeDrawer::rotate(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;

    rotate(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::scale(float x, float y, float z)
{
    transform_list.back()->scale[0].x *= x;
    transform_list.back()->scale[1].y *= y;
    transform_list.back()->scale[2].z *= z;

    transform_list.back()->local_recalc = true;
}

void CubeDrawer::scale(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;

    scale(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

void CubeDrawer::clear(float r, float g, float b)
{
    if (r < 0.0f || r > 1.0f || g < 0.0f || g > 1.0f || b < 0.0f || b > 1.0f)
        THROW_EXP("Invalid input, values must be in range [0, 1]", )

    draw_calls_arr.push_back({.type = CALL_CLEAR_TYPE,
                              .color = {
                                  .g = (unsigned char)(255.0f * g),
                                  .r = (unsigned char)(255.0f * r),
                                  .b = (unsigned char)(255.0f * b)}});
    if (draw_immediate)
        show();
}

void CubeDrawer::clear(float rgb)
{
    clear(rgb, rgb, rgb);
}

void CubeDrawer::clear(int rgb)
{
    float rgb2 = rgb / 255.0f;
    clear(rgb2, rgb2, rgb2);
}

void CubeDrawer::clear(int r, int g, int b)
{
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
        THROW_EXP("Invalid input, values must be in range [0, 255]", )

    clear(r / 255.0f, g / 255.0f, b / 255.0f);
}

void CubeDrawer::clear(PyObject *input)
{
    if (parse_num_input(input, 3) < 0)
        return;

    if (PyLong_Check(PyTuple_GetItem(input, 0)))
        clear(cur_parsed_args[0] / 255.0f, cur_parsed_args[1] / 255.0f, cur_parsed_args[2] / 255.0f);
    else
        clear(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

// void clear(float r, float g, float b);
// void clear(float rgb = 0.0);
// void clear(int rgb);
// void clear(int r, int g, int b);
// void clear(PyObject *input);

void CubeDrawer::show()
{
    render_texture();
    long long delta = min_frame_delay - (GET_MICROS() - prev_show_time);
    if (delta > 0)
    {
        // std::cout << "Sleeping: " << delta << "   min_del: " << min_frame_delay << "   prev_show_time: " << prev_show_time << "    cur_time: " << GET_MICROS() << std::endl;
        SLEEP_MICROS(delta);
    }

#ifndef SKIP_SHOW
#ifdef VIRT_CUBE
    if (wait_cube)
        while (!virt_hdls.size())
        {
            SLEEP_MICROS(100000);
            prev_show_time = GET_MICROS();
        }
    // Send back_buf to all oppened sockets
    for (auto t = virt_hdls.begin(); t != virt_hdls.end(); t++)
        ws_server.send(*t, (void *)back_buf, 12288, websocketpp::frame::opcode::BINARY);
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
#endif
    delta_time = (GET_MICROS() - prev_show_time) / 1000000.0f;
    prev_show_time = GET_MICROS();
}

void CubeDrawer::set_fps_cap(int fps)
{
    if (!fps)
        min_frame_delay = 0;
    else
        min_frame_delay = (1.0 / (float)fps) * 1000000.0f;

    std::cout << "Changed fps cap to be: " << fps << " fps or " << min_frame_delay << " us or " << min_frame_delay / 1000.0 << " ms" << std::endl;
}

////////// Color
void CubeDrawer::set_brigthness(float b)
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

void CubeDrawer::set_color(float r, float g, float b)
{
    if (r < 0.0f || r > 1.0f || g < 0.0f || g > 1.0f || b < 0.0f || b > 1.0f)
        THROW_EXP("Invalid input, values must be in range [0.0, 1.0]", )

    set_color((int)r * 255, (int)g * 255, (int)b * 255);
}

void CubeDrawer::set_color(float rgb)
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

////////// OpenGL
void CubeDrawer::init_gl()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return;
    }
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

#ifndef DEBUG_VIEW
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#endif

    context = glfwCreateWindow(16, 256, "", NULL, NULL);
    if (context == NULL)
    {
        std::cout << "Failed to create GLFW context" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(context);
    gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress);

    std::cout << glGetString(GL_VERSION) << std::endl;

    // GLenum err = glewInit();
    // if (err != GLEW_OK)
    // {
    //     std::cout << "Failed initializing GLEW, error code: " << err << std::endl;
    //     return;
    // }

#ifndef DYNAMIC_SHADER_INCLUDE
    std::cout << "IMINSHADER" << std::endl;
    std::ifstream in("C:\\Users\\user\\Desktop\\3D-Led-Cube\\src\\shaders\\main.vert");
    std::string tmp_vert = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const char *vert_str = tmp_vert.c_str();
    in.close();

    in = std::ifstream("C:\\Users\\user\\Desktop\\3D-Led-Cube\\src\\shaders\\main.frag");
    std::string tmp_frag = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const char *frag_str = tmp_frag.c_str();
    in.close();
#else
    const char *vert_str = src_shaders_main_vert;
    const char *frag_str = src_shaders_main_frag;
#endif
    GLuint vert_shade = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shade, 1, (const char **)&vert_str, NULL);
    glCompileShader(vert_shade);
    check_compile(vert_shade);

    GLuint frag_shade = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shade, 1, (const char **)&frag_str, NULL);
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

#ifndef DEBUG_VIEW
    glGenFramebuffers(1, &pix_buf);
    glBindFramebuffer(GL_FRAMEBUFFER, pix_buf);
    GLuint text;
    glGenTextures(1, &text);
    glBindTexture(GL_TEXTURE_2D, text);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, text, 0);

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer was not initialized" << std::endl;
        return;
    }
#endif
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    std::thread tmp_t(&CubeDrawer::pool_events, this);
    tmp_t.detach();

    // glViewport(0, 0, 16, 256);

    // DrawCall *new_point_call = new DrawCall({
    //     .type = CALL_SPHERE_TYPE,
    //     .color = {255, 0, 0},
    //     .data = {7.5f, 7.5f, 7.5f, 1.0f, 7.0f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    // });

    // draw_calls_arr.push_back(*new_point_call);
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
        std::cout << std::endl
                  << "Error compiling shader: " << obj << std::endl;
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
#ifndef DEBUG_VIEW
        glBindFramebuffer(GL_FRAMEBUFFER, pix_buf);
        glReadPixels(0, 0, 16, 256, GL_RGB, GL_UNSIGNED_BYTE, back_buf);
#else
        glfwSwapBuffers(context);
#endif
        clear_draw_call_buf();
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

void CubeDrawer::clear_draw_call_buf()
{
    // for (int i = 0; i < draw_calls_arr.size(); i++)
    //     delete &draw_calls_arr[i];
    draw_calls_arr.clear();
}
////////// \Opengl

////////// OpenGL Renderer API Binds
void CubeDrawer::apoint(float *p, float line_width)
{
    draw_calls_arr.push_back({
        .type = CALL_POINT_TYPE,
        .color = {.g = cur_brush.g, .r = cur_brush.r, .b = cur_brush.b},
        .data = {
            p[0], p[1], p[2], line_width / 2.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f},
    });

    if (draw_immediate)
        show();
}

void CubeDrawer::apoly(float *p1, float *p2, float *p3, float z_height)
{
    draw_calls_arr.push_back({
        .type = CALL_POLYGON_TYPE,
        .color = {.g = cur_brush.g, .r = cur_brush.r, .b = cur_brush.b},
        .data = {
            p1[0], p1[1], p1[2], 0.0f,
            p2[0], p2[1], p2[2], 0.0f,
            p3[0], p3[1], p3[2], 0.0f,
            z_height / 2.0f, 0.0f, 0.0f, 0.0f},
    });

    if (draw_immediate)
        show();
}

void CubeDrawer::atetr(float *p1, float *p2, float *p3, float *p4)
{
    draw_calls_arr.push_back({
        .type = CALL_TETR_TYPE,
        .color = {.g = cur_brush.g, .r = cur_brush.r, .b = cur_brush.b},
        .data = {
            p1[0], p1[1], p1[2], 0.0f,
            p2[0], p2[1], p2[2], 0.0f,
            p3[0], p3[1], p3[2], 0.0f,
            p4[0], p4[1], p4[2], 0.0f},
    });

    if (draw_immediate)
        show();
}

void CubeDrawer::aline(float *p1, float *p2, float line_width)
{
    draw_calls_arr.push_back({
        .type = CALL_LINE_TYPE,
        .color = {.g = cur_brush.g, .r = cur_brush.r, .b = cur_brush.b},
        .data = {
            p1[0], p1[1], p1[1], 0.0f,
            p2[0], p2[1], p2[1], 0.0f,
            line_width, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f},
    });

    if (draw_immediate)
        show();
}

void CubeDrawer::acircle(float *model_mat, float *r, bool filled, float z_height, float line_width)
{
    draw_calls_arr.push_back({
        .type = filled ? CALL_FCIRCLE_TYPE : CALL_CIRCLE_TYPE,
        .color = {.g = cur_brush.g, .r = cur_brush.r, .b = cur_brush.b},
    });

    DrawCall *cur_call = &(draw_calls_arr[draw_calls_arr.size() - 1]);

    memcpy(cur_call->data, model_mat, MAT4_SIZE);

    cur_call->data[3] = r[0];
    cur_call->data[7] = r[1];
    cur_call->data[11] = line_width / 2.0f;
    cur_call->data[15] = z_height / 2.0f;

    if (draw_immediate)
        show();
}

void CubeDrawer::asphere(float *model_mat, float *r, bool filled, float line_width)
{
    draw_calls_arr.push_back({
        .type = filled ? CALL_FSPHERE_TYPE : CALL_SPHERE_TYPE,
        .color = {.g = cur_brush.g, .r = cur_brush.r, .b = cur_brush.b},
    });

    DrawCall *cur_call = &(draw_calls_arr[draw_calls_arr.size() - 1]);

    memcpy(cur_call->data, model_mat, MAT4_SIZE);

    cur_call->data[3] = r[0];
    cur_call->data[7] = r[1];
    cur_call->data[11] = r[2];

    cur_call->data[15] = line_width / 2.0f;

    if (draw_immediate)
        show();
}

////////// \OpenGL Renderer API Binds

// void apoint(float x, float y, float z, float line_width = DEF_LINEW);
// void apoly(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float z_height = DEF_ZHEIGHT);
// void apoly_pyr(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
// void aline(float x1, float y1, float z1, float x2, float y2, float z2, float line_width = DEF_LINEW);
// void acircle(float *model_mat, float rx, float ry, bool filled = true, float z_height = DEF_ZHEIGHT, float line_width = DEF_LINEW);
// void asphere(float *model_mat, float rx, float ry, float rz, bool filled = true, float line_width = DEF_LINEW);

//// User friendly API Calls overloads
// CALL_POINT_TYPE
void CubeDrawer::point(float x, float y, float z)
{
    glm::vec4 p(x, y, z, 1.0f);
    apply_transforms(p);
    apoint(glm::value_ptr(p), DEF_LINEW);
}
void CubeDrawer::point(PyObject *p)
{
    if (parse_num_input(p, 3) < 0)
        return;

    point(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2]);
}

// CALL_POLYGON_TYPE
void CubeDrawer::poly(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float height)
{
    glm::vec4 p1(x1, y1, z1, 1.0f), p2(x2, y2, z2, 1.0f), p3(x3, y3, z3, 1.0f);

    apply_transforms(p1);
    apply_transforms(p2);
    apply_transforms(p3);

    apoly(glm::value_ptr(p1), glm::value_ptr(p2), glm::value_ptr(p3), height);
}

void CubeDrawer::poly(PyObject *p1, PyObject *p2, PyObject *p3, float height)
{
    glm::vec4 pp1(1.0f), pp2(1.0f), pp3(1.0f);

    if (parse_num_input(p1, 3) < 0)
        return;
    memcpy(glm::value_ptr(pp1), &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(p2, 3) < 0)
        return;
    memcpy(glm::value_ptr(pp2), &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(p3, 3) < 0)
        return;
    memcpy(glm::value_ptr(pp3), &cur_parsed_args[0], sizeof(float) * 3);

    apply_transforms(pp1);
    apply_transforms(pp2);
    apply_transforms(pp3);

    apoly(glm::value_ptr(pp1), glm::value_ptr(pp2), glm::value_ptr(pp3), height);
}

// CALL_POLYPYR_TYPE
void CubeDrawer::tetr(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4)
{
    glm::vec4 p1(x1, y1, z1, 1.0f), p2(x2, y2, z2, 1.0f), p3(x3, y3, z3, 1.0f), p4(x4, y4, z4, 1.0f);

    apply_transforms(p1);
    apply_transforms(p2);
    apply_transforms(p3);
    apply_transforms(p4);

    atetr(glm::value_ptr(p1), glm::value_ptr(p2), glm::value_ptr(p3), glm::value_ptr(p4));
}

void CubeDrawer::tetr(PyObject *p1, PyObject *p2, PyObject *p3, PyObject *p4)
{
    glm::vec4 pp1(1.0f), pp2(1.0f), pp3(1.0f), pp4(1.0f);

    if (parse_num_input(p1, 3) < 0)
        return;
    memcpy(glm::value_ptr(pp1), &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(p2, 3) < 0)
        return;
    memcpy(glm::value_ptr(pp2), &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(p3, 3) < 0)
        return;
    memcpy(glm::value_ptr(pp3), &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(p4, 3) < 0)
        return;
    memcpy(glm::value_ptr(pp4), &cur_parsed_args[0], sizeof(float) * 3);

    apply_transforms(pp1);
    apply_transforms(pp2);
    apply_transforms(pp3);
    apply_transforms(pp4);

    atetr(glm::value_ptr(pp1), glm::value_ptr(pp2), glm::value_ptr(pp3), glm::value_ptr(pp4));
}

// CALL_LINE_TYPE
void CubeDrawer::line(float x1, float y1, float z1, float x2, float y2, float z2, float line_width)
{
    glm::vec4 p1(x1, y1, z1, 1.0f), p2(x2, y2, z2, 1.0f);
    apply_transforms(p1);
    apply_transforms(p2);

    aline(glm::value_ptr(p1), glm::value_ptr(p2), line_width);
}
void CubeDrawer::line(PyObject *p1, PyObject *p2, float line_width)
{
    glm::vec4 pp1(1.0f), pp2(1.0f);

    if (parse_num_input(p1, 3) < 0)
        return;
    memcpy(glm::value_ptr(pp1), &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(p2, 3) < 0)
        return;
    memcpy(glm::value_ptr(pp1), &cur_parsed_args[0], sizeof(float) * 3);

    apply_transforms(pp1);
    apply_transforms(pp2);

    aline(glm::value_ptr(pp1), glm::value_ptr(pp2), line_width);
}
// CALL_CIRCLE_TYPE
void CubeDrawer::circle(float x, float y, float z, float rx, float ry, float line_width, float thickness)
{
    glm::mat4 local_mat;

    update_matrix();
    memcpy(glm::value_ptr(local_mat), glm::value_ptr(transform_list.back()->final), MAT4_SIZE);

    local_mat = glm::translate(local_mat, glm::vec3(x, y, z));
    local_mat = glm::scale(local_mat, glm::vec3(rx, ry, 1));

    float rr[2] = {rx, ry};
    local_mat = glm::inverse(local_mat);

    acircle(glm::value_ptr(local_mat), rr, false, thickness, line_width);
}
void CubeDrawer::circle(float x, float y, float z, float r)
{
    circle(x, y, z, r, r, 1.0f);
}

void CubeDrawer::circle(PyObject *p, float r, float line_width, float thickness)
{
    if (parse_num_input(p, 3) < 0)
        return;

    circle(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2], r, r, line_width);
}
void CubeDrawer::circle(PyObject *p, PyObject *r, float line_width, float thickness)
{
    float pp[3];

    if (parse_num_input(p, 3) < 0)
        return;
    memcpy(pp, &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(r, 2) < 0)
        return;

    circle(pp[0], pp[1], pp[2], cur_parsed_args[0], cur_parsed_args[1], line_width);
}

// CALL_FCIRCLE_TYPE
void CubeDrawer::filled_circle(float x, float y, float z, float rx, float ry, float thickness)
{
    glm::mat4 local_mat;

    update_matrix();
    memcpy(glm::value_ptr(local_mat), glm::value_ptr(transform_list.back()->final), MAT4_SIZE);

    local_mat = glm::translate(local_mat, glm::vec3(x, y, z));
    local_mat = glm::scale(local_mat, glm::vec3(rx, ry, 1));

    float rr[] = {rx, ry};
    local_mat = glm::inverse(local_mat);

    acircle(glm::value_ptr(local_mat), rr, true, thickness, 0.0f);
}

void CubeDrawer::filled_circle(PyObject *p, PyObject *r, float thickness)
{
    float pp[3];

    if (parse_num_input(p, 3) < 0)
        return;
    memcpy(pp, &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(r, 2) < 0)
        return;

    filled_circle(pp[0], pp[1], pp[2], cur_parsed_args[0], cur_parsed_args[1], thickness);
}
void CubeDrawer::filled_circle(float x, float y, float z, float r)
{
    filled_circle(x, y, z, r, r);
}
void CubeDrawer::filled_circle(PyObject *p, float r)
{
    if (parse_num_input(p, 3) < 0)
        return;

    filled_circle(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2], r, r);
}

// CALL_SPHERE_TYPE
void CubeDrawer::sphere(float x, float y, float z, float rx, float ry, float rz, float line_width)
{
    glm::mat4 local_mat;

    update_matrix();
    memcpy(glm::value_ptr(local_mat), glm::value_ptr(transform_list.back()->final), MAT4_SIZE);

    local_mat = glm::translate(local_mat, glm::vec3(x, y, z));
    local_mat = glm::scale(local_mat, glm::vec3(rx, ry, rz));

    float rr[] = {rx, ry, rz};
    local_mat = glm::inverse(local_mat);

    asphere(glm::value_ptr(local_mat), rr, false, line_width);
}
void CubeDrawer::sphere(float x, float y, float z, float r, float line_width)
{
    sphere(x, y, z, r, r, r, line_width);
}
void CubeDrawer::sphere(PyObject *p, PyObject *r, float line_width)
{
    float pp[3];

    if (parse_num_input(p, 3) < 0)
        return;
    memcpy(pp, &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(r, 3) < 0)
        return;

    sphere(pp[0], pp[1], pp[2], cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[1], line_width);
}

// CALL_FSPHERE_TYPE
void CubeDrawer::filled_sphere(float x, float y, float z, float rx, float ry, float rz)
{
    glm::mat4 local_mat;

    update_matrix();
    memcpy(glm::value_ptr(local_mat), glm::value_ptr(transform_list.back()->final), MAT4_SIZE);

    local_mat = glm::translate(local_mat, glm::vec3(x, y, z));
    local_mat = glm::scale(local_mat, glm::vec3(rx, ry, rz));

    float rr[] = {rx, ry, rz};
    local_mat = glm::inverse(local_mat);

    asphere(glm::value_ptr(local_mat), rr, true, 0.0f);
}

void CubeDrawer::filled_sphere(float x, float y, float z, float r)
{
    filled_sphere(x, y, z, r, r, r);
}

void CubeDrawer::filled_sphere(PyObject *p, float r)
{
    if (parse_num_input(p, 3) < 0)
        return;

    filled_sphere(cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[2], r, r, r);
}

void CubeDrawer::filled_sphere(PyObject *p, PyObject *r)
{
    float pp[3];

    if (parse_num_input(p, 3) < 0)
        return;
    memcpy(pp, &cur_parsed_args[0], sizeof(float) * 3);
    if (parse_num_input(r, 3) < 0)
        return;

    sphere(pp[0], pp[1], pp[2], cur_parsed_args[0], cur_parsed_args[1], cur_parsed_args[1]);
}
//// \User friendly API Calls overloads
