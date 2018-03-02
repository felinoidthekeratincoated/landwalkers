#include <iostream>
#include "Hotbar.hh"
#include "filepaths.hh"

// The number of slots in the hotbar

using namespace std;

// Render each texture from textures onto to, using the spacing variables
// from hotbar. The texture to is expected to have the correct width and
// height, and the vector is expected to have length 12. 
SDL_Texture *Hotbar::renderHotbarPart(int row, string path, 
        SDL_Texture *texture) const {
    assert(row == 0 || row == 1);
    // Make a texture if necessary
    if (texture == nullptr) {
        // Create texture to draw to
        // Get the width and height of the textures to render
        // This function assumes they are all the same
        int width = frame.getWidth();
        int height = frame.getHeight();
        int totalWidth = 12 * width + 12 * smallGap;
        totalWidth += 2 * largeGap;
        Uint32 pixelFormat = SDL_PIXELFORMAT_ARGB8888;
        texture = SDL_CreateTexture(Renderer::renderer, pixelFormat, 
        SDL_TEXTUREACCESS_TARGET, totalWidth, height);
        // Make the new texture have a transparent background
        SDL_SetRenderTarget(Renderer::renderer, texture);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Renderer::renderer, 0, 0, 0 ,0);
        SDL_RenderClear(Renderer::renderer);
    }

    // Set render settings
    SDL_SetRenderTarget(Renderer::renderer, texture);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // Set the draw color to white so it draws whatever it's drawing normally
    Renderer::setColorWhite();

    // Actually render
    SDL_Rect refRect;
    refRect.w = frame.getWidth();
    refRect.h = frame.getHeight();
    // For each slot
    for (int i = 0; i < 12; i++) {
        // We know the clickboxes have the correct spacing, but the first one 
        // probably isn't at 0, 0. So we just correct for that.
        refRect.x = clickBoxes[row][i].x - clickBoxes[row][0].x;
        refRect.y = clickBoxes[row][i].y - clickBoxes[row][0].y;
        /* Render the item. */
        // TODO
        /*
        if (actions[row][i]) {
            actions[row][i] -> render(refRect, path);
        }
        */
        if (false) {}
        else if (items[row][i]) {
            items[row][i] -> render(refRect, path);
        }

        /* Render the frame. */
        Sprite f = frame;
        if (selected == 12 * row + i) {
            f.rect.y += frame.getHeight();
        }
        f.render(refRect);
    }

    return texture;
}

// Draw the entire hotbar sprite to a texture. This only needs to be called 
// when the hotbar is first made, or when anything about it changes.
void Hotbar::updateSprite(string path) {
    /* Sprite will soon be updated. */
    isSpriteUpdated = true;
    // Make the texture to render the sprite of the hotbar to
    // Hardcoding 12 because I don't expect to change my mind (12 is the
    // number of F keys).
    int width = 12 * frame.getWidth() + 12 * smallGap;
    width += 2 * largeGap + offsetRight;
    int height = frame.getHeight() + offsetDown;
    sprite.rect.w = width;
    sprite.rect.h = height;
    Uint32 pixelFormat = SDL_PIXELFORMAT_ARGB8888;
    // Actually create the texture
    Texture *all;
    if (sprite.texture == nullptr) {
        all = new Texture(pixelFormat, SDL_TEXTUREACCESS_TARGET, width, height);
        // Set the sprite thing in the hotbar to the texture we just made
        sprite.texture.reset(all);
    }
    else {
        all = sprite.texture.get();
    }

    /* Render */
    SDL_Texture *front = renderHotbarPart(0, path, nullptr);
    SDL_Texture *back = renderHotbarPart(1, path, nullptr);

    all -> SetRenderTarget();
    // Tell SDL to do transparency when it renders
    all -> SetTextureBlendMode(SDL_BLENDMODE_BLEND);
    // Set draw color to transparent so the texture has a transparent 
    // background
    SDL_SetRenderDrawColor(Renderer::renderer, 0, 0, 0 ,0);
    SDL_RenderClear(Renderer::renderer);

    // Actually render the sprite onto the texture
    Renderer::setColorWhite();
    // TODO: make the back layer slightly transparent
    SDL_Rect rectTo;
    rectTo.x = 0;
    rectTo.y = 0;
    rectTo.w = width - offsetRight;
    rectTo.h = height - offsetDown;
    // Render the back layer
    SDL_RenderCopy(Renderer::renderer, back, NULL, &rectTo);
    // Render the front layer
    rectTo.x = offsetRight;
    rectTo.y = offsetDown;
    SDL_RenderCopy(Renderer::renderer, front, NULL, &rectTo);

    // Not leak memory
    SDL_DestroyTexture(front);
    SDL_DestroyTexture(back);
}

// Constructor, which fills it with default values
Hotbar::Hotbar(string path) : Inventory(12, 2, path) {
    // If you want to change these default settings, this is the place in the 
    // code to do it.
    smallGap = 4;
    largeGap = 16;
    offsetRight = 0;
    offsetDown = 36;
    // Where to draw the hotbar
    xStart = 20;
    yStart = 10;

    /* TODO: move magic numbers to a json file or something. */
    /* x, y, w, h, name */
    frame = Sprite(0, 0, 32, 32, "frame.png");

    int spriteWidth = 12 * frame.getWidth() + 12 * smallGap + 2 * largeGap 
        + offsetRight;
    int spriteHeight = frame.getHeight() + offsetDown;
    sprite = Sprite(0, 0, spriteWidth, spriteHeight, "");

    isSpriteUpdated = false;

    selected = 0;

    int x = xStart + offsetRight;
    // For each section of four
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            int index = 4 * i + j;
            clickBoxes[0][index].x = x;
            clickBoxes[0][index].y = yStart + offsetDown;
            clickBoxes[0][index].w = frame.getWidth();
            clickBoxes[0][index].h = frame.getHeight();
            clickBoxes[0][index].wasClicked = false;
            clickBoxes[0][index].containsMouse = false;
            clickBoxes[1][index].x = x - offsetRight;
            clickBoxes[1][index].y = yStart;
            clickBoxes[1][index].w = frame.getWidth();
            clickBoxes[1][index].h = frame.getHeight();
            clickBoxes[1][index].wasClicked = false;
            clickBoxes[1][index].containsMouse = false;
            x += frame.getWidth() + smallGap;
        }
        x += largeGap;
    }

    // Set every action * to NULL
    // TODO
    /*
    actions.resize(24);
    for (unsigned int i = 0; i < actions.size(); i++) {
        actions[i] = {NULL, false};
    }
    */

    /* Load the frame's texture. */
    frame.loadTexture(path + UI_SPRITE_PATH);
}

// Select a slot
void Hotbar::select(int slot) {
    if (slot != selected) {
        selected = slot;
        isSpriteUpdated = false;
    }
}

// Use mouse input, return true if the item the mouse was holding should
// be put in the inventory
void Hotbar::update(Action *&mouse, bool isInvOpen) {
    Inventory::update();
    if (isInvOpen && (!mouse || mouse -> isItem())) {
        Inventory::update_internal(mouse);
    }
    for (unsigned int row = 0; row < clickBoxes.size(); row++) {
        for (unsigned int col = 0; col < clickBoxes[row].size(); col++) {
            // Ignore mouse button up or mouse button held down
            if (clickBoxes[row][col].wasClicked && !clickBoxes[row][col].isHeld
                    && clickBoxes[row][col].event.type == SDL_MOUSEBUTTONDOWN ) {
                /* Select it if it wasn't already. */
                select(row * clickBoxes[row].size() + col);
                // See if we should put something in the slot
                if (clickBoxes[row][col].event.button == SDL_BUTTON_LEFT
                        && mouse) {
                    // TODO
                    /* And now the sprite needs to change. */
                    isSpriteUpdated = false;
                }
                // If it was a right click, we should remove that item from the
                // hotbar.
                else if (clickBoxes[row][col].event.button == SDL_BUTTON_RIGHT) {
                    // TODO
                    //actions[i].action == nullptr;
                    /* And again the sprite needs to change. */
                    isSpriteUpdated = false;
                }
                /* Now we've used this click. */
                clickBoxes[row][col].wasClicked = false;
            }
        }
    }
    Inventory::resetClicks();
}

// Return the selected action
Action *Hotbar::getSelected() {
    // TODO
    /*
    return actions[selected].action;
    */
    return items[selected / 12][selected % 12];
}

void Hotbar::render(string path) {
    // Re-render the sprite if necessary
    if (!isSpriteUpdated) {
        updateSprite(path);
    }

    // Make sure the renderer isn't rendering to a texture
    SDL_SetRenderTarget(Renderer::renderer, NULL);

    // Create a rect to render to
    SDL_Rect rectTo = {xStart, yStart, sprite.getWidth(), sprite.getHeight()};

    // Render
    sprite.render(rectTo);
}

