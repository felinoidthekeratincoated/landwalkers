#include <iostream>
#include "Collider.hh"

// Number of updates to stand on a platform before dropping through
#define PLATFORM_FALL_DELAY 4

using namespace std;

// Constructor
Collider::Collider(int tileWidth, int tileHeight) : TILE_WIDTH(tileWidth),
    TILE_HEIGHT(tileHeight) {
    // Or disable collisions to get a map viewer
    enableCollisions = true;
    xOffset = 1;
    yOffset = 1;
}

// Given that a collision happens left or right, update info accordingly.
inline void Collider::findXCollision(CollisionInfo &info, int dx, 
        int w, const Rect &stays) const {
    // Collision is left or right
    if (dx < 0) {
        info.type = CollisionType::LEFT;
        info.x = stays.x + stays.w;
    }
    else {
        info.type = CollisionType::RIGHT;
        info.x = stays.x - w;
    }

}

// Given that a collision is up or down, update info accordingly
inline void Collider::findYCollision(CollisionInfo &info, int dy, 
        int h, const Rect &stays) const {
    // Collision is up or down
    if (dy < 0) {
        info.type = CollisionType::DOWN;
        info.y = stays.y + stays.h;
    }
    else {
        info.type = CollisionType::UP;
        info.y = stays.y - h;
    }
}

bool Collider::collidesTiles(const Rect &rect, Map &map) const {
    /* Rect for tiles. */
    Rect stays;
    stays.worldWidth = TILE_WIDTH * map.getWidth();
    stays.w = TILE_WIDTH;
    stays.h = TILE_HEIGHT;

    /* width and height are how many tiles away to check for collisions
    with tiles that it was already colliding with. */
    int width = rect.w / TILE_WIDTH + 2;
    int height = rect.h / TILE_HEIGHT + 2;
    int startX = rect.x / TILE_WIDTH;
    int startY = rect.y / TILE_HEIGHT;
    // Collide with the tiles it starts on
    for (int k = startX; k < startX + width; k++) {
        /* Adjust so 0 <= l < map.getWidth() */
        int l = (k + map.getWidth()) % map.getWidth();
        stays.x = l * TILE_WIDTH + xOffset;
        assert(0 <= stays.x);
        assert(stays.x < stays.worldWidth);
        for (int j = startY; j < startY + height; j++) {
            stays.y = j * TILE_HEIGHT + yOffset;
            assert(0 <= stays.y);
            /* Ignore tiles that don't exist. */
            if (!map.isOnMap(l, j)) {
                break;
            }
            /* If the player starts off overlapping this tile */
            if (stays.intersects(rect) && enableCollisions) {
                /* Deal damage based on tile type. */
                const Tile *tile = map.getForeground(l, j);
                /* If the tile is solid, then there is a collision with a 
                solid tile. */
                if (tile -> getIsSolid()) {
                    return true;
                }
            }
        }
    }
    /* None of the tiles were solid. */
    return false;
}

// Return information about a collision between a moving thing and a non-moving
// thing. 
CollisionInfo Collider::findCollision(const Rect &from, const Rect &to, 
        const Rect &stays) const {
    CollisionInfo info;
    info.x = 0;
    info.y = 0;
    info.xCoefficient = 1;
    info.yCoefficient = 1;
    info.isInevitable = false;

    int dx = to.x - from.x;
    int dy = to.y - from.y;

    // Check for collisions
    if (stays.intersects(to)) {
        // There's a collision.
        // If exactly one of the sides didn't use to be in the collision area
        // and now it is, we know the direction. (Actually, it can be two
        // parallel ones, since we know velocity.)
        // If the left or right side started off in the collision area
        if (stays.intersectsX(from)) {
            // Since this should only be called for things that didn't start 
            // already colliding, we know neither y side started in the 
            // collision area.
            assert(!stays.intersectsY(from));
            // Since there definately is a collision, it has to be up or down
            assert(dy != 0);
            findYCollision(info, dy, to.h, stays);
            // Whether a collision happens doesn't depend on x velocity
            info.isInevitable = true; 
            return info;
        }
        // If the top or bottom started off in the collision area
        else if (stays.intersectsY(from)) {
            // Likewise, we know it must be left or right
            assert(dx != 0);
            findXCollision(info, dx, to.w, stays);
            // And whether it happens doesn't depend on y velocity
            info.isInevitable = true;
            return info;
        }
        // Now we know none of the edges started off in the collision area,
        // so since we know there is a collision, there must be at least two
        // edges in the collision area.
        // Also for any of these cases isInevitable should stay false.
        int tx = min(abs(from.x - stays.x - stays.w), 
            abs(from.x - from.w - stays.x));
        int ty = min(abs(from.y - stays.y - stays.h),
            abs(from.y - from.h - stays.y));
        // Multiplying is more efficient than dividing, and only their
        // relative values matter anyway
        tx *= abs(dy);
        ty *= abs(dx);
        if (tx < ty) {
            // Collision is left or right
            findXCollision(info, dx, to.w, stays);
        }
        else if (ty < tx) {
            // Collision is up or down
            findYCollision(info, dy, to.h, stays);
        }
        // tx == ty and it's exactly hitting the corner
        else {
            findXCollision(info, dx, to.w, stays);
            if (info.type == CollisionType::LEFT) {
                info.type = CollisionType::LEFT_CORNER;
            }
            else {
                info.type = CollisionType::RIGHT_CORNER;
            }
        }
    }
    else {
        info.type = CollisionType::NONE;
    }

    return info;
}

/* Goes through the tiles near the player and adds all collisions found to 
the vector collisions. If any are inevitable, it returns true. Otherwise, it
returns false. If dropDown is true, it will ignore collisions with platforms.
Note that to.x can be less than 0 or greater than worldWidth. */
bool Collider::listCollisions(vector<CollisionInfo> &collisions, Map &map,
    const Rect &to, const Rect &from, bool dropDown) const {
    // The rectangle of the current tile to check
    Rect stays;
    stays.w = TILE_WIDTH - 2 * xOffset;
    stays.h = TILE_HEIGHT - 2 * yOffset;
    int worldWidth = map.getWidth() * TILE_WIDTH;
    stays.worldWidth = worldWidth;

    bool anyInevitable = false;

    /* Only these tiles can be colliding, given that we're moving 
    up to half a tilewidth and tileheight at a time and that we've already
    checked for collisions with the tiles we started on. 
    (Though it may miss a corner that was just barely hit) */
    /* If to.x is negative, to.x / TILE_WIDTH will round in the wrong
    direction. */
    int toX = to.x + to.worldWidth;
    for (int k = toX / TILE_WIDTH;
            k < (toX + to.w) / TILE_WIDTH + 1; k++) {
        int l = (k + map.getWidth()) % map.getWidth();
        stays.x = l * TILE_WIDTH + xOffset;
        for (int j = to.y / TILE_HEIGHT;
                j < (to.y + to.h) / TILE_HEIGHT + 1; j++) {
            // Don't collide with tiles not on the map
            if (j < 0 || j >= map.getHeight()) {
                continue;
            }
            const Tile *tile = map.getForeground(l, j);
            // Skip non-collidable tiles
            if (!(tile -> getIsSolid() || tile -> getIsPlatform())) {
                continue;
            }
            // Skip platforms if we should drop through them
            if (tile -> getIsPlatform() && dropDown) {
                continue;
            }
            stays.y = j * TILE_HEIGHT + yOffset;
            // Cases where they start off colliding should be dealt with
            // elsewhere.
            if (stays.intersects(from)) {
                continue;
            }
            CollisionInfo info = findCollision(from, to, stays);
            // Include information about what tile was collided
            info.resolve(tile);

            // Add this collision to the list to deal with later
            // Also check whether any collision is inevitable
            if (info.type != CollisionType::NONE) {
                if (info.isInevitable) {
                    anyInevitable = true;
                }
                collisions.push_back(info);
            }
        }
    }
    return anyInevitable; 
}

//  A function that moves a movable to where it should end up on a map. This 
// assumes that no collisions with anything other than the map will affect the
// end location. It also assumes collisions between the very corner of the 
// movable and the very corner of a tile should be collisions in the x 
// direction. This is not always the case, so after calling this function once
// it's best to call it again with the results assuming the y position is 
// correct but the x may have farther to move.
void Collider::collide(Map &map, movable::Movable &movable) {
    // Calculate the world width and height
    int worldWidth = map.getWidth() * TILE_WIDTH;
    int worldHeight = map.getHeight() * TILE_HEIGHT;

    // from is the rectangle of the player the update before, to is the 
    // rectangle the player would move to if it didn't collide with anything,
    // and stays is the tile currently being checked for collisions with the
    // player.
    Rect from;
    Rect to;
    Rect stays;

    /* Set the height and width of the rectangle that will hold each tile. */
    stays.w = TILE_WIDTH - 2 * xOffset;
    stays.h = TILE_HEIGHT - 2 * yOffset;
    assert(0 <= stays.w);
    assert(0 <= stays.h);
    stays.worldWidth = worldWidth;

    /* Set the starting location and the width and height. */
    from = movable.getRect();
    from.worldWidth = worldWidth;
    from.x = (from.x + worldWidth) % worldWidth;
    to = movable.getRect();
    to.worldWidth = worldWidth;

    assert(0 <= from.y);
    assert(0 <= from.w);
    assert(0 <= from.h);

    assert(0 <= to.w);
    assert(0 <= to.h);

    // Collide with tiles
    /* width and height are how many tiles away to check for collisions
    with tiles that it was already colliding with. */
    int width = movable.getWidth() / TILE_WIDTH + 2;
    int height = movable.getHeight() / TILE_HEIGHT + 2;
    int xVelocity = movable.getVelocity().x;
    int yVelocity = movable.getVelocity().y;
    int startX = from.x / TILE_WIDTH;
    int startY = from.y / TILE_HEIGHT; 
    // Collide with the tiles it starts on
    for (int k = startX; k < startX + width; k++) {
        /* Adjust so 0 <= l < map.getWidth() */
        int l = (k + map.getWidth()) % map.getWidth();
        stays.x = l * TILE_WIDTH + xOffset;
        assert(0 <= stays.x);
        assert(stays.x < worldWidth);
        for (int j = startY; j < startY + height; j++) {
            stays.y = j * TILE_HEIGHT + yOffset;
            assert(0 <= stays.y);
            /* Ignore tiles that don't exist. */
            if (!map.isOnMap(l, j)) {
                break;
            }
            /* If the player starts off overlapping this tile */
            if (stays.intersects(from) && enableCollisions) {
                /* Deal damage based on tile type. */
                const Tile *tile = map.getForeground(l, j);
                tile -> dealOverlapDamage(movable);
                /* If the tile is solid, set velocity to 0. */
                if (tile -> getIsSolid()) {
                    xVelocity = 0;
                    yVelocity = 0;
                    /* But also actually set the movable's velocity. */
                    movable.setVelocity({0, 0});
                }
            }
        }
    }

    // Collide with tiles it doesn't start on
    double xCoefficient = 1;
    double yCoefficient = 1;
    while (xVelocity != 0 || yVelocity != 0) {
        // The n is in case moludo results in 0 inconveniently
        int n = max(min(1, xVelocity), -1);
        int dx = (xVelocity - n) % (TILE_WIDTH / 2) + n;
        n = max(min(1, yVelocity), -1);
        int dy = (yVelocity - n) % (TILE_HEIGHT / 2) + n;
        to.x = from.x + dx;
        to.y = from.y + dy;
        /* Collide with the bottom and top of the map. */
        movable.isCollidingDown = to.collideEdge(worldHeight);
        /* If there was a collision with the bottom or top of the map,
        set velocity to 0. */
        if (movable.isCollidingDown) {
            movable::Point velocity = movable.getVelocity();
            velocity.y = 0;
            movable.setVelocity(velocity);
        }

        int newX = from.x;
        int newY = from.y;
        // A separate variable so that corner collisions can only happen if
        // no other collisions happen.
        int cornerX = to.x;
        assert(abs(xVelocity - dx) <= abs(xVelocity));
        assert(abs(yVelocity - dy) <= abs(yVelocity));
        xVelocity -= dx;
        yVelocity -= dy;
        // Check whether there are andy inevitable collisions we should 
        // handle first (they do need to be first, otherwise the player can
        // climb walls).
        bool anyInevitable = false;
        // Make a list of all collisions between the movable and any tiles 
        vector<CollisionInfo> collisions;
        // Whether we should drop down through platforms
        bool dropDown = movable.isDroppingDown
            && (movable.ticksCollidingDown >= PLATFORM_FALL_DELAY);
        anyInevitable = listCollisions(collisions, map, to, from, dropDown);

        // Only collide if collisions are enabled
        while (enableCollisions && (collisions.size() != 0)) {
           // And actually handle the collisions
            for (unsigned int i = 0; i < collisions.size(); i++) {
                CollisionInfo info = collisions[i];
                // Skip this one if necessary
                if (anyInevitable && (!info.isInevitable)) {
                    continue;
                }
                xCoefficient *= info.xCoefficient;
                yCoefficient *= info.yCoefficient;
                if (info.newX != -1) {
                    newX = info.newX;
                    movable.isCollidingX = true;
                }
                if (info.type == CollisionType::DOWN) {
                    movable.isCollidingDown = true;
                }
                if (info.newY != -1) {
                    newY = info.newY;
                }
                if (info.cornerX != -1) {
                    cornerX = info.cornerX;
                }
            }

            // If there weren't any inevitable collisions, handle corner 
            // collisions and restart
            // Only check for corner collisions when there were no others
            bool usedCorner = false;
            if (!anyInevitable && xCoefficient != 0 && yCoefficient != 0) {
                movable.isCollidingX = true;
                newX = cornerX;
                xCoefficient = 0;
                usedCorner = true;
            }
 
            // Move the rest of the way
            dx = to.x - newX;
            dy = to.y - newY;
            dx *= xCoefficient;
            dy *= yCoefficient;
            xVelocity *= xCoefficient;
            yVelocity *= yCoefficient;

            // And repeat until we're done
            from.x = newX;
            from.y = newY;
            to.x = from.x + dx;
            to.y = from.y + dy;

           if (!anyInevitable && !usedCorner) {
                // This actually happens surprisingly rarely
                break;
            }
            else {
            }
            // Otherwise, check again for collisions.
            collisions.clear();
            anyInevitable = listCollisions(collisions, map, to, from, dropDown);
        }

        // Set to where we're actaully moving to.
        from.x = to.x;
        from.y = to.y;
    }
    // If there were collisions, set the velocity to 0
    // But only in the x direction because otherwise it breaks stepping up
    movable::Point velocity = movable.getVelocity();
    velocity.y *= yCoefficient;
    movable.setVelocity(velocity);

    movable.setX(from.x);
    movable.setY(from.y);
    // Collide with the edge of the map
    // Wrap in the x direction
    movable.setX((movable.getRect().x + worldWidth) % worldWidth);

    /* Now time to see if we can update the movable's collision rect. */
    Rect nextRect = movable.getNextRect();
    nextRect.worldWidth = worldWidth;
    nextRect.x += movable.getRect().x;
    nextRect.y += movable.getRect().y;
    /* If the next rect doesn't overlap anything, we can update it. */
    if (!collidesTiles(nextRect, map)) {
        /* Unfortunately this breaks stuff. TODO: fix bugs and implement. */
        //movable.advanceRect();
    }

}


void Collider::updateMovable(Map &map, movable::Movable *movable) {
    // Update the velocity

    // TODO: replace this with gravity as a function of height and map
    double gravity = -1.5;
    movable -> updateMotion(gravity);

    if (!movable -> collides) {
        movable -> setX(movable -> getRect().x + movable -> getVelocity().x);
        movable -> setY(movable -> getRect().y + movable -> getVelocity().y);
        return;
    }

    // toX is the x value the movable expects to end up having.
    int toX = movable -> getRect().x + movable -> getVelocity().x;
    collide(map, *movable);
    // Because collide() may stop things in the x direction before it 
    // should, we should try again.
    // Actually since after corner collisions it will just step up and 
    // continue, this won't be a noticable bug most of the time, but might
    // as well fix it anyway.
    if (movable -> isCollidingX) {
        // Have it move only the rest of the way in the x direction, now.
        movable::Point newVelocity;
        newVelocity.x = toX - movable -> getRect().x;
        newVelocity.y = 0;
        movable::Point oldVelocity = movable -> getVelocity();
        movable -> setVelocity(newVelocity);
        collide(map, *movable);
        movable -> setVelocity(oldVelocity);
    }

    // Do the thing where colliding with a wall one block high doesn't
    // stop you
    if (movable -> isCollidingX) {
        movable::Movable hypothetical(*(movable));
        /* Don't have hypothetical change collision rect. */
        hypothetical.resetRect();
        // See if it can go up one tile or less without colliding
        int oldY = hypothetical.getRect().y;
        // The amount to move by to go up one tile or less, assuming 
        // gravity is in the usual direction
        // TODO: it probably doesn't matter, but this is inaccurate when 
        // the movable is in the air
        int dy = TILE_HEIGHT - ((oldY + yOffset)  % TILE_HEIGHT);
        assert(dy <= TILE_HEIGHT);
        assert(dy > 0);
        hypothetical.setVelocity({0, (double)dy});
        collide(map, hypothetical);
        // If there wasn't a collision
        if (hypothetical.getRect().y == oldY + dy) {
            assert(hypothetical.getRect().x == movable -> getRect().x);
            assert(movable -> getRect().y == oldY);
            // check that it would end up standing on the tile, and not 
            // randomly jump up and fall back down
            // dx is how much more it could have gone in the x direction, 
            // if it didn't collide with the tile it's stepping up
            int dx = toX - hypothetical.getRect().x;
            hypothetical.setVelocity({(double)dx, 0});
            collide(map, hypothetical);
            if (hypothetical.getRect().x != movable -> getRect().x) {
                // Ok, so we do want to jump up and continue
                // doing that instantaneously would look like:
                // movables[i] -> x = hypothetical.x;
                // movables[i] -> y = hypothetical.y;
                // but we actually only want to go up by one x velocity
                // (not one y velocity because the whole point of this is 
                // that you go up without jumping).
                /* If it can jump up and still go sideways, or if it
                is within one tile of the bottom of the map, it can stand
                on the tile it's stepping up to. */
                if (dy < abs(movable -> getVelocity().x)
                            || movable -> getRect().y < TILE_HEIGHT) {
                    movable -> setX(hypothetical.getRect().x);
                    movable -> setY(hypothetical.getRect().y);
                }
                /* Else it will go partway up and temporarily ignore
                gravity. */
                else {
                    movable -> setY(movable -> getRect().y 
                            + abs(movable -> getVelocity().x));
                    movable -> isSteppingUp = true;
                }
            }
        }
    }
}

// A function to move and collide the movables
// Note that this only ever resets distance fallen when it hits the ground.
void Collider::update(Map &map, vector<Entity *> &entities,
        vector<DroppedItem *> droppedItems) {
    /* Update dropped items. This needs to happen between when map collisions
    get handled and when things try to attract dropped items. */
    for (unsigned int i = 0; i < droppedItems.size(); i++) {
        droppedItems[i] -> update();
    }

    // Have entities with inventories pick up dropped items if they can
    for (unsigned int i = 0; i < entities.size(); i++) {
        // Make sure worldwith is updated correctly
        entities[i] -> setWorldwidth(map.getWidth() * map.getTileWidth());

        // Skip things that can't pick up items
        if (!entities[i] -> getHasInventory()) {
            continue;
        }

        // Check for each item
        for (unsigned int j = 0; j < droppedItems.size(); j++) {
            entities[i] -> pickup(droppedItems[j]);
        }
    }

    // Have dropped items try to merge with each other
    for (unsigned int i = 0; i < droppedItems.size(); i++) {
        for (unsigned int j = i + 1; j < droppedItems.size(); j++) {
            /* Merging in this order means the older item will still be at 
            the front of the vector. */
            droppedItems[j] -> merge(droppedItems[i]);
        }
    }

    // TODO: collide entities and attackboxes

    // Update the velocity of everythingdelete
    for (unsigned i = 0; i < entities.size(); i++) {
        updateMovable(map, (movable::Movable *)entities[i]);
    }

    for (unsigned i = 0; i < droppedItems.size(); i++) {
        updateMovable(map, (movable::Movable *)droppedItems[i]);
    }

}

