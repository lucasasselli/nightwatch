#include "mapgen/room.h"

#include "random.h"
#include "utils.h"

void room_add_item(map_t* map, bounds_t b, int pos_x, int pos_y, item_t* item) {
    int rot_x;
    int rot_y;

    switch (b.r) {
        case 0:
            rot_x = pos_x;
            rot_y = pos_y;
            break;
        case 1:
            rot_x = b.height - pos_y - 1;
            rot_y = pos_x;
            break;
        case 2:
            rot_x = b.width - pos_x - 1;
            rot_y = b.height - pos_y - 1;
            break;
        case 3:
            rot_x = pos_y;
            rot_y = b.width - pos_x - 1;
            break;
    }

    map_item_add_xy(map, b.x + rot_x, b.y + rot_y, item);
}

map_tile_t room_get_tile(map_t* map, bounds_t b, int pos_x, int pos_y) {
    int rot_x;
    int rot_y;

    switch (b.r) {
        case 0:
            rot_x = pos_x;
            rot_y = pos_y;
            break;
        case 1:
            rot_x = b.height - pos_y - 1;
            rot_y = pos_x;
            break;
        case 2:
            rot_x = b.width - pos_x - 1;
            rot_y = b.height - pos_y - 1;
            break;
        case 3:
            rot_x = pos_y;
            rot_y = b.width - pos_x - 1;
            break;
    }

    debug("%d %d %d %d %d", b.r, b.x, b.y, rot_x, rot_y);

    return map_get_tile_xy(map, b.x + rot_x, b.y + rot_y);
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
    room_add_item(map, b, x, y, item_new_tex(OBJ_ID_PICTURE_SQUARE, randi(TEX_ID_PICTURE_0, TEX_ID_PICTURE_8), dir, false));
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

/*
 void add_door(map_t* map, int x, int y, dir_t dir, int pin) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->obj_id = OBJ_ID_SHUTTER_CLOSED;
    item->draw_mode = DRAW_MODE_MATERIAL;
    item->matgroup = &mat_shutter;
    item->dir = dir;
    item->action.type = ACTION_KEYPAD;
    item->action.arg = pin;
    map_item_add_xy(map, x, y, item);
}

 void add_note(map_t* map, int x, int y, int id) {
    item_t* item = item_new_tex(OBJ_ID_NOTE, TEX_ID_NOTE, DIR_NORTH, true);
    item->effects = EFFECT_SPIN;
    item->action.type = ACTION_NOTE;
    item->action.arg = id;
    map_item_add_xy(map, x, y, item);
}

 void add_inspect_tex(map_t* map, int x, int y, tex_id_t tex_id) {
    item_t* item = item_new();
    item->hidden = true;
    item->action.type = ACTION_INSPECT;
    item->action.arg = tex_id;
    map_item_add_xy(map, x, y, item);
}
*/
