#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <vector>
#include <memory>
#include <cmath>
#include <random>
#include <fstream>

///Debug usage
#include <iostream>

// Global Function

// - Recreate Game Setting.txt
void initializeTextFile(const std::string& filePath, float soundEffect, float backgroundMusic) {

    // Create file if not exist
    std::ifstream checkFileExist(filePath);
    if (!(checkFileExist.good())) {
        std::ofstream createNewFile(filePath);
        createNewFile.close();
    }

    // Initialize the file with default values when file exist
    std::ofstream outputFile(filePath);
    if (!outputFile) {
        std::cerr << "Failed to create file." << std::endl;
        return;
    }
    outputFile << soundEffect << " " << backgroundMusic << std::endl;
    outputFile.close();
}
// - Read fron Game Setting.txt to get user setting
void readTextFile(const std::string& filePath, float& soundEffect, float& backgroundMusic) {
    // If can't file file than init the file
    std::ifstream inputFile(filePath);
    if (!inputFile) {
        initializeTextFile(filePath, soundEffect, backgroundMusic);
        return;
    }

    // Read the values from the file
    if (!(inputFile >> soundEffect >> backgroundMusic)) { // Check if reading the values was successful
        soundEffect = 100.0f;
        backgroundMusic = 100.0f;
    }
    else {
        // Limit sound and music to be within [0, 100]
        soundEffect = ((soundEffect < 0) ? 0 : ((soundEffect > 100) ? 100 : soundEffect));
        backgroundMusic = ((backgroundMusic < 0) ? 0 : ((backgroundMusic > 100) ? 100 : backgroundMusic));
    }
    inputFile.close();
    initializeTextFile(filePath, soundEffect, backgroundMusic);
}

// - Recreate History Score.txt
void initializeHistoryFile(const std::string& historyFilePath, int pathHistoryScore[5]) {

    // Create file if not exist
    std::ifstream checkFileExist(historyFilePath);
    if (!(checkFileExist.good())) {
        std::ofstream createNewFile(historyFilePath);
        createNewFile.close();
    }
    
    // Initialize the file with default values when file exist
    std::ofstream outputFile(historyFilePath);
    if (!outputFile) {
        std::cerr << "Failed to create file." << std::endl;
        return;
    }

    for (int i = 0; i < 4; i++) {
        outputFile << pathHistoryScore[i] << " ";
    }
    outputFile << pathHistoryScore[4] << std::endl;

    outputFile.close();
}
// - Read fron Game Setting.txt to get user setting
void readHistoryTextFile(const std::string& historyFilePath, int pathHistoryScore[5]) {
    // If can't file file than init the file
    for (int i = 0; i < 5; i++) {
        pathHistoryScore[i] = 0;
    }

    std::ifstream inputFile(historyFilePath);
    if (!inputFile) {
        initializeHistoryFile(historyFilePath, pathHistoryScore);
        return;
    }

    // Read the values from the file
    for (int i = 0; i < 5; i++) {
        if (!(inputFile >> pathHistoryScore[i])) { // Check if reading the value was successful
            // Reset all value to 0
            for (int k = 0; k < i; k++) {
                pathHistoryScore[k] = 0;
            }
            initializeHistoryFile(historyFilePath, pathHistoryScore);
            return;
        }
    }

    inputFile.close();
    initializeHistoryFile(historyFilePath, pathHistoryScore);
}


// - Random from a range using a specific type
template <typename InputType>
InputType random(InputType Min, InputType Max) {
    // Uniformly-distributed integer random number generator
    std::random_device rd;
    // Mersenne twister (High quality number but with more storage space requirement)
    std::mt19937 generator(rd());
    // Produces random floating-point values x, uniformly distributed on the interval [Min,Max)
    std::uniform_real_distribution<> dist(Min, Max + 1);
    return InputType(dist(generator));
}

// Global Variable
float soundEffect = 100.0f; //Sound Effect
float backgroundMusic = 100.0f; // Music
const std::string filePath = "Game File/Game Setting.txt"; // Setting file Path
const std::string historyFilePath = "Game File/History Score.txt"; // Setting Historyfile Path

std::vector<sf::Vector2f> pathList[5];// Path list
int pathHistoryScore[5]; // History Highest

// Forward declarations
class SoundPlayer;
class Enemy;
class Tower;
class Bullet;
class Game;

// Global game instance
Game* g_game = nullptr;

//Audio Player class (Play audio)
class SoundPlayer {
/*
* How to use:
* SoundPlayer NameYouWant;
* NameYouWant.playSound("NameOfFileWithoutPath.wav", 100.f, 1.0f, VariableOfGameSettingLinkedToThisAudio, false);
* 
*/
public:

    // Play the sound with the input parameter (filename don't need path)
    void playSound(const std::string filename, float setVolume = 100.f, float Pitch = 1.0f, float SettingVolume = 1.0f, bool setLoop = false) {
        IsPaused = false;
        NoAudio = false;

        std::string filePathAndName = "src/assets/audio/" + filename;
        if (!buffer.loadFromFile(filePathAndName)) {
            // Error handling if audio loading fails
        }

        //Associate a sound buffer to the sound object
        sound.setBuffer(buffer);

        // Audio Setting
        sound.setPitch(Pitch);
        sound.setVolume(setVolume * SettingVolume / 100); // Setting is store in range [0, 100], we want it to be percentage
        sound.setLoop(setLoop);
        sound.play();
    }

    // Set the Volume
    void volume(float setVolume, float SettingVolume) {
        sound.setVolume(setVolume * SettingVolume / 100);
    }

    // Stop the audio
    void stop() {
        IsPaused = false;
        NoAudio = true;
        sound.stop();
    }

    // Pause the audio
    void pause() {
        if (IsPaused == false) {
            IsPaused = true;
            NoAudio = false;
            sound.pause();
        }
    }

    // Resume the audio
    void resume() {
        if (IsPaused == true) {
            IsPaused = false;
            NoAudio = false;
            sound.play();
        }
    }

    // Go to the next "offset" second
    void offset(float offset = 1.f) {
        sound.pause();
        sound.setPlayingOffset(sf::seconds(offset));
        sound.play();
    }

    // Return true if music pause
    bool isPause() {
        return IsPaused;
    }

    // Return true if no music playing
    bool isAudio() {
        return !NoAudio;
    }

    // Update method to check sound status and handle delayed loop
    void update() {
        if (sound.getStatus() == sf::Sound::Stopped && !NoAudio) {
            NoAudio = true;
        }
    }

private:
    //const char* filenameList[10];
    sf::SoundBuffer buffer;
    sf::Sound sound;
    bool IsPaused = false, NoAudio = true; // Also set to false when music is stop (Pause)
};

// Enemy class
class Enemy {
private:
    std::unique_ptr<sf::Shape> shape;
    sf::RectangleShape hpBar;
    sf::RectangleShape hpBarBackground;
    float speed;
    int health;
    int maxHealth;
    std::vector<sf::Vector2f> waypoints;
    size_t currentWaypoint;
    bool dead;

public:
    int getHealth() const {
        return health;
    }
    Enemy(std::vector<sf::Vector2f> path, float speed, int health, std::unique_ptr<sf::Shape> shape)
        : speed(speed), health(health), maxHealth(health), waypoints(path), currentWaypoint(0), dead(false), shape(std::move(shape)) {
        this->shape->setPosition(waypoints[0]);
        this->shape->setOrigin(this->shape->getLocalBounds().width / 2.0f, this->shape->getLocalBounds().height / 2.0f);

        // Set up HP bar
        hpBarBackground.setSize(sf::Vector2f(30, 5));
        hpBarBackground.setFillColor(sf::Color::Black);
        hpBarBackground.setOrigin(hpBarBackground.getSize().x / 2.0f, hpBarBackground.getSize().y / 2.0f);

        hpBar.setSize(sf::Vector2f(30, 5));
        hpBar.setFillColor(sf::Color::Red);
        hpBar.setOrigin(hpBar.getSize().x / 2.0f, hpBar.getSize().y / 2.0f);
    }

    void update(float deltaTime) {
        if (currentWaypoint < waypoints.size()) {
            sf::Vector2f direction = waypoints[currentWaypoint] - shape->getPosition();
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            if (length <= speed * deltaTime) {
                shape->setPosition(waypoints[currentWaypoint]);
                currentWaypoint++;
            }
            else {
                sf::Vector2f velocity = direction / length * speed;
                shape->move(velocity * deltaTime);
            }
        }

        // Update HP bar position
        hpBarBackground.setPosition(shape->getPosition() + sf::Vector2f(0, -20));
        hpBar.setPosition(shape->getPosition() + sf::Vector2f(0, -20));

        // Update HP bar size based on health
        float hpPercent = static_cast<float>(health) / maxHealth;
        hpBar.setSize(sf::Vector2f(30 * hpPercent, 5));
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(*shape);
        window.draw(hpBarBackground);
        window.draw(hpBar);
    }

    void damage(int amount) {
        health -= amount;
        if (health <= 0) {
            dead = true;
        }

        // Change color when receiving damage
        shape->setFillColor(sf::Color::White);
    }

    bool isDead() const {
        return dead;
    }

    bool isOutOfBounds() const {
        return currentWaypoint >= waypoints.size();
    }

    sf::Vector2f getPosition() const {
        return shape->getPosition();
    }

    void setHealth(int newHealth) {
        health = newHealth;
        if (health <= 0) {
            dead = true;
        }
    }
};

// Bullet class
class Bullet {
private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    int damage;
    bool dead;

public:
    Bullet(sf::Vector2f position, sf::Vector2f velocity, int damage)
        : velocity(velocity), damage(damage), dead(false) {
        shape.setRadius(5.0f);
        shape.setFillColor(sf::Color::Yellow);
        shape.setPosition(position);
        shape.setOrigin(5.0f, 5.0f);
    }

    void update(float deltaTime, std::vector<Enemy>& enemies) {
        shape.move(velocity * deltaTime);

        // Check collision with enemies
        for (auto& enemy : enemies) {
            if (!enemy.isDead() && shape.getGlobalBounds().intersects(sf::FloatRect(enemy.getPosition() - sf::Vector2f(10.0f, 10.0f), sf::Vector2f(20.0f, 20.0f)))) {
                enemy.damage(damage);
                dead = true;
                break;
            }
        }

        // Check if bullet is out of bounds
        if (shape.getPosition().x < 0 || shape.getPosition().x > 800 ||
            shape.getPosition().y < 0 || shape.getPosition().y > 600) {
            dead = true;
        }
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(shape);
    }

    bool isDead() const { return dead; }
};

// Tower class
class Tower {
private:
    sf::CircleShape shape;
    sf::CircleShape rangeCircle;
    sf::Text levelText;
    sf::Font font;
    float range;
    float attackTimer;
    float attackCooldown;
    float radius;
    int damage;
    std::vector<Bullet> bullets;
    sf::Color color;
    int level;
    SoundPlayer audioPlayer;

public:
    Tower(float x, float y, float range, int damage, float attackCooldown, sf::Color color, float radius)
        : range(range), attackTimer(0), attackCooldown(attackCooldown), damage(damage), color(color), level(1) {
        shape.setRadius(radius);
        shape.setFillColor(color);
        shape.setPosition(x, y);
        shape.setOrigin(radius, radius);

        rangeCircle.setRadius(range);
        rangeCircle.setFillColor(sf::Color::Transparent);
        rangeCircle.setOutlineThickness(1.0f);
        rangeCircle.setOutlineColor(sf::Color::White);
        rangeCircle.setPosition(x, y);
        rangeCircle.setOrigin(range, range);

        if (!font.loadFromFile("src/assets/font/Roboto-Black.ttf")) {
            // Error handling if font loading fails
        }

        levelText.setFont(font);
        levelText.setCharacterSize(12);
        levelText.setFillColor(sf::Color::White);
        levelText.setString("Lv. " + std::to_string(level));
        levelText.setPosition(x - 10, y - 10);
    }

    bool isPointWithinRange(const sf::Vector2f& point) const {
        return shape.getGlobalBounds().contains(point);
    }

    void update(float deltaTime, std::vector<Enemy>& enemies) {
        attackTimer += deltaTime;
        if (attackTimer >= attackCooldown) {
            Enemy* targetEnemy = nullptr;

            if (color == sf::Color::Blue) {
                // Sniper tower targets the enemy with the highest HP
                int maxHealth = 0;
                for (auto& enemy : enemies) {
                    if (!enemy.isDead() && enemy.getHealth() > maxHealth) {
                        maxHealth = enemy.getHealth();
                        targetEnemy = &enemy;
                    }
                }
            }
            else {
                // Other towers target the closest enemy
                float closestDist = range;
                for (auto& enemy : enemies) {
                    if (!enemy.isDead()) {
                        float dist = std::sqrt(
                            std::pow(enemy.getPosition().x - shape.getPosition().x, 2) +
                            std::pow(enemy.getPosition().y - shape.getPosition().y, 2)
                        );
                        if (dist < closestDist) {
                            closestDist = dist;
                            targetEnemy = &enemy;
                        }
                    }
                }
            }

            if (targetEnemy) {
                // Calculate bullet velocity towards the enemy
                sf::Vector2f direction = targetEnemy->getPosition() - shape.getPosition();
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                sf::Vector2f velocity = direction / length * 300.0f;

                // Spawn a bullet
                bullets.emplace_back(shape.getPosition(), velocity, damage);
                attackTimer = 0;

                //Play Sound Effect base on type
                if (color == sf::Color::Red) {
                    //Basic
                    audioPlayer.playSound("ArrowShoot2.wav", 100.f, 1.0f, soundEffect);
                } else if (color == sf::Color::Green) {
                    //Rapid
                    audioPlayer.playSound("ArrowShoot2.wav", 100.f, 2.0f, soundEffect);
                } else if (color == sf::Color::Blue) {
                    //Sniper
                    audioPlayer.playSound("ArrowShoot1.wav", 100.f, 1.0f, soundEffect);
                } else {
                    //If not set
                    audioPlayer.playSound("ArrowShoot2.wav", 100.f, 1.0f, soundEffect);
                }
            }
        }

        // Update bullets
        for (auto it = bullets.begin(); it != bullets.end();) {
            it->update(deltaTime, enemies);
            if (it->isDead()) {
                it = bullets.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(shape);
        window.draw(levelText);
        for (const auto& bullet : bullets) {
            bullet.draw(window);
        }
    }

    void drawRange(sf::RenderWindow& window) const {
        window.draw(rangeCircle);
    }

    void setPosition(const sf::Vector2f& position) {
        shape.setPosition(position);
        rangeCircle.setPosition(position);
        levelText.setPosition(position.x - 10, position.y - 10);
    }

    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }
    void upgrade() {
        level++;
        damage += 10;
        range += 20;
        attackCooldown *= 0.9f;

        rangeCircle.setRadius(range);
        rangeCircle.setOrigin(range, range);

        levelText.setString("Lv. " + std::to_string(level));
    }

    int getLevel() const {
        return level;
    }

    sf::Color getColour() const {
        return color;
    }

    float getRadius() const {
        return radius;
    }


};

// Meun class
class Menu {
private:
    // Game class
    class Game {
    private:
        sf::RenderWindow& window;
        std::vector<Tower> towers;
        std::vector<Enemy> enemies;
        std::vector<sf::Vector2f> path;
        sf::VertexArray pathVertices;

        // Tower selection
        sf::RectangleShape towerSelectionBar;
        std::vector<sf::RectangleShape> towerButtons;
        std::vector<sf::Text> towerTexts;
        sf::Font font;
        int selectedTower;
        bool placingTower;
        Tower* newTower;

        // UI elements
        sf::Text lifeText;
        sf::Text moneyText;
        sf::Text killsText;
        int playerLife;
        int playerMoney;
        int enemyKills;

        bool showTutorial = false;
        sf::RectangleShape tutorialButton;
        sf::RectangleShape tutorialBackground;
        sf::Text tutorialButtonText;
        sf::Text tutorialText;

        bool isPaused;
        sf::Text pauseText;

        bool gameOver;
        sf::Text gameOverText;
        sf::RectangleShape closeButton;
        sf::Text closeButtonText;

        bool toStart;
        sf::RectangleShape backToStartButton;
        sf::Text backToStartButtonText;

        int waveNumber;
        bool bossSpawned;
        float spawnTimer;
        float spawnInterval;
        int CurrentLevel;

        //variables for tracking difficulty and controling enemy health and spawn rate
        float difficultyTimer;     // Time elapsed to increase difficulty
        float healthMultiplier;      // Multiplier to increase enemy health
        float spawnRateMultiplier;     // Multiplier to make enemies spawn faster

    public:
        Game(sf::RenderWindow& window, int level) : window(window),
            isPaused(false), gameOver(false), toStart(false),
            selectedTower(0), placingTower(false), newTower(nullptr),
            waveNumber(1), bossSpawned(false), spawnTimer(0), spawnInterval(2.0f), difficultyTimer(0.0f), healthMultiplier(1.0f), spawnRateMultiplier(1.0f),
            pathVertices(sf::LineStrip),
            playerLife(100), playerMoney(500), enemyKills(0) {

            // Set up the list of path
            pathList[0] = { //default map
                sf::Vector2f(0, 100),
                sf::Vector2f(200, 100),
                sf::Vector2f(200, 200),
                sf::Vector2f(400, 200),
                sf::Vector2f(400, 100),
                sf::Vector2f(600, 100),
                sf::Vector2f(600, 300),
                sf::Vector2f(400, 300),
                sf::Vector2f(400, 400),
                sf::Vector2f(200, 400),
                sf::Vector2f(200, 500),
                sf::Vector2f(800, 500)
            };
            pathList[1] = {
                sf::Vector2f(0, 100),
                sf::Vector2f(700, 100),
                sf::Vector2f(700, 200),
                sf::Vector2f(100, 200),
                sf::Vector2f(100, 300),
                sf::Vector2f(700, 300),
                sf::Vector2f(700, 400),
                sf::Vector2f(100, 400),
                sf::Vector2f(100, 500),
                sf::Vector2f(800, 500)
            };
            pathList[2] = {
                sf::Vector2f(0, 100),
                sf::Vector2f(600, 300),
                sf::Vector2f(300, 150),
                sf::Vector2f(150, 300),
                sf::Vector2f(400, 300),
                sf::Vector2f(200, 600),
            };
            pathList[3] = {
                sf::Vector2f(0, 150),
                sf::Vector2f(600, 150),
                sf::Vector2f(600, 75),
                sf::Vector2f(200, 75),
                sf::Vector2f(200, 400),
                sf::Vector2f(50, 400),
                sf::Vector2f(50, 300),
                sf::Vector2f(575, 300),
                sf::Vector2f(575, 200),
                sf::Vector2f(700, 200),
                sf::Vector2f(700, 500),
                sf::Vector2f(300, 500),
                sf::Vector2f(300, 600)
            };
            pathList[4] = {
                sf::Vector2f(0, 300),
                sf::Vector2f(200, 300),
                sf::Vector2f(200, 150),
                sf::Vector2f(350, 150),
                sf::Vector2f(350, 300),
                sf::Vector2f(500, 300),
                sf::Vector2f(500, 450),
                sf::Vector2f(650, 450),
                sf::Vector2f(650, 300),
                sf::Vector2f(700, 300),
                sf::Vector2f(650, 300),
                sf::Vector2f(650, 450),
                sf::Vector2f(500, 450),
                sf::Vector2f(500, 300),
                sf::Vector2f(350, 300),
                sf::Vector2f(350, 150),
                sf::Vector2f(200, 150),
                sf::Vector2f(200, 300),
                sf::Vector2f(0, 300)
            };

            path = pathList[level];
            CurrentLevel = level;

            // Set up path vertices
            for (const auto& waypoint : path) {
                pathVertices.append(sf::Vertex(waypoint, sf::Color::White));
            }

            // Set up tower selection bar
            towerSelectionBar.setSize(sf::Vector2f(window.getSize().x, 50.0f));
            towerSelectionBar.setFillColor(sf::Color::White);
            towerSelectionBar.setPosition(0, window.getSize().y - 50.0f);

            // Load font
            if (!font.loadFromFile("src/assets/font/Roboto-Black.ttf")) {
                // Error handling if font loading fails
            }

            // Tutorial Button Setup
            tutorialButton.setSize(sf::Vector2f(70, 40));
            tutorialButton.setFillColor(sf::Color::Magenta);
            tutorialButton.setPosition(720, 555); 

            tutorialButtonText.setFont(font);
            tutorialButtonText.setString("Tutorial");
            tutorialButtonText.setCharacterSize(20);
            tutorialButtonText.setFillColor(sf::Color::Black);
            tutorialButtonText.setPosition(720, 560);

            // Set up Tutorial Text
            tutorialText.setFont(font);
            tutorialText.setString("How to Play:\n1. Click on any tower options to place towers \n     and defend against enemies.\n     Price:\n     Basic :100 Rapid: 150 Sniper: 200\n2. Left-click: upgrade tower; right-click: sell tower\n3. Press P to pause the game.");
            tutorialText.setCharacterSize(20);
            tutorialText.setFillColor(sf::Color::White);
            tutorialText.setPosition(170, 215);
            tutorialBackground.setSize(sf::Vector2f(460, 170)); 
            tutorialBackground.setFillColor(sf::Color::Blue);
            tutorialBackground.setPosition(170, 215);

            // Set up pause text
            pauseText.setFont(font);
            pauseText.setString("Game Paused\nPress P to Resume");
            pauseText.setCharacterSize(40);
            pauseText.setFillColor(sf::Color::White);
            pauseText.setPosition(250, 250);

            // Set up tower buttons
            std::vector<std::string> towerNames = { "Basic", "Rapid", "Sniper" };
            std::vector<sf::Color> towerColors = { sf::Color::Red, sf::Color::Green, sf::Color::Blue };
            sf::Vector2f buttonSize(100.0f, 40.0f);
            sf::Vector2f buttonPosition(10.0f, window.getSize().y - 45.0f);

            for (int i = 0; i < towerNames.size(); ++i) {
                sf::RectangleShape button(buttonSize);
                button.setFillColor(towerColors[i]);
                button.setPosition(buttonPosition);
                towerButtons.push_back(button);

                sf::Text text(towerNames[i], font, 16);
                text.setFillColor(sf::Color::White);
                text.setPosition(buttonPosition.x + 10.0f, buttonPosition.y + 10.0f);
                towerTexts.push_back(text);

                buttonPosition.x += buttonSize.x + 10.0f;
            }
            lifeText.setFont(font);
            lifeText.setCharacterSize(20);
            lifeText.setFillColor(sf::Color::White);
            lifeText.setPosition(10.0f, 10.0f);

            moneyText.setFont(font);
            moneyText.setCharacterSize(20);
            moneyText.setFillColor(sf::Color::White);
            moneyText.setPosition(10.0f, 40.0f);

            killsText.setFont(font);
            killsText.setCharacterSize(20);
            killsText.setFillColor(sf::Color::White);
            killsText.setPosition(10.0f, 70.0f);

            gameOverText.setFont(font);
            gameOverText.setString("Game Over");
            gameOverText.setCharacterSize(50);
            gameOverText.setFillColor(sf::Color::Red);
            gameOverText.setPosition(270, 200);

            // Set up close game button
            closeButton.setSize(sf::Vector2f(200, 50));
            closeButton.setFillColor(sf::Color::Blue);
            closeButton.setPosition(300, 300);

            // Set up close game button text
            closeButtonText.setFont(font);
            closeButtonText.setString("Game Over");
            closeButtonText.setCharacterSize(20);
            closeButtonText.setFillColor(sf::Color::White);
            closeButtonText.setPosition(350, 310);

            // Go back to Start button
            backToStartButton.setSize(sf::Vector2f(200, 50));
            backToStartButton.setFillColor(sf::Color::Blue);
            backToStartButton.setPosition(300, 375);

            // Go back to Start button text
            backToStartButtonText.setFont(font);
            backToStartButtonText.setString("Back To Start");
            backToStartButtonText.setCharacterSize(20);
            backToStartButtonText.setFillColor(sf::Color::White);
            backToStartButtonText.setPosition(350, 385);
            }

        void run() {
            sf::Clock clock;

            while (window.isOpen() && !toStart) {

                // Regenerated used BGM when start game each time
                BGMaudioPlayer.update(); // Update isAudio State
                if (!BGMaudioPlayer.isAudio() && !gameOver) {
                    std::string SongSelect = "InGameBGM" + std::to_string(random(1, 8)) + ".wav";
                    BGMaudioPlayer.playSound(SongSelect, 30, 1, backgroundMusic);
                }
                
                float deltaTime = clock.restart().asSeconds();

                handleEvents();
                update(deltaTime);
                render();
            }
        }

        bool getStart() {
            return toStart;
        }

    private:
        SoundPlayer ToweraudioPlayer, BGMaudioPlayer, GameaudioPlayer;
        void handleEvents() {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::P) {
                        isPaused = !isPaused; // Toggle pause state

                        // Audio changes if Pause game
                        GameaudioPlayer.playSound("Pause.wav", 100.f, 1.0f, soundEffect);
                        if (BGMaudioPlayer.isPause()) {
                            BGMaudioPlayer.resume();
                        } else {
                            BGMaudioPlayer.pause();
                        }
                        
                    } else {
                        // If player use hotkey to place tower
                        sf::Vector2f mousePosition(float(sf::Mouse::getPosition(window).x), float(sf::Mouse::getPosition(window).y));;
                        switch (event.key.code) {
                        case sf::Keyboard::B:
                        case sf::Keyboard::Num1:
                            selectedTower = 0;
                            placingTower = true;
                            break;
                        case sf::Keyboard::R:
                        case sf::Keyboard::Num2:
                            selectedTower = 1;
                            placingTower = true;
                            break;
                        case sf::Keyboard::S:
                        case sf::Keyboard::Num3:
                            selectedTower = 2;
                            placingTower = true;
                            break;
                        default:
                            placingTower = false;
                            break;
                        }
                        if (placingTower) {
                            ToweraudioPlayer.playSound("Building2.wav", 100.f, 1.0f, soundEffect);
                            newTower = createTower(selectedTower, mousePosition);
                        }
                        break;
                    }
                }
                else if (event.type == sf::Event::MouseButtonPressed && gameOver) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        // Check if the mouse is within the button bounds
                        if (closeButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                            window.close();
                        }
                    }

                    if (backToStartButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        toStart = true;
                    }
                }
                else if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2f mousePosition(event.mouseButton.x, event.mouseButton.y);
                        bool towerClicked = false;

                        // Check if the tutorial button is clicked
                        if (tutorialButton.getGlobalBounds().contains(mousePosition)) {
                            showTutorial = !showTutorial; // Toggle tutorial display
                            GameaudioPlayer.playSound("SelectSound.wav", 100.f, 1.0f, soundEffect);
                        }

                        // Check if a tower is clicked for upgrading or selling
                        for (auto it = towers.begin(); it != towers.end(); ++it) {
                            if (it->isPointWithinRange(mousePosition)) {
                                // Check if a tower is clicked for upgrading
                                
                                // Leave if in placing mode
                                if (placingTower) {
                                    ToweraudioPlayer.playSound("CannotPlaceHere.wav", 100.f, 1.0f, soundEffect);
                                    towerClicked = true;
                                    break;
                                }
                                // Clicked for upgrading
                                int upgradeCost = getUpgradeCost(*it);
                                if (playerMoney >= upgradeCost) {
                                    ToweraudioPlayer.playSound("Upgrade1.wav", 100.f, 1.0f, soundEffect);
                                    it->upgrade();
                                    playerMoney -= upgradeCost;
                                }
                                else if (playerMoney <  upgradeCost) {
                                    //Play sound effect when there are not enough money
                                    ToweraudioPlayer.playSound("NotEnoughMoney.wav", 100.f, 1.0f, soundEffect);
                                }
                                towerClicked = true;
                                break;
                            }
                        }

                        if (!towerClicked && !placingTower) {
                            // Check if a tower button is clicked
                            for (int i = 0; i < towerButtons.size(); ++i) {
                                if (towerButtons[i].getGlobalBounds().contains(mousePosition)) {
                                    selectedTower = i;
                                    placingTower = true;
                                    newTower = createTower(selectedTower, mousePosition);
                                    ToweraudioPlayer.playSound("Building2.wav", 100.f, 1.0f, soundEffect);
                                    break;
                                }
                            }
                        }
                        else if (!towerClicked && placingTower) {
                            // Place the tower if the mouse is not on the tower selection bar
                            if (mousePosition.y < window.getSize().y - 50.0f) {
                                int towerCost = getTowerCost(selectedTower);
                                if (playerMoney >= towerCost) {
                                    towers.push_back(*newTower);
                                    playerMoney -= towerCost;
                                    placingTower = false;
                                    newTower = nullptr;
                                    ToweraudioPlayer.playSound("Building1.wav", 100.f, 1.0f, soundEffect);
                                } else {
                                    // Cancil placing tower if not enough money
                                    ToweraudioPlayer.playSound("NotEnoughMoney.wav", 100.f, 1.0f, soundEffect);
                                    placingTower = false;
                                    break;
                                }
                            }
                        }
                    }
                    else if (event.mouseButton.button == sf::Mouse::Right) {
                        sf::Vector2f mousePosition(event.mouseButton.x, event.mouseButton.y);
                        bool towerClicked = false;
                        if (placingTower) {
                            placingTower = false;
                        }
                        else if (!placingTower) {
                            for (std::vector<Tower>::iterator it = towers.begin(); it != towers.end(); ++it) { // auto == std::vector<Tower>::iterator
                                //Right click cancil placing tower mode
                                if (it->isPointWithinRange(mousePosition)) {
                                    // Check if a tower is clicked for selling
                                    if (event.mouseButton.button == sf::Mouse::Right) {
                                        towers.erase(it); // Remove tower
                                        ToweraudioPlayer.playSound("GetMoney.wav", 100.f, 1.0f, soundEffect);
                                        towerClicked = true;
                                        playerMoney += (getTowerCost(selectedTower) / 2);
                                        break;
                                    }
                                }
                            }
                        }
                        
                    }
                }
                else if (event.type == sf::Event::MouseMoved) {
                    if (placingTower) {
                        sf::Vector2f mousePosition(event.mouseMove.x, event.mouseMove.y);
                        newTower->setPosition(mousePosition);
                    }
                }
            }
        }

        void update(float deltaTime) {
            if (isPaused || gameOver || showTutorial) {
                return; // Skip the update logic if the game is paused
            }

            difficultyTimer += deltaTime;
            if (difficultyTimer >= 30.0f) { // Every 30 seconds
                healthMultiplier += 0.2f;        // Increase enemy health by 20%
                spawnRateMultiplier *= 0.9f;     // Make spawn interval faster by 10%
                spawnInterval *= spawnRateMultiplier; // Update the spawn interval
                difficultyTimer = 0.0f;          // Reset the difficulty timer
                //std::cout << "Difficulty increased! Health x" << healthMultiplier << ", Spawn Interval x" << spawnRateMultiplier << std::endl;
            }

            spawnTimer += deltaTime;
            if (spawnTimer >= spawnInterval) {
                spawnEnemy();
                spawnTimer = 0;
            }

            for (auto& tower : towers) {
                tower.update(deltaTime, enemies);
            }

            for (auto it = enemies.begin(); it != enemies.end();) {
                it->update(deltaTime);

                if (it->isDead()) {
                    playerMoney += 50; // Increase player's money when an enemy is killed
                    GameaudioPlayer.playSound("GetMoney.wav", 100.f, 1.0f, soundEffect);
                    enemyKills++;
                    it = enemies.erase(it);
                }
                else if (it->isOutOfBounds()) {
                    playerLife -= 10; // Decrease player's life when an enemy reaches the end
                    GameaudioPlayer.playSound("LooseLife.wav", 100.f, 1.0f, soundEffect);
                    it = enemies.erase(it);
                }
                else {
                    ++it;
                }
            }

            // Check if player's life reaches zero
            if (playerLife <= 0) {
                if (!gameOver) {
                    //Handle with audio
                    BGMaudioPlayer.stop();
                    pathHistoryScore[CurrentLevel] = (pathHistoryScore[CurrentLevel] > enemyKills) ? pathHistoryScore[CurrentLevel] : enemyKills;
                    initializeHistoryFile(historyFilePath, pathHistoryScore);
                    GameaudioPlayer.playSound("GameOver.wav", 100.f, 1.0f, soundEffect);
                }
                gameOver = true; // Set game over state
            }

            lifeText.setString("Life: " + std::to_string(playerLife));
            moneyText.setString("Money: " + std::to_string(playerMoney));
            killsText.setString("Kills: " + std::to_string(enemyKills));
        }

        void render() {
            window.clear();

            // Draw path vertices
            window.draw(pathVertices);

            // Draw game objects
            for (const auto& enemy : enemies) {
                enemy.draw(window);
            }
            for (const auto& tower : towers) {
                tower.draw(window);
            }

            // Draw tower range if placing a tower
            if (placingTower) {
                newTower->drawRange(window);
            }

            // Draw UI elements
            window.draw(towerSelectionBar);
            for (const auto& button : towerButtons) {
                window.draw(button);
            }
            for (const auto& text : towerTexts) {
                window.draw(text);
            }

            // Draw UI texts
            window.draw(lifeText);
            window.draw(moneyText);
            window.draw(killsText);

            // Draw tutorial button
            window.draw(tutorialButton);
            window.draw(tutorialButtonText);

            if (showTutorial && !isPaused) {
                window.draw(tutorialBackground);
                window.draw(tutorialText); // Draw tutorial text
            }

            if (isPaused && !gameOver) {
                window.draw(pauseText); // Draw pause text
            }
                
            if (gameOver) {
                window.draw(gameOverText); // Draw Game Over
                window.draw(closeButton);
                window.draw(closeButtonText);
                window.draw(backToStartButton);
                window.draw(backToStartButtonText);
            }

            window.display();
        }

        void spawnEnemy() {
            if (!bossSpawned && waveNumber % 5 == 0) {
                // Spawn boss enemy
                auto bossShape = std::make_unique<sf::CircleShape>(20.0f);
                bossShape->setFillColor(sf::Color::Magenta);
                int bossHealth = static_cast<int>(1000 * healthMultiplier);
                enemies.emplace_back(path, 50.0f, 500, std::move(bossShape));
                bossSpawned = true;
            }
            else {
                // Spawn regular enemies
                if (waveNumber % 3 == 0) {
                    // Fast enemy with low health
                    auto fastShape = std::make_unique<sf::CircleShape>(5.0f);
                    fastShape->setFillColor(sf::Color::Cyan);
                    int fastHealth = static_cast<int>(200 * healthMultiplier);
                    enemies.emplace_back(path, 200.0f, 50, std::move(fastShape));
                }
                else if (waveNumber % 3 == 1) {
                    // Slow enemy with high health
                    auto slowShape = std::make_unique<sf::RectangleShape>(sf::Vector2f(20.0f, 20.0f));
                    slowShape->setFillColor(sf::Color::Green);
                    int slowHealth = static_cast<int>(200 * healthMultiplier);
                    enemies.emplace_back(path, 50.0f, 150, std::move(slowShape));
                }
                else {
                    // Normal enemy
                    auto normalShape = std::make_unique<sf::CircleShape>(10.0f);
                    normalShape->setFillColor(sf::Color::Red);
                    int normalEnemyHealth = static_cast<int>(200 * healthMultiplier);
                    enemies.emplace_back(path, 100.0f, 100, std::move(normalShape));
                }
            }

            waveNumber++;
        }

        Tower* createTower(int type, const sf::Vector2f& position) {
            switch (type) {
                // Tower(float x, float y, float range, int damage, float attackCooldown, sf::Color color, float radius)
            case 0:
                return new Tower(position.x, position.y, 100.0f, 50, 1.0f, sf::Color::Red, 20.0f);
            case 1:
                return new Tower(position.x, position.y, 80.0f, 30, 0.5f, sf::Color::Green, 15.0f);
            case 2:
                return new Tower(position.x, position.y, 150.0f, 100, 2.0f, sf::Color::Blue, 25.0f);
            default:
                return nullptr;
            }
        }

        int getTowerCost(int type) {
            switch (type) {
            case 0:
                return 100;
            case 1:
                return 150;
            case 2:
                return 200;
            default:
                return 0;
            }
        }

        int getUpgradeCost(const Tower& tower) {
            return tower.getLevel() * 100; // Adjust the upgrade cost formula as needed
        }
    };

    class StartScreen {
    private:

        class Slider {
        private:
            sf::RectangleShape bar;
            sf::RectangleShape handle;
            sf::Text valueText; // Text written on RHS
            sf::Font font;
            bool isDragging;
            float value;

        public:
            Slider(const sf::Font& font, float x, float y, float value) : isDragging(false) {
                // Configure the slider bar
                bar.setSize(sf::Vector2f(200, 5));
                bar.setFillColor(sf::Color::White);
                bar.setPosition(x, y);

                // Configure the slider handle
                handle.setSize(sf::Vector2f(20, 30));
                handle.setFillColor(sf::Color::Red);
                // Calculate handle x position based on given value (E.g. If volume = 57 -> Stay in 57)
                handle.setPosition(x + (value / 100.0f) * (bar.getSize().x - handle.getSize().x), y - 10);

                // Configure the slider value text (Right number)
                valueText.setFont(font);
                valueText.setCharacterSize(24);
                valueText.setFillColor(sf::Color::White);
                valueText.setPosition(bar.getPosition().x + bar.getSize().x + 10, bar.getPosition().y - 10);
                valueText.setString(std::to_string(int(value))); // Change value to string in order to show the value

            }

            // 1. Need to get current state of window (Mouse position) 2. Evebt such as Mouse click
            void handleEvent(const sf::RenderWindow& window, const sf::Event& event) {
                // If Mouse Left click the handle
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (handle.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        isDragging = true;
                    }
                }

                // If Mouse Left click is released
                if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = false;
                }

                // If Mouse is Ledt clicked and not release (Click in handle)
                if (isDragging) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    // Get New x position of handle base on mouse position in Bar
                    float newHandleX = mousePos.x - handle.getSize().x / 2.0f;

                    // Bar from where to where (x)
                    float barStart = bar.getPosition().x;
                    float barEnd = barStart + bar.getSize().x - handle.getSize().x; //Because x coordinate is count from left

                    // Ensure the handle in inside the bar (limit value like > 100 or < 0)
                    if (newHandleX < barStart) {
                        newHandleX = barStart;
                    } else if (newHandleX > barEnd) {
                        newHandleX = barEnd;
                    }

                    handle.setPosition(newHandleX, handle.getPosition().y);

                    // Update the slider value (0 to 100)
                    value = ((newHandleX - barStart) / (barEnd - barStart)) * 100;
                    //Set the right number
                    valueText.setString(std::to_string(int(value)));
                }
            }

            void render(sf::RenderWindow& window) const {
                window.draw(bar);
                window.draw(handle);
                window.draw(valueText);
            }

            float getValue() {
                return value;
            }

            void setValue(float inputValue) {
                value = inputValue;
            }

            bool getisDragging() {
                return isDragging;
            }
        };


        sf::Font font;
        sf::Text titleText, Sound, Music;
        sf::Text startButtonText, exitButtonText;
        sf::RectangleShape startButton, exitButton;
        sf::RenderWindow& window;

        Slider soundEffectSlider;
        Slider backgroundMusicSlider;

        SoundPlayer BGMaudioPlayer, EffectaudioPlayer;

        bool startGame;

    public:
        // Init Button ()
        void initializeButton(sf::RectangleShape& button, const sf::Vector2f& size, const sf::Color& color, const sf::Vector2f& position) {
            button.setSize(size);
            button.setFillColor(color);
            button.setPosition(position);
        }
        // Init Button Text
        void initializeButtonText(sf::Text& buttonText, sf::Font& font, const std::string& text, unsigned int characterSize, const sf::Color& color, const sf::RectangleShape& button) {
            buttonText.setFont(font);
            buttonText.setString(text);
            buttonText.setCharacterSize(characterSize);
            buttonText.setFillColor(color);
            // Center the text within the button
            buttonText.setPosition( // ButtonCoordinate + (ButtonHeight -Textheight) / 2 - TextTopCoordinate
                button.getPosition().x + (button.getSize().x - buttonText.getLocalBounds().width) / 2,
                button.getPosition().y + (button.getSize().y - buttonText.getLocalBounds().height) / 2 - buttonText.getLocalBounds().top
            );
        }

        StartScreen(sf::RenderWindow& window)
            : window(window), soundEffectSlider(font, 540, 15, soundEffect), backgroundMusicSlider(font, 540, 50, backgroundMusic) {
            // Load font
            if (!font.loadFromFile("src/assets/font/Roboto-Black.ttf")) {
                throw std::runtime_error("Failed to load font");
            }

            // Configure title text
            titleText.setFont(font);
            titleText.setString("Tower Defense");
            titleText.setCharacterSize(50);
            titleText.setFillColor(sf::Color::White);
            titleText.setPosition(225, 100);

            // Configure Sound Slider label
            Sound.setFont(font);
            Sound.setString("Audio");
            Sound.setCharacterSize(24);
            Sound.setFillColor(sf::Color::White);
            Sound.setPosition(470, 0);

            // Configure Music Slider label
            Music.setFont(font);
            Music.setString("Music");
            Music.setCharacterSize(24);
            Music.setFillColor(sf::Color::White);
            Music.setPosition(470, 35);
            
            // Configure start button and text
            initializeButton(startButton, sf::Vector2f(200, 50), sf::Color::Green, sf::Vector2f(300, 300));
            initializeButtonText(startButtonText, font, "Start Game", 24, sf::Color::Black, startButton);

            // Configure exit button and text
            initializeButton(exitButton, sf::Vector2f(50, 50), sf::Color::Yellow, sf::Vector2f(0, 0));
            initializeButtonText(exitButtonText, font, "Exit", 24, sf::Color::Black, exitButton);
        }
        // Main loop for the StartScreen
        void run() {
            sf::Clock clock;

            //Init sound and music
            soundEffectSlider.setValue(soundEffect);
            backgroundMusicSlider.setValue(backgroundMusic);

            while (window.isOpen() && !startGame) { // Loop until the user starts the game
                // Regenerated used BGM when enter start screen each time
                BGMaudioPlayer.update(); // Update isAudio State
                if (!BGMaudioPlayer.isAudio()) {
                    std::string SongSelect = "TitleBGM" + std::to_string(random(1, 2)) + ".wav";
                    BGMaudioPlayer.playSound(SongSelect, 30, 1, backgroundMusic, true);
                }

                float deltaTime = clock.restart().asSeconds();
                
                handleEvents();
                render();
            }
        }

        void initialize() {
            startGame = false;
        }

        bool getStart() {
            return startGame;
        }
        
        float getSound() {
            return soundEffectSlider.getValue();
        }

        float getMusic() {
            return backgroundMusicSlider.getValue();
        }

    private:
        void handleEvents() {
            sf::Event event;
            
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                
                soundEffectSlider.handleEvent(window, event);
                soundEffect = soundEffectSlider.getValue();
                backgroundMusicSlider.handleEvent(window, event);
                backgroundMusic = backgroundMusicSlider.getValue();
                
                //Play Demo Sound if Dragging Sound
                if (soundEffectSlider.getisDragging()) {
                    if (!EffectaudioPlayer.isAudio()) {
                        EffectaudioPlayer.playSound("SelectSound.wav", 100.f, 1.0f, soundEffect, true);
                    } else {
                        EffectaudioPlayer.volume(100, soundEffect);
                    }
                } else {
                    EffectaudioPlayer.stop();
                }
                //Update BGM volume
                BGMaudioPlayer.volume(30, backgroundMusic);


                if (event.type == sf::Event::MouseButtonPressed) {
                    initializeTextFile(filePath, soundEffect, backgroundMusic);
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        if (startButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            startGame = true; // Signal to transition to the game
                            EffectaudioPlayer.playSound("SelectSound.wav", 100.f, 1.0f, soundEffect);
                            BGMaudioPlayer.stop();
                        } else if (exitButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            EffectaudioPlayer.playSound("SelectSound.wav", 100.f, 1.0f, soundEffect);
                            exit(0);
                        }
                    }
                }
            }
        }

        void render() {
            window.clear();
            window.draw(titleText);
            window.draw(startButton);
            window.draw(startButtonText);
            window.draw(exitButton);
            window.draw(exitButtonText);
            window.draw(Music);
            window.draw(Sound);
            soundEffectSlider.render(window);
            backgroundMusicSlider.render(window);

            window.display();
        }
    };

    class SelectionScreen {
    private:
        sf::RenderWindow& window;
        sf::Font font;
        sf::Text titleText, HistoryHighest;
        std::vector<sf::Text> levelTexts, LevelHistoryHighest;
        sf::RectangleShape backButton;
        sf::Text backButtonText;
        std::vector<sf::RectangleShape> levelButton;

        std::vector<std::string> levels;

        SoundPlayer BGMaudioPlayer, EffectaudioPlayer;

        int selectedIndex;
        bool selected, isMainMenu, HighestScoreRendered = false;

    public:
        // Init Button ()
        void initializeButton(sf::RectangleShape& button, const sf::Vector2f& size, const sf::Color& color, const sf::Vector2f& position) {
            button.setSize(size);
            button.setFillColor(color);
            button.setPosition(position);
        }
        // Init Button Text
        void initializeButtonText(sf::Text& buttonText, sf::Font& font, const std::string& text, unsigned int characterSize, const sf::Color& color, const sf::RectangleShape& button) {
            buttonText.setFont(font);
            buttonText.setString(text);
            buttonText.setCharacterSize(characterSize);
            buttonText.setFillColor(color);
            // Center the text within the button
            buttonText.setPosition(
                button.getPosition().x + (button.getSize().x - buttonText.getLocalBounds().width) / 2,
                button.getPosition().y + (button.getSize().y - buttonText.getLocalBounds().height) / 2 - buttonText.getLocalBounds().top
            );
        }

        SelectionScreen(sf::RenderWindow& window)
            : window(window), levels({ "Level 1", "Level 2", "Level 3", "Level 4", "Level 5" }) {
            // Load font
            if (!font.loadFromFile("src/assets/font/Roboto-Black.ttf")) {
                throw std::runtime_error("Failed to load font");
            }


            initializeLevelButtons(levels);

            // Configure title text
            titleText.setFont(font);
            titleText.setString("Select Level");
            titleText.setCharacterSize(50);
            titleText.setFillColor(sf::Color::White);
            titleText.setPosition(250, 50);

            // Configure History score text
            HistoryHighest.setFont(font);
            HistoryHighest.setString("Highest Kill");
            HistoryHighest.setCharacterSize(30);
            HistoryHighest.setFillColor(sf::Color::Cyan);
            HistoryHighest.setPosition(450, 120);

            // Configure back button and text
            initializeButton(backButton, sf::Vector2f(50, 50), sf::Color::Green, sf::Vector2f(0, 0));
            initializeButtonText(backButtonText, font, "Back", 24, sf::Color::Black, backButton);

        }

        void run() {
            while (window.isOpen() && !selected) {
                // Regenerated used BGM when enter selection screen each time
                BGMaudioPlayer.update(); // Update isAudio State
                if (!BGMaudioPlayer.isAudio()) {
                    std::string SongSelect = "SelectionBGM" + std::to_string(random(1, 5)) + ".wav";
                    BGMaudioPlayer.playSound(SongSelect, 30, 1, backgroundMusic, true);
                }
                // Re-get Highest score each time when the game ended and go back here
                if (!HighestScoreRendered) {
                    window.clear();
                    initializeLevelHistoryHighest(LevelHistoryHighest);
                    render();
                    HighestScoreRendered = true;
                }

                handleEvents();
                render();
            }
        }

        int getSelectedLevel() {
            return selectedIndex;
        }

        bool isGameSelected() {
            return selected;
        }

        bool getBackToMainMenu() {
            return isMainMenu;
        }

        void initialize() {
            selected = isMainMenu = selectedIndex =false;
        }

    private:

        void initializeLevelButtons(const std::vector<std::string>& levelNames) {
            float buttonWidth = 200.0f;
            float buttonHeight = 50.0f;
            float startX = 150.0f;
            float startY = 180.0f;
            float spacing = 60.0f;


            for (size_t i = 0; i < levelNames.size(); ++i) {
                sf::RectangleShape button(sf::Vector2f(buttonWidth, buttonHeight));
                
                button.setPosition(startX, startY + i * spacing);

                levelButton.push_back(button);
            }

            // Configure level texts
            float yPosition = startY;
            for (const auto& level : levelNames) {
                sf::Text levelText;
                levelText.setFont(font);
                levelText.setString(level);
                levelText.setCharacterSize(30);
                levelText.setFillColor(sf::Color::Black);
                levelText.setPosition(startX + 50, yPosition);
                levelTexts.push_back(levelText);
                yPosition += 60.0f;
            }


        }

        void initializeLevelHistoryHighest(std::vector<sf::Text>& LevelHistoryHighest) {
            LevelHistoryHighest.clear();
            float startX = 450.0f;
            float startY = 180.0f;

            // Configure Level History Highest texts
            float yPosition = startY;
            for (size_t i = 0; i < levels.size(); i++) {
                sf::Text levelHistoryText;
                levelHistoryText.setFont(font);
                levelHistoryText.setString(std::to_string(pathHistoryScore[i]));
                levelHistoryText.setCharacterSize(30);
                levelHistoryText.setFillColor(sf::Color::White);
                levelHistoryText.setPosition(startX + 50, yPosition);
                LevelHistoryHighest.push_back(levelHistoryText);
                yPosition += 60.0f;
            }
        }

        void handleEvents() {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Up) {
                        EffectaudioPlayer.playSound("SelectSound.wav", 100.f, 1.0f, soundEffect);
                        selectedIndex = (selectedIndex - 1 + levelTexts.size()) % (levelTexts.size());
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        EffectaudioPlayer.playSound("SelectSound.wav", 100.f, 1.0f, soundEffect);
                        selectedIndex = (selectedIndex + 1) % (levelTexts.size());
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        EffectaudioPlayer.playSound("SelectSound.wav", 100.f, 1.0f, soundEffect);
                        selected = true;
                        HighestScoreRendered = false;
                        BGMaudioPlayer.stop();
                    }
                } else if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                        if (backButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            //currentState = State::StartScreen;
                            isMainMenu = true; // Signal to transition to MainMenu
                            EffectaudioPlayer.playSound("SelectSound.wav", 100.f, 1.0f, soundEffect);
                            selected = true;
                            BGMaudioPlayer.stop();
                        } else {
                            for (size_t i = 0; i < levelButton.size(); ++i) {
                                if (levelButton[i].getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                    selectedIndex = i;
                                    selected = true;
                                    HighestScoreRendered = false;
                                    EffectaudioPlayer.playSound("SelectSound.wav", 100.f, 1.0f, soundEffect);
                                    BGMaudioPlayer.stop();
                                }
                            }
                        }
                    }
                }
            }
        }

        void render() {
            
            window.clear();

            // Draw title
            window.draw(titleText);
            window.draw(HistoryHighest);

            // Draw Back Button
            window.draw(backButton);
            window.draw(backButtonText);

            // Draw level button
            for (size_t i = 0; i < levelButton.size(); ++i) {
                if (static_cast<int>(i) == selectedIndex) {
                    levelButton[i].setFillColor(sf::Color::Yellow);
                }
                else {
                    levelButton[i].setFillColor(sf::Color::White);
                }
                window.draw(levelButton[i]);
            }

            // Draw level text
            for (size_t i = 0; i < levelTexts.size(); ++i) {
                window.draw(levelTexts[i]);
            }

            // Draw Level History Highest text
            for (size_t i = 0; i < LevelHistoryHighest.size(); ++i) {
                window.draw(LevelHistoryHighest[i]);
            }

            window.display();
        }
    };


    sf::RenderWindow window;
    sf::Font font;
    StartScreen startScreen;
    SelectionScreen selectionScreen;


    enum class State { StartScreen, LevelSelection, Playing};
    State currentState;

    // Paths for levels
    std::vector<std::string> paths;

    // Audio Player

public:

    Menu()
        : window(sf::VideoMode(800, 600), "Tower Defense"),
        currentState(State::StartScreen),
        paths({ "path1.txt", "path2.txt", "path3.txt", "path4.txt", "path5.txt" }),
        startScreen(window),
        selectionScreen(window) {

        if (!font.loadFromFile("src/assets/font/Roboto-Black.ttf")) {
            throw std::runtime_error("Failed to load font");
        }
    }
    void run() {

        // Set Exe Icon
        sf::Image* icon = new sf::Image();
        if (icon->loadFromFile("src/assets/textures/icon.jpg")) {
            window.setIcon(icon->getSize().x, icon->getSize().y, icon->getPixelsPtr());
        }
        delete icon; // No longer use


        while (window.isOpen()) {
            switch (currentState) {
            case State::StartScreen:
                // Run the StartScreen loop
                startScreen.initialize();
                selectionScreen.initialize();

                //Play BGM in Start Screen

                startScreen.run();
                if (startScreen.getStart()) {
                    currentState = State::LevelSelection;
                } 
                break;

            case State::LevelSelection:
                //Init the pervious screen and current screen
                selectionScreen.initialize();
                startScreen.initialize();
                
                selectionScreen.run();
                if (selectionScreen.getBackToMainMenu()) {
                    currentState = State::StartScreen;
                } else if (selectionScreen.isGameSelected()) {
                    currentState = State::Playing;
                }
                break;

            case State::Playing:
                Game game(window, selectionScreen.getSelectedLevel());
                game.run();

                startScreen.initialize();

                if (game.getStart()) {
                    currentState = State::StartScreen;
                }

                break;
            }
        }
    }
};

int main() {
    //Read fron Game Setting.txt to get user setting
    readTextFile(filePath, soundEffect, backgroundMusic);
    readHistoryTextFile(historyFilePath, pathHistoryScore);

    Menu menu;
    menu.run();

    return 0;
}