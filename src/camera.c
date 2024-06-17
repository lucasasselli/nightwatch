#include "camera.h"

#include "minigl/minigl.h"

void camera_init(camera_t* camera) {
    camera->yaw = -90.0f;
    glm_vec2_copy((vec3){0.0f, -1.0f}, camera->front);
    glm_perspective(glm_rad(CAMERA_FOV), ((float)SCREEN_SIZE_X) / ((float)SCREEN_SIZE_Y), CAMERA_MIN_Z, CAMERA_MAX_Z, camera->proj);
}

void camera_update_trans(camera_t* camera) {
    mat4 view;
    vec3 camera_center;

    vec3 vec3_pos;
    vec3_pos[0] = camera->pos[0];
    vec3_pos[1] = camera->bob;
    vec3_pos[2] = camera->pos[1];

    vec3 vec3_front;
    vec3_front[0] = camera->front[0];
    vec3_front[1] = 0.0f;
    vec3_front[2] = camera->front[1];

    glm_vec3_add(vec3_pos, vec3_front, camera_center);

    // Update camera position

    glm_vec3_add(vec3_pos, vec3_front, camera_center);
    glm_lookat(vec3_pos, camera_center, CAMERA_UP, view);
    glm_mat4_mul(camera->proj, view, camera->trans);
}
