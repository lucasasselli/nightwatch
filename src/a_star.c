#include "a_star.h"

#include "utils.h"

void a_star_list_add(a_star_node_t** list, a_star_node_t* new) {
    // Ensure that new item is not pointing to something else!
    new->next = NULL;

    if (*list == NULL) {
        *list = new;
        return;
    }

    a_star_node_t* this = *list;
    while (this != NULL) {
        if (this->next == NULL) {
            this->next = new;
            return;
        }
        this = this->next;
    }
}

void a_star_list_del(a_star_node_t** list, a_star_node_t* query) {
    assert(*list != NULL);

    a_star_node_t* this = *list;

    // First item match
    if (this == query) {
        *list = this->next;
        return;
    }

    // Check next
    while (this != NULL) {
        if (this->next == query) {
            this->next = query->next;
            return;
        }
        this = this->next;
    }

    assert(0);
}

a_star_node_t* a_star_list_search(a_star_node_t* list, ivec2 pos) {
    if (list != NULL) {
        a_star_node_t* this = list;
        while (this != NULL) {
            if (glm_ivec2_eqv(this->pos, pos)) {
                return this;
            }
            this = this->next;
        }
    }

    return NULL;
}

a_star_node_t* a_star_list_free(a_star_node_t* list) {
    assert(list != NULL);
    a_star_node_t* this = list;
    while (this != NULL) {
        a_star_node_t* f = this;
        this = this->next;
        free(f);
    }
    return NULL;
}

void a_star_list_print(a_star_node_t** list) {
    assert(list != NULL);
    a_star_node_t* this = *list;
    int i = 0;
    while (this != NULL) {
        debug("%d %d %d", ++i, this->pos[0], this->pos[1]);
        this = this->next;
    }
}

a_star_node_t* a_star_node_new(void) {
    a_star_node_t* new = malloc(sizeof(a_star_node_t));
    new->next = NULL;
    return new;
}

a_star_node_t* a_star_find_best(a_star_node_t* list) {
    a_star_node_t* this = list;
    a_star_node_t* best = this;
    while (this != NULL) {
        if ((this->g + this->h) < (best->g + best->h)) {
            best = this;
        }
        this = this->next;
    }
    return best;
}

static int a_star_manhattan_dist(ivec2 a, ivec2 b) {
    ivec2 t;
    glm_ivec2_sub(a, b, t);
    glm_ivec2_abs(t, t);

    return t[0] + t[1];
}

bool a_star_navigate(map_t map, ivec2 start, ivec2 stop, a_star_path_t* path) {
    // A* pathfinding
    // https://web.archive.org/web/20171022224528/http://www.policyalmanac.org:80/games/aStarTutorial.htm
    a_star_node_t* closed_list = NULL;
    a_star_node_t* open_list = NULL;

    // FIXME: This prevents asserts from firing when start = stop, but shouldn't be needed
    if (glm_ivec2_eqv(start, stop)) {
        path->size = 0;
        return true;
    }

    a_star_node_t* new = a_star_node_new();
    new->parent = NULL;
    glm_ivec2_copy(start, new->pos);
    new->h = a_star_manhattan_dist(new->pos, stop);
    new->g = 0;
    a_star_list_add(&open_list, new);

    a_star_node_t* best;
    while (1) {
        // If open list is empty: Error!!!
        if (open_list == NULL) {
            debug("Unable to plot path!");
            a_star_list_free(closed_list);
            path->size = 0;
            return false;
        }

        // Search the on the open list
        best = a_star_find_best(open_list);

        if (glm_ivec2_eqv(best->pos, stop)) {
            // Found stop!
            break;
        } else {
            // Add it to the closed list (and remove it from the open list)
            a_star_list_del(&open_list, best);
            a_star_list_add(&closed_list, best);
        }

        ivec2 x_range;
        x_range[0] = best->pos[0] - 1;
        x_range[1] = best->pos[0] + 2;

        ivec2 y_range;
        y_range[0] = best->pos[1] - 1;
        y_range[1] = best->pos[1] + 2;

        glm_ivec2_clamp(y_range, 0, MAP_SIZE - 1);
        glm_ivec2_clamp(x_range, 0, MAP_SIZE - 1);

        ivec2 pos;
        for (pos[1] = y_range[0]; pos[1] < y_range[1]; pos[1]++) {
            for (pos[0] = x_range[0]; pos[0] < x_range[1]; pos[0]++) {
                map_tile_t tile = map_get_tile_ivec2(map, pos);

                int h = a_star_manhattan_dist(pos, stop);
                int g = best->g + 1;  // FIXME: Should diag be more expensive?

                if (a_star_list_search(closed_list, pos) == NULL && !map_tile_collide(tile)) {
                    // Check if on open list
                    a_star_node_t* match = a_star_list_search(open_list, pos);
                    if (match == NULL) {
                        // Not on the list:
                        // Add to list, make parent square the active square
                        a_star_node_t* new = a_star_node_new();
                        glm_ivec2_copy(pos, new->pos);
                        new->parent = best;
                        new->g = g;
                        new->h = h;
                        a_star_list_add(&open_list, new);
                    } else {
                        // On the list:
                        // If it is on the open list already, check to see if this path to that square is better,
                        // using G cost as the measure.
                        if (match->g > g) {
                            // This is a better path!
                            // change the parent of the square to the current square, and recalculate the G and F scores of the square.
                            match->g = g;
                            glm_ivec2_copy(pos, match->parent);
                        }
                    }
                }
            }
        }
    }

    // Create the path...
    a_star_node_t* this = best;
    path->size = 0;
    while (this->parent != NULL) {
        path->size++;
        this = this->parent;
    }

    this = best;
    for (int i = path->size - 1; i >= 0; i--) {
        glm_ivec2(this->pos, path->pos[i]);
        this = this->parent;
    }

    // Free internal lists
    a_star_list_free(closed_list);
    a_star_list_free(open_list);

    return true;
}
