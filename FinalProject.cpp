//#include <iostream>
//#include <SFML/Graphics.hpp>
//#include <SFML/Window.hpp>
//#include <SFML/System.hpp>
//#include <cmath> // Include cmath for sqrt function
//#include <cstdlib>
//#include <ctime> // Include ctime for time function
//
//using namespace std;
//using namespace sf;
//
//class Bullet
//{
//public:
//    CircleShape shape;
//    Vector2f currVelocity;
//    float maxspeed;
//
//    Bullet(float radius = 5.f) : currVelocity(0.f, 0.f), maxspeed(20.f)
//    {
//        this->shape.setRadius(radius);
//        this->shape.setFillColor(Color::Red);
//    };
//};
//
//class Bazuka : public Bullet
//{
//public:
//    CircleShape triangle;
//
//};
//
//int main()
//{
//    srand(time(NULL)); // Initialising random function
//
//    RenderWindow window(VideoMode(1920, 1080), "360° Monster Shooter", Style::Default);
//    window.setFramerateLimit(60);
//
//    //Player
//    CircleShape player(25.f);
//    player.setFillColor(Color::White);
//    int pHp = 10;
//
//    //Bullets
//    Bullet b1;
//    vector<Bullet> bullets;
//
//    //Enemy
//    RectangleShape enemy;
//    enemy.setFillColor(Color::Magenta);
//    enemy.setSize(Vector2f(30.f, 30.f));
//    int EnemySpawnTimer = 35;
//
//    vector<RectangleShape> enemies;
//
//    //Hp bar
//    RectangleShape hpBar;
//    hpBar.setFillColor(Color::Red);
//    hpBar.setSize(Vector2f(200, 20));
//    hpBar.setPosition(600.f, 10.f);
//
//    //Vectors
//    Vector2f PlayerCenter;
//    Vector2f MousePosWindow;
//    Vector2f aimDir;
//    Vector2f aimDirNorm;
//    Vector2f enemyDir;
//    Vector2f enemyDirNorm;
//
//    int shootTimer = 0;
//    int killed = 0;
//    int stime = 35;
//
//    while (window.isOpen() && pHp > 0)
//    {
//        Event event;
//        while (window.pollEvent(event))
//        {
//            if (event.type == Event::Closed)
//                window.close();
//            else if (event.key.code == Keyboard::Escape)
//                window.close();
//        }
//
//        // UPDATE
//
//        if (shootTimer < 10)
//        {
//            shootTimer++;
//        }
//
//        //Vectors:
//        PlayerCenter = Vector2f(player.getPosition().x + player.getRadius(), player.getPosition().y + player.getRadius()); // Use assignment operator '=' instead of '=='
//        MousePosWindow = Vector2f(Mouse::getPosition(window));
//        aimDir = MousePosWindow - PlayerCenter; // Basic vector math
//        float aimDirLength = sqrt(pow(aimDir.x, 2) + pow(aimDir.y, 2)); // Magnitude
//        if (aimDirLength != 0)
//        {
//            aimDirNorm = aimDir / aimDirLength;
//        }
//        else
//        {
//            aimDirNorm = Vector2f(0.f, 0.f); // Set aimDirNorm to zero vector if aimDir length is zero
//        }
//
//        //Player
//        if ((Keyboard::isKeyPressed(Keyboard::A)) && (player.getPosition().x > 0))
//        {
//            player.move(-10.f, 0);
//        }
//        else if (Keyboard::isKeyPressed(Keyboard::D) && (player.getPosition().x < (window.getSize().x - 2 * player.getRadius())))
//        {
//            player.move(10.f, 0);
//        }
//        else if ((Keyboard::isKeyPressed(Keyboard::W)) && (player.getPosition().y > 0))
//        {
//            player.move(0, -10.f);
//        }
//        else if (Keyboard::isKeyPressed(Keyboard::S) && (player.getPosition().y < (window.getSize().y - 2 * player.getRadius())))
//        {
//            player.move(0.f, 10.f);
//        }
//
//        //Shooting
//        if (Mouse::isButtonPressed(Mouse::Left) && shootTimer >= 5)
//        {
//            b1.shape.setPosition(PlayerCenter);
//            b1.currVelocity = aimDirNorm * b1.maxspeed;
//
//            bullets.push_back(Bullet(b1));
//            shootTimer = 0;
//        }
//
//        for (size_t i = 0; i < bullets.size(); i++)
//        {
//            bullets[i].shape.move(bullets[i].currVelocity);
//
//            if (bullets[i].shape.getPosition().x < 0 || bullets[i].shape.getPosition().x > window.getSize().x || bullets[i].shape.getPosition().y < 0 || bullets[i].shape.getPosition().y > window.getSize().y)
//            {
//                bullets.erase(bullets.begin() + i);
//            }
//        }
//
//
//        // Update enemies
//        for (size_t i = 0; i < enemies.size(); i++)
//        {
//            // Calculate direction towards the player
//            enemyDir = PlayerCenter - enemies[i].getPosition();
//            float enemyDirLength = sqrt(pow(enemyDir.x, 2) + pow(enemyDir.y, 2)); // Magnitude
//            if (enemyDirLength != 0)
//            {
//                enemyDirNorm = enemyDir / enemyDirLength;
//            }
//            else
//            {
//                enemyDirNorm = Vector2f(0.f, 0.f); // Set enemyDirNorm to zero vector if enemyDir length is zero
//            }
//
//            // Move the enemy towards the player
//            enemies[i].move(enemyDirNorm * 2.f); // Adjust the multiplier for speed
//        }
//
//
//        //Spawn enemies
//        if (EnemySpawnTimer < stime)
//        {
//            EnemySpawnTimer++;
//        }
//        else if (EnemySpawnTimer >= stime)
//        {
//            // Determine the edge from which to spawn the enemy
//            int edge = rand() % 4; // 0: top, 1: right, 2: bottom, 3: left
//
//            int spawnX, spawnY;
//            switch (edge)
//            {
//            case 0: // Top edge
//                spawnX = rand() % int(window.getSize().x - enemy.getSize().x);
//                spawnY = 0 - enemy.getSize().y; // Spawn above the window
//                break;
//            case 1: // Right edge
//                spawnX = window.getSize().x;
//                spawnY = rand() % int(window.getSize().y - enemy.getSize().y);
//                break;
//            case 2: // Bottom edge
//                spawnX = rand() % int(window.getSize().x - enemy.getSize().x);
//                spawnY = window.getSize().y;
//                break;
//            case 3: // Left edge
//                spawnX = 0 - enemy.getSize().x; // Spawn to the left of the window
//                spawnY = rand() % int(window.getSize().y - enemy.getSize().y);
//                break;
//            }
//
//            enemy.setPosition(spawnX, spawnY);
//            enemies.push_back(RectangleShape(enemy));
//            EnemySpawnTimer = 0;
//        }
//
//        //Collision
//        for (size_t i = 0; i < bullets.size(); i++)
//        {
//            for (size_t k = 0; k < enemies.size(); k++)
//            {
//                if (bullets[i].shape.getGlobalBounds().intersects(enemies[k].getGlobalBounds()))
//                {
//                    bullets.erase(bullets.begin() + i);
//                    enemies.erase(enemies.begin() + k);
//
//                    killed++;
//                    //Decrementing only i to prevent skipping elements after erasing projectiles
//                    i--;
//                    break; //exiting inner loop if collision detected
//                }
//            }
//        }
//
//        if (killed == 10)
//        {
//            stime = 30;
//            enemy.setFillColor(Color::Green);
//        }
//        else if (killed == 40)
//        {
//            stime = 25;
//            enemy.setFillColor(Color::Yellow);
//        }
//        else if (killed == 70)
//        {
//            stime = 20;
//            enemy.setFillColor(Color::Red);
//        }
//        else if (killed == 100)
//        {
//            stime = 30;
//            enemy.setSize(Vector2f(60.f, 60.f));
//            enemy.setFillColor(Color::Green);
//        }
//        else if (killed == 150)
//        {
//            stime = 20;
//            enemy.setFillColor(Color::Yellow);
//        }
//        else if (killed == 190)
//        {
//            stime = 10;
//            enemy.setFillColor(Color::Red);
//        }
//
//        //Collision of enemies
//        for (size_t i = 0; i < enemies.size(); i++)
//        {
//            if (player.getGlobalBounds().intersects(enemies[i].getGlobalBounds()))
//            {
//                pHp--;
//                enemies.erase(enemies.begin() + i);
//            }
//        }
//
//        //Updating UI (HP bar....etc)
//        hpBar.setSize(Vector2f(pHp * 20, 20));
//
//
//        // DRAW
//        window.clear();
//
//        window.draw(player);
//
//        for (size_t i = 0; i < bullets.size(); i++)
//        {
//            window.draw(bullets[i].shape);
//        }
//
//        for (size_t j = 0; j < enemies.size(); j++)
//        {
//            window.draw(enemies[j]);
//        }
//
//        window.draw(hpBar);
//
//        window.display();
//    }
//    return 0;
//}