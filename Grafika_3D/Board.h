#pragma once
#include "Element.h"
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H


struct Spherical
{
	float distance, theta, phi;
	Spherical(float gdistance, float gtheta, float gphi) : distance(gdistance), theta(gtheta), phi(gphi) { }
	float getX() { return distance * cos(phi) * cos(theta); }
	float getY() { return distance * sin(phi); }
	float getZ() { return distance * cos(phi) * sin(theta); }
};

struct Character {
	GLuint TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	GLuint Advance;
};

class Board {
private:
	bool _win = false;
	int _size;
	int _bombsCount;
	std::vector<Element> _elements;

public:
	Board(int size, int bombCount);
	~Board();
	void logBoard();
	void addNeighbours(int i, Element& element);
	Element* getElementFromCoordinates(std::string boardPlacement, int x, int y) {
		for (auto& element : _elements) {
			if (element.boardPlacement() == boardPlacement && element.xPosition() == x && element.yPosition() == y) {
				return &element;
			}
		}
	std::cout << "tried to get element from " + boardPlacement + " x: " + std::to_string(x) + " y: " + std::to_string(y) + "\n";
	}
	std::vector<Element> getElements() const { return _elements; }
	void drawCube(float size);
};

void draw1_3(int number, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void draw2_3(int number, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void draw3_3(int number, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void drawFull(int number, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);

void draw1(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void draw2(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void draw3(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void draw4(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void draw5(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void draw6(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void draw7(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);
void draw8(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 poin4);

