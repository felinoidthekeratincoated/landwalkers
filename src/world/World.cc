#include "World.hh"

#define ITEM_LIMIT 400

using namespace std;

World::World(string filename, int tileWidth, int tileHeight) 
        : map(filename, tileWidth, tileHeight), player(),
        collider(tileWidth, tileHeight) {
    
    entities.push_back(&player);
    /* Set the player's position to the spawnpoint. */
    player.setX(map.getSpawn().x * tileWidth);
    player.setY(map.getSpawn().y * tileHeight);
}

World::~World() {
    while (!droppedItems.empty()) {
        delete droppedItems.back();
        droppedItems.erase(droppedItems.end() - 1);
    }
}

void World::update() {

    /* TODO: update all entities. */
    player.update(droppedItems);


    /* Move things around. */
    collider.update(map, entities, droppedItems);

    /* Despawn dropped items that don't exist anymore. */
    vector<DroppedItem*>::iterator iter = droppedItems.begin();
    while (iter != droppedItems.end()) {
        if (*iter == nullptr || (*iter) -> item == nullptr) {
            delete *iter;
            iter = droppedItems.erase(iter);
        }
        else {
            ++iter;
        }
    }

    /* Despawn items if there are too many. */
    /* Note: erasing from the front of a vector is costly. If this is a problem,
    maybe only do this once every few seconds. Alternately, can use an array
    with some mechanism of keeping track of where the line between newest and
    oldest items is. */
    if (droppedItems.size() > ITEM_LIMIT) {
        for (int i = 0; i < (int)droppedItems.size() - ITEM_LIMIT; i++) {
            delete droppedItems[i];
            droppedItems[i] = nullptr;
        }
        droppedItems.erase(droppedItems.begin(), 
            droppedItems.begin() + (droppedItems.size() - ITEM_LIMIT));
    }

    /* Have every movable take fall damage. */
    for (unsigned int i = 0; i < entities.size(); i++) {
        entities[i] -> takeFallDamage();
    }

    /* Have the map update itself and relevent entities. */
    map.update(droppedItems);

}
