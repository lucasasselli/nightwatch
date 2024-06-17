#include "item.h"

#include <string.h>

item_t* item_new(void) {
    item_t* item = malloc(sizeof(item_t));
    memset((void*)item, 0, sizeof(item_t));
    return item;
}

item_t* item_new_tex(obj_id_t obj_id, tex_id_t tex_id, dir_t dir) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->dir = dir;
    item->obj_id = obj_id;
    item->tex_id = tex_id;
    item->tex_mode = TEX_MODE_IMAGE;
    return item;
}

item_t* item_new_color(obj_id_t obj_id, uint8_t color, dir_t dir) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->dir = dir;
    item->obj_id = obj_id;
    item->color = color;
    return item;
}

item_t* item_new_mdbb(tex_mdbb_id_t tex_id, dir_t dir) {
    item_t* item = item_new();
    item->type = ITEM_BILLBOARD;
    item->dir = dir;
    item->obj_id = OBJ_ID_STATUE;
    item->tex_mdbb_id = tex_id;
    item->tex_mode = TEX_MODE_MDBB;
    return item;
}

void item_set_color(item_t* item, uint8_t color) {
    item->tex_mode = TEX_MODE_NONE;
    item->color = color;
}
