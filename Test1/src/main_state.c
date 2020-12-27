#include <main_state.h>
#include <glad/glad.h>
#include <math.h>

#include <rafgl.h>


typedef struct vec2
{
    double x;
    double y;
} vec2_t;

static inline vec2_t vec2(double x , double y)
{
    vec2_t result;
    result.x = x;
    result.y = y;
    return result;
}
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

typedef struct player
{
    vec2_t pos;
    int height;
    int width;
}player_t;

typedef struct bonker
{
    int active;
    vec2_t pos;
    int height;
    int width;
    vec2_t mov;
}bonker_t;

#define bonker_count   10

typedef struct table
{
    int height;
    int width;
    bonker_t bonkers[bonker_count];
    int gameOver;
}table_t;

void player_int(player_t * player, double x, double y)
{
    player->pos.x = x;
    player->pos.y = y;
    player->height=120;
    player->width=120;
}

void bonker_int(table_t * table,bonker_t * bonker)
{
    bonker->active=1;
    bonker->height=120;
    bonker->width=120;
    bonker->pos.x = 1;//rand() % (table->width - bonker->width);
    bonker->pos.y = 1;//rand() % (table->height - bonker->height);
    bonker->mov.x = 4 + randf()*14;
    bonker->mov.y = 4 + randf()*14;
}

void table_init(table_t* table, int width, int height)
{
    table->width = width;
    table->height = height;
    for(int i = 0; i <bonker_count ;i++)
        bonker_int(table,&table->bonkers[i]);   
    table->gameOver=0; 
}

void move_bonkers(table_t* table, float delta_time)
{
    for(int i = 0; i < bonker_count; i++){
        if(!table->bonkers[i].active)
            continue;
        table->bonkers[i].pos = vec2_add(table->bonkers[i].pos, vec2_mul(table->bonkers[i].mov,delta_time));
        double x = table->bonkers[i].pos.x;
        double y = table->bonkers[i].pos.y;
        if(y < 0||y>=table->height-table->bonkers[i].height)
        {
            table->bonkers[i].mov.y *= -1.06;
            table->bonkers[i].pos.y = rafgl_clampi(y,0,table->height-table->bonkers[i].height - 1);   
        }
        if(x < 0||x>=table->width-table->bonkers[i].width)
        {
            table->bonkers[i].mov.x *= -1.06;
            table->bonkers[i].pos.x = rafgl_clampi(x,0,table->width - table->bonkers[i].width -1);   
        }
    }
}

int bonker_colision(bonker_t bonker,int x, int y)
{
    return x>= bonker.pos.x && x < bonker.pos.x + bonker.width
        && y>= bonker.pos.y && y < bonker.pos.y + bonker.height;
}




rafgl_raster_t raster;
rafgl_raster_t result_raster;

rafgl_texture_t texture;
int size_width = 1280;
int size_height = 720;

rafgl_pixel_rgb_t color_TL;
rafgl_pixel_rgb_t color_TR;
rafgl_pixel_rgb_t color_BL;
rafgl_pixel_rgb_t color_BR;

table_t table;

rafgl_raster_t game_over_rasters[4];
rafgl_spritesheet_t bonk_spreadsheet;

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    rafgl_raster_init(&raster,size_width, size_height);
    rafgl_raster_init(&result_raster,size_width, size_height);
    rafgl_texture_init(&texture);
    color_TL.rgba = rafgl_RGB(0,0,100);
    color_TR.rgba = rafgl_RGB(100,50,150);
    color_BL.rgba = rafgl_RGB(0,0,200);
    color_BR.rgba = rafgl_RGB(100,50,250);
    table_init(&table,width,height);

    char tile_path[256];
    for(int i = 1; i <= 4; i++){
        sprintf(tile_path,"res/bonk%d.png",i);
        rafgl_raster_load_from_image(&game_over_rasters[i - 1], tile_path);
    }

    rafgl_spritesheet_init(&bonk_spreadsheet,"res/bonk_sprite.png",4,1);
}

float frame = 0;
void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    if(game_data->keys_down[RAFGL_KEY_R])
    {
        frame = 0;
        table_init(&table,table.width,table.height);
        return;
    }
    if(table.gameOver)
        return;
    frame += 10 * delta_time;
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
    for(int i = 0; i<bonker_count;i++){
        if(!table.bonkers[i].active)
            return;
        rafgl_raster_draw_spritesheet(&raster,&bonk_spreadsheet,(int)frame % 4,0,table.bonkers[i].pos.x, table.bonkers[i].pos.y);
        if(bonker_colision(table.bonkers[i],game_data->mouse_pos_x,game_data->mouse_pos_y))
        {
            table.gameOver = 1;
            return;
        }
    }
    rafgl_raster_copy(&result_raster, &raster);
    vec2_t center = vec2(game_data->mouse_pos_x, game_data->mouse_pos_y);
    float maxr = rafgl_distance2D(0,0,center.x,center.y);
    if(maxr < rafgl_distance2D(0,table.height,center.x,center.y)) maxr = rafgl_distance2D(0,table.height,center.x,center.y);
    if(maxr < rafgl_distance2D(table.width,0,center.x,center.y)) maxr = rafgl_distance2D(table.width,0,center.x,center.y);
    if(maxr < rafgl_distance2D(table.width,table.height,center.x,center.y)) maxr = rafgl_distance2D(table.width,table.height,center.x,center.y);
    maxr *= 0.9;
    for(int y =0 ;y < size_height; y++)
    {
        float v = 1.0f * y / table.height;
        for(int x =0 ;x < size_width; x++)
        {
            vec2_t pos = vec2(x,y);
            float u = 1.0f * x / table.width;
            float alpha = vec2_angle(vec2_sub(pos,center));
            float r = rafgl_distance2D(x,y,center.x,center.y);
            float rnorm = r / maxr;
            r = rnorm * r;

            u = (center.x + cosf(alpha) *r) / table.width;
            v = (center.y + sinf(alpha) *r) / table.height;

            pixel_at_m(raster,x,y) = rafgl_point_sample(&result_raster,u,v);
        } 
    }
    move_bonkers(&table,delta_time);
}

void main_state_render(GLFWwindow *window, void *args)
{
    if(table.gameOver == 0)
        rafgl_texture_load_from_raster(&texture,&raster);
    else if(table.gameOver++ ==1)
        rafgl_texture_load_from_raster(&texture,&game_over_rasters[rand()%4]);
    
    rafgl_texture_show(&texture,0);
}

void main_state_cleanup(GLFWwindow *window, void *args)
{

}
