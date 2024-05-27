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

void tile_item_replace(map_t map, map_item_t item, int x, int y) {
    map_tile_t *tile = &map[y][x];
    tile->item_cnt = 0;
    tile->items[tile->item_cnt] = item;
    tile->item_cnt++;
    assert(tile->item_cnt < MAP_TILE_MAX_ITEMS);
}

bool tile_has_item(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y) {
    if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE) return false;
    map_tile_t *tile = &map[y][x];
    for (int i = 0; i < tile->item_cnt; i++) {
        map_item_t item = tile->items[i];
        if (item.type == type && (item.dir == dir || dir == DIR_ANY)) return true;
    }
    return false;
}

void tile_item_add(map_t map, map_item_type_t type, map_item_dir_t dir, int x, int y) {
    map_item_t item;
    item.type = type;
    item.dir = dir;
    map_tile_t *tile = &map[y][x];

    // Don't add double walls if you are next to an existing room!
    if (item.type == ITEM_WALL) {
        switch (item.dir) {
            case DIR_NORTH:
                if (tile_has_item(map, ITEM_WALL, DIR_SOUTH, x, y - 1)) return;
                break;

            case DIR_EAST:
                if (tile_has_item(map, ITEM_WALL, DIR_WEST, x + 1, y)) return;
                break;

            case DIR_SOUTH:
                if (tile_has_item(map, ITEM_WALL, DIR_NORTH, x, y + 1)) return;
                break;

            case DIR_WEST:
                if (tile_has_item(map, ITEM_WALL, DIR_EAST, x - 1, y)) return;
                break;
        }
    }

    assert(tile->item_cnt < MAP_TILE_MAX_ITEMS);
    tile->items[tile->item_cnt] = item;
    tile->item_cnt++;
}

bool mapgen_room_check_collision(map_t map, map_room_t room) {
    // Check against map boundaries
    if (room.x < 0 || (room.x + room.size.x >= MAP_SIZE)) return false;
    if (room.y < 0 || (room.y + room.size.y >= MAP_SIZE)) return false;

    // Check against other rooms
    bool match = false;

    for (int i = 0; i < room_cnt; i++) {
        map_room_t other = rooms[i];

        if ((room.x + room.size.x) <= other.x) {
            continue;
        }

        if (room.x >= (other.x + other.size.x)) {
            continue;
        }

        if ((room.y + room.size.y) <= other.y) {
            continue;
        }

        if (room.y >= (other.y + other.size.y)) {
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

        assert((room.y + room.size.y) < MAP_SIZE);
        assert((room.x + room.size.x) < MAP_SIZE);

        switch (room.type) {
            case ROOM_CORRIDOR:
            case ROOM_BASE:

                // Floor
                for (int y = 0; y < room.size.y; y++) {
                    for (int x = 0; x < room.size.x; x++) {
                        tile_item_add(map, ITEM_FLOOR, DIR_ANY, room.x + x, room.y + y);
                    }
                }

                // Walls
                grid_add_item_row(map, ITEM_WALL, DIR_NORTH, room.x, room.y, room.size.x);
                grid_add_item_col(map, ITEM_WALL, DIR_EAST, room.x + room.size.x - 1, room.y, room.size.y);
                grid_add_item_row(map, ITEM_WALL, DIR_SOUTH, room.x, room.y + room.size.y - 1, room.size.x);
                grid_add_item_col(map, ITEM_WALL, DIR_WEST, room.x, room.y, room.size.y);

                // Add bases
                if (room.type != ROOM_CORRIDOR) {
                    int base_num = sqrt(room.size.x * room.size.y) / 2;
                    for (int i = 0; i < base_num; i++) {
                        int x, y;
                        do {
                            x = rand_range(room.x, room.x + room.size.x);
                            y = rand_range(room.y, room.y + room.size.y);
                        } while (tile_has_item(map, ITEM_STATUE, DIR_ANY, x, y));

                        tile_item_add(map, ITEM_STATUE, rand() % 4, x, y);
                    }
                }

                // Doors
                if (room.way_in.x > 0 && room.way_in.y > 0) {
                    tile_item_replace(map, (map_item_t){ITEM_FLOOR, DIR_ANY}, room.way_in.x, room.way_in.y);
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

    room.way_in.x = -1;
    room.way_in.y = -1;

    switch (room.type) {
        case ROOM_BASE:
            room.size.x = rand_range(5, 10);
            room.size.y = rand_range(5, 10);
            room.exit_cnt = rand_range(1, 3);
            break;

        case ROOM_CORRIDOR:
            if (rand() % 1) {
                // North-south
                room.size.x = 2;
                room.size.y = rand_range(4, 20);
            } else {
                // East-west
                room.size.x = rand_range(4, 20);
                room.size.y = 2;
            }
            room.exit_cnt = rand_range(3, 5);
            break;
    }

    room.x = rand_range(0, MAP_SIZE - room.size.x);
    room.y = rand_range(0, MAP_SIZE - room.size.y);

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
    int min_x = this.x - (next->size.x);
    int max_x = this.x + (this.size.x);
    int min_y = this.y - (next->size.y);
    int max_y = this.y + (this.size.y);

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
            next->x = rand_range(min_x + ROOM_MARGIN, max_x - ROOM_MARGIN);

            int door_min = maxi(this.x + DOOR_MARGIN, next->x + DOOR_MARGIN);
            int door_max = mini(this.x + this.size.x - DOOR_MARGIN, next->x + next->size.x - DOOR_MARGIN);

            if (door_max - door_min == 0) continue;

            next->way_in.x = rand_range(door_min, door_max);
            if (side == 0) {
                // North
                next->y = min_y;
                next->way_in.y = this.y;
            } else {
                // South
                next->y = max_y;
                next->way_in.y = max_y - 1;
            }
        }

        if (side == 1 || side == 3) {
            // East - West
            next->y = rand_range(min_y + ROOM_MARGIN, max_y - ROOM_MARGIN);

            int door_min_y = maxi(this.y + DOOR_MARGIN, next->y + DOOR_MARGIN);
            int door_max_y = mini(this.y + this.size.y - DOOR_MARGIN, next->y + next->size.y - DOOR_MARGIN);

            if (door_max_y - door_min_y == 0) continue;

            next->way_in.y = rand_range(door_min_y, door_max_y);

            if (side == 1) {
                // East
                next->x = max_x;
                next->way_in.x = next->x - 1;
            } else {
                // West
                next->x = min_x;
                next->way_in.x = this.x;
            }
        }

        break;
    } while (true);
}

void mapgen_room_add(map_t map, map_room_t room) {
    assert(room_cnt < MAP_MAX_ROOMS);
    rooms[room_cnt] = room;
    room_cnt++;
}

void mapgen_gen_adjacent(map_t map, map_room_t room) {
    // Pick a point on the room walls and use this to generate the next room
    for (int i = 0; i < room.exit_cnt; i++) {
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

        mapgen_room_add(map, next_room);
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
    room.x = MAP_SIZE / 2 - room.size.x / 2;
    room.y = MAP_SIZE / 2 - room.size.y / 2;

    // Should have at least 1 exit
    if (room.exit_cnt < 1) {
        room.exit_cnt = 1;
    }
    mapgen_room_add(map, room);

    if (MAP_MAX_ROOMS > 1) {
        // Create adjacent rooms
        mapgen_gen_adjacent(map, room);
    }

    // Update the grid
    mapgen_grid_update(map);

    // Clenup
    free(rooms);
}
