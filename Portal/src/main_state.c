#include <main_state.h>
#include <glad/glad.h>
#include <math.h>

#include <rafgl.h>

vec3_t object_colour = RAFGL_BLUE;
vec3_t light_colour = RAFGL_WHITE;
vec3_t light_direction = vec3m(-1.0f, -1.0f, -1.0f);
vec3_t ambient = RAFGL_GRAYX(0.16f);

float fov = 90.0f;

vec3_t camera_position = vec3m(0.0f, 0.0f, -4.0f);
vec3_t camera_target = vec3m(0.0f, 0.0f, 0.0f);
vec3_t camera_up = vec3m(0.0f, 1.0f, 0.0f);
vec3_t aim_dir = vec3m(0.0f, 0.0f, -1.0f);

float camera_angle = -M_PIf * 0.5f;
float angle_speed = 0.2f * M_PIf;
float move_speed = 2.4f;

float hoffset = 0;

float time = 0.0f;
int reshow_cursor_flag = 0;
int last_lmb = 0;

mat4_t view, projection, view_projection;

GLuint  quad_vao,quad_vbo;
GLfloat quad_vertices[] =
{
    1.0f , 1.0f,  0.0f,  1.0f, 1.0f,
    -1.0f , 1.0f,  0.0f,  0.0f, 1.0f,
    -1.0f , -1.0f,  0.0f,  0.0f, 0.0f,

    1.0f , 1.0f,  0.0f,  1.0f, 1.0f,
    -1.0f , -1.0f,  0.0f,  0.0f, 0.0f,
    1.0f , -1.0f,  0.0f,  1.0f, 0.0f,
};


#define  objects_len 3
mat4_t objects[objects_len];
float object_movement[objects_len];

typedef struct shader_data
{
    GLuint shader;
    GLuint uni_M;
    GLuint uni_VP;
    GLuint uni_object_colour;
    GLuint uni_light_colour;
    GLuint uni_light_direction;
    GLuint uni_ambient;
    GLuint uni_camera_position;
    GLuint uni_V;
    GLuint uni_P;
    const char * shader_name;
}shader_data_t;

typedef struct _vertex_t
{                       /* offsets      */
    vec3_t position;    /* 0            */
    vec3_t colour;      /* 3 * float    */
    float alpha;        /* 6 * float    */
    float u, v;         /* 7 * float    */
    vec3_t normal;       /* 9 * float    */
} vertex_t;

vertex_t vertex(vec3_t pos, vec3_t col, float alpha, float u, float v, vec3_t normal)
{
    vertex_t vert;

    vert.position = pos;
    vert.colour = col;
    vert.alpha = alpha;
    vert.u = u;
    vert.v = v;
    vert.normal = normal;
    return vert;
}

typedef struct Portal
{
    vertex_t vertices[6];
    GLuint vao, vbo;
    mat4_t model_portal1;
    mat4_t model_portal2;
} portal_t;

typedef struct Tiles
{
    vertex_t vertices[6];
    GLuint vao, vbo;
    int m,n;
    float r;
    mat4_t models[100][100];
} tiles_t;


void portal_init(portal_t * portal,float width, float height)
{

    portal->vertices[0] = vertex(vec3( -1.0f * width, 1.0f * height, 0.0f), RAFGL_RED, 1.0f, 0.0f, 0.0f, RAFGL_VEC3_NEGZ);
    portal->vertices[1] = vertex(vec3( -1.0f * width, -1.0f * height, 0.0f), RAFGL_GREEN, 1.0f, 0.0f, 1.0f, RAFGL_VEC3_NEGZ);
    portal->vertices[2] = vertex(vec3(  1.0f * width, 1.0f * height, 0.0f), RAFGL_GREEN, 1.0f, 1.0f, 0.0f, RAFGL_VEC3_NEGZ);

    portal->vertices[3] = vertex(vec3(  1.0f * width, 1.0f * height, 0.0f), RAFGL_GREEN, 1.0f, 1.0f, 0.0f, RAFGL_VEC3_NEGZ);
    portal->vertices[4] = vertex(vec3( -1.0f * width, -1.0f * height, 0.0f), RAFGL_GREEN, 1.0f, 0.0f, 1.0f, RAFGL_VEC3_NEGZ);
    portal->vertices[5] = vertex(vec3(  1.0f * width, -1.0f * height, 0.0f), RAFGL_BLUE, 1.0f, 1.0f, 1.0f, RAFGL_VEC3_NEGZ);


    glGenVertexArrays(1, &portal->vao);
    glGenBuffers(1, &portal->vbo);

    glBindVertexArray(portal->vao);
    glBindBuffer(GL_ARRAY_BUFFER, portal->vbo);

    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vertex_t), portal->vertices, GL_STATIC_DRAW);

    /* position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) 0);

    /* colour */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) (sizeof(vec3_t)));

    /* alpha */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) (2 * sizeof(vec3_t)));

    /* UV coords */
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) (2 * sizeof(vec3_t) + 1 * sizeof(float)));

    /* normal */
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) (2 * sizeof(vec3_t) + 3 * sizeof(float)));


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    portal->model_portal1 =  m4_identity();// m4_translation(vec3(2,-4,-4));
    portal->model_portal2 =  m4_translation(vec3(2,0,-6)); //m4_mul(m4_rotation_x(M_PIf  / 2),);
    //portal.model_portal2 = m4_mul(portal.model_portal2,  m4_rotation_y(M_PIf));
}


void tiles_init(tiles_t * tile)
{


    tile->m = 10;
    tile->n = 10;
    tile->r =1;

    tile->vertices[0] = vertex(vec3( -1.0f * tile->r, 1.0f * tile->r, 0.0f), RAFGL_RED, 1.0f, 0.0f, 0.0f, RAFGL_VEC3_NEGZ);
    tile->vertices[1] = vertex(vec3( -1.0f * tile->r, -1.0f * tile->r, 0.0f), RAFGL_GREEN, 1.0f, 0.0f, 1.0f, RAFGL_VEC3_NEGZ);
    tile->vertices[2] = vertex(vec3(  1.0f * tile->r, 1.0f * tile->r, 0.0f), RAFGL_GREEN, 1.0f, 1.0f, 0.0f, RAFGL_VEC3_NEGZ);

    tile->vertices[3] = vertex(vec3(  1.0f * tile->r, 1.0f * tile->r, 0.0f), RAFGL_GREEN, 1.0f, 1.0f, 0.0f, RAFGL_VEC3_NEGZ);
    tile->vertices[4] = vertex(vec3( -1.0f * tile->r, -1.0f * tile->r, 0.0f), RAFGL_GREEN, 1.0f, 0.0f, 1.0f, RAFGL_VEC3_NEGZ);
    tile->vertices[5] = vertex(vec3(  1.0f * tile->r, -1.0f * tile->r, 0.0f), RAFGL_BLUE, 1.0f, 1.0f, 1.0f, RAFGL_VEC3_NEGZ);


    glGenVertexArrays(1, &tile->vao);
    glGenBuffers(1, &tile->vbo);

    glBindVertexArray(tile->vao);
    glBindBuffer(GL_ARRAY_BUFFER, tile->vbo);

    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vertex_t), tile->vertices, GL_STATIC_DRAW);

    /* position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) 0);

    /* colour */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) (sizeof(vec3_t)));

    /* alpha */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) (2 * sizeof(vec3_t)));

    /* UV coords */
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) (2 * sizeof(vec3_t) + 1 * sizeof(float)));

    /* normal */
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*) (2 * sizeof(vec3_t) + 3 * sizeof(float)));


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for(int i = 0; i< tile->m; i++)
        for(int j = 0; j< tile->n; j++)
        {
            tile->models[i][j] =  m4_mul(m4_translation(vec3(i,-3,-j)),m4_rotation_x(M_PIf/2));
        }

}


void shader_init(shader_data_t * shader_data, char * name)
{
    //rafgl_log(RAFGL_INFO, "Compiling shader_data: %s\n", name);
    GLuint shader = rafgl_program_create_from_name(name);
    shader_data->shader_name = name;
    shader_data->shader = shader;
    shader_data->uni_M= glGetUniformLocation(shader, "uni_M");
    shader_data->uni_VP = glGetUniformLocation(shader, "uni_VP");
    shader_data->uni_object_colour = glGetUniformLocation(shader, "uni_object_colour");
    shader_data->uni_light_colour = glGetUniformLocation(shader, "uni_light_colour");
    shader_data->uni_light_direction = glGetUniformLocation(shader, "uni_light_direction");
    shader_data->uni_ambient = glGetUniformLocation(shader, "uni_ambient");
    shader_data->uni_camera_position = glGetUniformLocation(shader, "uni_camera_position");
    shader_data->uni_V = glGetUniformLocation(shader, "uni_V");
    shader_data->uni_P = glGetUniformLocation(shader, "uni_P");
}

void shader_update(shader_data_t * shader_data, mat4_t model , mat4_t view_projection)
{
    glUniformMatrix4fv(shader_data->uni_M, 1, GL_FALSE, (void*) model.m);
    glUniformMatrix4fv(shader_data->uni_VP, 1, GL_FALSE, (void*) view_projection.m);

    glUniform3f(shader_data->uni_object_colour, object_colour.x, object_colour.y, object_colour.z);
    glUniform3f(shader_data->uni_light_colour, light_colour.x, light_colour.y, light_colour.z);
    glUniform3f(shader_data->uni_light_direction, light_direction.x, light_direction.y, light_direction.z);
    glUniform3f(shader_data->uni_ambient, ambient.x, ambient.y, ambient.z);
    glUniform3f(shader_data->uni_camera_position, camera_position.x, camera_position.y, camera_position.z);
}
static rafgl_meshPUN_t mesh;
static shader_data_t shad;
static shader_data_t shad2;
static shader_data_t shad3;
static shader_data_t shad_skybox;
static portal_t portal;
static tiles_t tiles;

static rafgl_raster_t doge_raster;
static rafgl_texture_t doge_tex;
rafgl_framebuffer_simple_t fbo;
rafgl_framebuffer_simple_t fbo2;
rafgl_framebuffer_simple_t fbo3;
static rafgl_texture_t skybox;
static rafgl_meshPUN_t skybox_mesh;


void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    fbo = rafgl_framebuffer_simple_create(width,height);
    fbo2 = rafgl_framebuffer_simple_create(width,height);
    fbo3 = rafgl_framebuffer_simple_create(width,height);


    glGenVertexArrays(1,&quad_vao);
    glGenBuffers(1, &quad_vbo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER,quad_vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(quad_vertices),quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(void*)(3*sizeof(GLfloat)));
    //glDisableVertexAttribArray(1);
    //glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    rafgl_texture_load_cubemap_named(&skybox,"sky","jpg");
    rafgl_meshPUN_init(&skybox_mesh);
    rafgl_meshPUN_load_cube(&skybox_mesh, 1.0f);

    rafgl_log_fps(RAFGL_TRUE);
    object_colour = vec3(0.8f, 0.40f, 0.0f);


    rafgl_meshPUN_init(&mesh);
    rafgl_meshPUN_load_from_OBJ(&mesh, "res/models/monkey.obj");

    shader_init(&shad, "v8PVD");
    shader_init(&shad2, "v8PVD2");
    shader_init(&shad3, "PostProcess");
    shader_init(&shad_skybox, "skyboxShader");

    portal_init(&portal,1,2);
    tiles_init(&tiles);

    rafgl_raster_load_from_image(&doge_raster, "res/images/img_1.png");

    /* rezervisemo texture slot.  */
    rafgl_texture_init(&doge_tex);
    rafgl_texture_load_from_raster(&doge_tex, &doge_raster);
    glBindTexture(GL_TEXTURE_2D, doge_tex.tex_id); /* bajndujemo doge teksturu */
    /*  Filtriranje teksture, za slucaj umanjenja (MIN) i uvecanja (MAG)  */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    /*  Sta raditi ako su UV koordinate van 0-1 opsega? Ograniciti na ivicu (CLAMP) ili ponavljati (REPEAT) */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0); /* unbajndujemo doge teksturu */



    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    light_direction = v3_norm(light_direction);


    float add_angle = 2 * M_PIf / 10;
    float angle = M_PIf * 1.5;
    float radius = 15;
    for(int i = 0;  i < objects_len; i++, angle += add_angle)
    {
        objects[i] = m4_translation(vec3(radius *  cos(angle),(i - 5) / 2.0f,radius * sin(angle)));
        objects[i] = m4_mul(objects[i],m4_rotation_y(angle));
        object_movement[i] = i * 0.01f;
    }
}


mat4_t portal_view(mat4_t model)
{
    mat4_t mv = m4_mul(projection, model);
    return mv;
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    time += delta_time;


    if(game_data->is_lmb_down)
    {

        if(reshow_cursor_flag == 0)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }

        float ydelta = game_data->mouse_pos_y - game_data->raster_height / 2;
        float xdelta = game_data->mouse_pos_x - game_data->raster_width / 2;

        if(!last_lmb)
        {
            ydelta = 0;
            xdelta = 0;
        }

        hoffset -= ydelta / game_data->raster_height;
        camera_angle += xdelta / game_data->raster_width;

        glfwSetCursorPos(window, game_data->raster_width / 2, game_data->raster_height / 2);
        reshow_cursor_flag = 1;
    }
    else if(reshow_cursor_flag)
    {
        reshow_cursor_flag = 0;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    last_lmb = game_data->is_lmb_down;

    aim_dir = v3_norm(vec3(cosf(camera_angle), hoffset, sinf(camera_angle)));

    if(game_data->keys_down['W']) camera_position = v3_add(camera_position, v3_muls(aim_dir, move_speed * delta_time));
    if(game_data->keys_down['S']) camera_position = v3_add(camera_position, v3_muls(aim_dir, -move_speed * delta_time));

    vec3_t right = v3_cross(aim_dir, vec3(0.0f, 1.0f, 0.0f));
    if(game_data->keys_down['D']) camera_position = v3_add(camera_position, v3_muls(right, move_speed * delta_time));
    if(game_data->keys_down['A']) camera_position = v3_add(camera_position, v3_muls(right, -move_speed * delta_time));

    if(game_data->keys_down[RAFGL_KEY_ESCAPE]) glfwSetWindowShouldClose(window, GLFW_TRUE);



    if(game_data->keys_down[RAFGL_KEY_SPACE]) camera_position.y += 1.0f * delta_time;
    if(game_data->keys_down[RAFGL_KEY_LEFT_SHIFT]) camera_position.y -= 1.0f * delta_time;


    float aspect = ((float)(game_data->raster_width)) / game_data->raster_height;
    projection = m4_perspective(fov, aspect, 0.1f, 100.0f);

    if(!game_data->keys_down['T'])
    {
        view = m4_look_at(camera_position, v3_add(camera_position, aim_dir), camera_up);
    }
    else
    {
        view = m4_look_at(camera_position, vec3(0.0f, 0.0f, 0.0f), camera_up);
    }

    for(int i = 0; i < objects_len; i++)
    {
        int y = objects[i].m31 += object_movement[i];
        if(y > 2 || y < -2)
            object_movement[i] *= -1;
        objects[i] = m4_mul(objects[i], m4_rotation_y(1.0f * delta_time));
    }

    if(game_data->keys_down['H']) portal.model_portal1.m30 += 0.1f;
    if(game_data->keys_down['J']) portal.model_portal1.m30 += -0.1f;

    if(game_data->keys_down['K']) portal.model_portal2.m30 += 0.1f;
    if(game_data->keys_down['L']) portal.model_portal2.m30 += -0.1f;

    //model
    //if(game_data->keys_down['B'])
    //model = m4_mul(model, m4_rotation_x(1.0f  * delta_time));
    //model = m4_mul(model, m4_rotation_y(1.0f  * delta_time));
    //model = m4_mul(model, m4_rotation_z(1.0f * delta_time));

    //model2 = m4_mul(model2, m4_rotation_z(1.0f * delta_time));

    //portal.model_portal2 = m4_mul(portal.model_portal2,  m4_rotation_y(1.0f * delta_time));

    view_projection = m4_mul(projection, view);

}

void main_state_render(GLFWwindow *window, void *args)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo_id);
    glClearColor(0.0f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glUseProgram(shad2.shader);
    glBindVertexArray(mesh.vao_id);
    for(int i = 0; i < objects_len; i++)
    {
        shader_update(&shad2,objects[i],portal_view(portal.model_portal2));
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo2.fbo_id);
    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthMask(GL_FALSE);

    glUseProgram(shad_skybox.shader);
    glUniformMatrix4fv(shad_skybox.uni_V,1,GL_FALSE,(void*)view.m);
    glUniformMatrix4fv(shad_skybox.uni_P,1,GL_FALSE,(void*)projection.m);

    glBindVertexArray(skybox_mesh.vao_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP,skybox.tex_id);
    glDrawArrays(GL_TRIANGLES, 0, skybox_mesh.vertex_count);

    glDepthMask(GL_TRUE);


    glUseProgram(shad.shader);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fbo.tex_id);

    glBindVertexArray(portal.vao);

    shader_update(&shad,portal.model_portal2,view_projection);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo_id);
    glClearColor(0.0f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shad2.shader);
    glBindVertexArray(mesh.vao_id);
    for(int i = 0; i < objects_len; i++)
    {
        shader_update(&shad2,objects[i],portal_view(portal.model_portal1));
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo2.fbo_id);

    glUseProgram(shad.shader);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fbo.tex_id);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_CLAMP_TO_EDGE);

    glBindVertexArray(portal.vao);
    shader_update(&shad,portal.model_portal1,view_projection);
    glDrawArrays(GL_TRIANGLES, 0, 6);


    glUseProgram(shad2.shader);

    glBindVertexArray(mesh.vao_id);
    for(int i = 0; i < objects_len; i++)
    {
        shader_update(&shad2,objects[i],view_projection);
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);
    }


    glUseProgram(shad.shader);
    //glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,  doge_tex.tex_id);//doge_tex.tex_id);

    glBindVertexArray(tiles.vao);
    for(int i = 0; i< tiles.m; i++)
        for(int j = 0; j< tiles.n; j++)
        {
            shader_update(&shad, tiles.models[i][j], view_projection);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

    glBindTexture(GL_TEXTURE_2D,0);

    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(2);


    glBindVertexArray(quad_vao);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo3.fbo_id);
    glClearColor(0.0f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,1280, 720);
    glUseProgram(shad3.shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo2.tex_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //glActiveTexture(GL_TEXTURE1);
    //GLint var = glad_glGetUniformLocation(shad3.shader, "albedo");
    //glUniform1i(var, 0);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //draw fbo
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo3.fbo_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 1280, 720, 0, 0, 1280, 720,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindVertexArray(0);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glad_glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void main_state_cleanup(GLFWwindow *window, void *args)
{

}
