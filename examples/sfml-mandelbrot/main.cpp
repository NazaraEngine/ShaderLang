#include <SFML/Graphics.hpp>
#include <NZSL/GlslWriter.hpp>
#include <NZSL/Parser.hpp>
#include <algorithm>
#include <iostream>

int main()
{
	sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1280, 720)), "SFML + NZSL Shader", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	window.setVerticalSyncEnabled(true);

	auto mandelbrotShader = nzsl::ParseFromFile("mandelbrot.nzsl");

	nzsl::GlslWriter glslWriter;
	auto glslShader = glslWriter.Generate(*mandelbrotShader);

	sf::Shader shader;
	if (!shader.loadFromMemory(glslShader.code, sf::Shader::Type::Fragment))
	{
		std::cerr << "failed to load fragment shader" << std::endl;
		return 0;
	}

	sf::Texture palette;
	if (!palette.loadFromFile("palette.png"))
	{
		std::cerr << "failed to load palette" << std::endl;
		return 0;
	}

	float scale = 2.2f;
	float targetScale = scale;
	int iterCount = 120;
	sf::Vector2f center = sf::Vector2f(window.getSize()) / 2.f;

	shader.setUniform("palette", palette);
	shader.setUniform("screen_size", sf::Vector2f(window.getSize()));
	shader.setUniform("center", center);
	shader.setUniform("scale", scale);
	shader.setUniform("iteration_count", iterCount);

	// use a fullscreen shape to apply shader on screen
	sf::RectangleShape fullscreenShape;
	fullscreenShape.setSize(sf::Vector2f(window.getSize()));
	fullscreenShape.setFillColor(sf::Color::Yellow);

	sf::Vector2i mousePos = sf::Mouse::getPosition(window);

	sf::Clock updateClock;
	while (window.isOpen())
	{
		while (std::optional<sf::Event> event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
				break;
			}
			else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
			{
				if (keyPressed->code == sf::Keyboard::Key::Escape)
					window.close();
			}
			else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
			{
				sf::Vector2i delta = mouseMoved->position - mousePos;
				mousePos = mouseMoved->position;

				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					center += scale * sf::Vector2f(sf::Vector2i(delta.x, -delta.y));
					shader.setUniform("center", center);
				}
			}
			else if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>())
				targetScale = std::clamp(targetScale - targetScale * 0.1f * mouseWheelScrolled->delta, 0.000001f, 3.f);
			else if (const auto* resized = event->getIf<sf::Event::Resized>())
			{
				sf::Vector2f newSize(float(resized->size.x), float(resized->size.y));

				shader.setUniform("screen_size", newSize);
				fullscreenShape.setSize(newSize);

				sf::FloatRect visibleArea(sf::Vector2f(0.f, 0.f), sf::Vector2f(newSize.x, newSize.y));
				window.setView(sf::View(visibleArea));
			}
		}

		// Smooth zoom a bit
		if (targetScale != scale)
		{
			float diff = targetScale - scale;
			if (targetScale < scale)
				scale = std::max(scale + diff * 0.02f * updateClock.getElapsedTime().asSeconds(), targetScale);
			else if (targetScale > scale)
				scale = std::min(scale + diff * 0.02f * updateClock.getElapsedTime().asSeconds(), targetScale);

			shader.setUniform("scale", scale);
		}

		window.draw(fullscreenShape, &shader);

		window.display();
	}

	return EXIT_SUCCESS;
}
