#include "minimap.h"

#include "game_state.h"
#include "map.h"
#include "pd_api.h"
#include "utils.h"

LCDBitmap* ico_player;
LCDBitmap* ico_enemy;

LCDBitmap* debug_minimap;

extern PlaydateAPI* pd;

void minimap_init(void) {
    ico_player = pd->graphics->loadBitmap("res/icons/minimap_player.pdi", NULL);
    ico_enemy = pd->graphics->loadBitmap("res/icons/minimap_player.pdi", NULL);
    debug_minimap = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);
}

static void minimap_item_draw(item_t* item, int tile_x, int tile_y, uint8_t* bitmap_data, int bitmap_rowbytes) {
    // Object
    for (int y = 0; y < MINIMAP_TILE_SIZE; y++) {
        for (int x = 0; x < MINIMAP_TILE_SIZE; x++) {
            bool draw = false;
            if (item->type == ITEM_WALL) {
                switch (item->dir) {
                    case DIR_NORTH:
                        if (y == 0) draw = true;
                        break;
                    case DIR_EAST:
                        if (x == (MINIMAP_TILE_SIZE - 1)) draw = true;
                        break;
                    case DIR_SOUTH:
                        if (y == (MINIMAP_TILE_SIZE - 1)) draw = true;
                        break;
                    case DIR_WEST:
                        if (x == 0) draw = true;
                        break;
                }
            } else if (item->type != ITEM_FLOOR) {
                draw = true;
            }

            if (draw) clearpixel(bitmap_data, tile_x + x, tile_y + y, bitmap_rowbytes);
        }
    }
}

static void minimap_debug_tile_draw(map_tile_t tile, int x, int y, uint8_t* bitmap_data, int bitmap_rowbytes) {
    item_t* item = tile.items;
    while (item != NULL) {
        minimap_item_draw(item, x, y, bitmap_data, bitmap_rowbytes);
        item = item->next;
    }
}

void minimap_debug_draw(int x, int y, game_state_t* gs) {
    uint8_t* bitmap_data = NULL;
    int bitmap_rowbytes = 0;
    pd->graphics->getBitmapData(debug_minimap, NULL, NULL, &bitmap_rowbytes, NULL, &bitmap_data);
    pd->graphics->clearBitmap(debug_minimap, kColorBlack);

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            int mmap_x = MINIMAP_TILE_SIZE * x;
            int mmap_y = MINIMAP_TILE_SIZE * y;
            minimap_debug_tile_draw(gs->map[y][x], mmap_x, mmap_y, bitmap_data, bitmap_rowbytes);
            if (map_viz_xy(gs->map, gs->camera.pos, x, y)) clearpixel(bitmap_data, mmap_x + 2, mmap_y + 2, bitmap_rowbytes);
        }
    }

    for (int i = 0; i < gs->path_to_player.size; i++) {
        ivec2 node;
        glm_ivec2_copy(gs->path_to_player.pos[i], node);
        int mmap_x = node[0] * MINIMAP_TILE_SIZE;
        int mmap_y = node[1] * MINIMAP_TILE_SIZE;
        clearpixel(bitmap_data, mmap_x + 2, mmap_y + 2, bitmap_rowbytes);
    }

    pd->graphics->setDrawMode(kDrawModeBlackTransparent);
    pd->graphics->drawBitmap(debug_minimap, x, y, kBitmapUnflipped);
    pd->graphics->drawRotatedBitmap(ico_player, x + gs->camera.pos[0] * MINIMAP_TILE_SIZE, y + gs->camera.pos[1] * MINIMAP_TILE_SIZE, gs->camera.yaw + 90, 0.5f,
                                    0.5f, 0.2f, 0.2f);
    pd->graphics->drawScaledBitmap(ico_enemy, x + gs->enemy_tile[0] * MINIMAP_TILE_SIZE, y + gs->enemy_tile[1] * MINIMAP_TILE_SIZE, 0.2f, 0.2f);
}
