#include <SFML/Graphics.hpp>
#include <NZSL/GlslWriter.hpp>
#include <NZSL/Parser.hpp>
#include <algorithm>
#include <iostream>

int main()
{
	sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML + NZSL Shader", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	window.setVerticalSyncEnabled(true);

	auto mandelbrotShader = nzsl::ParseFromFile("mandelbrot.nzsl");

	nzsl::GlslWriter glslWriter;
	auto glslShader = glslWriter.Generate(*mandelbrotShader);

	std::cout << glslShader.code << std::endl;

	sf::Shader shader;
	if (!shader.loadFromMemory(glslShader.code, sf::Shader::Fragment))
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
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
				{
					window.close();
					break;
				}

				case sf::Event::KeyPressed:
				{
					if (event.key.code == sf::Keyboard::Escape)
						window.close();
					break;
				}

				case sf::Event::MouseMoved:
				{
					sf::Vector2i newPos(event.mouseMove.x, event.mouseMove.y);
					sf::Vector2i delta = newPos - mousePos;
					mousePos = newPos;

					if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
					{
						center += scale * sf::Vector2f(sf::Vector2i(delta.x, -delta.y));
						shader.setUniform("center", center);
						break;
					}

					break;
				}

				case sf::Event::MouseWheelScrolled:
				{
					targetScale = std::clamp(targetScale - targetScale * 0.1f * event.mouseWheelScroll.delta, 0.000001f, 3.f);
					break;
				}

				case sf::Event::Resized:
				{
					sf::Vector2f newSize(float(event.size.width), float(event.size.height));

					shader.setUniform("screen_size", newSize);
					fullscreenShape.setSize(newSize);

					sf::FloatRect visibleArea(0.f, 0.f, newSize.x, newSize.y);
					window.setView(sf::View(visibleArea));
					break;
				}

				default:
					break;
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
