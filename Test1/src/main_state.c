#include <main_state.h>
#include <glad/glad.h>
#include <math.h>

#include <rafgl.h>


typedef struct vec2
{
    double x;
    double y;
} vec2_t;

static inline vec2_t vec2_add(vec2_t vec1, vec2_t vec2)
{
    vec2_t add_result;
    add_result.x = vec1.x + vec2.x;
    add_result.y = vec1.y + vec2.y;
    return add_result;
}
static inline vec2_t vec2_sub(vec2_t vec1, vec2_t vec2)
{
    vec2_t sub_result;
    sub_result.x = vec1.x - vec2.x;
    sub_result.y = vec1.y - vec2.y;
    return sub_result;
}
static inline vec2_t vec2_mul(vec2_t vec, double scale)
{
    vec2_t mul_result;
    mul_result.x = vec.x * scale;
    mul_result.y = vec.y * scale;
    return vec;
}

static inline vec2_t vec2_invert(vec2_t vec)
{
    return vec2_mul(vec, -1);
}

double vec2_angle(vec2_t vec)
{
    return atan2(vec.y,vec.x);
}
double vec2_len(vec2_t vec)
{
    return sqrt(pow(vec.x,2) + pow(vec.y,2));
}
double vec2_dot(vec2_t vec1,vec2_t vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y;
}

double vec2_dist(vec2_t vec1,vec2_t vec2)
{
    return vec2_len(vec2_sub(vec1,vec2));
}


rafgl_raster_t raster;
rafgl_texture_t texture;
int size_width = 1280;
int size_height = 720;

rafgl_pixel_rgb_t color_TL;
rafgl_pixel_rgb_t color_TR;
rafgl_pixel_rgb_t color_BL;
rafgl_pixel_rgb_t color_BR;

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    rafgl_raster_init(&raster,size_width, size_height);
    rafgl_texture_init(&texture);
    color_TL.rgba = rafgl_RGB(0,0,100);
    color_TR.rgba = rafgl_RGB(100,50,150);
    color_BL.rgba = rafgl_RGB(0,0,200);
    color_BR.rgba = rafgl_RGB(100,50,250);
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    rafgl_pixel_rgb_t lt = rafgl_lerppix(color_TL,color_TR, rafgl_clampf(game_data->mouse_pos_x/size_width,0.0f,0.9f));
    rafgl_pixel_rgb_t lb = rafgl_lerppix(color_BL,color_BR, rafgl_clampf(game_data->mouse_pos_x/size_width,0.0f,0.9f));
    rafgl_pixel_rgb_t colour = rafgl_lerppix(lt,lb, rafgl_clampf(game_data->mouse_pos_y/size_height,0.0f,0.9f));
    for(int y =0 ;y < size_height; y++)
    {
        for(int x =0 ;x < size_width; x++)
        {
            pixel_at_m(raster,x,y) = colour;
        } 
    }
    rafgl_raster_draw_circle(&raster,rafgl_clampi((int)game_data->mouse_pos_x,30,size_width-31),rafgl_clampi((int)game_data->mouse_pos_y,30,size_height-31),30,rafgl_RGB(40,100,30));
}

void main_state_render(GLFWwindow *window, void *args)
{
    rafgl_texture_load_from_raster(&texture,&raster);
    rafgl_texture_show(&texture,0);
}

void main_state_cleanup(GLFWwindow *window, void *args)
{

}
