#include "map_generator.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

map_room_t *rooms;
int room_cnt;

/*
char tile_to_char(map_tile_t tile) {
    if (tile.item_cnt == 0) {
        return ' ';
    } else if (tile.item_cnt > 1) {
        return '+';
    } else {
        switch (tile.items[0].type) {
            case ITEM_FLOOR:
                return '.';
            case TILE_STATUE:
                return 'o';
            case TILE_WALL_N:
            case TILE_WALL_S:
                return '-';
            case TILE_WALL_E:
            case TILE_WALL_W:
                return '|';
        }
    }
}*/

bool tile_has_item(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y) {
    if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE) return false;
    map_tile_t *tile = &map[y][x];
    for (int i = 0; i < tile->item_cnt; i++) {
        map_item_t item = tile->items[i];
        if (item.type == type && (item.dir == dir || dir == DIR_ANY)) return true;
    }
    return false;
}

bool tile_remove_item(map_t map, map_item_t query, int x, int y) {
    if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE) return false;
    map_tile_t *tile = &map[y][x];
    int j = 0;
    bool found = false;
    for (int i = 0; i < tile->item_cnt; i++) {
        map_item_t item = tile->items[i];
        if (item.type == query.type && (item.dir == query.dir)) {
            found = true;
            continue;
        }

        tile->items[j] = tile->items[i];
        j++;
    }

    tile->item_cnt = j;
    return found;
}

void tile_item_add(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y) {
    map_item_t item;
    item.type = type;
    item.dir = dir;
    map_tile_t *tile = &map[y][x];

    bool add_floor = false;
    if (item.type == ITEM_WALL) {
        switch (item.dir) {
            case DIR_NORTH:
                if (tile_remove_item(map, (map_item_t){ITEM_WALL, DIR_SOUTH}, x, y - 1)) {
                    return;
                }
                break;

            case DIR_EAST:
                if (tile_remove_item(map, (map_item_t){ITEM_WALL, DIR_WEST}, x + 1, y)) {
                    return;
                }
                break;

            case DIR_SOUTH:
                if (tile_remove_item(map, (map_item_t){ITEM_WALL, DIR_NORTH}, x, y + 1)) {
                    return;
                }
                break;

            case DIR_WEST:
                if (tile_remove_item(map, (map_item_t){ITEM_WALL, DIR_EAST}, x - 1, y)) {
                    return;
                }
                break;
        }
    }

    if (add_floor) {
    }

    assert(tile->item_cnt < MAP_TILE_MAX_ITEMS);
    tile->items[tile->item_cnt] = item;
    tile->item_cnt++;
}

bool mapgen_room_check_oob(map_t map, map_room_t room) {
    if (room.pos[0] < 0 || (room.pos[0] + room.size[0] >= MAP_SIZE)) return true;
    if (room.pos[1] < 0 || (room.pos[1] + room.size[1] >= MAP_SIZE)) return true;
    return false;
}

bool mapgen_room_check_collision(map_t map, map_room_t room) {
    // Check against map boundaries
    if (mapgen_room_check_oob(map, room)) return false;

    // Check against other rooms
    bool match = false;

    for (int i = 0; i < room_cnt; i++) {
        map_room_t other = rooms[i];

        if ((room.pos[0] + room.size[0]) <= other.pos[0]) {
            continue;
        }

        if (room.pos[0] >= (other.pos[0] + other.size[0])) {
            continue;
        }

        if ((room.pos[1] + room.size[1]) <= other.pos[1]) {
            continue;
        }

        if (room.pos[1] >= (other.pos[1] + other.size[1])) {
            continue;
        }

        match = true;
    }

    return !match;
}

void grid_add_item_col(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y, int size) {
    for (int i = 0; i < size; i++) {
        tile_item_add(map, type, dir, x, y + i);
    }
}

void grid_add_item_row(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y, int size) {
    for (int i = 0; i < size; i++) {
        tile_item_add(map, type, dir, x + i, y);
    }
}

void mapgen_grid_update(map_t map) {
    for (int i = 0; i < room_cnt; i++) {
        map_room_t room = rooms[i];

        assert((room.pos[1] + room.size[1]) < MAP_SIZE);
        assert((room.pos[0] + room.size[0]) < MAP_SIZE);

        switch (room.type) {
            case ROOM_CORRIDOR:
            case ROOM_BASE:

                // Floor
                for (int y = 0; y < room.size[1]; y++) {
                    for (int x = 0; x < room.size[0]; x++) {
                        tile_item_add(map, ITEM_FLOOR, DIR_ANY, room.pos[0] + x, room.pos[1] + y);
                    }
                }

                // Walls
                grid_add_item_row(map, ITEM_WALL, DIR_NORTH, room.pos[0], room.pos[1], room.size[0]);
                grid_add_item_col(map, ITEM_WALL, DIR_EAST, room.pos[0] + room.size[0] - 1, room.pos[1], room.size[1]);
                grid_add_item_row(map, ITEM_WALL, DIR_SOUTH, room.pos[0], room.pos[1] + room.size[1] - 1, room.size[0]);
                grid_add_item_col(map, ITEM_WALL, DIR_WEST, room.pos[0], room.pos[1], room.size[1]);

                // Add bases
                if (room.type != ROOM_CORRIDOR) {
                    int base_num = sqrt(room.size[0] * room.size[1]) / 2;
                    for (int i = 0; i < base_num; i++) {
                        int x, y;
                        do {
                            x = rand_range(room.pos[0], room.pos[0] + room.size[0]);
                            y = rand_range(room.pos[1], room.pos[1] + room.size[1]);
                        } while (tile_has_item(map, ITEM_STATUE, DIR_ANY, x, y));

                        tile_item_add(map, ITEM_STATUE, rand() % 4, x, y);
                    }
                }

                break;
        }
    }
}

void mapgen_grid_print(map_t map) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            // printf("%c ", tile_to_char(map[y][x]));
        }
        printf("\n");
    }
}

map_room_t mapgen_room_randomize(void) {
    map_room_t room;

    // FIXME: Weight ROOM_CORRIDOR to be less likely
    room.type = rand() % ROOM_TYPE_SIZE;

    switch (room.type) {
        case ROOM_BASE:
            room.size[0] = rand_range(5, 10);
            room.size[1] = rand_range(5, 10);
            break;

        case ROOM_CORRIDOR:
            if (rand() % 1) {
                // North-south
                room.size[0] = 2;
                room.size[1] = rand_range(4, 20);
            } else {
                // East-west
                room.size[0] = rand_range(4, 20);
                room.size[1] = 2;
            }
            break;
    }

    room.pos[0] = rand_range(0, MAP_SIZE - room.size[0]);
    room.pos[1] = rand_range(0, MAP_SIZE - room.size[1]);

    return room;
}

void mapgen_room_randomize_next(map_room_t this, map_room_t *next) {
    const int ROOM_MARGIN = 2;
    const int DOOR_MARGIN = 1;

    //     ┌─────┐
    //     │Next │
    //     │Room │
    //     └─────┘
    //
    //       min_y
    // min_x┌───────────────────┐max_x
    //      │                   │
    //      │                   │
    //      │     ┌─────────────┤
    //      │     │             │
    //      │     │             │
    //      │     │    This     │
    //      │     │    Room     │
    //      │     │             │
    //      │     │             │
    //      │     │             │
    //      └─────┴─────────────┘
    //       max_y
    //
    // Next room will be randomized on the outher borders of
    // The perimeter of the big triangle, with a inner padding
    // of 2 to allow to fit the door!

    // Calculate the min and max coordinates for next room origin
    int min_x = this.pos[0] - (next->size[0]);
    int max_x = this.pos[0] + (this.size[0]);
    int min_y = this.pos[1] - (next->size[1]);
    int max_y = this.pos[1] + (this.size[1]);

    do {
        // Pick a side for the next room
        int side = rand_range(0, 4);

        // Check that the side can fit a room and a door
        if (side % 2 == 0) {
            // North - south
            if (max_x - min_x < (ROOM_MARGIN * 2)) continue;
        } else {
            // East - west
            if (max_y - min_y < (ROOM_MARGIN * 2)) continue;
        }

        if (side == 0 || side == 2) {
            // North - South
            next->pos[0] = rand_range(min_x + ROOM_MARGIN, max_x - ROOM_MARGIN);

            int door_min = maxi(this.pos[0] + DOOR_MARGIN, next->pos[0] + DOOR_MARGIN);
            int door_max = mini(this.pos[0] + this.size[0] - DOOR_MARGIN, next->pos[0] + next->size[0] - DOOR_MARGIN);

            if (door_max - door_min == 0) continue;

            if (side == 0) {
                // North
                next->pos[1] = min_y;
            } else {
                // South
                next->pos[1] = max_y;
            }
        }

        if (side == 1 || side == 3) {
            // East - West
            next->pos[1] = rand_range(min_y + ROOM_MARGIN, max_y - ROOM_MARGIN);

            int door_min_y = maxi(this.pos[1] + DOOR_MARGIN, next->pos[1] + DOOR_MARGIN);
            int door_max_y = mini(this.pos[1] + this.size[1] - DOOR_MARGIN, next->pos[1] + next->size[1] - DOOR_MARGIN);

            if (door_max_y - door_min_y == 0) continue;

            if (side == 1) {
                // East
                next->pos[0] = max_x;
            } else {
                // West
                next->pos[0] = min_x;
            }
        }

        break;
    } while (true);
}

void room_add(map_t map, map_room_t room) {
    assert(room_cnt < MAP_MAX_ROOMS);
    rooms[room_cnt] = room;
    room_cnt++;
}

void mapgen_gen_adjacent(map_t map, map_room_t room) {
    // Pick a point on the room walls and use this to generate the next room
    int exit_cnt = rand_range(1, 4);
    for (int i = 0; i < exit_cnt; i++) {
        if (room_cnt >= (MAP_MAX_ROOMS - 1)) {
            return;
        }

        map_room_t next_room;
        int tries = 0;
        do {
            next_room = mapgen_room_randomize();
            mapgen_room_randomize_next(room, &next_room);
            tries++;
        } while (!mapgen_room_check_collision(map, next_room) && tries < MAP_RAND_EFFORT);

        if (tries >= MAP_RAND_EFFORT) {
            debug("Max effort reached!\n");
            continue;
        }

        room_add(map, next_room);
        mapgen_gen_adjacent(map, next_room);
    }
}

void mapgen_gen(map_t map) {
    // Initialize
    rooms = malloc(sizeof(map_room_t) * MAP_MAX_ROOMS);
    room_cnt = 0;

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            // Initialize the tile
            map[y][x].item_cnt = 0;  // No item by default
            map[y][x].items = malloc(sizeof(map_item_t) * MAP_TILE_MAX_ITEMS);
        }
    }

    // Draw the first room
    map_room_t room;
    room = mapgen_room_randomize();

    // Put first room at center of map
    room.pos[0] = MAP_SIZE / 2 - room.size[0] / 2;
    room.pos[1] = MAP_SIZE / 2 - room.size[1] / 2;

    room_add(map, room);

    if (MAP_MAX_ROOMS > 1) {
        // Create adjacent rooms
        mapgen_gen_adjacent(map, room);
    }

    // Update the grid
    mapgen_grid_update(map);

    // Clenup
    free(rooms);
}
