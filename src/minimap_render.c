#include "map.h"
#include "minimap_renderer.h"
#include "pd_api.h"
#include "utils.h"

LCDBitmap* minimap;
LCDBitmap* minimap_mask;
LCDBitmap* ico_player;
LCDBitmap* ico_enemy;

LCDBitmap* debug_minimap;

extern PlaydateAPI* pd;

void minimap_init(void) {
    ico_player = pd->graphics->loadBitmap("res/icons/minimap_player.pdi", NULL);
    ico_enemy = pd->graphics->loadBitmap("res/icons/minimap_player.pdi", NULL);
    debug_minimap = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);
}

void minimap_item_draw(map_item_t item, int tile_x, int tile_y, uint8_t* bitmap_data, int bitmap_rowbytes) {
    // Object
    for (int y = 0; y < MINIMAP_TILE_SIZE; y++) {
        for (int x = 0; x < MINIMAP_TILE_SIZE; x++) {
            bool draw = false;
            if (item.type == ITEM_WALL) {
                switch (item.dir) {
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
                    case DIR_ANY:
                        assert(0);  // Not allowed
                        break;
                }
            } else if (item.type == ITEM_COLUMN || item.type == ITEM_STATUE) {
                draw = true;
            }

            if (draw) clearpixel(bitmap_data, tile_x + x, tile_y + y, bitmap_rowbytes);
        }
    }
}

void minimap_draw(int x, int y, game_state_t* state) {
    int off_x = ((int)state->camera.pos[0]) - MINIMAP_SIZE_X / 2;
    int off_y = ((int)state->camera.pos[1]) - MINIMAP_SIZE_Y / 2;

    uint8_t* bitmap_data = NULL;
    int bitmap_rowbytes = 0;
    pd->graphics->clearBitmap(minimap_mask, kColorBlack);
    pd->graphics->getBitmapData(minimap_mask, NULL, NULL, &bitmap_rowbytes, NULL, &bitmap_data);

    for (int y = off_y; y < off_y + MINIMAP_SIZE_Y; y++) {
        for (int x = off_x; x < off_x + MINIMAP_SIZE_X; x++) {
            if (x < MAP_SIZE * MINIMAP_TILE_SIZE && y < MAP_SIZE * MINIMAP_TILE_SIZE) {
                clearpixel(bitmap_data, x, y, bitmap_rowbytes);
            }
        }
    }
    pd->graphics->setBitmapMask(debug_minimap, minimap_mask);

    pd->graphics->drawBitmap(minimap, x - off_x, y - off_y, kBitmapUnflipped);
    pd->graphics->drawRect(x, y, MINIMAP_SIZE_X, MINIMAP_SIZE_Y, kColorWhite);
    pd->graphics->drawRotatedBitmap(ico_player, x + MINIMAP_SIZE_X / 2, y + MINIMAP_SIZE_Y / 2, state->camera.yaw + 90, 0.5f, 0.5f, 0.2f, 0.2f);
}

void minimap_gen(map_t map) {
    minimap = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);
    minimap_mask = pd->graphics->newBitmap(MAP_SIZE * MINIMAP_TILE_SIZE, MAP_SIZE * MINIMAP_TILE_SIZE, kColorBlack);

    uint8_t* bitmap_data = NULL;
    int bitmap_rowbytes = 0;
    pd->graphics->getBitmapData(minimap, NULL, NULL, &bitmap_rowbytes, NULL, &bitmap_data);

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            map_tile_t tile = map[y][x];
            for (int i = 0; i < tile.item_cnt; i++) {
                minimap_item_draw(tile.items[i], MINIMAP_TILE_SIZE * x, MINIMAP_TILE_SIZE * y, bitmap_data, bitmap_rowbytes);
            }
        }
    }
}

void minimap_debug_tile_draw(map_tile_t tile, int x, int y, uint8_t* bitmap_data, int bitmap_rowbytes) {
    for (int i = 0; i < tile.item_cnt; i++) {
        minimap_item_draw(tile.items[i], x, y, bitmap_data, bitmap_rowbytes);
    }
}

void minimap_debug_draw(int x, int y, game_state_t* state) {
    uint8_t* bitmap_data = NULL;
    int bitmap_rowbytes = 0;
    pd->graphics->getBitmapData(debug_minimap, NULL, NULL, &bitmap_rowbytes, NULL, &bitmap_data);
    pd->graphics->clearBitmap(debug_minimap, kColorBlack);

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            int mmap_x = MINIMAP_TILE_SIZE * x;
            int mmap_y = MINIMAP_TILE_SIZE * y;
            minimap_debug_tile_draw(state->map[y][x], mmap_x, mmap_y, bitmap_data, bitmap_rowbytes);
            if (map_viz_xy(state->map, state->camera.pos, x, y)) clearpixel(bitmap_data, mmap_x + 2, mmap_y + 2, bitmap_rowbytes);
        }
    }

    for (int i = 0; i < gs.path_to_player.size; i++) {
        ivec2 node;
        glm_ivec2_copy(gs.path_to_player.pos[i], node);
        int mmap_x = node[0] * MINIMAP_TILE_SIZE;
        int mmap_y = node[1] * MINIMAP_TILE_SIZE;
        clearpixel(bitmap_data, mmap_x + 2, mmap_y + 2, bitmap_rowbytes);
    }

    pd->graphics->setDrawMode(kDrawModeBlackTransparent);
    pd->graphics->drawBitmap(debug_minimap, x, y, kBitmapUnflipped);
    pd->graphics->drawRotatedBitmap(ico_player, x + state->camera.pos[0] * MINIMAP_TILE_SIZE, y + state->camera.pos[1] * MINIMAP_TILE_SIZE,
                                    state->camera.yaw + 90, 0.5f, 0.5f, 0.2f, 0.2f);
    pd->graphics->drawScaledBitmap(ico_enemy, x + state->enemy_tile[0] * MINIMAP_TILE_SIZE, y + state->enemy_tile[1] * MINIMAP_TILE_SIZE, 0.2f, 0.2f);
}
