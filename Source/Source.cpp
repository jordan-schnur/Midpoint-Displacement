#include <Stormcloud.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\glm.hpp>
#include <GL\glew.h>
#include <vector>
#include <random>
#include <ctime>


struct line {
	glm::vec2 pointA;
	glm::vec2 pointB;
	line(glm::vec2 a, glm::vec2 b) {
		pointA = a;
		pointB = b;
	}
};

struct Camera {
	int camXMin = 0, camXMax = 1024 * 2;
	float xOffset = 0;
	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	/**
	Update the camera based on input. Finally recalculate the Camera(View) matrix based on the X offset.

	@param The window class, kept in Stormcloud. Used to seperate code from the Stormcloud class.
	@return void
	*/
	void update(GLFWwindow* Window) {
		if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS) {
			if (xOffset < camXMin) {
				xOffset += 3;
			}
		}
		else if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS) {
			if (xOffset < camXMax) {
				xOffset -= 3;
			}
		}
		cameraMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, 0.0f, 0.0f));
	}
};


class Game : public Stormcloud {
public:
	std::vector<line> lines;
	GLuint p, vb, va;
	virtual void Load() {
		//Use Stormcloud to complile our program andshaders
		Utils::compileShader("../data/midpoint.vert", GL_VERTEX_SHADER);
		Utils::compileShader("../data/midpoint.frag", GL_FRAGMENT_SHADER);
		Utils::compileProgram(&p);

		//OpenGL code: Create VertexArrays and VertexBuffers. Next bind them.
		glGenVertexArrays(1, &va);
		glGenBuffers(1, &vb);
		glBindBuffer(GL_ARRAY_BUFFER, vb);
		glBufferData(GL_ARRAY_BUFFER, (2*250)*sizeof(float), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		srand(time(NULL));

		//Our first line. The line the algorithm will use.
		lines.push_back(line(glm::vec2(0.0f, 288.0f), glm::vec2(1024.0f*2, 288.0f)));
	}
	/**

	Dynamicly generate a float array from our vector of lines.
	*/
	void addLines() {
		float *verts = new float[lines.size() * 4]; //Dynamicly create a float array with size of lines vector * 4 vertices
		for (int i = 0; i < lines.size(); i++) {
			verts[i * 4] = lines[i].pointA.x;
			verts[(i * 4) + 1] = lines[i].pointA.y;
			verts[(i * 4) + 2] = lines[i].pointB.x;
			verts[(i * 4) + 3] = lines[i].pointB.y;
		}
		glUseProgram(p);
		glBindBuffer(GL_ARRAY_BUFFER, vb);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*(lines.size() * 4), verts, GL_DYNAMIC_DRAW); //Assign the data to the ArrayBuffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		delete verts; //Delete dynamic array for cleanup
	}
	/**

	The acutal midpoint displacement method.
	*/
	void midpointDisplacement(int max, int res) {
		for (int i = 0; i < res; i++) { //
			std::vector<line> tempLines;
			for (int j = 0; j < lines.size(); j++) {
				line l = lines[j];
				float midX = (l.pointA.x + l.pointB.x) / 2; //Midpoint Formula x1+x2/2 = Mid
				float midY = (l.pointA.y + l.pointB.y) / 2;
				float newHeight = rand() % (max * 2);
				newHeight -= max;
				if (midY + newHeight > 568.0f || midY + newHeight < 0) { -newHeight; }
				tempLines.push_back(line(l.pointA, glm::vec2(midX, midY + newHeight))); //Add two new lines updated with the new height.
				tempLines.push_back(line(glm::vec2(midX, midY + newHeight), l.pointB));

			}
			lines.clear();
			max /= 2;
			for (int j = 0; j < tempLines.size(); j++) { lines.push_back(tempLines[j]); }
		}
		addLines();
	}
	/**

		The main game loop. Run from Stormcloud.
	*/
	virtual void MainLoop() {
		Camera cam;
		midpointDisplacement(400, 8);

		while (!glfwWindowShouldClose(Window)) //Main game loop
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cam.update(Window); //Update camera. Deals with movement. 
			/**
			
				OpenGL Code:
				Bind the vertex array and program. Enable the attributes and define them.
				Nexr send the Orthographic matrix and the Camera(View) matrix to the GPU.
				Finally, draw the lines based on the amount of lines * 4 vertices.
			*/
			glBindVertexArray(va);
			glUseProgram(p);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vb);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glUniformMatrix4fv(glGetUniformLocation(p,"ortho"), 1, GL_FALSE, glm::value_ptr(OrthoMatrix * cam.cameraMatrix));
			glDrawArrays(GL_LINES, 0, lines.size()*4);

			//When S is pressed, generate new terrain
			if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS) {
				lines.clear();
				lines.push_back(line(glm::vec2(0.0f, 288.0f), glm::vec2(1024.0f * 2, 288.0f)));

				midpointDisplacement(400, 8); //TODO: Once text rendering is added to stormcloud, add the ability to change these values.
			}
			
			glfwSwapBuffers(Window);
			glfwPollEvents();
		}
		glfwTerminate();
	}
};



int main() {
	Stormcloud* game = Game().Start();
	return 0;
}