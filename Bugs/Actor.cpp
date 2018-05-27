#include "Actor.h"
#include "StudentWorld.h"
#include "Compiler.h"
#include <cmath>
using namespace std;

/*-------------------------------------ACTOR CLASS IMPLEMENTATIONS-------------------------------------*/

Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, Direction startDir, int depth)
:GraphObject(imageID, startX, startY, startDir, depth), m_World(world)
{
}
//only energyholders die
bool Actor::isAlive() const
{
    return true;
}

//most actors don't block movement, only pebbles block movement
bool Actor::blocksMovement() const
{
    return false;
}

//only insects can be bitten, poisoned, or stunnned; most cannot
void Actor::getBitten(int amt)
{
    return;
}

void Actor::getPoisoned()
{
    return;
}

void Actor::getStunned()
{
    return;
}

//most actors aren't edible, only food is edible
bool Actor::isEdible() const
{
    return false;
}
//most actors aren't pheromones
bool Actor::isPheromone(int colony) const
{
    return false;
}

//most actors aren't enemies
bool Actor::isEnemy(int colony) const
{
    return false;
}

//enemies are dangerous
bool Actor::isDangerous(int colony) const
{
    return isEnemy(colony);
}

//most actors aren't anthills
bool Actor::isAntHill(int colony) const
{
    return false;
}

//only insects become food when dead
bool Actor::becomesFoodUponDeath() const
{
    return false;
}

//const accessor to world
StudentWorld* Actor::getWorld() const
{
    return m_World;
}
//virtual destructor(base class)
Actor::~Actor()
{
}

/*------------------------------------PEBBLE CLASS IMPLEMENTATIONS-----------------------------------*/

//pebbles start out facing right with a depth of 1
Pebble::Pebble(StudentWorld* world, int startX, int startY)
:Actor(world, IID_ROCK, startX, startY, right, pebbleDepth)
{
}

void Pebble::doSomething()
{
    //pebbles do nothing!
    return;
}

bool Pebble::blocksMovement() const
{
    return true;
}

/*---------------------------------ENERGYHOLDER CLASS IMPLEMENATIONS---------------------------------*/

//energy holders include: food, insects, pheromones, anthills
EnergyHolder::EnergyHolder(StudentWorld* world, int imageID, int startX, int startY, Direction startDir, int depth, int energy)
:Actor(world, imageID, startX, startY, startDir, depth)
{
    //energies have m_energy
    m_energy = energy;
}

//dies if energy becomes 0 or less
bool EnergyHolder::isAlive() const
{
    return m_energy > 0;
}

//accessor method
int EnergyHolder::getEnergy() const
{
    return m_energy;
}

//adds or removes energy
void EnergyHolder::updateEnergy(int amt)
{
    m_energy += amt;
}

//food eaten from location
int EnergyHolder::pickupAndEatFood(int amt)
{
    Food *f = dynamic_cast<Food*>(getWorld()->getEdibleAt(getX(), getY()));
    //food exists
    if(f != nullptr){
        //food has less hit points than trying to pick up
        if(f -> getEnergy() < amt){
            //can only eat as much food as exists
            //remove from food, add to own energy
            updateEnergy(f->getEnergy());
            f -> updateEnergy(-f->getEnergy());
            return f->getEnergy();
        }else{ //food has equal or more hit points than picking up
            //eat up to limit
            updateEnergy(amt);
            f -> updateEnergy(-amt);
            return amt;
        }
    }
    //no food, nothing to be picked up
    return 0;
}

//add food to position
void EnergyHolder::addFood(int amt)
{
    Food* f = dynamic_cast<Food*>(getWorld()->getEdibleAt(getX(), getY()));
    //if already exists, just add
    if(f != nullptr){
        f -> updateEnergy(amt);
    }else{
        //otherwise, make new food
        getWorld() -> addActor(new Food(getWorld(), getX(), getY(), amt));
    }
}

//destructor(base class)
EnergyHolder::~EnergyHolder()
{
}
/*--------------------------------------FOOD CLASS IMPLEMENTATION--------------------------------------*/
//food starts out facing right with a depth of 2
Food::Food(StudentWorld* world, int startX, int startY, int energy)
:EnergyHolder(world, IID_FOOD, startX, startY, right, foodDepth, energy)
{
}

void Food::doSomething()
{
    //Food does nothing!
    return;
}

bool Food::isEdible() const
{
    return true;
}

/*-----------------------------------PHEROMONE CLASS IMPLEMENTATION-----------------------------------*/

//pheromones start out facing right with a depth of 2 and starting hit points of 256
Pheromone::Pheromone(StudentWorld* world, int imageID, int startX, int startY, int colony)
:EnergyHolder(world, imageID, startX, startY, right, pheromoneDepth, startingPheromoneHitPoints)
{
    m_colony = colony;
}

void Pheromone::doSomething()
{
    //decreases strength by 1during each tick
    updateEnergy(-1);
}

bool Pheromone::isPheromone(int colony) const
{
    //ants can only detect the pheromone from their own colony
    return m_colony == colony;
}

void Pheromone::increaseStrength()
{
    //max cap for pheromone strength is 768
    if(getEnergy()<maxPheromoneHitPoints){
        //each increment of strength can add up to 256 energy hit points
        if(getEnergy() + startingPheromoneHitPoints > maxPheromoneHitPoints){
            updateEnergy(maxPheromoneHitPoints - getEnergy());
        }else{
        updateEnergy(startingPheromoneHitPoints);
        }
    }
}

/*------------------------------------ANTHILL CLASS IMPLEMENTATION------------------------------------*/
//anthills start off facing right with a depth of 2 and starting hit points of 8999

AntHill::AntHill(StudentWorld* world, int startX, int startY, int colony, Compiler* program)
:EnergyHolder(world, IID_ANT_HILL, startX, startY, right, antHillDepth, startingAntHillHitPoints)
{
    m_colonyNumber = colony;
    m_compiler = program;
}

void AntHill::doSomething()
{
    //decrease queen's hit points by 1 unit
    updateEnergy(-1);
    if(!isAlive()){
        //died, must immediately return
        return;
    }
    int antHillX = getX();
    int antHillY = getY();
    //if there is food on the anthill
    Food* f = dynamic_cast<Food*>(getWorld() -> getEdibleAt(antHillX, antHillY));
    if(f != nullptr)
    {
        int foodEnergy = f -> getEnergy();
        //can eat up to 10000
        if(foodEnergy > maxAntHillEat){
            foodEnergy = maxAntHillEat;
        }
        //remove from food, update anthill energy
        f -> updateEnergy(-foodEnergy);
        updateEnergy(foodEnergy);
        return;
    }
    //if anthill has enough energy, create a new ant based on that colony
    if(getEnergy() > newAntThreshhold){
        int colony = m_colonyNumber;
        int imageID;
        switch(colony){
            case 0:
                imageID = IID_ANT_TYPE0;
                break;
            case 1:
                imageID = IID_ANT_TYPE1;
                break;
            case 2:
                imageID = IID_ANT_TYPE2;
                break;
            case 3:
                imageID = IID_ANT_TYPE3;
                break;
            default:
                break;
        }
        getWorld() -> addActor(new Ant(getWorld(), imageID, antHillX, antHillY, colony, m_compiler));
        updateEnergy(-energyToCreateNewAnt);
        //let StudentWorld know about the new ant
        getWorld() -> increaseScore(colony);
    }
}

//is this the anthill for the ant's colony?
bool AntHill::isAntHill(int colony) const
{
    return m_colonyNumber == colony;
}

/*-------------------------------TRIGGERABLEACTOR CLASS IMPLEMENTATION--------------------------------*/
//triggerable actors include: water pools and poison, which start facing right with a depth of 2

TriggerableActor::TriggerableActor(StudentWorld* world, int imageID, int startX, int startY)
:Actor(world, imageID, startX, startY, right, triggerableActorDepth)
{
}

bool TriggerableActor::isDangerous(int colony) const
{
    //all triggerable actors are dangerous
    return true;
}

// virtual destructor (base class)
TriggerableActor::~TriggerableActor()
{
}

/*------------------------------------WATERPOOL CLASS IMPLEMENTAION------------------------------------*/
WaterPool::WaterPool(StudentWorld* world, int startX, int startY)
:TriggerableActor(world, IID_WATER_POOL, startX, startY)
{
}

void WaterPool::doSomething()
{
    //water pools attempt to stun all insects on the square
    //allocate work to StudentWorld function
    getWorld() -> stunAllStunnableAt(getX(), getY());
}

/*-------------------------------------POISON CLASS IMPLEMENTAION--------------------------------------*/

Poison::Poison(StudentWorld* world, int startX, int startY)
:TriggerableActor(world, IID_POISON, startX, startY)
{
}

void Poison::doSomething()
{
    //poison attempts to poison all insects on the square
    //allocate work to StudentWorld function
    getWorld() -> poisonAllPoisonableAt(getX(), getY());
}

/*--------------------------------------INSECT CLASS IMPLEMENTAION-------------------------------------*/
//insects start out in a random direction with a depth of 1
Insect::Insect(StudentWorld* world, int imageID, int startX, int startY, int energy)
:EnergyHolder(world, imageID, startX, startY, static_cast<Direction>(randInt(1,4)), insectDepth, energy)
{
    //random direction
    m_walkingDistance = randInt(2, 10);
    //starts out awake, unstunned, unbitten, unpoisoned
    m_sleepingTicks = 0;
    m_stunned = false;
    m_bitten = false;
    m_poisoned = false;
}

void Insect::doSomething()
{
    //decrement ticks
    updateEnergy(-1);
    if(m_sleepingTicks != 0){
        //if asleep, decrement sleeping ticks
        m_sleepingTicks--;
        return;
    }
    doSomethingDifferent();
}

//most insects have this behavior when bitten
void Insect::getBitten(int amt)
{
    updateEnergy(-amt);
        
}

//most insects have this behavior when poisoned
void Insect::getPoisoned()
{
    //not already poisoned here
    if(!m_poisoned){
        updateEnergy(-poisonEffect);
        m_poisoned = true;
    }
}

//most insects have this behavior when stunned
void Insect::getStunned()
{
    //not already stunned here
    if(!m_stunned){
        increaseSleepTicks(2);
        m_stunned = true;
    }
}

//almost all insects are enemies
bool Insect::isEnemy(int colony) const
{
    return true;
}

//all insects become food upon death
bool Insect::becomesFoodUponDeath() const
{
    return true;
}

//sets x and y to the coordinates of x and y if insect moves 1 unit in direction its facing
void Insect::getXYInFrontOfMe(int &x, int &y) const
{
    Direction dir = getDirection();
    //four different directions, (0,0) is the bottom left corner
    if(dir == up){
        y = y + 1;
    }
    if(dir == right){
        x = x + 1;
    }
    if(dir == down){
        y = y - 1;
    }
    if(dir == left){
        x = x - 1;
    }
}

//attempts to move in direction
bool Insect::moveForwardIfPossible()
{
    int x = getX();
    int y = getY();
    //get coordinates in front
    getXYInFrontOfMe(x, y);
    //if we can move, move
    if(getWorld()->canMoveTo(x, y)){
        moveTo(x, y);
        m_walkingDistance--;
        //now can be stunned, poisoned again
        setStunned(false);
        setPoisoned(false);
        setBitten(false);
        return true;
    }else{ //otherwise set to 0 so on next active tick, we can change directions
        m_walkingDistance = 0;
        return false;
    }
}

//increases the amount of ticks spent sleeping by amt
void Insect::increaseSleepTicks(int amt)
{
    m_sleepingTicks += amt;
}

//mutator and accessor methods
void Insect::setWalkingDistance(int distance)
{
    m_walkingDistance = distance;
}

int Insect::getSleepTicks() const
{
    return m_sleepingTicks;
}

int Insect::getWalkingDistance() const
{
    return m_walkingDistance;
}

void Insect::setStunned(bool stun)
{
    m_stunned = stun;
}

void Insect::setPoisoned(bool poison)
{
    m_poisoned = poison;
}

void Insect::setBitten(bool bitten)
{
    m_bitten = bitten;
}

bool Insect::isBitten() const
{
    return m_bitten;
}

//virtual destructor(base class)
Insect::~Insect()
{
}
/*--------------------------------------ANT CLASS IMPLEMENTATION--------------------------------------*/
//ants have their own colony and compiler, each colony of ant has their unique imageID
Ant::Ant(StudentWorld* world, int imageID, int startX, int startY, int colony, Compiler* program)
:Insect(world, imageID, startX, startY, startingAntHitPoints)
{
    //tracks if we were blocked from moving
    m_blocked = false;
    m_colony = colony;
    //ants can hold food
    m_foodUnit = 0;
    //instruction counter for compiler
    m_ic = 0;
    m_compiler = program;
    m_lastRandomNumber = -1;
}

void Ant::doSomethingDifferent()
{
    Compiler::Command cmd;
    for(int i = 0; i < antMaxCommandNumber; i++){
        if(!m_compiler -> getCommand(m_ic, cmd)){
            // error fetching command, kill ant and return
            updateEnergy(-getEnergy());
            return;
        }
        switch(cmd.opcode){
            case Compiler::moveForward:
                //try to move forward
                moveForwardIfPossible();
                ++m_ic;
                return;
            case Compiler::eatFood:
                //if already holding food
                if(getFood() > 0){
                    if(getFood() < maxAntEat){
                        updateEnergy(getFood());
                        setFood(0);
                    }else{
                        updateEnergy(maxAntEat);
                        updateFood(-maxAntEat);
                    }
                }
                ++m_ic;
                return;
            case Compiler::dropFood:
                //holding food?
                if(getFood() > 0){
                    //add food onto square
                    addFood(getFood());
                    //remove from hold
                    setFood(0);
                }
                ++m_ic;
                return;
            case Compiler::bite:
                getWorld()->biteEnemyAt(this, m_colony, antBiteDamage);
                ++m_ic;
                return;
            case Compiler::pickupFood:
                //can pick up a maximum units of food
                if(getFood() < antMaxFoodHold){
                    if(getFood() + antPickUpFood > antMaxFoodHold){
                        pickupFood(antMaxFoodHold - getFood());
                    }else{
                        pickupFood(antPickUpFood);
                    }
                }
                ++m_ic;
                return;
            case Compiler::emitPheromone:
                //delegate work to studentworld
                getWorld()->addToPheromoneAt(getX(), getY(), m_colony);
                ++m_ic;
                return;
            case Compiler::faceRandomDirection:
                //set random direction
                setDirection(static_cast<Direction>(randInt(1, 4)));
                ++m_ic;
                return;
            case Compiler::rotateClockwise:
                if(getDirection() == up) {
                    setDirection(right);
                }
                else if(getDirection() == right) {
                    setDirection(down);
                }
                else if(getDirection() == down) {
                    setDirection(left);
                }
                else if(getDirection() == left) {
                   setDirection(up);
                }
                ++m_ic;
                return;
            case Compiler::rotateCounterClockwise:
                if(getDirection() == up) {
                    setDirection(left);
                }
                else if(getDirection() == right) {
                    setDirection(up);
                }
                else if(getDirection()== down) {
                    setDirection(right);
                }
                else if(getDirection() == left) {
                    setDirection(down);
                }
                ++m_ic;
                return;
            case Compiler::generateRandomNumber: //up to operand1
                m_lastRandomNumber = randInt(0, stoi(cmd.operand1));
                ++m_ic;
                break;
            case Compiler::goto_command:
                m_ic = stoi(cmd.operand1);
                break;
            case Compiler::if_command:
                int operand = stoi(cmd.operand1);
                bool func = false;
                int x = getX();
                int y = getY();
                if(operand == 0){ //i_smell_danger_in_front_of_me
                    getXYInFrontOfMe(x, y);
                    func = getWorld() -> isDangerAt(x, y, m_colony);
                }else if(operand == 1){ //i_smell_pheromone_in_front_of_me
                    getXYInFrontOfMe(x, y);
                    func = (getWorld() -> getPheromoneAt(x, y, m_colony)) != nullptr;
                }else if(operand == 2){ //i_was_bit
                    func = isBitten();
                }else if(operand == 3){ //i_am_carrying_food
                    func = getFood() > 0;
                }else if(operand == 4){ //i_am_hungry
                    func = getEnergy() <= antHungryThreshhold;
                }else if(operand == 5){ //i_am_standing_on_my_anthill
                    func = (getWorld() -> isAntHillAt(x, y, m_colony));
                }else if(operand == 6){ //i_am_standing_on_food
                    func = (getWorld() -> getEdibleAt(x, y)) != nullptr;
                }else if(operand == 7){ //i_am_standing_with_an_enemy
                    func = getWorld() -> isEnemyAt(x, y, m_colony);
                }else if(operand == 8){ //i_was_blocked_from_moving
                    func = getBlocked();
                }else if(operand == 9){ //last_random_number_was_zero
                    func = m_lastRandomNumber == 0;
                }
                if(func){ // if true, go to operand2
                    m_ic = stoi(cmd.operand2);
                }
                else{ //otherwise, keep going
                    ++m_ic;
                }
                break;
        }
    }
}

//remember that they were bitten
void Ant::getBitten(int amt){
    Insect::getBitten(amt);
    setBitten(true);
}

//pick up food at a location
int Ant::pickupFood(int amt)
{
    Food *f = dynamic_cast<Food*>(getWorld()->getEdibleAt(getX(), getY()));
    //food exists
    if(f != nullptr){
        //how much food exists
        int foodAvailable = f -> getEnergy();
        if(foodAvailable < amt){
            //can only get as much food as exists
            f -> updateEnergy(-foodAvailable);
            updateFood(foodAvailable);
            return foodAvailable;
        }else{
            f -> updateEnergy(-amt);
            updateFood(amt);
            return amt;
        }
    }
    return 0;
}

bool Ant::moveForwardIfPossible()
//remember that it was blocked from moving
{
    bool b = Insect::moveForwardIfPossible();
    setBlocked(!b);
    return b;
}

bool Ant::isEnemy(int colony) const
//ants are not enemies if from same colony
{
    return colony != m_colony;
}

//accessor and mutator methods

bool Ant::getBlocked()
{
    return m_blocked;
}

void Ant::setBlocked(bool block)
{
    m_blocked = block;
}

int Ant::getFood()
{
    return m_foodUnit;
}

//set food to specific amount

void Ant::setFood(int amt)
{
    m_foodUnit = amt;
}

//update food by specific amount

void Ant::updateFood(int amt)
{
    m_foodUnit += amt;
}

/*----------------------------------GRASSHOPPER CLASS IMPLEMENTATION----------------------------------*/

Grasshopper::Grasshopper(StudentWorld* world, int imageID, int startX, int startY, int energy)
:Insect(world, imageID, startX, startY, energy)
{
}

void Grasshopper::doSomethingDifferent()
{
    if(isAlive()){
        //if alive and active
        if(getSleepTicks() == 0){
            //baby grasshopeprs and adult grasshoppers differ here
            int x = doDifferentStuff();
            if(x == 0){ //baby grasshopper moulted into adult grasshopper -> return
                return;
            }
            if(x == 1){ //baby grasshopper didn't moult, adult grasshopper didn't jump or bite
                if(pickupAndEatFood(maxGrassHopperEat) > 0){ //ate some food
                    //1/2 chance of wanting to sleep
                    int x = randInt(1, 2);
                    if(x == 1){
                        increaseSleepTicks(2);
                        return;
                    }
                }
                //if done walking in that direction
                if(getWalkingDistance() == 0){
                    //get new random direction and walking distance
                    setDirection(static_cast<Direction>(randInt(1, 4)));
                    setWalkingDistance(randInt(2, 10));
                }
                if(!moveForwardIfPossible()){//if couldn't move, set distance to 0
                    setWalkingDistance(0);
                }else{ //else remember moved once in that direction
                    setWalkingDistance(getWalkingDistance()-1);
                }
            }
            //grasshoppers are asleep 2/3 ticks
            increaseSleepTicks(2);
        }
    }
}

//virtual destructor(base class)
Grasshopper::~Grasshopper()
{
}
/*--------------------------------BABY GRASSHOPPER CLASS IMPLEMENTATION--------------------------------*/

BabyGrasshopper::BabyGrasshopper(StudentWorld* world, int startX, int startY)
:Grasshopper(world, IID_BABY_GRASSHOPPER, startX, startY, startingBabyGrassHopperHitPoints)
{
}

int BabyGrasshopper::doDifferentStuff()
{
    //if a baby grasshopper has enough energy to turn into an adult
    if(getEnergy() > babyToAdultGHThreshhold){
        //kill baby grasshopper
        updateEnergy(-getEnergy());
        //add adult in its place
        getWorld() -> addAdultGH(getX(), getY());
        //return 0 if moulted into adult
        return 0;
    }
    //return 1 to signal that grasshopper didn't moult
    return 1;
}

/*-------------------------------ADULT GRASSHOPPER CLASS IMPLEMENTATION--------------------------------*/

AdultGrasshopper::AdultGrasshopper(StudentWorld* world, int startX, int startY)
:Grasshopper(world, IID_ADULT_GRASSHOPPER, startX, startY, babyToAdultGHThreshhold)
{
}

//grasshoppers can't be poisoned or stunned
void AdultGrasshopper::getStunned()
{
    return;
}

void AdultGrasshopper::getPoisoned()
{
    return;
}

void AdultGrasshopper::getBitten(int amt)
{
    Insect::getBitten(amt);
    int x = randInt(1, 2);
    //50% chance of biting back
    if(x == 1){
        //bite back
        getWorld()->biteEnemyAt(this, -1, adultGHBiteDamage);
    }
}

int AdultGrasshopper::doDifferentStuff()
{
    //1/3 chance that the grasshopper will try to bite an enemy at location
    int x = randInt(1, 3);
    if(x == 1){
        //if bit, return 2 to signal to function to skip to set sleeping ticks
        if(getWorld()->biteEnemyAt(this, -1, adultGHBiteDamage)){
            return 2;
        }
    }
    //otherwise, 1/10 chance that the grasshopper will try to jump to a new location
    x = randInt(1, 10);
    if(x == 1){
        //rectangle of coordinates 10 away
        //minimum between getX() + adultGHJumpRadius and VIEW_HEIGHT
        int maxX = getX() + adultGHJumpRadius > VIEW_HEIGHT ? VIEW_HEIGHT : getX() + adultGHJumpRadius;
        //maximum between getX() - adultGHJumpRadius and 0
        int minX = getX() - adultGHJumpRadius > 0 ? getX() - adultGHJumpRadius : 0;
        //minimum between getY() + adultGHJumpRadius and VIEW_WIDTH
        int maxY = getY() + adultGHJumpRadius > VIEW_WIDTH ? VIEW_WIDTH : getY() + adultGHJumpRadius;
        //maximum between getY() - adultGHJumpRadius and 0
        int minY = getY() - adultGHJumpRadius > 0 ? getY() - adultGHJumpRadius : 0;
        //declare struct pair to hold location x and y
        struct Pair {
            int x;
            int y;
        };
        //empty vector of locations
        std::vector<Pair> locations;
        //set a counter, if we reach x locations, we can just pick one randomly out of those
        //semi random function
        int count = 0;
        for(int i = minX; i < maxX; i++){
            for(int j = minY; j < maxY; j++){
                //distance function, make sure that the location is within 10 radius away
                int distanceSquared = (i - getX())*(i - getX())+ (j - getY()) * (j - getY());
                if(distanceSquared <= adultGHJumpRadius*adultGHJumpRadius){
                    //if we can move there,
                    if(getWorld()->canMoveTo(i, j)){
                        Pair location;
                        location.x = i;
                        location.y = j;
                        //add to vector list
                        locations.push_back(location);
                        count++;
                        //don't need to go through the whole list, just pick one randomly out of the x
                        if(count > grassHopperMaxCommandNumber){
                            break;
                        }
                    }
                }
            }
        }
        //if there were places we could move to
        if(locations.size() > 0){
            //pick random location from vector
            int rand = randInt(0, locations.size()-1);
            Pair location = locations[rand];
            moveTo(location.x, location.y);
            //return 2 to signal to other function to skip to set sleepingTicks
            return 2;
        }
    }
    //return 1 to signal to other function to continue
    return 1;
}
