#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>
#include <cstdlib> // For rand() and srand()
#include <ctime> // For seeding random number generator


using namespace std;

const float DEG2RAD = 3.14159 / 180;



enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

// Define Playable Area Constants
const float LEFT_BOUNDARY = -0.9f;
const float RIGHT_BOUNDARY = 0.9f;
const float TOP_BOUNDARY = 0.9f;
const float BOTTOM_BOUNDARY = -0.7f; // Positioned above the paddle at y = -0.9f

bool spacebarPressed = false;  // Tracks if the spacebar is currently pressed



class Brick
{
public:
	float x, y, width;
	int toughness;
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(float xx, float yy, float ww)
	{
		x = xx;
		y = yy;
		width = ww;
		toughness = rand() % 5 + 1;  // Random toughness between 1 and 5
		brick_type = REFLECTIVE;  // All bricks start as reflective
		onoff = ON;
		setColorBasedOnToughness();  // Set initial color based on toughness
	}

	void setColorBasedOnToughness()
	{
		// Change color based on toughness value
		switch (toughness) {
		case 5: glColor3f(0.0f, 0.0f, 1.0f); break; // Blue for toughness 5
		case 4: glColor3f(0.0f, 1.0f, 0.0f); break; // Green for toughness 4
		case 3: glColor3f(1.0f, 1.0f, 0.0f); break; // Yellow for toughness 3
		case 2: glColor3f(1.0f, 0.5f, 0.0f); break; // Orange for toughness 2
		case 1: glColor3f(1.0f, 0.0f, 0.0f); break; // Red for toughness 1 (destructible)
		}
	}

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			// Set the color before drawing
			setColorBasedOnToughness();

			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}

	void handleCollision()
	{
		if (toughness > 0) {
			toughness--;
			if (toughness == 1) {
				brick_type = DESTRUCTABLE;
			}
			else if (toughness == 0) {
				onoff = OFF;  // Turn off the brick when toughness reaches 0
			}
		}
	}
};

// Function to spawn bricks
void spawnBricks(int numBricks, float brickWidth, std::vector<Brick>& bricks) {
	srand(time(NULL));

	bricks.clear();

	for (int i = 0; i < numBricks; i++) {
		float xPos = LEFT_BOUNDARY + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (RIGHT_BOUNDARY - LEFT_BOUNDARY)));
		float yPos = BOTTOM_BOUNDARY + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (TOP_BOUNDARY - BOTTOM_BOUNDARY)));

		bricks.push_back(Brick(xPos, yPos, brickWidth));
	}
}

class Paddle {
public:
	float red, green, blue;
	float x, y;
	const float width = 0.4f;  // Longer and thinner paddle
	const float height = 0.05f; // Thin paddle height

	Paddle(float xx, float yy, float rr, float gg, float bb) {
		x = xx;
		y = yy;
		red = rr;
		green = gg;
		blue = bb;
	}

	void drawPaddle() {
		double halfWidth = width / 2;
		double halfHeight = height / 2;

		glColor3d(red, green, blue);
		glBegin(GL_POLYGON);

		glVertex2d(x + halfWidth, y + halfHeight);
		glVertex2d(x + halfWidth, y - halfHeight);
		glVertex2d(x - halfWidth, y - halfHeight);
		glVertex2d(x - halfWidth, y + halfHeight);

		glEnd();
	}

	// Update paddle's x position based on mouse movement
	void updatePosition(float mouseX) {
		x = mouseX;

		// Ensure the paddle stays within screen bounds
		if (x + width / 2 > 1.0f) {
			x = 1.0f - width / 2;
		}
		else if (x - width / 2 < -1.0f) {
			x = -1.0f + width / 2;
		}
	}
};

class Circle {
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float dx;  // Velocity in the x direction
	float dy;  // Velocity in the y direction
	float speed;

	Circle(float xx, float yy, float rad, float initialSpeed, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rad;
		speed = initialSpeed;
		red = r;
		green = g;
		blue = b;

		// Set angle to 90 degrees (?/2 radians) for straight up movement
		float angle = 90.0f * DEG2RAD;  // Convert 90 degrees to radians

		// Initialize velocities dx and dy based on angle
		dx = speed * cos(angle);
		dy = speed * sin(angle);
	}

	void CheckCollision(Brick* brk, Paddle& paddle)
	{
		// Check collision with the brick
		if (brk->onoff == ON) {
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) &&
				(y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				brk->handleCollision();  // Decrease toughness and update brick state

				// Reverse the y-velocity to simulate a bounce
				dy = -dy;
				std::cout << "Collision with Brick at (" << brk->x << ", " << brk->y << ") - Toughness: " << brk->toughness << std::endl;
			}
		}

		// Check collision with the paddle
		if ((x > paddle.x - paddle.width / 2 && x < paddle.x + paddle.width / 2) &&
			(y - radius <= paddle.y + paddle.height / 2 && y - radius >= paddle.y - paddle.height / 2))
		{
			// Calculate the hit position relative to the center of the paddle
			float hitPosition = (x - paddle.x) / (paddle.width / 2);  // Normalized value between -1 (left edge) and 1 (right edge)

			// Adjust dx based on where the ball hits the paddle
			dx = hitPosition * speed;

			// Reverse the y-velocity to simulate a bounce off the paddle
			dy = -dy;

			// Adjust the circle's position slightly to avoid getting stuck in the paddle
			y = paddle.y + paddle.height / 2 + radius + 0.01f;

			std::cout << "Collision with Paddle at (" << paddle.x << ", " << paddle.y << ")" << std::endl;
			std::cout << "New dx: " << dx << " after hitting the paddle at position: " << hitPosition << std::endl;
		}
	}

	void CheckCircleCollision(Circle& other)
	{
		// Calculate the distance between the two circles
		float distanceX = other.x - x;
		float distanceY = other.y - y;
		float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

		// Check if the distance is less than the sum of the radii (collision detected)
		if (distance < radius + other.radius)
		{
			// Simple elastic collision: swap velocities
			float tempDx = dx;
			float tempDy = dy;
			dx = other.dx;
			dy = other.dy;
			other.dx = tempDx;
			other.dy = tempDy;

			// Optional: Move the circles apart slightly to avoid sticking
			float overlap = 0.5f * (distance - radius - other.radius);
			x -= overlap * (x - other.x) / distance;
			y -= overlap * (y - other.y) / distance;
			other.x += overlap * (x - other.x) / distance;
			other.y += overlap * (y - other.y) / distance;

			// Change colors of both circles upon collision
			red = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			green = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			blue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

			other.red = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			other.green = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			other.blue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

			std::cout << "Circle collision detected! Colors changed." << std::endl;
		}
	}



	void MoveOneStep(std::vector<Circle>& circles, int index)
	{
		x += dx;
		y += dy;

		// Handle wall collisions
		if (x + radius > 1.0f || x - radius < -1.0f) {  // Left or Right wall
			dx = -dx;  // Bounce back horizontally
		}

		if (y + radius > 1.0f) {  // Top wall
			dy = -dy;  // Reverse vertical direction to bounce back
			y = 1.0f - radius - 0.01f;  // Adjust position slightly below the top boundary
		}

		if (y - radius < -1.0f) {  // Bottom wall
			// Remove the circle if it hits the bottom wall
			circles.erase(circles.begin() + index);
			return;  // Exit the function after deleting the circle
		}

	}


	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};



vector<Circle> world;

void processInput(GLFWwindow* window, Paddle& paddle);

int main(void) {
	std::vector<Brick> bricks;
	int numBricks = 8;  // Initial number of bricks to spawn

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Spawn bricks
	float brickWidth = 0.1f;
	spawnBricks(8, brickWidth, bricks);

	// Create a paddle with a fixed size
	Paddle paddle(0.0f, -0.9f, 0.5f, 0.5f, 0.5f); // Paddle is gray in color

	while (!glfwWindowShouldClose(window)) {
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		// Capture mouse position
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Convert mouse x-coordinate to normalized device coordinates
		float normalizedX = (mouseX / width) * 2.0f - 1.0f;

		// Update the paddle's position
		paddle.updatePosition(normalizedX);

		// Process input (spawning circles)
		processInput(window, paddle);

		// Remove bricks that are turned off
		bricks.erase(std::remove_if(bricks.begin(), bricks.end(),
			[](const Brick& brk) { return brk.onoff == OFF; }), bricks.end());

		// Check if all bricks are destroyed and respawn with more bricks
		if (bricks.empty()) {
			numBricks++;  // Increment the number of bricks to spawn
			spawnBricks(numBricks, brickWidth, bricks);
			std::cout << "All bricks destroyed! Spawning " << numBricks << " new bricks." << std::endl;
		}

		// Move and draw all circles
		for (int i = 0; i < world.size(); i++) {
			for (auto& brick : bricks) {
				world[i].CheckCollision(&brick, paddle);
			}

			// Check for circle-circle collisions
			for (int j = i + 1; j < world.size(); j++) {
				world[i].CheckCircleCollision(world[j]);
			}

			world[i].MoveOneStep(world, i);
			if (i < world.size()) {
				world[i].DrawCircle();
			}
		}

		// Draw all bricks
		for (auto& brick : bricks) {
			brick.drawBrick();
		}

		// Draw the paddle
		paddle.drawPaddle();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window, Paddle& paddle)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Check if spacebar is pressed
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		// Only spawn if the spacebar was not pressed in the previous frame
		if (!spacebarPressed) {
			// Random color components
			float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

			// Create a circle at the paddle's position with a radius of 0.05
			// Speed of 0.03f and initialized dx, dy will handle the movement
			Circle newCircle(paddle.x, paddle.y + paddle.height * 2, 0.05f, 0.01f, r, g, b);
			world.push_back(newCircle);

			// Set spacebar pressed state to true
			spacebarPressed = true;
		}
	}
	else {
		// Reset the spacebar pressed state when it's released
		spacebarPressed = false;
	}
}


