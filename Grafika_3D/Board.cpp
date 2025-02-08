#include "Board.h"
#include "Element.h"
#include "pch.h"
#include <map>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::map<char, Character> Characters;
GLuint VAO, VBO;
GLuint shaderProgram;

Board::Board(int size, int bombCount) : _size(size), _bombsCount(bombCount) {
	int numberOfElements = _size * _size * 6;
	std::string boardPlacementNames[] = {"front", "back", "top", "bottom", "left", "right"};
	int boardPlacementCounter = 0;
	int x = 0;
	int y = 0;
	for(int i=0; i<numberOfElements; i++){
		if (i == (_size * _size) * (boardPlacementCounter + 1)) {
			boardPlacementCounter++;
			x = 0;
			y = 0;
		 }
		Element element(false, boardPlacementNames[boardPlacementCounter], x, y);
		x++;
		if (x == _size) {
			x = 0;
			y++;
		}
		_elements.push_back(element);
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, numberOfElements - 1);

	for (int i = 0; i < _bombsCount; ++i) {
		int index = dis(gen);
		_elements[index].setIsBomb(true);
	}

	for (int i = 0; i < numberOfElements; ++i) {
		Element& element = _elements[i];
		addNeighbours(i, element);
	}
}

Board::~Board() {

}

void Board::logBoard() {
	std::map<std::string, std::vector<std::vector<char>>> boardRepresentation;

	std::string boardPlacementNames[] = { "front", "back", "top", "bottom", "left", "right" };

	for (const auto& placement : boardPlacementNames) {
		boardRepresentation[placement] = std::vector<std::vector<char>>(_size, std::vector<char>(_size, '0'));
	}

	for (const auto& element : _elements) {
		char symbol = '0' + element.bombsNeighbours();
		if (element.isBomb()) {
			symbol = '*';
		}

		boardRepresentation[element.boardPlacement()][element.yPosition()][element.xPosition()] = symbol;
	}

	for (const auto& placement : boardPlacementNames) {
		std::cout << placement << ":\n";
		for (const auto& row : boardRepresentation[placement]) {
			for (const auto& cell : row) {
				std::cout << cell;
			}
			std::cout << '\n';
		}
		std::cout << '\n';
	}
}


void Board::drawCube(float size) {
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	std::string boardPlacementNames[] = { "front", "back", "top", "bottom", "left", "right" };

	glPushMatrix();

	glTranslatef(_size / 2.0f , _size / 2.0f, _size / 2.0f);

	for (int side = 0; side < 6; ++side) {
		glPushMatrix();

		switch (side) {
		case 0: // front
			glTranslatef(0, 0, 0); 
			glColor3f(1.0f, 0.0f, 0.0f); 
			break; 
		case 1: // back
			glTranslatef(-_size, 0, -_size); 
			glRotatef(180, 0, 1, 0); 
			glColor3f(0.0f, 1.0f, 0.0f);
			break; 
		case 2: // top
			glTranslatef(-_size, 0, 0); 
			glRotatef(90, 0, 0, 1); 
			glColor3f(0.0f, 0.0f, 1.0f);
			break; 
		case 3: // bottom
			glTranslatef(0, -_size, 0); 
			glRotatef(-90, 0, 0, 1); 
			glColor3f(1.0f, 1.0f, 0.0f);
			break; 
		case 4: // left
			glTranslatef(-_size, 0, 0); 
			glRotatef(-90, 0, 1, 0); 
			glColor3f(0.0f, 1.0f, 1.0f);
			break; 
		case 5: //right
			glTranslatef(0, 0, -_size); 
			glRotatef(90, 0, 1, 0); 
			glColor3f(1.0f, 0.0f, 1.0f); break;
		}

		glBegin(GL_QUADS);
			glVertex3f(0, 0, 0);
			glVertex3f(0, -size * _size, 0);
			glVertex3f(0, -size * _size, -size * _size);
			glVertex3f(0, 0, -size * _size);
		glEnd();

		for (int y = 0; y < _size; ++y) {
			for (int x = 0; x < _size; ++x) {
				glPushMatrix();
				glTranslatef(0.1, -x * size, -y * size);

				Element* element = getElementFromCoordinates(boardPlacementNames[side], y, x);
				float tmp = -size;
				glm::vec3 point1(0.0f, 0.0f, 0.0f);
				glm::vec3 point2(0.0f, tmp, 0.0f);
				glm::vec3 point3(0.0f, tmp, tmp);
				glm::vec3 point4(0.0f, 0.0f, tmp);

				if (!element->getIsShowed()) {
					glColor3f(0.32f, 0.32f, 0.32f);

					glBegin(GL_QUADS);
					glVertex3f(0, 0, 0);
					glVertex3f(0, -size, 0);
					glVertex3f(0, -size, -size);
					glVertex3f(0, 0, -size);
					glEnd();

					glColor3f(0.0f, 0.0f, 0.0f);

					glBegin(GL_LINES);
					glVertex3f(0.05, -size, -size);
					glVertex3f(0.05, 0, -size);
					glEnd();

					glBegin(GL_LINES);
					glVertex3f(0.05, -size, -size);
					glVertex3f(0.05, -size, 0);
					glEnd();

					draw1_3(side + 1, point1, point2, point3, point4);
					draw2_3(y + 1, point1, point2, point3, point4);
					draw3_3(x + 1, point1, point2, point3, point4);
				}
				else {
					if (element->isBomb()) {
						glColor3f(0.0f, 0.0f, 0.0f);
						glBegin(GL_QUADS);
							glVertex3f(0, 0, 0);
							glVertex3f(0, -size, 0);
							glVertex3f(0, -size, -size);
							glVertex3f(0, 0, -size);
						glEnd();
						if (!_win) {
							for (Element& e : _elements) {
								e.setShowed(true);
							}
							glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
						}
					}
					else if (element->bombsNeighbours()) {
						drawFull(element->bombsNeighbours(), point1, point2, point3, point4);
					}
					else {
						glColor3f(0.64f, 0.64f, 0.64f);
						glBegin(GL_QUADS);
							glVertex3f(0, 0, 0);
							glVertex3f(0, -size, 0);
							glVertex3f(0, -size, -size);
							glVertex3f(0, 0, -size);
						glEnd();
						std::vector<Element*> _neighbours = element->getNeighbours();
						for (Element* e : _neighbours) {
							e->setShowed(true);
						}
					}
				}

				glPopMatrix();
			}
		}

		glPopMatrix();
	}

	bool winFlag = true;
	for (Element& e : _elements) {
		if (e.isBomb() && e.getIsShowed()) {
			winFlag = false;
		}
		if (!e.isBomb() && !e.getIsShowed()) {
			winFlag = false;
		}
	}
	if (winFlag) {
		for (Element& e : _elements) {
			e.setShowed(true);
		}
		_win = true;
		glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
	}

	glPopMatrix();
}


void draw1_3(int number, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glm::vec3 point1_toDraw(point1.x, point1.y, point1.z);
	glm::vec3 point2_toDraw(point2.x, point2.y, point2.z);
	glm::vec3 point3_toDraw(point3.x, point3.y, point3.z * 1/3);
	glm::vec3 point4_toDraw(point4.x, point4.y, point4.z * 1/3);

	switch (number) {
	case 1:
		draw1(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 2:
		draw2(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 3:
		draw3(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 4:
		draw4(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 5:
		draw5(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 6:
		draw6(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 7:
		draw7(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 8:
		draw8(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	}
}

void draw2_3(int number, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glm::vec3 point1_toDraw(point1.x, point1.y, point1.z + (point3.z * 1/3));
	glm::vec3 point2_toDraw(point2.x, point2.y, point2.z + (point4.z * 1/3));
	glm::vec3 point3_toDraw(point3.x, point3.y, point3.z * 2/3);
	glm::vec3 point4_toDraw(point4.x, point4.y, point4.z * 2/3);

	switch (number) {
	case 1:
		draw1(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 2:
		draw2(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 3:
		draw3(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 4:
		draw4(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 5:
		draw5(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 6:
		draw6(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 7:
		draw7(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 8:
		draw8(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	}
}

void draw3_3(int number, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glm::vec3 point1_toDraw(point1.x, point1.y, point1.z + (point3.z * 2 / 3));
	glm::vec3 point2_toDraw(point2.x, point2.y, point2.z + (point4.z * 2 / 3));
	glm::vec3 point3_toDraw(point3.x, point3.y, point3.z);
	glm::vec3 point4_toDraw(point4.x, point4.y, point4.z);

	switch (number) {
	case 1:
		draw1(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 2:
		draw2(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 3:
		draw3(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 4:
		draw4(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 5:
		draw5(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 6:
		draw6(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 7:
		draw7(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	case 8:
		draw8(point1_toDraw, point2_toDraw, point3_toDraw, point4_toDraw);
		break;
	}
}

void drawFull(int number, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	switch (number) {
	case 1:
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_QUADS);
			glVertex3f(point1.x, point1.y, point1.z);
			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f(point3.x, point3.y, point3.z);
			glVertex3f(point4.x, point4.y, point4.z);
		glEnd();
		draw1(point1, point2, point3, point4);
		break;
	case 2:
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
			glVertex3f(point1.x, point1.y, point1.z);
			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f(point3.x, point3.y, point3.z);
			glVertex3f(point4.x, point4.y, point4.z);
		glEnd();
		draw2(point1, point2, point3, point4);
		break;
	case 3:
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
			glVertex3f(point1.x, point1.y, point1.z);
			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f(point3.x, point3.y, point3.z);
			glVertex3f(point4.x, point4.y, point4.z);
		glEnd();
		draw3(point1, point2, point3, point4);
		break;
	case 4:
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
			glVertex3f(point1.x, point1.y, point1.z);
			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f(point3.x, point3.y, point3.z);
			glVertex3f(point4.x, point4.y, point4.z);
		glEnd();
		draw4(point1, point2, point3, point4);
		break;
	case 5:
		glColor3f(0.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
			glVertex3f(point1.x, point1.y, point1.z);
			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f(point3.x, point3.y, point3.z);
			glVertex3f(point4.x, point4.y, point4.z);
		glEnd();
		draw5(point1, point2, point3, point4);
		break;
	case 6:
		glColor3f(1.0f, 0.0f, 1.0f);
		glBegin(GL_QUADS);
			glVertex3f(point1.x, point1.y, point1.z);
			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f(point3.x, point3.y, point3.z);
			glVertex3f(point4.x, point4.y, point4.z);
		glEnd();
		draw6(point1, point2, point3, point4);
		break;
	case 7:
		glColor3f(0.5f, 1.0f, 0.5f);
		glBegin(GL_QUADS);
			glVertex3f(point1.x, point1.y, point1.z);
			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f(point3.x, point3.y, point3.z);
			glVertex3f(point4.x, point4.y, point4.z);
		glEnd();
		draw7(point1, point2, point3, point4);
		break;
	case 8:
		glColor3f(0.5f, 0.5f, 1.0f);
		glBegin(GL_QUADS);
			glVertex3f(point1.x, point1.y, point1.z);
			glVertex3f(point2.x, point2.y, point2.z);
			glVertex3f(point3.x, point3.y, point3.z);
			glVertex3f(point4.x, point4.y, point4.z);
		glEnd();
		draw8(point1, point2, point3, point4);
		break;
	}
}

void draw1(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glColor3f(0.64f, 0.64f, 0.64f);

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1/7, point1.z + (point3.z - point1.z) * 1/2);
		glVertex3f(point1.x + 0.1, point2.y * 5/7, point1.z + (point3.z - point1.z) * 1 / 2);
	glEnd();
}

void draw2(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glColor3f(0.64f, 0.64f, 0.64f);

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3/ 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();
}

void draw3(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glColor3f(0.64f, 0.64f, 0.64f);

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();
}

void draw4(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glColor3f(0.64f, 0.64f, 0.64f);

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 3 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 3 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();
}

void draw5(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glColor3f(0.64f, 0.64f, 0.64f);

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();
}

void draw6(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glColor3f(0.64f, 0.64f, 0.64f);

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();
}

void draw7(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glColor3f(0.64f, 0.64f, 0.64f);

	glColor3f(0.64f, 0.64f, 0.64f);

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();
}

void draw8(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, glm::vec3 point4) {
	glColor3f(0.64f, 0.64f, 0.64f);

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 3 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x + 0.1, point2.y * 1 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
		glVertex3f(point1.x + 0.1, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();

	glBegin(GL_LINES);
		glVertex3f(point1.x, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 1 / 5);
		glVertex3f(point1.x, point2.y * 5 / 7, point1.z + (point3.z - point1.z) * 4 / 5);
	glEnd();
}


void Board::addNeighbours(int i, Element& element) {
	int x = element.xPosition();
	int y = element.yPosition();
	std::string boardPlacement = element.boardPlacement();

	if (x != 0 && x != _size - 1 && y != 0 && y != _size - 1) {
		Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
		Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
		Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
		Element* neighbour4 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
		Element* neighbour5 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
		Element* neighbour6 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
		Element* neighbour7 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
		Element* neighbour8 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
		element.addNeighbour(neighbour1);
		element.addNeighbour(neighbour2);
		element.addNeighbour(neighbour3);
		element.addNeighbour(neighbour4);
		element.addNeighbour(neighbour5);
		element.addNeighbour(neighbour6);
		element.addNeighbour(neighbour7);
		element.addNeighbour(neighbour8);
	}

	if (x == 0 && y == 0) {
		if (boardPlacement == "front") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("left", _size - 1, 0);
			Element* neighbour5 = getElementFromCoordinates("left", _size - 1, 1);
			Element* neighbour6 = getElementFromCoordinates("top", 0, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("top", 1, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "back") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("right", _size - 1, 0);
			Element* neighbour5 = getElementFromCoordinates("right", _size - 1, 1);
			Element* neighbour6 = getElementFromCoordinates("top", _size - 1, 0);
			Element* neighbour7 = getElementFromCoordinates("top", _size - 2, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "top") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("left", 0, 0);
			Element* neighbour5 = getElementFromCoordinates("left", 1, 0);
			Element* neighbour6 = getElementFromCoordinates("back", _size - 2, 0);
			Element* neighbour7 = getElementFromCoordinates("back", _size - 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "bottom") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("left", _size - 1, _size - 1);
			Element* neighbour5 = getElementFromCoordinates("left", _size - 2, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("front", 0, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("front", 1, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "left") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("back", _size - 1, 0);
			Element* neighbour5 = getElementFromCoordinates("back", _size - 1, 1);
			Element* neighbour6 = getElementFromCoordinates("top", 0, 0);
			Element* neighbour7 = getElementFromCoordinates("top", 0, 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "right") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("front", _size - 1, 0);
			Element* neighbour5 = getElementFromCoordinates("front", _size - 1, 1);
			Element* neighbour6 = getElementFromCoordinates("top", _size - 1, _size - 2);
			Element* neighbour7 = getElementFromCoordinates("top", _size - 1, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
	}


	if (x == _size - 1 && y == 0) {
		if (boardPlacement == "front") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("right", 0, 0);
			Element* neighbour5 = getElementFromCoordinates("right", 0, 1);
			Element* neighbour6 = getElementFromCoordinates("top", _size - 1, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("top", _size - 2, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "back") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("left", 0, 0);
			Element* neighbour5 = getElementFromCoordinates("left", 0, 1);
			Element* neighbour6 = getElementFromCoordinates("top", 0, 0);
			Element* neighbour7 = getElementFromCoordinates("top", 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "top") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("right", _size - 1, 0);
			Element* neighbour5 = getElementFromCoordinates("right", _size - 2, 0);
			Element* neighbour6 = getElementFromCoordinates("back", 0, 0);
			Element* neighbour7 = getElementFromCoordinates("back", 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "bottom") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("right", 0, _size - 1);
			Element* neighbour5 = getElementFromCoordinates("right", 1, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("front", _size - 1, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("front", _size - 2, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "left") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("front", 0, 0);
			Element* neighbour5 = getElementFromCoordinates("front", 0, 1);
			Element* neighbour6 = getElementFromCoordinates("top", 0, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("top", 0, _size - 2);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "right") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates("back", 0, 0);
			Element* neighbour5 = getElementFromCoordinates("back", 0, 1);
			Element* neighbour6 = getElementFromCoordinates("top", _size - 1, 0);
			Element* neighbour7 = getElementFromCoordinates("top", _size - 1, 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
	}

	if (x == 0 && y == _size - 1) {
		if (boardPlacement == "front") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("left", _size - 1, _size - 2);
			Element* neighbour5 = getElementFromCoordinates("left", _size - 1, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", 0, 0);
			Element* neighbour7 = getElementFromCoordinates("bottom", 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "back") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("right", _size - 1, _size - 2);
			Element* neighbour5 = getElementFromCoordinates("right", _size - 1, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", _size - 1, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("bottom", _size - 2, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "top") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("left", _size - 2, 0);
			Element* neighbour5 = getElementFromCoordinates("left", _size - 1, 0);
			Element* neighbour6 = getElementFromCoordinates("front", 0, 0);
			Element* neighbour7 = getElementFromCoordinates("front", 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "bottom") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("left", 0, _size - 1);
			Element* neighbour5 = getElementFromCoordinates("left", 1, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("back", _size - 1, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("back", _size - 2, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "left") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("back", _size - 1, _size - 2);
			Element* neighbour5 = getElementFromCoordinates("back", _size - 1, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", 0, _size - 2);
			Element* neighbour7 = getElementFromCoordinates("bottom", 0, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "right") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("front", _size - 1, _size - 2);
			Element* neighbour5 = getElementFromCoordinates("front", _size - 1, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", _size - 1, 0);
			Element* neighbour7 = getElementFromCoordinates("bottom", _size - 1, 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
	}

	if (x == _size - 1 && y == _size - 1) {
		if (boardPlacement == "front") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("right", 0, _size - 2);
			Element* neighbour5 = getElementFromCoordinates("right", 0, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", _size - 2, 0);
			Element* neighbour7 = getElementFromCoordinates("bottom", _size - 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "back") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("left", 0, _size - 2);
			Element* neighbour5 = getElementFromCoordinates("left", 0, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", 0, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("bottom", 1, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "top") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("front", _size - 2, 0);
			Element* neighbour5 = getElementFromCoordinates("front", _size - 1, 0);
			Element* neighbour6 = getElementFromCoordinates("right", 0, 0);
			Element* neighbour7 = getElementFromCoordinates("right", 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "bottom") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("right", _size - 2, _size - 1);
			Element* neighbour5 = getElementFromCoordinates("right", _size - 1, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("back", 0, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("back", 1, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "left") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("front", 0, _size - 2);
			Element* neighbour5 = getElementFromCoordinates("front", 0, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", 0, 0);
			Element* neighbour7 = getElementFromCoordinates("bottom", 0, 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
		if (boardPlacement == "right") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour4 = getElementFromCoordinates("back", 0, _size - 2);
			Element* neighbour5 = getElementFromCoordinates("back", 0, _size - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", _size - 1, _size - 2);
			Element* neighbour7 = getElementFromCoordinates("bottom", _size - 1, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
		}
	}

	if (x != 0 && x != _size - 1 && y == 0) {
		if (boardPlacement == "front") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour6 = getElementFromCoordinates("top", x - 1, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("top", x - 0, _size - 1);
			Element* neighbour8 = getElementFromCoordinates("top", x + 1, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "back") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour6 = getElementFromCoordinates("top", _size - 2 - x, 0);
			Element* neighbour7 = getElementFromCoordinates("top", _size - 1 - x, 0);
			Element* neighbour8 = getElementFromCoordinates("top", _size - 0 - x, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "top") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour6 = getElementFromCoordinates("back", _size - 2 - x, 0);
			Element* neighbour7 = getElementFromCoordinates("back", _size - 1 - x, 0);
			Element* neighbour8 = getElementFromCoordinates("back", _size - 0 - x, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "bottom") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour6 = getElementFromCoordinates("front", x - 1, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("front", x - 0, _size - 1);
			Element* neighbour8 = getElementFromCoordinates("front", x + 1, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "left") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour6 = getElementFromCoordinates("top", 0, x + 1);
			Element* neighbour7 = getElementFromCoordinates("top", 0, x + 0);
			Element* neighbour8 = getElementFromCoordinates("top", 0, x - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "right") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour6 = getElementFromCoordinates("top", _size - 1, _size - 2 - x);
			Element* neighbour7 = getElementFromCoordinates("top", _size - 1, _size - 1 - x);
			Element* neighbour8 = getElementFromCoordinates("top", _size - 1, _size - 0 - x);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
	}

	if (x != 0 && x != _size - 1 && y == _size - 1) {
		if (boardPlacement == "front") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", x - 1, 0);
			Element* neighbour7 = getElementFromCoordinates("bottom", x - 0, 0);
			Element* neighbour8 = getElementFromCoordinates("bottom", x + 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "back") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", _size - 2 - x, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("bottom", _size - 1 - x, _size - 1);
			Element* neighbour8 = getElementFromCoordinates("bottom", _size - 0 - x, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "top") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour6 = getElementFromCoordinates("front", x - 1, 0);
			Element* neighbour7 = getElementFromCoordinates("front", x - 0, 0);
			Element* neighbour8 = getElementFromCoordinates("front", x + 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "bottom") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour6 = getElementFromCoordinates("back", _size - 2 - x, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("back", _size - 1 - x, _size - 1);
			Element* neighbour8 = getElementFromCoordinates("back", _size - 0 - x, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "left") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", 0, _size - 2 - x);
			Element* neighbour7 = getElementFromCoordinates("bottom", 0, _size - 1 - x);
			Element* neighbour8 = getElementFromCoordinates("bottom", 0, _size - 0 - x);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "right") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour6 = getElementFromCoordinates("bottom", _size - 1, x - 1);
			Element* neighbour7 = getElementFromCoordinates("bottom", _size - 1, x - 0);
			Element* neighbour8 = getElementFromCoordinates("bottom", _size - 1, x + 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
	}

	if (y != 0 && y != _size - 1 && x == 0) {
		if (boardPlacement == "front") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("left", _size - 1, y - 1);
			Element* neighbour7 = getElementFromCoordinates("left", _size - 1, y - 0);
			Element* neighbour8 = getElementFromCoordinates("left", _size - 1, y + 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "back") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("right", _size - 1, y - 1);
			Element* neighbour7 = getElementFromCoordinates("right", _size - 1, y - 0);
			Element* neighbour8 = getElementFromCoordinates("right", _size - 1, y + 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "top") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("left", y - 1, 0);
			Element* neighbour7 = getElementFromCoordinates("left", y - 0, 0);
			Element* neighbour8 = getElementFromCoordinates("left", y + 1, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "bottom") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("left", _size - 2 - y, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("left", _size - 1 - y, _size - 1);
			Element* neighbour8 = getElementFromCoordinates("left", _size - 0 - y, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "left") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("back", _size - 1, y - 1);
			Element* neighbour7 = getElementFromCoordinates("back", _size - 1, y - 0);
			Element* neighbour8 = getElementFromCoordinates("back", _size - 1, y + 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "right") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x + 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x + 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x + 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("front", _size - 1, y - 1);
			Element* neighbour7 = getElementFromCoordinates("front", _size - 1, y - 0);
			Element* neighbour8 = getElementFromCoordinates("front", _size - 1, y + 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
	}

	if (y != 0 && y != _size - 1 && x == _size - 1) {
		if (boardPlacement == "front") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("right", 0, y - 1);
			Element* neighbour7 = getElementFromCoordinates("right", 0, y - 0);
			Element* neighbour8 = getElementFromCoordinates("right", 0, y + 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "back") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("left", 0, y - 1);
			Element* neighbour7 = getElementFromCoordinates("left", 0, y - 0);
			Element* neighbour8 = getElementFromCoordinates("left", 0, y + 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "top") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("right", _size - 2 - y, 0);
			Element* neighbour7 = getElementFromCoordinates("right", _size - 1 - y, 0);
			Element* neighbour8 = getElementFromCoordinates("right", _size - 0 - y, 0);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "bottom") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("right", y - 1, _size - 1);
			Element* neighbour7 = getElementFromCoordinates("right", y - 0, _size - 1);
			Element* neighbour8 = getElementFromCoordinates("right", y + 1, _size - 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "left") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("front", 0, y - 1);
			Element* neighbour7 = getElementFromCoordinates("front", 0, y - 0);
			Element* neighbour8 = getElementFromCoordinates("front", 0, y + 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
		if (boardPlacement == "right") {
			Element* neighbour1 = getElementFromCoordinates(boardPlacement, x - 0, y - 1);
			Element* neighbour2 = getElementFromCoordinates(boardPlacement, x - 0, y + 1);
			Element* neighbour3 = getElementFromCoordinates(boardPlacement, x - 1, y - 1);
			Element* neighbour4 = getElementFromCoordinates(boardPlacement, x - 1, y + 1);
			Element* neighbour5 = getElementFromCoordinates(boardPlacement, x - 1, y - 0);
			Element* neighbour6 = getElementFromCoordinates("back", 0, y - 1);
			Element* neighbour7 = getElementFromCoordinates("back", 0, y - 0);
			Element* neighbour8 = getElementFromCoordinates("back", 0, y + 1);
			element.addNeighbour(neighbour1);
			element.addNeighbour(neighbour2);
			element.addNeighbour(neighbour3);
			element.addNeighbour(neighbour4);
			element.addNeighbour(neighbour5);
			element.addNeighbour(neighbour6);
			element.addNeighbour(neighbour7);
			element.addNeighbour(neighbour8);
		}
	}

	element.updateBombsNeighbours();
}
