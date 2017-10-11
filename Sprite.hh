#ifndef SPRITE_HH
#define SPRITE_HH

#include <SDL2/SDL.h>
#include <string>
#include <memory>
#include "Texture.hh"
#include "SpriteBase.hh"
#include "json.hpp"

// Forward declare
struct SDL_Texture;

class Sprite: public SpriteBase {
public:
    /* Rectangle of where to render from. */
    SDL_Rect rect;

    virtual int getWidth() const;

    virtual int getHeight() const;

    /* Returns the number of columns spriteWidth apart this spritesheet can 
    hold. */
    inline int getCols() const {
        return getTextureWidth() / rect.w;
    }

    /* Returns the number of rows of spriteHeight this spritesheet can hold. */
    inline int getRows() const {
        return getTextureHeight() / rect.h;
    }

    /* The constructor. */
    inline Sprite() {
        name = "";
        rect.x = 0;
        rect.y = 0;
        rect.w = 0;
        rect.h = 0;
    }

    /* Constructor that takes arguments. */
    inline Sprite(int x, int y, int w, int h, std::string spritename) {
        name = spritename;
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
    }

    /* Assignment operator. */
    Sprite &operator=(const Sprite &sprite);

    /* Render itself. */
    virtual void render(const SDL_Rect &rectTo);

    /* Use a different part of the spritesheet. */
    inline void move(int x, int y) {
        rect.x = x;
        rect.y = y;
    }

    inline void setColorMod(const Light &light) {
        color = light;
    }
};

/* Get a sprite from a json. */
void from_json(const nlohmann::json &j, Sprite &sprite);

#endif
