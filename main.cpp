#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream> 

#ifdef SFML_SYSTEM_IOS
#include <SFML/Main.hpp>
#endif

std::string resourcesDir()
{
#ifdef SFML_SYSTEM_IOS
    return "";
#else
    return "resources/";
#endif
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // Define some constants
    const float pi = 3.14159f;
    const float gameWidth = 800;
    const float gameHeight = 600;
    sf::Vector2f paddleSize(25, 100);
    float ballRadius = 10.f;

    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(gameWidth), static_cast<unsigned int>(gameHeight), 32), "SFML Tennis",
        sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    sf::SoundBuffer ballSoundBuffer;
    if (!ballSoundBuffer.loadFromFile(resourcesDir() + "ball.wav"))
        return EXIT_FAILURE;
    sf::Sound ballSound(ballSoundBuffer);

    sf::Texture sfmlLogoTexture;
    if (!sfmlLogoTexture.loadFromFile(resourcesDir() + "sfml_logo.png"))
        return EXIT_FAILURE;
    sf::Sprite sfmlLogo;
    sfmlLogo.setTexture(sfmlLogoTexture);
    sfmlLogo.setPosition(170, 50);

    sf::RectangleShape leftPaddle;
    leftPaddle.setSize(paddleSize - sf::Vector2f(3, 3));
    leftPaddle.setOutlineThickness(3);
    leftPaddle.setOutlineColor(sf::Color::Black);
    leftPaddle.setFillColor(sf::Color(100, 100, 200));
    leftPaddle.setOrigin(paddleSize / 2.f);

    sf::RectangleShape rightPaddle;
    rightPaddle.setSize(paddleSize - sf::Vector2f(3, 3));
    rightPaddle.setOutlineThickness(3);
    rightPaddle.setOutlineColor(sf::Color::Black);
    rightPaddle.setFillColor(sf::Color(200, 100, 100));
    rightPaddle.setOrigin(paddleSize / 2.f);

    sf::CircleShape ball;
    ball.setRadius(ballRadius - 3);
    ball.setOutlineThickness(2);
    ball.setOutlineColor(sf::Color::Black);
    ball.setFillColor(sf::Color::Red); // 기본 공 색상을 빨간색으로 변경
    ball.setOrigin(ballRadius / 2, ballRadius / 2);

    sf::Font font;
    if (!font.loadFromFile(resourcesDir() + "tuffy.ttf"))
        return EXIT_FAILURE;

    sf::Text pauseMessage;
    pauseMessage.setFont(font);
    pauseMessage.setCharacterSize(40);
    pauseMessage.setPosition(170.f, 200.f);
    pauseMessage.setFillColor(sf::Color::White);

#ifdef SFML_SYSTEM_IOS
    pauseMessage.setString("Welcome to SFML Tennis!\nTouch the screen to start the game.");
#else
    pauseMessage.setString("Welcome to SFML Tennis!\n\nPress space to start the game.");
#endif

    sf::Clock AITimer;
    const sf::Time AITime = sf::seconds(0.1f);
    const float paddleSpeed = 400.f;
    float rightPaddleSpeed = 0.f;
    float ballSpeed = 500.f; 
    float ballAngle = 0.f; 

    sf::Clock clock;
    bool isPlaying = false;
    int playerScore = 0; 
    int computerScore = 0; 
    sf::Text scoreText; 
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setPosition(gameWidth / 2.f - 50.f, 10.f);
    scoreText.setFillColor(sf::Color::White);

    auto resetBall = [&]() {
        ball.setPosition(gameWidth / 2.f, gameHeight / 2.f);
        do
        {
            ballAngle = static_cast<float>(std::rand() % 360) * 2.f * pi / 360.f;
        } while (std::abs(std::cos(ballAngle)) < 0.7f);
    };

    auto handleWallCollisions = [&]() {
        if (ball.getPosition().x - ballRadius < 0.f)
        {
            computerScore++;
            ballSound.play();
            resetBall();
        }
        if (ball.getPosition().x + ballRadius > gameWidth)
        {
            playerScore++;
            ballSound.play();
            resetBall();
        }
    };

    while (window.isOpen())
    {
        scoreText.setString(std::to_string(playerScore) + " - " + std::to_string(computerScore));

        window.clear(sf::Color(50, 50, 50));

        if (isPlaying)
        {
            window.draw(leftPaddle);
            window.draw(rightPaddle);
            window.draw(ball);
            window.draw(scoreText);
        }
        else
        {
            window.draw(pauseMessage);
            window.draw(sfmlLogo);
        }

        window.display();
        sf::Event event;
        while (window.pollEvent(event))
        {
            if ((event.type == sf::Event::Closed) ||
                ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window.close();
                break;
            }

            if (((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space)) ||
                (event.type == sf::Event::TouchBegan))
            {
                if (!isPlaying)
                {
                    isPlaying = true;
                    clock.restart();
                    playerScore = 0; 
                    computerScore = 0; 

                    leftPaddle.setPosition(10.f + paddleSize.x / 2.f, gameHeight / 2.f);
                    rightPaddle.setPosition(gameWidth - 10.f - paddleSize.x / 2.f, gameHeight / 2.f);
                    resetBall();
                }
            }

            if (event.type == sf::Event::Resized)
            {
                sf::View view;
                view.setSize(gameWidth, gameHeight);
                view.setCenter(gameWidth / 2.f, gameHeight / 2.f);
                window.setView(view);
            }
        }

        if (isPlaying)
        {
            float deltaTime = clock.restart().asSeconds();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) &&
                (leftPaddle.getPosition().y - paddleSize.y / 2 > 5.f))
            {
                leftPaddle.move(0.f, -paddleSpeed * deltaTime);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) &&
                (leftPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f))
            {
                leftPaddle.move(0.f, paddleSpeed * deltaTime);
            }
            if (sf::Touch::isDown(0))
            {
                sf::Vector2i pos = sf::Touch::getPosition(0);
                sf::Vector2f mappedPos = window.mapPixelToCoords(pos);
                leftPaddle.setPosition(leftPaddle.getPosition().x, mappedPos.y);
            }

            if (((rightPaddleSpeed < 0.f) && (rightPaddle.getPosition().y - paddleSize.y / 2 > 5.f)) ||
                ((rightPaddleSpeed > 0.f) && (rightPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f)))
            {
                rightPaddle.move(0.f, rightPaddleSpeed * deltaTime);
            }

            if (AITimer.getElapsedTime() > AITime)
            {
                AITimer.restart();
                if (ball.getPosition().y + ballRadius > rightPaddle.getPosition().y + paddleSize.y / 2)
                    rightPaddleSpeed = paddleSpeed;
                else if (ball.getPosition().y - ballRadius < rightPaddle.getPosition().y - paddleSize.y / 2)
                    rightPaddleSpeed = -paddleSpeed;
                else
                    rightPaddleSpeed = 0.f;
            }

            float factor = ballSpeed * deltaTime;
            ball.move(std::cos(ballAngle) * factor, std::sin(ballAngle) * factor);

            handleWallCollisions();

            if (ball.getPosition().x - ballRadius < leftPaddle.getPosition().x + paddleSize.x / 2 &&
                ball.getPosition().x - ballRadius > leftPaddle.getPosition().x &&
                ball.getPosition().y + ballRadius >= leftPaddle.getPosition().y - paddleSize.y / 2 &&
                ball.getPosition().y - ballRadius <= leftPaddle.getPosition().y + paddleSize.y / 2)
            {
                if (ball.getPosition().y > leftPaddle.getPosition().y)
                    ballAngle = pi - ballAngle + static_cast<float>(std::rand() % 20) * pi / 180;
                else
                    ballAngle = pi - ballAngle - static_cast<float>(std::rand() % 20) * pi / 180;

                ballSound.play();
                ball.setPosition(leftPaddle.getPosition().x + ballRadius + paddleSize.x / 2 + 0.1f, ball.getPosition().y);
            }

            if (ball.getPosition().x + ballRadius > rightPaddle.getPosition().x - paddleSize.x / 2 &&
                ball.getPosition().x + ballRadius < rightPaddle.getPosition().x &&
                ball.getPosition().y + ballRadius >= rightPaddle.getPosition().y - paddleSize.y / 2 &&
                ball.getPosition().y - ballRadius <= rightPaddle.getPosition().y + paddleSize.y / 2)
            {
                if (ball.getPosition().y > rightPaddle.getPosition().y)
                    ballAngle = pi - ballAngle + static_cast<float>(std::rand() % 20) * pi / 180;
                else
                    ballAngle = pi - ballAngle - static_cast<float>(std::rand() % 20) * pi / 180;

                ballSound.play();
                ball.setPosition(rightPaddle.getPosition().x - ballRadius - paddleSize.x / 2 - 0.1f, ball.getPosition().y);
            }
        }

        if (playerScore >= 5 || computerScore >= 5) {
            isPlaying = false;
        }
    }

    return EXIT_SUCCESS;
}
