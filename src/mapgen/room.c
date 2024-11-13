#include "mapgen/room.h"

#include "random.h"

static void get_map_coord(bounds_t b, int pos_x, int pos_y, int* map_x, int* map_y) {
    switch (b.r) {
        case 0:
            *map_x = b.x + pos_x;
            *map_y = b.y + pos_y;
            break;
        case 1:
            *map_x = b.x + b.height - pos_y - 1;
            *map_y = b.y + pos_x;
            break;
        case 2:
            *map_x = b.x + b.width - pos_x - 1;
            *map_y = b.y + b.height - pos_y - 1;
            break;
        case 3:
            *map_x = b.x + pos_y;
            *map_y = b.y + b.width - pos_x - 1;
            break;
    }
}

void room_add_item(map_t* map, bounds_t b, int pos_x, int pos_y, item_t* item) {
    int map_x, map_y;
    get_map_coord(b, pos_x, pos_y, &map_x, &map_y);
    map_item_add_xy(map, map_x, map_y, item);
}

void room_add_item_rand(map_t* map, bounds_t b, item_t* item) {
    int map_x, map_y;
    do {
        int x = randi_range(0, b.width);
        int y = randi_range(0, b.height);
        get_map_coord(b, x, y, &map_x, &map_y);
        break;
    } while (map_get_collide_xy(map, map_x, map_y));
    map_item_add_xy(map, map_x, map_y, item);
}

map_tile_t room_get_tile(map_t* map, bounds_t b, int pos_x, int pos_y) {
    int map_x, map_y;
    get_map_coord(b, pos_x, pos_y, &map_x, &map_y);
    return map_get_tile_xy(map, map_x, map_y);
}

void room_clone_tile(map_t* map, bounds_t b, int a_x, int a_y, int b_x, int b_y) {
    int map_a_x, map_a_y;
    int map_b_x, map_b_y;
    get_map_coord(b, b_x, b_y, &map_b_x, &map_b_y);
    get_map_coord(b, a_x, a_y, &map_a_x, &map_a_y);
    map_tile_clone_xy(map, map_a_x, map_a_y, map_b_x, map_b_y);
}

bounds_t room_norm_r(bounds_t room) {
    if (room.r % 2) {
        int t = room.width;
        room.width = room.height;
        room.height = t;
    }

    room.r = 0;

    return room;
}

void room_add_floor(map_t* map, bounds_t b, int pos_x, int pos_y, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            item_t* item = item_new();
            item->type = ITEM_FLOOR;
            item->obj_id = OBJ_ID_FLOOR;
            if (((x + y) % 2) == 0) {
                item_set_color(item, 16);
            } else {
                item_set_color(item, 24);
            }

            room_add_item(map, b, pos_x + x, pos_y + y, item);
        }
    }
}

void room_remove_floor(map_t* map, bounds_t b, int pos_x, int pos_y, int width, int height) {
    int map_x, map_y;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            get_map_coord(b, pos_x + x, pos_y + y, &map_x, &map_y);
            map_tile_remove_all_xy(map, map_x, map_y);
        }
    }
}

void add_picture(map_t* map, bounds_t b, int x, int y, dir_t dir) {
    // Add only if tile has a wall in the same direction.
    map_tile_t tile;
    switch (dir) {
        case DIR_NORTH:
            tile = room_get_tile(map, b, x, y - 1);
            break;
        case DIR_WEST:
            tile = room_get_tile(map, b, x - 1, y);
            break;
        case DIR_SOUTH:
            tile = room_get_tile(map, b, x, y + 1);
            break;
        case DIR_EAST:
            tile = room_get_tile(map, b, x + 1, y);
            break;
    }

    if (tile_has_item(tile, ITEM_FLOOR, -1, -1)) {
        return;
    }

    dir = dir_rotate(dir, b.r);
    int obj_id = randi_range(OBJ_ID_PICTURE_SQUARE_S, OBJ_ID_PICTURE_SQUARE_L + 1);
    room_add_item(map, b, x, y, item_new_tex(obj_id, randi_range(TEX_ID_PICTURE_0, TEX_ID_PICTURE_8 + 1), dir, false));
}

static void add_barrier(map_t* map, bounds_t b, int pos_x, int pos_y, int width, int height) {
    for (int y = 0; y < height; y++) {
        room_add_item(map, b, pos_x, pos_y + y, item_new_color(OBJ_ID_BARRIER, 64, DIR_WEST, true));
        room_add_item(map, b, pos_x + width - 1, pos_y + y, item_new_color(OBJ_ID_BARRIER, 64, DIR_EAST, true));
    }

    for (int x = 0; x < width; x++) {
        room_add_item(map, b, pos_x + x, pos_y, item_new_color(OBJ_ID_BARRIER, 64, DIR_NORTH, true));
        room_add_item(map, b, pos_x + x, pos_y + height - 1, item_new_color(OBJ_ID_BARRIER, 64, DIR_SOUTH, true));
    }
}

minigl_matgroup_t mat_sculpture = {.size = 2, .color = {128, 64}};

void add_sculpture(map_t* map, bounds_t b, int x, int y) {
    // TODO: Add more sculptures and randomize
    room_add_item(map, b, x, y, item_new_mat(OBJ_ID_BASE, &mat_sculpture, DIR_NORTH, true));
    room_add_item(map, b, x, y, item_new_mdbb(TEX_ID_VENUS, DIR_SOUTH, true));
    /*add_barrier(map, b, x, y, 1, 1);*/
}

minigl_matgroup_t mat_shutter = {.size = 3, .color = {160, 128, 96}};

void room_add_door(map_t* map, bounds_t b, int pos_x, int pos_y, int width, dir_t dir) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->obj_id = OBJ_ID_SHUTTER_CLOSED;
    item->draw_mode = DRAW_MODE_MATERIAL;
    item->matgroup = &mat_shutter;
    item->dir = dir;
    item->action.type = ACTION_KEYPAD;
    item->action.arg = 0;  // TODO

    room_add_item(map, b, pos_x, pos_y, item);

    for (int i = 1; i < width; i++) {
        if (b.r % 2) {
            room_clone_tile(map, b, pos_x, pos_y + i, pos_x, pos_y);
        } else {
            room_clone_tile(map, b, pos_x + i, pos_y, pos_x, pos_y);
        }
    }
}

/*

 void add_inspect_tex(map_t* map, int x, int y, tex_id_t tex_id) {
    item_t* item = item_new();
    item->hidden = true;
    item->action.type = ACTION_INSPECT;
    item->action.arg = tex_id;
    map_item_add_xy(map, x, y, item);
}
*/
