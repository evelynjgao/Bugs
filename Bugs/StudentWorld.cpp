#include "StudentWorld.h"
#include "Actor.h"
#include "Field.h"
#include "Compiler.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

//constructs new StudentWorld
StudentWorld::StudentWorld(std::string assetDir)
: GameWorld(assetDir)
{
    //initialize ticks to 0
    m_countTicks = 0;
    //initialize empty array with no ants
    for(int i = 0; i < MAX_ANT_COLONIES; i++){
        m_numberOfAnts[i] = 0;
    }
    //initialize with no winning ant
    m_winningAnt = -1;
    //initialize name vector with the names of the programs
    fileNames = getFilenamesOfAntPrograms();
}

StudentWorld::~StudentWorld()
{
    //allocate work to cleanUp()
    cleanUp();
}

int StudentWorld::init()
{
    //Load the field
    Field f;
    string fieldFile = getFieldFilename();
    //error in loading field, return error status
    string error;
    if(f.loadField(fieldFile, error) != Field::LoadResult::load_success){
        setError(fieldFile + " " + error);
        return GWSTATUS_LEVEL_ERROR;
    }
    //Load entrant files
    fileNames = getFilenamesOfAntPrograms();
    for(int i = 0; i < fileNames.size(); i++){
        compilersForEntrants[i] = new Compiler;
    }
    std::string errors;
    //error in loading files, return error status
    for(int i = 0; i < fileNames.size(); i++){
        if(!compilersForEntrants[i]->compile(fileNames[i], errors)){
            setError(fileNames[i] + " " + errors);
            return GWSTATUS_LEVEL_ERROR;
        }
    }
    //field succesfully loaded, access contents of the field
    for(int i = 0; i < VIEW_HEIGHT; i++){
        for(int j = 0; j < VIEW_WIDTH; j++){
            Field::FieldItem item = f.getContentsOf(i, j);
            if(item == Field::FieldItem::rock){
                m_actors[i][j].push_back(new Pebble(this, i, j));
            }
            if(item == Field::FieldItem::grasshopper){
                m_actors[i][j].push_back(new BabyGrasshopper(this, i, j));
            }
            if(item == Field::FieldItem::food){
                m_actors[i][j].push_back(new Food(this, i, j, startingFoodHitPoints));
            }
            if(item == Field::FieldItem::water){
                m_actors[i][j].push_back(new WaterPool(this, i, j));
            }
            if(item == Field::FieldItem::poison){
                m_actors[i][j].push_back(new Poison(this, i, j));
            }
            if(item == Field::FieldItem::anthill0){
               m_actors[i][j].push_back(new AntHill(this, i, j, 0, compilersForEntrants[0]));
            }
            if(item == Field::FieldItem::anthill1){
                m_actors[i][j].push_back(new AntHill(this, i, j, 1, compilersForEntrants[1]));
            }
            if(item == Field::FieldItem::anthill2){
                m_actors[i][j].push_back(new AntHill(this, i, j, 2, compilersForEntrants[2]));
            }
            if(item == Field::FieldItem::anthill3){
                m_actors[i][j].push_back(new AntHill(this, i, j, 3, compilersForEntrants[3]));
            }
        }
    }
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    updateTickCount(); //updates the current tick # in the simulation
    //give each actor in each list at each location the chance to do something if it is alive
    for(int i = 0; i < VIEW_HEIGHT; i++){
        for(int j = 0; j < VIEW_WIDTH; j++){
            for(list<Actor*>::iterator it = m_actors[i][j].begin(); it != m_actors[i][j].end();){
                if((*it) -> isAlive()){
                    (*it) -> doSomething();
                }
                //check to see if actor has moved, and update the lists accordingly
                if((*it) -> getX() != i || (*it) -> getY() != j){
                    m_actors[(*it)->getX()][(*it)->getY()].push_back(*it);
                    it = m_actors[i][j].erase(it);
                }else{
                    it++;
                }
                
            }
        }
    }
    removeDeadActors();
    updateDisplayText();
    //if game is over (2000 ticks)
    if(m_countTicks == m_ticksToGameOver){
        //see if there is a winning ant
        if(m_winningAnt >= 0){
            //format name of winning ant
            string str= fileNames[m_winningAnt];
            std::size_t found = str.find_last_of("/\\");
            std::size_t found1 = str.find_last_of(".");
            if(found1 > found){
                str = str.substr(found + 1, found1 - found - 1);
            }else{
                str = str.substr(found + 1);
            }
            setWinner(str);
            return GWSTATUS_PLAYER_WON;
        }
        //otherwise, no winner
        return GWSTATUS_NO_WINNER;
    }
    //not yet reached the end
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    //delete all dynamically allocated actors
    for(int i = 0; i < VIEW_HEIGHT; i++){
        for(int j = 0; j < VIEW_WIDTH; j++){
            for(list<Actor*>::iterator it = m_actors[i][j].begin(); it != m_actors[i][j].end(); it++){
                delete *it;
            }
            //clear the vector
            m_actors[i][j].clear();
        }
    }
    //delete all compilers
    for(int i = 0; i < fileNames.size(); i++){
        delete compilersForEntrants[i];
    }
    //clear vector
    fileNames.clear();
}

void StudentWorld::removeDeadActors()
{
    for(int i = 0; i < VIEW_WIDTH; i++){
        for(int j = 0; j < VIEW_WIDTH; j++){
            for(list<Actor*>::iterator it = m_actors[i][j].begin(); it != m_actors[i][j].end();){
                //if actor is daed
                if(!(*it)->isAlive()){
                    //if it leaves food when it dies
                    if((*it)->becomesFoodUponDeath()){
                        Food* f = dynamic_cast<Food*>(getEdibleAt(i, j));
                        //no food exists on that square, add food
                        if(f == nullptr){
                            addActor(new Food(this, i, j, deadInsectFoodHitPoints));
                        }else{ //add to existing food
                            f -> updateEnergy(deadInsectFoodHitPoints);
                        }
                    }
                    //deallocate memory and remove from list
                    delete *it;
                    it = m_actors[i][j].erase(it);
                }
                else{
                    it++;
                }
            }
        }
    }
}

void StudentWorld::updateDisplayText()
{
    int ticks = getCurrentTicks();
    string s = formatString(ticks);
    setGameStatText(s);
}

string StudentWorld::formatString(int ticks) const
{
    ostringstream oss;
    oss << "Ticks:";
    //ticks should be five spaces, right centered
    oss << setw(5) << m_ticksToGameOver-ticks;
    oss << " - ";
    for(int i = 0; i < fileNames.size(); i++){
        //format string name
        string str = fileNames[i];
        std::size_t found = str.find_last_of("/\\");
        std::size_t found1 = str.find_last_of(".");
        if(found1 > found){
            str = str.substr(found + 1, found1 - found - 1);
        }else{
            str = str.substr(found + 1);
        }
        oss << str;
        //if winner, put an asterick by the name
        if(i == m_winningAnt){
            oss << "*";
        }
        oss << ": ";
        //amount of ants are 2 characters, filled with 0
        oss << setfill('0');
        oss << setw(2) << m_numberOfAnts[i] << " ants";
        //two spaces apart between each player
        if(i != fileNames.size()){
            oss << "  ";
        }
    }
    string s = oss.str();
    return s;
}

void StudentWorld::updateTickCount(){
    m_countTicks++;
}

int StudentWorld::getCurrentTicks() const{
    return m_countTicks;
}

//add actor pointer the list at that location
void StudentWorld::addActor(Actor *a)
{
    int x = a -> getX();
    int y = a -> getY();
    m_actors[x][y].push_back(a);
}

//add a new adult grasshopper at that location for when baby grasshopper dies
void StudentWorld::addAdultGH(int x, int y)
{
    m_actors[x][y].push_back(new AdultGrasshopper(this, x, y));
}

//returns edible at location if exists, otherwise returns null pointer
Actor* StudentWorld::getEdibleAt(int x, int y) const
{
    for(list<Actor*>::const_iterator it = m_actors[x][y].begin(); it != m_actors[x][y].end(); it++){
        if((*it) -> isAlive()){
            if((*it) -> isEdible()){
                return *it;
            }
        }
    }
    return nullptr;
}

//returns pheromone at location if it exists, otherwise returns null pointer
Actor* StudentWorld::getPheromoneAt(int x, int y, int colony) const
{
    for(list<Actor*>::const_iterator it = m_actors[x][y].begin(); it != m_actors[x][y].end(); it++){
        if((*it) -> isPheromone(colony)){
            return *it;
        }
    }
    return nullptr;
}

//checks if enemy of the colony is at location
bool StudentWorld::isEnemyAt(int x, int y, int colony) const
{
    for(list<Actor*>::const_iterator it = m_actors[x][y].begin(); it != m_actors[x][y].end(); it++){
        if((*it) -> isEnemy(colony)){
            if((*it) -> isAlive()){
                return true;
            }
        }
    }
    return false;
}

//check if there is danger to the colony at location
bool StudentWorld::isDangerAt(int x, int y, int colony) const
{
    for(list<Actor*>::const_iterator it = m_actors[x][y].begin(); it != m_actors[x][y].end(); it++){
        if((*it) -> isDangerous(colony)){
            return true;
        }
    }
    return false;
}

//checks if there is an anthill that corresponds to the colony at the location
bool StudentWorld::isAntHillAt(int x, int y, int colony) const
{
    for(list<Actor*>::const_iterator it = m_actors[x][y].begin(); it != m_actors[x][y].end(); it++){
        if((*it) -> isAlive()){
            if((*it) -> isAntHill(colony)){
                return true;
            }
        }
    }
    return false;
}

//stuns all stunnable actors (insects)
bool StudentWorld::stunAllStunnableAt(int x, int y)
{
    for(list<Actor*>::iterator it = m_actors[x][y].begin(); it != m_actors[x][y].end(); it++){
        //distinguishes insect actors
        if((*it) -> becomesFoodUponDeath()){
            (*it) -> getStunned();
        }
    }
    return true;
}

//poisons all poisonable actors(insects)
bool StudentWorld::poisonAllPoisonableAt(int x, int y)
{
    for(list<Actor*>::const_iterator it = m_actors[x][y].cbegin(); it != m_actors[x][y].cend(); it++){
        if((*it) -> becomesFoodUponDeath()){
            (*it) -> getPoisoned();
        }
    }
    return true;
}

//adds strength to pheromone if already exists at that location, otherwise creates new pheromone
void StudentWorld::addToPheromoneAt(int x, int y, int colony)
{
    Pheromone *p = dynamic_cast<Pheromone*>(getPheromoneAt(x, y, colony));
    //Pheromone already exists at that x and y
    if(p != nullptr)
    {
        p -> increaseStrength();
    }else{//Pheromone object does not yet exist
        int imageID;
        switch (colony) {
            case 0:
                imageID = IID_PHEROMONE_TYPE0;
                break;
            case 1:
                imageID = IID_PHEROMONE_TYPE1;
                break;
            case 2:
                imageID = IID_PHEROMONE_TYPE2;
                break;
            case 3:
                imageID = IID_PHEROMONE_TYPE3;
            default:
                break;
        }
        addActor(new Pheromone(this, imageID, x, y, colony));
    }
}

//returns true if able to bite an enemy at the location
bool StudentWorld::biteEnemyAt(Actor *me, int colony, int biteDamage)
{
    int x = me -> getX();
    int y = me -> getY();
    //list of enemies at location
    std::vector<Actor*> enemies;
    for(list<Actor*>::iterator it = m_actors[x][y].begin(); it != m_actors[x][y].end(); it++){
        if((*it)->isEnemy(colony)){
            if((*it)->isAlive()){
                //make sure that doesn't count self as enemy
                if((*it)!= me){
                    enemies.push_back(*it);
                }
            }
        }
    }
    //if only one enemy, bite that enemy
    if(enemies.size() == 1){
        enemies[0] -> getBitten(biteDamage);
        return true;
    }
    //more than one enemy, pick random enemy to bite
    else if(enemies.size() > 1){
        int rand = randInt(0, enemies.size()-1);
        enemies[rand] -> getBitten(biteDamage);
        return true;
    }
    //did not bite
    return false;
}

//returns if able to move to location, not blocked
bool StudentWorld::canMoveTo(int x, int y) const
{
    //check if there exists an actor at that x and y that would block movement
    for(list<Actor*>::const_iterator it = m_actors[x][y].cbegin(); it != m_actors[x][y].cend(); it++)
    {
        if((*it) -> blocksMovement()){
            return false;
        }
    }
    return true;
}

//keep track of score of simulation
void StudentWorld::increaseScore(int colony)
{
    m_numberOfAnts[colony]++;
    //if no winning ant yet,
    if(m_winningAnt == -1){
        //first colony to reach more than 5 becomes the leading ant
        if(m_numberOfAnts[colony] > 5){
            m_winningAnt = colony;
        }
    }
    //if already a winning ant, see if there are now more ants than the leader
    else if(m_numberOfAnts[colony] > m_numberOfAnts[m_winningAnt]){
        m_winningAnt = colony;
    }
}

/* ------debugger method--------
void StudentWorld::printField(){
    cout << "Ticks: " << m_countTicks << endl;
    for(int i = 0; i < VIEW_HEIGHT; i++){
        for(int j = 0; j < VIEW_WIDTH; j++){
            for(list<Actor*>::iterator it = m_actors[i][j].begin(); it != m_actors[i][j].end(); it++){
                int id = (*it) -> getID();
                switch(id){
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        cout << "ant" << id << "   ";
                        break;
                    case 5:
                        cout << "poison   ";
                        break;
                    case 6:
                        cout << "food   ";
                        break;
                    case 7:
                        cout << "water pool   ";
                        break;
                    case 8:
                        cout << "rock   ";
                        break;
                    case 9:
                        cout << "bby grasshopper   ";
                        break;
                    case 10:
                        cout << "adult grasshopper   ";
                        break;
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                        cout << "Slot [" << i << ", " << j << "] :";
                        cout << "pheromone   ";
                        cout << endl;
                        break;
                }
            }
        }
    }
    cout << "-----------------------" << endl;
}*/
