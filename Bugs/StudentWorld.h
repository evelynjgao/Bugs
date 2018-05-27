#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <list>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class Actor;
class Compiler;

const int m_ticksToGameOver = 2000;

//depths
const int pebbleDepth = 1;
const int foodDepth = 2;
const int pheromoneDepth = 2;
const int antHillDepth = 2;
const int insectDepth = 1;
const int triggerableActorDepth = 2;

//starting hit points
const int startingPheromoneHitPoints = 256;
const int startingAntHitPoints = 1500;
const int startingAntHillHitPoints = 8999;
const int startingBabyGrassHopperHitPoints = 500;
const int babyToAdultGHThreshhold = 1600;
const int startingFoodHitPoints = 6000;
const int deadInsectFoodHitPoints = 100;

//max hit point effect
const int poisonEffect = 150;
const int antBiteDamage = 15;
const int adultGHBiteDamage = 50;
const int maxGrassHopperEat = 200;
const int maxAntEat = 100;
const int maxAntHillEat = 10000;
const int newAntThreshhold = 2000;
const int energyToCreateNewAnt = 1500;
const int maxPheromoneHitPoints = 768;

const int antHungryThreshhold = 25;
const int antMaxFoodHold = 1800;
const int antPickUpFood = 400;
const int antMaxCommandNumber = 10;
const int grassHopperMaxCommandNumber = 100;

const int adultGHJumpRadius = 10;


class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();
    
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    // Can an insect move to x,y?
    bool canMoveTo(int x, int y) const;
    
    // Add an actor to the world
    void addActor(Actor* a);
    
    // If an item that can be picked up to be eaten is at x,y, return a
    // pointer to it; otherwise, return a null pointer.  (Edible items are
    // only ever going be food.)
    Actor* getEdibleAt(int x, int y) const;
    
    // If a pheromone of the indicated colony is at x,y, return a pointer
    // to it; otherwise, return a null pointer.
    Actor* getPheromoneAt(int x, int y, int colony) const;
    
    // Is an enemy of an ant of the indicated colony at x,y?
    bool isEnemyAt(int x, int y, int colony) const;
    
    // Is something dangerous to an ant of the indicated colony at x,y?
    bool isDangerAt(int x, int y, int colony) const;
    
    // Is the anthill of the indicated colony at x,y?
    bool isAntHillAt(int x, int y, int colony) const;
    
    // Bite an enemy of an ant of the indicated colony at me's location
    // (other than me; insects don't bite themselves).  Return true if an
    // enemy was bitten.
    bool biteEnemyAt(Actor* me, int colony, int biteDamage);
    
    // Poison all poisonable actors at x,y.
    bool poisonAllPoisonableAt(int x, int y);
    
    // Stun all stunnable actors at x,y.
    bool stunAllStunnableAt(int x, int y);
    
    // Record another ant birth for the indicated colony.
    void increaseScore(int colony);
    
    // Add an adult grasshopper at x,y.
    void addAdultGH(int x, int y);
    
    // Add to pheromone at x, y or create new pheromone
    void addToPheromoneAt(int x, int y, int colony);
    
    // Gets current tick count
    int getCurrentTicks() const;
    
private:
    // 2D array of lists of actor pointers
    std::list<Actor*> m_actors[VIEW_HEIGHT][VIEW_WIDTH];
    
    // vector of entrant file fileNames
    std::vector<std::string> fileNames;
    
    // array to keep track of how many ants each colony has
    int m_numberOfAnts[MAX_ANT_COLONIES];
    
    // compiler array, index corresponds to colony
    Compiler* compilersForEntrants[MAX_ANT_COLONIES];
    
    //keeps track of winning ant
    int m_winningAnt;
    
    //keeps track of how many ticks have passed
    int m_countTicks;
    
    //helper functions for move
    void updateTickCount();
    void removeDeadActors();
    void updateDisplayText();
    std::string formatString(int ticks) const;
    
    //debugging method
    //void printField();
};

#endif // STUDENTWORLD_H_
