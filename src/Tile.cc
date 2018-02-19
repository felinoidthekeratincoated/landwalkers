#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include "Tile.hh"
#include "Map.hh"
#include "Movable.hh"
#include "json.hpp"
#include "filepaths.hh"
#include <SDL2/SDL.h>

// For convenience
using json = nlohmann::json;

using namespace std;

/* Get the filename of the json for this tiletype. */
std::string Tile::getFilename(TileType tileType) {
    string filename;
    string prefix = "tiles/";

    /* Figure out the right json file to use. */
    switch(tileType) {
        case TileType::EMPTY : 
            filename = "empty.json";
            break;
        case TileType::WATER :
            filename = "water.json";
            break;
        case TileType::DIRT :
            filename = "dirt.json";
            break;
        case TileType::TOPSOIL :
            filename = "topsoil.json";
            break;
        case TileType::CLAY :
            filename = "clay.json";
            break;
        case TileType::CALCAREOUS_OOZE :
            filename = "calcareous_ooze.json";
            break;
        case TileType::SNOW :
            filename = "snow.json";
            break;
        case TileType::ICE :
            filename = "ice.json";
            break;
        case TileType::STONE :
            filename = "stone.json";
            break;
        case TileType::GRANITE :
            filename = "granite.json";
            break;
        case TileType::BASALT : 
            filename = "basalt.json";
            break;
        case TileType::LIMESTONE : 
            filename = "limestone.json";
            break;
        case TileType::MUDSTONE :
            filename = "mudstone.json";
            break;
        case TileType::PERIDOTITE :
            filename = "peridotite.json";
            break;
        case TileType::SANDSTONE :
            filename = "sandstone.json";
            break;
        case TileType::RED_SANDSTONE :
            filename = "red_sandstone.json";
            break;
        case TileType::PLATFORM :
            filename = "platform.json";
            break;
        case TileType::LUMBER :
            filename = "lumber.json";
            break;
        case TileType::RED_BRICK :
            filename = "red_brick.json";
            break;
        case TileType::GRAY_BRICK :
            filename = "gray_brick.json";
            break;
        case TileType::DARK_BRICK : 
            filename = "dark_brick.json";
            break;
        case TileType::SAND :
            filename = "sand.json";
            break;
        case TileType::MUD :
            filename = "mud.json";
            break;
        case TileType::CLOUD :
            filename = "cloud.json";
            break;
        case TileType::BOULDER :
            filename = "boulder.json";
            break;
        case TileType::GLACIER :
            filename = "glacier.json";
            break;
    }

    return prefix + filename;
}

// All the access functions
bool Tile::getIsPlatform() const {
    return isPlatform;
}

bool Tile::getIsSolid() const {
    return isSolid;
}

int Tile::getOpacity() const {
    return opacity;
}

void Tile::render(uint8_t spritePlace, const Light &light, 
        const SDL_Rect &rectTo) {
    if (!sprite.hasTexture()) {
        return;
    }

    /* Use darkness. */
    sprite.setColorMod(light);
    Location spriteLocation;
    SpaceInfo::fromSpritePlace(spriteLocation, spritePlace);
    assert(spriteLocation.x >= 0);
    assert(spriteLocation.y >= 0);
    assert(sprite.getWidth() > 0);
    assert(sprite.getHeight() > 0);
    sprite.move(spriteLocation.x * sprite.getWidth(), 
            spriteLocation.y * sprite.getHeight());
    sprite.render(rectTo);
}



int Tile::getMaxHealth() const {
    return maxHealth;
}

/* Deal damage to whatever is overlapping this, and stop it if this tile is 
solid. */
void Tile::dealOverlapDamage(movable::Movable &movable) const {
    movable.takeDamage(overlapDamage);
}

uint8_t Tile::getSpritePlace(Map &map, const Location &place) const {
    int x;
    int y;
    /* Some (empty) tiles have no sprite. */
    if (!sprite.hasTexture()) {
        x = 0;
        y = 0;
        return SpaceInfo::toSpritePlace(x, y);
    }
    y = map.bordering(place);
    x = rand() % sprite.getCols() / 2;
    /* On the sprite, the equivalent background tile is moved over by
    sprite.cols / 2. */
    if (place.layer == MapLayer::BACKGROUND) {
        x += sprite.getCols() / 2;
    }

    return SpaceInfo::toSpritePlace(x, y);
}

uint8_t Tile::updateSprite(Map &map, const Location &place) const {
    int y = map.bordering(place);
    int x = SpaceInfo::getX(map.getSprite(place));
    return SpaceInfo::toSpritePlace(x, y);
}

/* Change the map in whatever way needs doing. */
bool Tile::update(Map &map, Location place, int tick) {
    return false;
}

// Constructor, based on the tile type
Tile::Tile(TileType tileType, string path) 
        : type(tileType) {
    string filename = path + getFilename(tileType);
    /* Put data in json. */
    ifstream infile(filename);
    /* Check that file was opened successfully. */
    if (!infile) {
        cerr << "Can't open " << filename << "\n";
    }
    json j = json::parse(infile);
    /* Set each of this tile's non-const values equal to the json's values. */
    sprite = j["sprite"].get<Sprite>();
    color = j["color"].get<Light>();
    isSolid = j["isSolid"];
    isPlatform = j["isPlatform"];
    overlapDamage = j["overlapDamage"].get<Damage>();
    maxHealth = j["maxHealth"];
    opacity = j["opacity"];
    int edgeInt = j["edgeType"];
    edgeType = (EdgeType)edgeInt;
    sprite.loadTexture(path + TILE_SPRITE_PATH);
}

/* Virtual destructor. */
Tile::~Tile() {}

/* Whether the tile will ever need to call its update function. */
bool Tile::canUpdate(const Map &map, const Location &place) {
    return false;
}

void Tile::render(uint8_t spritePlace, const Light &light, 
        const SDL_Rect &rectTo);