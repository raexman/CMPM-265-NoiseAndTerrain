#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "noise\noise.h"
#include <string>
#include <iostream>

int main();
void generateNoise();
bool drawMap(sf::RenderWindow*);
void drawUI(sf::RenderWindow*);
int getRange(double, int, int, int);

//functions;
void update();


class TileMap : public sf::Drawable, public sf::Transformable
{
public:

	bool load(const std::string& tileset, sf::Vector2u tileSize, int* tiles, unsigned int width, unsigned int height)
	{
		// load the tileset texture
		if (!m_tileset.loadFromFile(tileset))
			return false;

		// resize the vertex array to fit the level size
		m_vertices.setPrimitiveType(sf::Quads);
		m_vertices.resize(width * height * 4);

		// populate the vertex array, with one quad per tile
		for (unsigned int i = 0; i < width; ++i)
			for (unsigned int j = 0; j < height; ++j)
			{
				// get the current tile number
				int tileNumber = tiles[i + j * width];

				// find its position in the tileset texture
				int tu = tileNumber % (m_tileset.getSize().x / tileSize.x);
				int tv = tileNumber / (m_tileset.getSize().x / tileSize.x);

				// get a pointer to the current tile's quad
				sf::Vertex* quad = &m_vertices[(i + j * width) * 4];

				// define its 4 corners
				quad[0].position = sf::Vector2f(i * tileSize.x, j * tileSize.y);
				quad[1].position = sf::Vector2f((i + 1) * tileSize.x, j * tileSize.y);
				quad[2].position = sf::Vector2f((i + 1) * tileSize.x, (j + 1) * tileSize.y);
				quad[3].position = sf::Vector2f(i * tileSize.x, (j + 1) * tileSize.y);

				// define its 4 texture coordinates
				quad[0].texCoords = sf::Vector2f(tu * tileSize.x, tv * tileSize.y);
				quad[1].texCoords = sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
				quad[2].texCoords = sf::Vector2f((tu + 1) * tileSize.x, (tv + 1) * tileSize.y);
				quad[3].texCoords = sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
			}

		return true;
	}

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		// apply the transform
		states.transform *= getTransform();

		// apply the tileset texture
		states.texture = &m_tileset;

		// draw the vertex array
		target.draw(m_vertices, states);
	}

	sf::VertexArray m_vertices;
	sf::Texture m_tileset;
};

//Variables
sf::RenderWindow *window;
TileMap tileMap;
const int rows = 280;
const int cols = 240;
int tileValues[rows*cols];
unsigned int tileTypes = 8;
unsigned int maxTileTypes = 8;
double zValue = 0;
sf::Clock gameTime;
bool isAlive = false;
bool isMutating = false;
double direction = 1;
int worldSeed = 0;
int tileWidth = 4;
int tileHeight = 4;
double seedRate = 0.001;
double baseSeedRate = 0.001;
int offset = 0;
double sine = 0;
double sineDirection = 1;
int sineLoopDuration = 100;
double maxSine = (maxTileTypes - offset) * sineLoopDuration;
sf::Font font;

//Main
int main()
{
	using namespace std;

	int a = 0;
	int*b = &a;

	cout << "A: " << a << " B: " << b << endl;

	window = new sf::RenderWindow(sf::VideoMode(1920, 1080), "Terrain");
	if (!font.loadFromFile("pixeled.ttf"))
	{

	}
	drawMap(window);
	update();

	return 0;
}

//Update
void update()
{
	while (window->isOpen())
	{
		sf::Time elapsed = gameTime.restart();

		sf::Event event;
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window->close();
		}

		//
		if (isMutating)
		{
			sine += elapsed.asMilliseconds() * sineDirection;
			if (sine < 0)
			{
					sineDirection = 1;
			}

			if (sine > maxSine)
			{
				sineDirection = -1;
			}

			tileTypes = sine / sineLoopDuration;
			if (tileTypes < 1)
				tileTypes = 1;


		}
		//Start or stop mutation.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
		{
			isMutating = true;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
		{
			isMutating = false;
		}

		//Start or stop time.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
		{
			isAlive = true;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		{
			isAlive = false;
		}

		//Change time direction.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		{
			direction = 1.00;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		{
			direction = -1.00;
		}

		//Increase or decrease SEED OF ZA WARUDO
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
		{
			worldSeed--;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
		{
			worldSeed++;
		}

		//Reset, decrease or increase the world seed rate.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X))
		{
			seedRate = baseSeedRate;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z))
		{
			seedRate -= 0.00005;

			//Avoid going in reverse.
			if (seedRate < 0)
				seedRate = 0;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C))
		{
			seedRate += 0.00005;
		}

		//If alive, increase zValue over time.
		if (isAlive)
		{
			zValue += elapsed.asMilliseconds() * direction * seedRate;
		}
		//Otherwise allow for manual zValue change.
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		{
			zValue += 0.01;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		{
			zValue -= 0.01;
		}

		//Decrease or increase types of tiles.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		{
			tileTypes--;
			if (tileTypes < 1)
				tileTypes = 1;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		{
			tileTypes++;
			if (tileTypes > maxTileTypes - offset)
				tileTypes = maxTileTypes - offset;
		}

		//Offset terrain tiles.
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2))
		{
			offset = 0;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1))
		{
			offset++;
			if (offset > maxTileTypes - 1)
				offset = maxTileTypes - 1;
			if ((tileTypes + offset) >= maxTileTypes)
				tileTypes = maxTileTypes - offset;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3))
		{
			offset--;
			if (offset < 0)
				offset = 0;
			if ((tileTypes + offset) >= maxTileTypes)
				tileTypes = maxTileTypes - offset;
		}

		window->clear();
		drawMap(window);
		drawUI(window);
		window->display();
	}
}


void generateNoise()
{
	using namespace noise;

	module::Perlin noiseModule;
	noiseModule.SetSeed(worldSeed);

	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < cols; col++)
		{
			double value = noiseModule.GetValue(row*0.1, col*0.1, zValue);
			tileValues[row * cols + col] = getRange(value, tileTypes, -1, 1);
		}
	}
}

int getRange(double value, int numOfSegments, int min, int max)
{
	bool print = false;
	if (max < min) return -1;

	//Account for negative minimum values.
	if (min < 0)
	{
		max += abs(min);
		value += abs(min);
		min += abs(min);
	}

	double segmentWidth = double(max) / double(numOfSegments);
	int range = round(value / segmentWidth);
	if (print)
	{
		std::cout << "NumOfSegments: " << numOfSegments << std::endl;
		std::cout << "SegmentWidth: " << segmentWidth << std::endl;
		std::cout << "Range: " << range << std::endl;
		std::cout << "Final Range: " << (range % numOfSegments) << std::endl;
	}
	return  (range % numOfSegments) + offset;
}

bool drawMap(sf::RenderWindow *window)
{
	bool print = false;
	generateNoise();

	if (print)
	{
		for (int n = 0; n < rows*cols; n++)
		{
			std::cout << "Tile #" << n << ": " << tileValues[n] << std::endl;
		}
	}

	if (!tileMap.load("tileset.png", sf::Vector2u(tileWidth, tileHeight), tileValues, cols, rows))
		return -1;

	window->draw(tileMap);
}

void drawUI(sf::RenderWindow *window)
{
	using namespace std;
	sf::Text label;
	string text;
	text = "Tile Amount\n[Left] - Decrease | [Right] - Increase \n\n";
	text += "Tile Offset\n[1] - Decrease | [2] - Reset | [3] - Increase \n\n";
	text += "Mutation (Oscillating Tile Terrains)\n[R] - Enable | [F] - Disable \n\n";
	text += "Seed\n[Q] - Decrease | [E] - Increase \n";
	text += "Seed Change Rate\n[Z] - Decrease | [X] - Reset | [C] - Increase \n\n";
	text += "Z Value Over Time\n[W] - Start | [S] - Stop \n\n";
	text += "Z Value Direction (While time is active)\n[A] - Backward | [D] - Forward \n\n";
	text += "Z Value Manual Steps\n[Down] - Decrease | [Up] - Increase \n\n";
	label.setString(text);
	label.setCharacterSize(24);
	label.setFont(font);
	label.setFillColor(sf::Color::White);
	label.setPosition(1000, 24);

	window->draw(label);
}