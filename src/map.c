#include "map.h"

#include <assert.h>

#include "constants.h"

map_tile_t map_get_tile_xy(map_t map, int x, int y) {
    assert(x < MAP_SIZE && y < MAP_SIZE);

    return map[y][x];
}

map_tile_t map_get_tile_ivec2(map_t map, ivec2 pos) {
    return map_get_tile_xy(map, pos[0], pos[1]);
}

map_tile_t map_get_tile_vec2(map_t map, vec2 pos) {
    return map_get_tile_xy(map, pos[0], pos[1]);
}

bool map_tile_collide(map_tile_t tile) {
    return (tile.item_cnt == 0 || tile.collide);
}

/*
static void dda_raycast(map_t map, vec3 player_pos, vec3 tile_pos, vec2 tile_dir) {
    // Digital Differential Analysis
    // NOTE: https://lodev.org/cgtutor/raycasting.html

    float rayDirX = 0;
    float rayDirY = 0;
    float deltaDistX = (rayDirX == 0) ? 1e30 : fabsf(1 / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30 : fabsf(1 / rayDirY);

    // length of ray from current position to next x or y-side
    float sideDistX;
    float sideDistY;

    // what direction to step in x or y-direction (either +1 or -1)
    int stepX;
    int stepY;

    int hit = 0;  // was there a wall hit?
    int side;     // was a NS or a EW wall hit?

    float posX = player_pos[0];
    float posY = player_pos[2];

    int mapX;
    int mapY;

    // calculate step and initial sideDist
    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
    }
    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
    }

    // perform DDA
    while (hit == 0) {
        // jump to next map square, either in x-direction, or in y-direction
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }
        // Check if ray has hit a wall
        if (worldMap[mapX][mapY] > 0) hit = 1;
    }

    // Calculate distance projected on camera direction. This is the shortest distance from the point where the wall is
    // hit to the camera plane. Euclidean to center camera point would give fisheye effect!
    // This can be computed as (mapX - posX + (1 - stepX) / 2) / rayDirX for side == 0, or same formula with Y
    // for size == 1, but can be simplified to the code below thanks to how sideDist and deltaDist are computed:
    // because they were left scaled to |rayDir|. sideDist is the entire length of the ray above after the multiple
    // steps, but we subtract deltaDist once because one step more into the wall was taken above.
    if (side == 0)
        perpWallDist = (sideDistX - deltaDistX);
    else
        perpWallDist = (sideDistY - deltaDistY);
}*/

void map_update_viz(map_t map, camera_t camera) {
    const float FOV_HEADROOM_ANGLE = 20.0f;

    // TODO: limit viz analysis to a smaller square
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            // NOTE: Use the cross product of the position-tile vector and the
            // camera direction to check if the tile is within the FOV.
            vec3 tile_pos;
            tile_pos[0] = (((float)x) + 0.5f);
            tile_pos[1] = (((float)y) + 0.5f);

            vec2 tile_dir;
            tile_dir[0] = tile_pos[0] - camera.pos[0];
            tile_dir[1] = tile_pos[1] - camera.pos[1];
            glm_vec2_normalize(tile_dir);

            // Check agains FOV angle
            float a = glm_vec2_dot(tile_dir, camera.front);
            if (a >= cosf(glm_rad(CAMERA_FOV / 2.0f + FOV_HEADROOM_ANGLE))) {
                // Tile is in FOV, is there anything in between?
                // dda_raycast(map, camera.pos, tile_pos, tile_dir);
                map[y][x].visible = true;
            } else {
                map[y][x].visible = false;
            }
        }
    }
}
