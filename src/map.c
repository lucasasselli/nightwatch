#include "map.h"

#include <assert.h>

void pos_tile_to_world(ivec2 tile, vec3 world) {
    world[0] = (((float)tile[0]) + 0.5f) * MAP_TILE_SIZE;
    // world[1] = 0.0f;
    world[2] = (((float)tile[1]) + 0.5f) * MAP_TILE_SIZE;
}

void pos_world_to_tile(vec3 world, ivec2 tile) {
    tile[0] = (world[0]) / MAP_TILE_SIZE;
    tile[1] = (world[2]) / MAP_TILE_SIZE;
}

map_tile_t map_get_tile(map_t map, ivec2 pos) {
    int x = pos[0];
    int y = pos[1];
    assert(x < MAP_SIZE && y < MAP_SIZE);

    return map[y][x];
}

bool map_tile_collide(map_tile_t tile) {
    return (tile.item_cnt == 0 || tile.collide);
}
