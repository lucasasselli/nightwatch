#include "object.h"

minigl_obj_t obj_array[OBJ_ID_NUM];

#include "random.h"

void obj_rand(minigl_obj_t* obj) {
    const float VOLUME_MAX = 0.7;

    minigl_obj_read_file("res/models/cube.obj", obj, 0);

    for (int i = 0; i < obj->vcoord_size; i++) {
        obj->vcoord_ptr[i][0] += randf(-VOLUME_MAX, VOLUME_MAX, 0.1f);
        obj->vcoord_ptr[i][1] += randf(-VOLUME_MAX, VOLUME_MAX, 0.1f);
        obj->vcoord_ptr[i][2] += randf(-VOLUME_MAX, VOLUME_MAX, 0.1f);
    }

    mat4 trans;
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-0.0f, 0.5f, -0.0f});
    glm_scale(trans, (vec3){0.4f, 0.4f, 0.4f});
    minigl_obj_trans(obj, trans);
}

int obj_init(void) {
    const float WALL_SCALE_Y = 1.5f;
    const float FLOOR_Y_OFF = WALL_SCALE_Y / 2.0f;

    minigl_obj_t obj_tile;
    minigl_obj_t obj_wall;
    minigl_obj_t obj_cube;

    int result = 0;
    result |= minigl_obj_read_file("res/models/tile.obj", &obj_tile, MINIGL_OBJ_TEXFLIPY);
    result |= minigl_obj_read_file("res/models/wall.obj", &obj_wall, 0);
    result |= minigl_obj_read_file("res/models/cube.obj", &obj_cube, 0);
    result |= minigl_obj_read_file("res/models/base.obj", &obj_array[OBJ_ID_BASE], 0);
    result |= minigl_obj_read_file("res/models/wetfloor.obj", &obj_array[OBJ_ID_WETFLOOR], MINIGL_OBJ_TEXFLIPY);
    result |= minigl_obj_read_file("res/models/bench.obj", &obj_array[OBJ_ID_BENCH], MINIGL_OBJ_TEXFLIPY);
    result |= minigl_obj_read_file("res/models/sink.obj", &obj_array[OBJ_ID_WC_SINK], MINIGL_OBJ_TEXFLIPY);
    result |= minigl_obj_read_file("res/models/shutter_closed.obj", &obj_array[OBJ_ID_SHUTTER_CLOSED], MINIGL_OBJ_TEXFLIPY);
    result |= minigl_obj_read_file("res/models/shutter_open.obj", &obj_array[OBJ_ID_SHUTTER_OPEN], MINIGL_OBJ_TEXFLIPY);
    result |= minigl_obj_read_file("res/models/barrier.obj", &obj_array[OBJ_ID_BARRIER], 0);

    if (result) return result;

    mat4 trans;

    // Floor
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, -FLOOR_Y_OFF, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){1.0f, 0.0f, 0.0f});
    minigl_obj_copy_trans(obj_tile, trans, &obj_array[OBJ_ID_FLOOR]);

    // Wall
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, 0.0f, -0.5f});
    glm_scale(trans, (vec3){1.0f, WALL_SCALE_Y, 1.0});
    minigl_obj_copy_trans(obj_wall, trans, &obj_array[OBJ_ID_WALL]);

    // Picture square
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-0.0f, 0.0f, -0.48f});
    glm_scale(trans, (vec3){0.9f, 0.9, 0.9});
    minigl_obj_copy_trans(obj_tile, trans, &obj_array[OBJ_ID_PICTURE_SQUARE]);

    // Sign small side
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-0.3f, 0.2f, -0.49f});
    glm_scale(trans, (vec3){0.2f, 0.2, 0.2});
    minigl_obj_copy_trans(obj_tile, trans, &obj_array[OBJ_ID_SIGN_SIDE_SMALL]);

    // Enemy
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale(trans, (vec3){1.0f, 1.5f, 1.0});
    minigl_obj_copy_trans(obj_tile, trans, &obj_array[OBJ_ID_ENEMY]);

    // Base
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, -0.6f, 0.0f});
    glm_scale(trans, (vec3){0.35f, 0.1, 0.35});
    minigl_obj_trans(&obj_array[OBJ_ID_BASE], trans);

    // Statue
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-0.0f, 0.2f, 0.0f});
    glm_scale(trans, (vec3){1.0f, WALL_SCALE_Y, 1.0});
    minigl_obj_copy_trans(obj_tile, trans, &obj_array[OBJ_ID_STATUE]);

    // Column
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, 0.8f, 0.0f});
    glm_scale(trans, (vec3){0.4f, 3.0, 0.4});
    minigl_obj_copy_trans(obj_cube, trans, &obj_array[OBJ_ID_COLUMN]);

    // Shutter open/closed
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale(trans, (vec3){0.5f, 1.0, 0.5});
    glm_translate(trans, (vec3){0.0f, -1.0f, -1.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    minigl_obj_trans(&obj_array[OBJ_ID_SHUTTER_CLOSED], trans);
    minigl_obj_trans(&obj_array[OBJ_ID_SHUTTER_OPEN], trans);

    // Wetfloor
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale_uni(trans, 0.7f);
    glm_translate(trans, (vec3){0.0f, -1.2f, 0.0f});
    minigl_obj_trans(&obj_array[OBJ_ID_WETFLOOR], trans);

    // Note
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale(trans, (vec3){0.3f, 0.4, 1.0});

    minigl_obj_copy_trans(obj_tile, trans, &obj_array[OBJ_ID_NOTE]);

    // Bench
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale(trans, (vec3){0.9, 0.8, 0.8});
    glm_translate(trans, (vec3){0.0f, -0.9f, 0.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    minigl_obj_trans(&obj_array[OBJ_ID_BENCH], trans);

    // WC Panel
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){0.0f, 0.0f, -0.5f});
    glm_scale(trans, (vec3){1.0f, 1.2, 1.0});
    minigl_obj_copy_trans(obj_wall, trans, &obj_array[OBJ_ID_WC_PANEL]);

    // WC Column
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-0.5f, 0.0f, -0.5f});
    glm_scale(trans, (vec3){0.1f, 1.4, 0.1});
    minigl_obj_copy_trans(obj_cube, trans, &obj_array[OBJ_ID_WC_COLUMN]);

    // WC Sink
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_scale(trans, (vec3){0.2f, 0.2, 0.2});
    glm_translate(trans, (vec3){0.0f, -3.5f, -1.0f});
    glm_rotate_at(trans, (vec3){0.0f, 0.0f, 0.0f}, glm_rad(90), (vec3){0.0f, 1.0f, 0.0f});
    minigl_obj_trans(&obj_array[OBJ_ID_WC_SINK], trans);

    // Barrier
    glm_mat4_copy(GLM_MAT4_IDENTITY, trans);
    glm_translate(trans, (vec3){-0.0f, -0.7f, -0.5f});
    glm_scale(trans, (vec3){0.1f, 0.07f, 0.1f});
    minigl_obj_trans(&obj_array[OBJ_ID_BARRIER], trans);

    obj_rand(&obj_array[OBJ_ID_RANDOM]);

    return 0;
}

minigl_obj_t* obj_get(obj_id_t id) {
    assert(id < OBJ_ID_NUM);
    return &obj_array[id];
}
