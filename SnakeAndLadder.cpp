#include <iostream>
#include <vector>
#include <map>
#include <deque>
#include <cstdlib>
#include <ctime>

using namespace std;

// Observer Pattern
class IObserver {
public:
    virtual void update(string msg) = 0;
    virtual ~IObserver() {}
};

// Sample observer implementation
class SnakeAndLadderConsoleNotifier : public IObserver {
public:
    void update(string msg) override {
        cout << "[GAME NOTICE] " << msg << endl;
    }
};

// Dice class
class Dice {
private:
    int faceCount;
    
public:
    Dice(int f) {
        faceCount = f;
        srand(time(0));
    }
    
    int roll() {
        return (rand() % faceCount) + 1;
    }
};

// Base class for Snake and Ladder (both have start and end positions)
class BoardEntity {
protected:
    int startIndex;
    int endIndex;
    
public:
    BoardEntity(int start, int end) {
        startIndex = start;
        endIndex = end;
    }
    
    int getStart() { 
        return startIndex; 
    }

    int getEnd() { 
        return endIndex;
    }
    
    virtual void display() = 0;
    virtual string name() = 0;
    virtual ~BoardEntity() {}
};

// Snake class
class Snake : public BoardEntity {
public:
    Snake(int start, int end) : BoardEntity(start, end) {
        if(end >= start) {
            cout << "Invalid snake configuration: endIndex must be less than startIndex." << endl;
        }
    }
    
    void display() override {
        cout << "Snake: " << startIndex << " -> " << endIndex << endl;
    }

    string name() override {
        return "SNAKE" ;
    }
};

// Ladder class
class Ladder : public BoardEntity {
public:
    Ladder(int start, int end) : BoardEntity(start, end) {
        if(end <= start) {
            cout << "Invalid ladder configuration: endIndex must be greater than startIndex." << endl;
        }
    }
    
    void display() override {
        cout << "Ladder: " << startIndex << " -> " << endIndex << endl;
    }

    string name() override {
        return "LADDER" ;
    }
};

class BoardSetupStrategy;

// Board class
class Board {
private:
    int cellCount; // total cells on the board (size*size)
    vector<BoardEntity*> entitiesList;
    map<int, BoardEntity*> entityMap;
    
public:
    Board(int s) {
        cellCount = s * s;  // m*m board
    }
    
    bool canAddEntity(int position) {
        return entityMap.find(position) == entityMap.end();
    }
    
    void addBoardEntity(BoardEntity* boardEntity) {
        if(canAddEntity(boardEntity->getStart())) {
            entitiesList.push_back(boardEntity);
            entityMap[boardEntity->getStart()] = boardEntity;
        }
    }
    
    void setupBoard(BoardSetupStrategy* strategy);
    
    BoardEntity* getEntity(int position) {
        if(entityMap.find(position) != entityMap.end()) {
            return entityMap[position];
        }
        return nullptr;
    }
    
    int getBoardSize() { 
        return cellCount;
    }
    
    void display() {
        cout << "\n=== Board Configuration ===" << endl;
        cout << "Total Cells: " << cellCount << endl;

        int snakeCount = 0;
        int ladderCount = 0;
        for(auto entity : entitiesList) {
            if(entity->name() == "SNAKE") snakeCount++;
            else ladderCount++;
        }
        
        cout << "\nSnakes: " << snakeCount << endl;
        for(auto entity : entitiesList) {
            if(entity->name() == "SNAKE") {
                entity->display();
            }
        }
        
        cout << "\nLadders: " << ladderCount << endl;
        for(auto entity : entitiesList) {
            if(entity->name() == "LADDER") {
                entity->display();
            }
        }
        cout << "=========================" << endl;
    }
    
    ~Board() {
        for(auto entity : entitiesList) {
            delete entity;
        }
    }
};


// Strategy Pattern for Board Setup
class BoardSetupStrategy {
public:
    virtual void setupBoard(Board* board) = 0;
    virtual ~BoardSetupStrategy() {}
};

// Random Strategy with difficulty levels
class RandomBoardSetupStrategy : public BoardSetupStrategy {
public:
    enum Difficulty {
        EASY,    // More ladders, fewer snakes
        MEDIUM,  // Equal snakes and ladders
        HARD     // More snakes, fewer ladders
    };
    
private:
    Difficulty difficulty;
    
    void setupWithProbability(Board* board, double snakeProbability) {
        int totalCells = board->getBoardSize();
        int entityCount = totalCells / 10; // Roughly 10% of board has entities
        
        for(int i = 0; i < entityCount; i++) {
            double randomVal = (double)rand() / RAND_MAX;
            
            if(randomVal < snakeProbability) {
                // Add snake
                int attemptCount = 0;
                while(attemptCount < 50) {
                    int startIdx = rand() % (totalCells - 10) + 10;
                    int endIdx = rand() % (startIdx - 1) + 1;
                    
                    if(board->canAddEntity(startIdx)) {
                        board->addBoardEntity(new Snake(startIdx, endIdx));
                        break;
                    }
                    attemptCount++;
                }
            } else {
                // Add ladder
                int attemptCount = 0;
                while(attemptCount < 50) {
                    int startIdx = rand() % (totalCells - 10) + 1;
                    int endIdx = rand() % (totalCells - startIdx) + startIdx + 1;
                    
                    if(board->canAddEntity(startIdx) && endIdx < totalCells) {
                        board->addBoardEntity(new Ladder(startIdx, endIdx));
                        break;
                    }
                    attemptCount++;
                }
            }
        }
    }
    
public:
    RandomBoardSetupStrategy(Difficulty d) {
        difficulty = d;
    }
    
    void setupBoard(Board* board) override {
        switch(difficulty) {
            case EASY:
                setupWithProbability(board, 0.3);  // 30% snakes, 70% ladders
                break;
            case MEDIUM:
                setupWithProbability(board, 0.5);  // 50% snakes, 50% ladders
                break;
            case HARD:
                setupWithProbability(board, 0.7);  // 70% snakes, 30% ladders
                break;
        }
    }
};

// Custom Strategy - User provides count
class CustomCountBoardSetupStrategy : public BoardSetupStrategy {
private:
    int snakeCount;
    int ladderCount;
    bool useRandomPlacement;
    vector<pair<int, int>> snakePlacements;
    vector<pair<int, int>> ladderPlacements;
    
public:
    CustomCountBoardSetupStrategy(int snakes, int ladders, bool random) {
        snakeCount = snakes;
        ladderCount = ladders;
        useRandomPlacement = random;
    }
    
    void addSnakePosition(int start, int end) {
        snakePlacements.push_back(make_pair(start, end));
    }
    
    void addLadderPosition(int start, int end) {
        ladderPlacements.push_back(make_pair(start, end));
    }
    
    void setupBoard(Board* board) override {
        if(useRandomPlacement) {
            // Random placement with user-defined counts
            int totalCells = board->getBoardSize();
            
            // Add snakes
            int snakesAdded = 0;
            while(snakesAdded < snakeCount) {
                int startIdx = rand() % (totalCells - 10) + 10;
                int endIdx = rand() % (startIdx - 1) + 1;
                
                if(board->canAddEntity(startIdx)) {
                    board->addBoardEntity(new Snake(startIdx, endIdx));
                    snakesAdded++;
                }
            }
            
            // Add ladders
            int laddersAdded = 0;
            while(laddersAdded < ladderCount) {
                int startIdx = rand() % (totalCells - 10) + 1;
                int endIdx = rand() % (totalCells - startIdx) + startIdx + 1;
                
                if(board->canAddEntity(startIdx) && endIdx < totalCells) {
                    board->addBoardEntity(new Ladder(startIdx, endIdx));
                    laddersAdded++;
                }
            }
        } 
        else {
            // User-defined positions
            for(auto& pos : snakePlacements) {
                if(board->canAddEntity(pos.first)) {
                    board->addBoardEntity(new Snake(pos.first, pos.second));
                }
            }
            
            for(auto& pos : ladderPlacements) {
                if(board->canAddEntity(pos.first)) {
                    board->addBoardEntity(new Ladder(pos.first, pos.second));
                }
            }
        }
    }
};

// Standard Board Strategy - Traditional Snake & Ladder positions
class StandardBoardSetupStrategy : public BoardSetupStrategy {
public:
    void setupBoard(Board* board) override {
        // Only works for 10x10 board (100 cells)
        if(board->getBoardSize() != 100) {
            cout << "Standard configuration supports only a 10x10 board (100 cells)." << endl;
            return;
        }
        
        // Standard snake positions (based on traditional board)
        board->addBoardEntity(new Snake(99, 54));
        board->addBoardEntity(new Snake(95, 75));
        board->addBoardEntity(new Snake(92, 88));
        board->addBoardEntity(new Snake(89, 68));
        board->addBoardEntity(new Snake(74, 53));
        board->addBoardEntity(new Snake(64, 60));
        board->addBoardEntity(new Snake(62, 19));
        board->addBoardEntity(new Snake(49, 11));
        board->addBoardEntity(new Snake(46, 25));
        board->addBoardEntity(new Snake(16, 6));
        
        // Standard ladder positions
        board->addBoardEntity(new Ladder(2, 38));
        board->addBoardEntity(new Ladder(7, 14));
        board->addBoardEntity(new Ladder(8, 31));
        board->addBoardEntity(new Ladder(15, 26));
        board->addBoardEntity(new Ladder(21, 42));
        board->addBoardEntity(new Ladder(28, 84));
        board->addBoardEntity(new Ladder(36, 44));
        board->addBoardEntity(new Ladder(51, 67));
        board->addBoardEntity(new Ladder(71, 91));
        board->addBoardEntity(new Ladder(78, 98));
        board->addBoardEntity(new Ladder(87, 94));
    }
};

// Now defining setupBoard for Board class
void Board::setupBoard(BoardSetupStrategy* strategy) {
    strategy->setupBoard(this);
}

// Player class
class SnakeAndLadderPlayer {
private:
    int id;
    string playerName;
    int currentPosition;
    int winCount;
    
public:
    SnakeAndLadderPlayer(int playerId, string n) {
        this->id = playerId;
        playerName = n;
        currentPosition = 0;
        winCount = 0;
    }
    
    // Getters and Setters
    string getName() { 
        return playerName;
    }
    int getPosition() { 
        return currentPosition; 
    }
    void setPosition(int pos) { 
        currentPosition = pos; 
    }
    int getScore() { 
        return winCount;
    }
    void incrementScore() { 
        winCount++; 
    }
};

// Strategy Pattern for game rules
class SnakeAndLadderRules {
public:
    virtual bool isValidMove(int currentPos, int diceValue, int boardSize) = 0;
    virtual int calculateNewPosition(int currentPos, int diceValue, Board* board) = 0;
    virtual bool checkWinCondition(int position, int boardSize) = 0;
    virtual ~SnakeAndLadderRules() {}
};

// Standard rules
class StandardSnakeAndLadderRules : public SnakeAndLadderRules {
public:
    bool isValidMove(int currentPos, int diceValue, int boardSize) override {
        return (currentPos + diceValue) <= boardSize;
    }
    
    int calculateNewPosition(int currentPos, int diceValue, Board* board) override {
        int newPos = currentPos + diceValue;
        BoardEntity* entity = board->getEntity(newPos);
        
        if(entity != nullptr) {
            return entity->getEnd();
        }
        return newPos;
    }
    
    bool checkWinCondition(int position, int boardSize) override {
        return position == boardSize;
    }
};

// Game class
class SnakeAndLadderGame {
private:
    Board* gameBoard;
    Dice* gameDice;
    deque<SnakeAndLadderPlayer*> turnQueue;
    SnakeAndLadderRules* gameRules;
    vector<IObserver*> subscriberList;
    bool isGameOver;
    
public:
    SnakeAndLadderGame(Board* b, Dice* d) {
        gameBoard = b;
        gameDice = d;
        gameRules = new StandardSnakeAndLadderRules();
        isGameOver = false;
    }
    
    void addPlayer(SnakeAndLadderPlayer* player) {
        turnQueue.push_back(player);
    }
    
    void addObserver(IObserver* observer) {
        subscriberList.push_back(observer);
    }

    void notify(string msg) {
        for(auto observer : subscriberList) {
            observer->update(msg);
        }
    }
    
    void displayPlayerPositions() {
        cout << "\n=== Current Player Positions ===" << endl;
        for(auto player : turnQueue) {
            cout << player->getName() << ": " << player->getPosition() << endl;
        }
        cout << "==============================" << endl;
    }
    
    void play() {
        if(turnQueue.size() < 2) {
            cout << "A minimum of 2 players is required to start the game." << endl;
            return;
        }
        
        notify("Game initiated.");

        gameBoard->display();
        
        while(!isGameOver) {
            SnakeAndLadderPlayer* currentPlayer = turnQueue.front();
            
            cout << "\n" << currentPlayer->getName() << "'s turn. Press Enter to roll the dice...";
            cin.ignore();
            cin.get();
            
            int rollValue = gameDice->roll();
            cout << "Dice result: " << rollValue << endl;
            
            int currentPos = currentPlayer->getPosition();
            
            if(gameRules->isValidMove(currentPos, rollValue, gameBoard->getBoardSize())) {
                int intermediatePos = currentPos + rollValue;
                int newPos = gameRules->calculateNewPosition(currentPos, rollValue, gameBoard);
                
                currentPlayer->setPosition(newPos);
                
                // Check if player encountered snake or ladder
                BoardEntity* entity = gameBoard->getEntity(intermediatePos);
                if(entity != nullptr) {
                    bool encounteredSnake = (entity->name() == "SNAKE");
                    if(encounteredSnake) {
                        cout << "Encountered snake at " << intermediatePos << ". Moving down to " << newPos << "." << endl;
                        notify(currentPlayer->getName() + " encountered a snake at " + to_string(intermediatePos) + " and moved down to " + to_string(newPos));
                    }
                    else {
                        cout << "Encountered ladder at " << intermediatePos << ". Moving up to " << newPos << "." << endl;
                        notify(currentPlayer->getName() + " encountered a ladder at " + to_string(intermediatePos) + " and moved up to " + to_string(newPos));
                    }
                }
                
                notify(currentPlayer->getName() + " completed a move. Current position: " + to_string(newPos));
                displayPlayerPositions();
                
                if(gameRules->checkWinCondition(newPos, gameBoard->getBoardSize())) {
                    cout << "\n" << currentPlayer->getName() << " has won the game." << endl;
                    currentPlayer->incrementScore();

                    notify(string("Game concluded. Winner: ") + currentPlayer->getName());
                    isGameOver = true;
                }
                else {
                    // Move player to back of queue
                    turnQueue.pop_front();
                    turnQueue.push_back(currentPlayer);
                }
            }
            else {
                cout << "Exact roll required to reach cell " << gameBoard->getBoardSize() << "." << endl;
                // Move player to back of queue
                turnQueue.pop_front();
                turnQueue.push_back(currentPlayer);
            }
        }
    }
    
    ~SnakeAndLadderGame() {
        delete gameRules;
    }
};

// Factory Pattern
class SnakeAndLadderGameFactory {
public:
    static SnakeAndLadderGame* createStandardGame() {
        Board* board = new Board(10);  // Standard 10x10 board
        BoardSetupStrategy* strategy = new StandardBoardSetupStrategy();
        board->setupBoard(strategy);
        delete strategy;
        
        Dice* dice = new Dice(6);  // Standard 6-faced dice
        
        return new SnakeAndLadderGame(board, dice);
    }
    
    static SnakeAndLadderGame* createRandomGame(int boardSize, RandomBoardSetupStrategy::Difficulty difficulty) {
        Board* board = new Board(boardSize);
        BoardSetupStrategy* strategy = new RandomBoardSetupStrategy(difficulty);
        board->setupBoard(strategy);
        delete strategy;
        
        Dice* dice = new Dice(6);
        
        return new SnakeAndLadderGame(board, dice);
    }
    
    static SnakeAndLadderGame* createCustomGame(int boardSize, BoardSetupStrategy* strategy) {
        Board* board = new Board(boardSize);
        board->setupBoard(strategy);
        
        Dice* dice = new Dice(6);
        
        return new SnakeAndLadderGame(board, dice);
    }
};

// Main function for Snake and Ladder
int main() {
    cout << "=== SNAKES & LADDERS ===" << endl;
    
    SnakeAndLadderGame* game = nullptr;
    Board* board = nullptr;
    
    cout << "Select game configuration:" << endl;
    cout << "1. Standard Configuration (10x10 board with canonical positions)" << endl;
    cout << "2. Random Configuration (user-specified board size and difficulty)" << endl;
    cout << "3. Custom Configuration (user-specified entities)" << endl;
    
    int choice;
    cin >> choice;
    
    if(choice == 1) {
        // Standard game
        game = SnakeAndLadderGameFactory::createStandardGame();
        board = new Board(10);
        
    }
    else if(choice == 2) {
        // Random game with difficulty
        int boardSize;
        cout << "Enter board size (e.g., 10 for a 10x10 board): ";
        cin >> boardSize;
        
        cout << "Select difficulty level:" << endl;
        cout << "1. Easy (fewer snakes, more ladders)" << endl;
        cout << "2. Medium (balanced)" << endl;
        cout << "3. Hard (more snakes, fewer ladders)" << endl;
        
        int diffChoice;
        cin >> diffChoice;
        
        RandomBoardSetupStrategy::Difficulty diff;
        switch(diffChoice) {
            case 1: diff = RandomBoardSetupStrategy::EASY; break;
            case 2: diff = RandomBoardSetupStrategy::MEDIUM; break;
            case 3: diff = RandomBoardSetupStrategy::HARD; break;
            default: diff = RandomBoardSetupStrategy::MEDIUM;
        }
        
        game = SnakeAndLadderGameFactory::createRandomGame(boardSize, diff);
        board = new Board(boardSize);
        
    } 
    else if(choice == 3) {
        // Custom game
        int boardSize;
        cout << "Enter board size (e.g., 10 for a 10x10 board): ";
        cin >> boardSize;
        
        cout << "Select custom setup mode:" << endl;
        cout << "1. Specify counts only (random placement)" << endl;
        cout << "2. Specify exact positions for each entity" << endl;
        
        int customChoice;
        cin >> customChoice;
        
        if(customChoice == 1) {
            int numSnakes, numLadders;
            cout << "Enter number of snakes: ";
            cin >> numSnakes;
            cout << "Enter number of ladders: ";
            cin >> numLadders;
            
            BoardSetupStrategy* strategy = new CustomCountBoardSetupStrategy(numSnakes, numLadders, true);
            game = SnakeAndLadderGameFactory::createCustomGame(boardSize, strategy);
            delete strategy;
            
        } 
        else {
            int numSnakes, numLadders;
            cout << "Enter number of snakes: ";
            cin >> numSnakes;
            cout << "Enter number of ladders: ";
            cin >> numLadders;
            
            CustomCountBoardSetupStrategy* strategy = new CustomCountBoardSetupStrategy(numSnakes, numLadders, false);
            
            // Get snake positions
            for(int i = 0; i < numSnakes; i++) {
                int startIdx, endIdx;
                cout << "Enter snake " << (i+1) << " start and end indices: ";
                cin >> startIdx >> endIdx;
                strategy->addSnakePosition(startIdx, endIdx);
            }
            
            // Get ladder positions
            for(int i = 0; i < numLadders; i++) {
                int startIdx, endIdx;
                cout << "Enter ladder " << (i+1) << " start and end indices: ";
                cin >> startIdx >> endIdx;
                strategy->addLadderPosition(startIdx, endIdx);
            }
            
            game = SnakeAndLadderGameFactory::createCustomGame(boardSize, strategy);
            delete strategy;
        }
        
        board = new Board(boardSize);
    }
    
    if(game == nullptr) {
        cout << "Invalid selection." << endl;
        return 1;
    }
    
    // Add observer
    IObserver* notifier = new SnakeAndLadderConsoleNotifier();
    game->addObserver(notifier);
    
    // Create players
    int numPlayers;
    cout << "Enter number of players: ";
    cin >> numPlayers;
    
    for(int i = 0; i < numPlayers; i++) {
        string name;
        cout << "Enter name for player " << (i+1) << ": ";
        cin >> name;
        SnakeAndLadderPlayer* player = new SnakeAndLadderPlayer(i+1, name);
        game->addPlayer(player);
    }
    
    // Play the game
    game->play();
    
    // Cleanup
    delete game;
    delete board;
    delete notifier;
    
    return 0;
}
