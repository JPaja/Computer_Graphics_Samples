#include <main_state.h>
#include <glad/glad.h>
#include <math.h>

#include <rafgl.h>

vec3_t object_colour = RAFGL_BLUE;
vec3_t light_colour = RAFGL_WHITE;
vec3_t light_direction = vec3m(-1.0f, -1.0f, -1.0f);
vec3_t ambient = RAFGL_GRAYX(0.16f);

float fov = 90.0f;

vec3_t camera_position = vec3m(0.0f, 0.0f, 3.5f);
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

void portal_init(portal_t * portal, float width, float height)
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
}



void shader_init(shader_data_t * shader_data, char * name)
{
    rafgl_log(RAFGL_INFO, "Compiling shader_data: %s\n", name);
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
}

void shader_update(shader_data_t * shader_data, mat4_t model)
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
static portal_t portal;

static rafgl_raster_t doge_raster;
static rafgl_texture_t doge_tex;

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    rafgl_log_fps(RAFGL_TRUE);
    object_colour = vec3(0.8f, 0.40f, 0.0f);


    rafgl_meshPUN_init(&mesh);
    rafgl_meshPUN_load_from_OBJ(&mesh, "res/models/portal.obj");

    shader_init(&shad, "v8PVD");

    portal_init(&portal,1,2);


    rafgl_raster_load_from_image(&doge_raster, "res/images/img.png");

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
    portal.model_portal1 =  m4_identity();// m4_translation(vec3(2,-4,-4));
    portal.model_portal2 =  m4_translation(vec3(2,0,-10)); //m4_mul(m4_rotation_x(M_PIf  / 2),);
    portal.model_portal2 = m4_mul(portal.model_portal2,  m4_rotation_y(M_PIf));
//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_BACK);
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shad.shader);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, doge_tex.tex_id);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glBindVertexArray(portal.vao);

//    glUniformMatrix4fv(uni_M[current_shader], 1, GL_FALSE, (void*) model.m);
//    glUniformMatrix4fv(uni_VP[current_shader], 1, GL_FALSE, (void*) view_projection.m);
//
//    glUniform3f(uni_object_colour[current_shader], object_colour.x, object_colour.y, object_colour.z);
//    glUniform3f(uni_light_colour[current_shader], light_colour.x, light_colour.y, light_colour.z);
//    glUniform3f(uni_light_direction[current_shader], light_direction.x, light_direction.y, light_direction.z);
//    glUniform3f(uni_ambient[current_shader], ambient.x, ambient.y, ambient.z);
//    glUniform3f(uni_camera_position[current_shader], camera_position.x, camera_position.y, camera_position.z);

    shader_update(&shad,portal.model_portal1);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_update(&shad,portal.model_portal2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void main_state_cleanup(GLFWwindow *window, void *args)
{

}
