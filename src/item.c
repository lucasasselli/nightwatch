#include "item.h"

#include <string.h>

item_t* item_new(void) {
    item_t* item = malloc(sizeof(item_t));
    memset((void*)item, 0, sizeof(item_t));
    return item;
}

void item_list_free(item_t* item) {
    item_t* this = item;
    while (this != NULL) {
        item_t* i = this;
        this = this->next;
        free(i);
    }
}

item_t* item_new_color(obj_id_t obj_id, uint8_t color, dir_t dir, bool collide) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->dir = dir;
    item->obj_id = obj_id;
    item->color = color;
    item->collide = collide;
    return item;
}

item_t* item_new_tex(obj_id_t obj_id, tex_id_t tex_id, dir_t dir, bool collide) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->dir = dir;
    item->obj_id = obj_id;
    item->tex_id = tex_id;
    item->draw_mode = DRAW_MODE_TEX;
    item->collide = collide;
    return item;
}

item_t* item_new_mat(obj_id_t obj_id, minigl_matgroup_t* matgroup, dir_t dir, bool collide) {
    item_t* item = item_new();
    item->type = ITEM_NORMAL;
    item->dir = dir;
    item->obj_id = obj_id;
    item->matgroup = matgroup;
    item->draw_mode = DRAW_MODE_MATERIAL;
    item->collide = collide;
    return item;
}

item_t* item_new_mdbb(tex_mdbb_id_t tex_id, dir_t dir, bool collide) {
    item_t* item = item_new();
    item->type = ITEM_BILLBOARD;
    item->dir = dir;
    item->obj_id = OBJ_ID_STATUE;
    item->tex_mdbb_id = tex_id;
    item->draw_mode = DRAW_MODE_MDBB;
    item->collide = collide;
    return item;
}

void item_set_color(item_t* item, uint8_t color) {
    item->draw_mode = DRAW_MODE_COLOR;
    item->color = color;
}
