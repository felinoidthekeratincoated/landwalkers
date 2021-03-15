#ifndef PLAYER_HH
#define PLAYER_HH

#include <SDL2/SDL.h>
#include "Entity.hh"
#include "../ui/Hotbar.hh"
#include "../ui/Inventory.hh"
#include "../ui/StatBar.hh"
#include <string>

// Forward declare
class Action;

/* A class for a player. Maybe there should be a Character class it can 
    inherit from? 
    If it's unclear whether something belongs in the player class or as part of
    the eventhandler, the rule of thumb is to ask whether it belongs in the
    player's savefile or in the game's config file. */
class Player : public Entity {
    // How far can they reach to place tiles
    int tileReachUp;
    int tileReachDown;
    int tileReachSideways;

    /* Drop the item the mouse is holding on the ground and return the dropped
    item. */
    DroppedItem *drop();
public:

    /* Info for rendering stat amounts. */
    StatBar healthBar;
    StatBar fullnessBar;
    StatBar manaBar;

    /* The use time of the last item / skill used, minus the number of ticks 
    since using it. Items and skills can only be used when this is 0. */
    int useTimeLeft;

    // Where the player is being rendered on the screen
    int screenX, screenY;

    bool isInventoryOpen;
    Inventory inventory;
    Inventory trash;

    // So the mouse can move around items and put them in the hotbar
    Action *mouseSlot;

    Hotbar hotbar;

    // Constructor
    Player();

    // Switch the open / closed state of the inventory.
    void toggleInventory();

    // Whether a place is within range for placing tiles
    // x is distance from the player horizontally, in tiles.
    // y is distance from the player in tiles, with positive being above the 
    // player. bonus is the bonus range from possible unknown curcumstances
    // (e.g. this type of tile can be placed farther away, or this pickax has
    // better range).
    bool canReach(int x, int y, int bonus) const;

    // Whether the player can use an item / skill right now.
    bool canUse();

    // Use the item held or selected
    void useAction(InputType type, int x, int y, World &world);

    /* Update self, including statbars (so changes to stats actually render). */
    void update(std::vector<DroppedItem*> &drops);

    /* Place an item in the inventory or the hotbar. Return the item if it 
    doesn't fit. */
    Item *pickup(Item *item);

    // Try to pick up an item
    virtual void pickup(DroppedItem *item);

    /* Drop the items the mouse is holding to the ground and add it to the 
    vector. */
    inline void toss(std::vector<DroppedItem *> &drops) {
        DroppedItem *d = drop();
        if (d) {
            drops.push_back(d);
        }
    }
};

#endif
