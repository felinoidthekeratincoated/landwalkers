#ifndef TILE_HH
#define TILE_HH

#include <vector>
#include <string>
#include "Light.hh"

/* Forward declare! */
class Map;
struct Location;
struct SDL_Rect;

// A class for keeping track of which tiles there are
enum class TileType : short {
    EMPTY,
    WATER,
    DIRT,
    TOPSOIL,
    CLAY,
    CALCAREOUS_OOZE,
    SNOW,
    ICE,
    STONE,
    GRANITE,
    BASALT,
    LIMESTONE,
    MUDSTONE,
    PERIDOTITE,
    SANDSTONE,
    RED_SANDSTONE,
    PLATFORM,
    LUMBER,
    RED_BRICK,
    GRAY_BRICK,
    DARK_BRICK,
    GLASS,
    GLOWSTONE,
    TORCH,
    SAND,
    MUD,
    CLOUD,
    BOULDER,
    GLACIER,
    FIRST_TILE = EMPTY,
    FIRST_ITEMED_TILE = DIRT, // First tile with an equivalent item
    LAST_TILE = GLACIER,
    LAST_PURE_TILE = TORCH, // Last tile that's not a subclass
    FIRST_BOULDER = SAND,
    LAST_BOULDER = GLACIER,
};

/* When looking at the tiles next to them in picking a sprite, tiles with
the same edgetype count as next to them. */
enum class EdgeType {
    LIQUID,
    SOLID,
    PLATFORM,
    SOLITARY
};

/* A class to make tiles based on their type, and store their infos. */
// Maybe since maps are filled with pointers to the same tile, everything
// should be constant?
class Tile {
    // Collision-related varables
    /* Whether the tile is a platform. Palyers only colide with the tops side
    of platforms. */
    bool isPlatform;
    /* Whether players can pass through the tile. */
    bool isSolid;

    /* Whether it can be placed in the background layer. */
    bool canBackground;

    /* Whether it drops itself as an item when water touches it. */
    bool waterBreaks;

    // Display-related variables
    bool isAnimated;

    /* Nonzero if the block creates light. */
    Light emitted;

    /* How much light it blocks for each color, aside from the amount lost by
    distance, for edges. */
    DLight absorbed;

    /* Whether it is a source of natural light. For instance, an empty tile
    or one that is mostly transparent. */
    bool isSky;

    // Mining-related variables
    /* How much health the tile has determines how many hits it can take from
    a pickax before it breaks. */
    int maxHealth;

    /* How strong of a pickaxe is needed to break the tile at all. */
    int tier;

    /* What color should represent it in images. */
    Light color;

    /* How it should decide which tiles count as next to it for the purpose of
    picking a sprite. */
    EdgeType edgeType;

protected:

    /* Return the filename of the json file for that tiletype. */
    static std::string getFilename(TileType tileType);

public:
    // The name of this type of tile
    const TileType type;

    /* Access functions. */
    inline bool getIsSky() const {
        return isSky;
    }

    inline Light getColor() const {
        return color;
    }

    inline EdgeType getEdge() const {
        return edgeType;
    }

    inline bool getCanBackground() const {
        return canBackground;
    }

    inline bool getTier() const {
        return tier;
    }

    inline Light getEmitted() const {
        return emitted;
    }

    inline DLight getAbsorbed() const {
        return absorbed;
    }

    // Variables for how it interacts with the players
    bool getIsPlatform() const;
    bool getIsSolid() const;

    /* Basically the number of hits with a pickaxe to break it. */
    int getMaxHealth() const;

    /* Change the map in whatever way needs doing. */
    virtual bool update(Map &map, Location place, int tick);

    // Constructor, based on the tile type
    Tile(TileType tileType, std::string path);

    /* Virtual destructor. */
    virtual ~Tile();

    /* Whether the tile will ever need to call its update function. */
    virtual bool canUpdate(const Map &map, const Location &place);
};

#endif
