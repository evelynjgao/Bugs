#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "Compiler.h"

class StudentWorld;

/*-------------------------------------ACTOR CLASS DECLARATIONS------------------------------------*/
class Actor : public GraphObject{
public:
    // Constructor
    Actor(StudentWorld* world, int imageID, int startX, int startY, Direction startDir, int depth);
    
    // Action to perform each tick.
    virtual void doSomething() = 0;
    
    // Is this actor dead?
    virtual bool isAlive() const;
    
    // Does this actor block movement?
    virtual bool blocksMovement() const;
    
    // Cause this actor to be be bitten, suffering an amount of damage.
    virtual void getBitten(int amt);
    
    // Cause this actor to be be poisoned.
    virtual void getPoisoned();
    
    // Cause this actor to be be stunned.
    virtual void getStunned();
    
    // Can this actor be picked up to be eaten?
    virtual bool isEdible() const;
    
    // Is this actor detected by an ant as a pheromone?
    virtual bool isPheromone(int colony) const;
    
    // Is this actor an enemy of an ant of the indicated colony?
    virtual bool isEnemy(int colony) const;
    
    // Is this actor detected as dangerous by an ant of the indicated colony?
    virtual bool isDangerous(int colony) const;
    
    // Is this actor the anthill of the indicated colony?
    virtual bool isAntHill(int colony) const;
    
    virtual bool becomesFoodUponDeath() const;
    
    // Get this actor's world.
    StudentWorld* getWorld() const;
    
    // Destructor
    virtual ~Actor();
    
private:
    // member variables
    StudentWorld* m_World;
};

/*-------------------------------------PEBBLE CLASS DECLARATIONS-------------------------------------*/

class Pebble : public Actor{
public:
    Pebble(StudentWorld* world, int startX, int startY);
    virtual void doSomething();
    virtual bool blocksMovement() const;
};

/*-----------------------------------ENERGY HOLDER CLASS DECLARATIONS--------------------------------*/

class EnergyHolder : public Actor{
public:
    // Constructor
    EnergyHolder(StudentWorld* world, int imageID, int startX, int startY, Direction startDir, int depth, int energy);
    // Energy holders are alive if their energy is above 0
    virtual bool isAlive() const;
    // Get this actor's amount of energy (for a Pheromone, same as strength).
    int getEnergy() const;
    
    // Adjust this actor's amount of energy upward or downward.
    void updateEnergy(int amt);
    
    // Add an amount of food to this actor's location.
    void addFood(int amt);
    
    // Have this actor pick up an amount of food and eat it.
    int pickupAndEatFood(int amt);
    
    // virtual destructor for base class
    virtual ~EnergyHolder();
private:
    int m_energy;
};

/*--------------------------------------FOOD CLASS DECLARATIONS----------------------------------------*/

class Food : public EnergyHolder
{
public:
    Food(StudentWorld* world, int startX, int startY, int energy);
    virtual void doSomething();
    virtual bool isEdible() const;
};

/*------------------------------------PHEROMONE CLASS DECLARATIONS-------------------------------------*/

class Pheromone : public EnergyHolder
{
public:
    Pheromone(StudentWorld* world, int imageID, int startX, int startY, int colony);
    virtual void doSomething();
    virtual bool isPheromone(int colony) const;
    
    // Increase the strength (i.e., energy) of this pheromone.
    void increaseStrength();
private:
    int m_colony;
};

/*-----------------------------------ANTHILL CLASS DECLARATIONS----------------------------------------*/

class AntHill : public EnergyHolder
{
public:
    AntHill(StudentWorld* world, int startX, int startY, int colony, Compiler* program);
    virtual void doSomething();
    virtual bool isAntHill(int colony) const;
private:
    //each anthill has a colony number and a compiler
    int m_colonyNumber;
    Compiler* m_compiler;
};

/*----------------------------TRIGGERABLE ACTORS: WATERPOOLS AND POISON--------------------------------*/

class TriggerableActor : public Actor
{
public:
    TriggerableActor(StudentWorld* world, int imageID, int startX, int startY);
     // all triggerable actors are dangerous
    virtual bool isDangerous(int colony) const;
     // virtual destructor in base class
    virtual ~TriggerableActor();
};

/*--------------------------------------WATER POOL CLASS DECLARATIONS----------------------------------*/

class WaterPool : public TriggerableActor
{
public:
    WaterPool(StudentWorld* world, int startX, int startY);
    virtual void doSomething();
};

/*---------------------------------------POISON CLASS DECLARATIONS-------------------------------------*/

class Poison : public TriggerableActor
{
public:
    Poison(StudentWorld* world, int startX, int startY);
    virtual void doSomething();
};

/*--------------------------INSECT CLASS DECLARATIONS: ANTS AND GRASSHOPPERS---------------------------*/

class Insect : public EnergyHolder
{
public:
    Insect(StudentWorld* world, int imageID, int startX, int startY, int energy);
    virtual void doSomething();
    // only insects can be bitten, poisoned, or stunned
    virtual void getBitten(int amt);
    virtual void getPoisoned();
    virtual void getStunned();
    // most insects are enemies
    virtual bool isEnemy(int colony) const;
    // all insects become food upon death
    virtual bool becomesFoodUponDeath() const;
    
    // Set x,y to the coordinates of the spot one step in front of this insect.
    void getXYInFrontOfMe(int &x, int &y) const;
    
    // Move this insect one step forward if possible, and return true;
    // otherwise, return false without moving.
    virtual bool moveForwardIfPossible();
    
    // Increase the number of ticks this insect will sleep by the indicated amount.
    void increaseSleepTicks(int amt);
    
    // mutator and accessor methods
    int getWalkingDistance() const;
    int getSleepTicks() const;
    void setWalkingDistance(int distance);
    bool isBitten() const;
    void setStunned(bool stun);
    void setPoisoned(bool poison);
    void setBitten(bool bitten);
    
    // virtual destructor for base class
    virtual ~Insect();
private:
    virtual void doSomethingDifferent() = 0;
    int m_sleepingTicks;
    int m_walkingDistance;
    // boolean keeps track if it was stunned, poisoned, or bitten
    bool m_stunned;
    bool m_poisoned;
    bool m_bitten;
};

/*-------------------------------------ANT CLASS DECLARATIONS------------------------------------------*/

class Ant : public Insect
{
public:
    Ant(StudentWorld* world, int imageID, int startX, int startY, int colony, Compiler* program);
    virtual bool isEnemy(int colony) const;
    virtual void getBitten(int amt);
    virtual bool moveForwardIfPossible();
    // Have this actor pick up an amount of food.
    int pickupFood(int amt);
private:
    virtual void doSomethingDifferent();
    //accessor and mutator methods
    bool getBlocked();
    void setBlocked(bool block);
    int getFood();
    void setFood(int amt);
    void updateFood(int amt);
    //member variables
    bool m_blocked;
    int m_colony;
    int m_foodUnit;
    int m_lastRandomNumber;
    int m_ic;
    Compiler* m_compiler;
};

/*------------------------------------GRASSHOPPER CLASS DECLARATIONS-----------------------------------*/

class Grasshopper : public Insect
{
public:
    Grasshopper(StudentWorld* world, int imageID, int startX, int startY, int energy);
    virtual ~Grasshopper();
private:
    //pure virtual method -> grasshopper is an abstract class
    virtual void doSomethingDifferent();
    virtual int doDifferentStuff() = 0;
};

/*----------------------------------BABY GRASSHOPPER CLASS DECLARATIONS--------------------------------*/

class BabyGrasshopper : public Grasshopper
{
public:
    BabyGrasshopper(StudentWorld* world, int startX, int startY);
private:
    virtual int doDifferentStuff();
};

/*------------------------------------ADULT GRASSHOPPER CLASS DECLARATIONS-----------------------------*/

class AdultGrasshopper : public Grasshopper
{
public:
    AdultGrasshopper(StudentWorld* world, int startX, int startY);
    virtual void getBitten(int amt);
    // adult grasshopprs can't be poisoned or stunned
    virtual void getPoisoned();
    virtual void getStunned();
private:
    virtual int doDifferentStuff();
};

#endif // ACTOR_H_
