#include "mapgen.h"

char mapgen_tile_char(map_tile_t tile) {
    switch (tile) {
        case TILE_EMPTY:
            return ' ';
        case TILE_FLOOR:
            return '.';
        case TILE_CORNER0:
        case TILE_CORNER1:
        case TILE_CORNER2:
        case TILE_CORNER3:
            return '+';
        case TILE_WALL_NS:
            return '|';
        case TILE_WALL_EW:
            return '-';
        case TILE_DOOR_NS:
        case TILE_DOOR_EW:
            return 'D';
    }
}

void mapgen_init(map_t *map) {
    map->rooms = malloc(sizeof(map_room_t) * MAP_MAX_ROOMS);
    map->room_cnt = 0;
    map->grid = malloc(sizeof(map_tile_t *) * MAP_SIZE);
    for (int y = 0; y < MAP_SIZE; y++) {
        map->grid[y] = (map_tile_t *)malloc(sizeof(map_tile_t) * MAP_SIZE);
        for (int x = 0; x < MAP_SIZE; x++) {
            map->grid[y][x] = TILE_EMPTY;
        }
    }
}

bool mapgen_room_check_collision(map_t *map, map_room_t room) {
    // Check against map boundaries
    if (room.x < 0 || (room.x + room.size.x >= MAP_SIZE)) return false;
    if (room.y < 0 || (room.y + room.size.y >= MAP_SIZE)) return false;

    // Check against other rooms
    bool match = false;

    for (int i = 0; i < map->room_cnt; i++) {
        map_room_t other = map->rooms[i];

        if ((room.x + room.size.x - 1) <= other.x) {
            continue;
        }

        if (room.x >= (other.x + other.size.x - 1)) {
            continue;
        }

        if ((room.y + room.size.y - 1) <= other.y) {
            continue;
        }

        if (room.y >= (other.y + other.size.y - 1)) {
            continue;
        }

        match = true;
    }

    return !match;
}

void mapgen_grid_update(map_t *map) {
    for (int i = 0; i < map->room_cnt; i++) {
        map_room_t room = map->rooms[i];

        assert((room.y + room.size.y) < MAP_SIZE);
        assert((room.x + room.size.x) < MAP_SIZE);

        switch (room.type) {
            case ROOM_CORRIDOR:
            case ROOM_BASE:
                for (int y = room.y; y < (room.y + room.size.y); y++) {
                    for (int x = room.x; x < (room.x + room.size.x); x++) {
                        map_tile_t t = TILE_FLOOR;

                        bool west_side = (x == room.x);
                        bool east_side = (x == (room.x + room.size.x - 1));
                        bool north_side = (y == room.y);
                        bool south_side = (y == (room.y + room.size.y - 1));

                        if (north_side || south_side) t = TILE_WALL_EW;

                        if (east_side || west_side) t = TILE_WALL_NS;

                        if (west_side && north_side) t = TILE_CORNER0;

                        if (east_side && north_side) t = TILE_CORNER1;

                        if (west_side && south_side) t = TILE_CORNER2;

                        if (east_side && south_side) t = TILE_CORNER3;

                        if (map->grid[y][x] == TILE_EMPTY) map->grid[y][x] = t;
                    }
                }

                // Add way_in
                if (room.way_in.x >= 0 && room.way_in.y >= 0) {
                    if (map->grid[room.way_in.y][room.way_in.x] == TILE_WALL_NS) {
                        map->grid[room.way_in.y][room.way_in.x] = TILE_DOOR_NS;
                    }
                    if (map->grid[room.way_in.y][room.way_in.x] == TILE_WALL_EW) {
                        map->grid[room.way_in.y][room.way_in.x] = TILE_DOOR_EW;
                    }
                }
                break;
        }
    }
}

void mapgen_grid_print(map_t map) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            printf("%c ", mapgen_tile_char(map.grid[y][x]));
        }
        printf("\n");
    }
}

map_room_t mapgen_room_randomize(void) {
    map_room_t room;
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
                room.size.x = 3;
                room.size.y = rand_range(3, 20);
            } else {
                // East-west
                room.size.x = rand_range(3, 20);
                room.size.y = 3;
            }
            room.exit_cnt = rand_range(3, 5);
            break;
    }

    room.x = rand_range(0, MAP_SIZE - room.size.x);
    room.y = rand_range(0, MAP_SIZE - room.size.y);

    return room;
}

void mapgen_room_randomize_next(map_room_t this, map_room_t *next) {
    const int margin = 2;

    //     ┌─────┐
    //     │Next │
    //     │Room │
    //     └─────┘
    //
    //
    //     ┌───────────────────┐
    //     │                   │
    //     │                   │
    //     │     ┌─────────────┤
    //     │     │             │
    //     │     │             │
    //     │     │    This     │
    //     │     │    Room     │
    //     │     │             │
    //     │     │             │
    //     │     │             │
    //     └─────┴─────────────┘
    //
    // Next room will be randomized on the outher borders of
    // The perimeter of the big triangle, with a inner padding
    // of 2 to allow to fit the door!
    //
    // NOTE: Room smallest dimension is at least 3

    int min_x = this.x - (next->size.x - 1);
    int max_x = this.x + (this.size.x);
    int min_y = this.y - (next->size.y - 1);
    int max_y = this.y + (this.size.y);

    int side = rand() % 4;

    if (side == 0 || side == 2) {
        // North - South
        next->x = rand_range(min_x + margin, max_x - margin);
        next->way_in.x = rand_range(maxi(this.x + 1, next->x + 1), mini(this.x + this.size.x - 1, next->x + next->size.x - 1));
        if (side == 0) {
            // North
            next->y = min_y;
            next->way_in.y = this.y;
        } else {
            // South
            next->y = max_y - 1;
            next->way_in.y = max_y - 1;
        }
    }

    if (side == 1 || side == 3) {
        // East - West
        next->y = rand_range(min_y + margin, max_y - margin);
        next->way_in.y = rand_range(maxi(this.y + 1, next->y + 1), mini(this.y + this.size.y - 1, next->y + next->size.y - 1));
        if (side == 1) {
            // East
            next->x = max_x - 1;
            next->way_in.x = next->x;
        } else {
            // West
            next->x = min_x;
            next->way_in.x = this.x;
        }
    }
}

void mapgen_room_add(map_t *map, map_room_t room) {
    map->rooms[map->room_cnt] = room;
    map->room_cnt++;
    assert(map->room_cnt < MAP_MAX_ROOMS);
}

void mapgen_gen_adjacent(map_t *map, map_room_t room) {
    // Pick a point on the room walls and use this to generate the next room
    for (int i = 0; i < room.exit_cnt; i++) {
        if (map->room_cnt >= (MAP_MAX_ROOMS - 2)) {
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

void mapgen_gen(map_t *map) {
    // Draw the first room
    map_room_t room;
    room = mapgen_room_randomize();
    if (room.exit_cnt < 1) {
        room.exit_cnt = 1;
    }
    mapgen_room_add(map, room);

    // Create adjacent rooms
    mapgen_gen_adjacent(map, room);

    // Update the grid
    mapgen_grid_update(map);
}
