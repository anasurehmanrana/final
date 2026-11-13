#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "SFML 3.1.0 Movement Test");
    window.setFramerateLimit(60);

    // Create a player rectangle
    sf::RectangleShape player({ 60.f, 60.f });
    player.setFillColor(sf::Color::Cyan);
    player.setPosition({ 370.f, 500.f });

    float speed = 350.f; // Pixels per second
    sf::Clock clock;

    while (window.isOpen())
    {
        // Calculate delta time (time elapsed since last frame)
        float dt = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Real-time keyboard input (A/D or Left/Right arrows)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        {
            player.move({ -speed * dt, 0.f });
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        {
            player.move({ speed * dt, 0.f });
        }

        window.clear(sf::Color(30, 30, 30)); // Dark gray background
        window.draw(player);
        window.display();
    }

    return 0;
}