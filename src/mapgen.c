#include "mapgen.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

char tile_to_char(map_tile_t tile) {
    if (tile.item_cnt == 0) {
        return ' ';
    } else if (tile.item_cnt > 1) {
        return '+';
    } else {
        switch (tile.items[0].type) {
            case TILE_FLOOR:
                return '.';
            case TILE_WALL_N:
            case TILE_WALL_S:
                return '-';
            case TILE_WALL_E:
            case TILE_WALL_W:
                return '|';
            case TILE_DOOR_NS:
            case TILE_DOOR_EW:
                return 'D';
        }
    }
}

void tile_item_add(map_t *map, map_item_type_t type, int x, int y) {
    map_item_t item;
    item.type = type;
    item.style = STYLE0;  // FIXME: Pick the style from the room!
    map_tile_t *tile = &map->grid[y][x];

    if (type == TILE_DOOR_EW || type == TILE_DOOR_NS) {
        tile->item_cnt = 0;  // Doors replace walls
    }

    if (tile->item_cnt == 1) {
        if (tile->items[0].type == TILE_DOOR_EW || tile->items[0].type == TILE_DOOR_NS) {
            return;  // Don't replace doors
        }
    }

    tile->items[tile->item_cnt] = item;
    tile->item_cnt++;
    assert(tile->item_cnt < MAP_TILE_MAX_ITEMS);
}

void mapgen_init(map_t *map) {
    map->rooms = malloc(sizeof(map_room_t) * MAP_MAX_ROOMS);
    map->room_cnt = 0;
    map->grid = malloc(sizeof(map_tile_t *) * MAP_SIZE);
    for (int y = 0; y < MAP_SIZE; y++) {
        map->grid[y] = malloc(sizeof(map_tile_t) * MAP_SIZE);
        for (int x = 0; x < MAP_SIZE; x++) {
            // Initialize the tile
            map->grid[y][x].item_cnt = 0;  // No item by default
            map->grid[y][x].items = malloc(sizeof(map_item_t) * MAP_TILE_MAX_ITEMS);
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

void grid_add_item_col(map_t *map, map_item_type_t type, int x, int y, int size) {
    for (int i = 0; i < size; i++) {
        tile_item_add(map, type, x, y + i);
    }
}

void grid_add_item_row(map_t *map, map_item_type_t type, int x, int y, int size) {
    for (int i = 0; i < size; i++) {
        tile_item_add(map, type, x + i, y);
    }
}

void mapgen_grid_update(map_t *map) {
    for (int i = 0; i < map->room_cnt; i++) {
        map_room_t room = map->rooms[i];

        assert((room.y + room.size.y) < MAP_SIZE);
        assert((room.x + room.size.x) < MAP_SIZE);

        switch (room.type) {
            case ROOM_CORRIDOR:
            case ROOM_BASE:
                // Walls
                grid_add_item_row(map, TILE_WALL_N, room.x + 1, room.y, room.size.x - 2);
                grid_add_item_row(map, TILE_WALL_S, room.x + 1, room.y + room.size.y - 1, room.size.x - 2);
                grid_add_item_col(map, TILE_WALL_E, room.x + room.size.x - 1, room.y + 1, room.size.y - 2);
                grid_add_item_col(map, TILE_WALL_W, room.x, room.y + 1, room.size.y - 2);

                // Floor
                for (int y = 1; y < room.size.y - 1; y++) {
                    for (int x = 1; x < room.size.x - 1; x++) {
                        tile_item_add(map, TILE_FLOOR, room.x + x, room.y + y);
                    }
                }

                // Doors
                if (room.way_in.x > 0 && room.way_in.y > 0) {
                    if (room.way_in.x == room.x || room.way_in.x == (room.x + room.size.x - 1)) {
                        tile_item_add(map, TILE_DOOR_EW, room.way_in.x, room.way_in.y);
                    }
                    if (room.way_in.y == room.y || room.way_in.y == (room.y + room.size.y - 1)) {
                        tile_item_add(map, TILE_DOOR_NS, room.way_in.x, room.way_in.y);
                    }
                }

                break;
        }
    }
}

void mapgen_grid_print(map_t map) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            printf("%c ", tile_to_char(map.grid[y][x]));
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
