/*  Base code by Alun Evans 2016 LaSalle (aevanss@salleurl.edu) modified by: Conrado Ruiz, Ferran Ruiz 2024*/

// student name: 
//
// Chan, Nyles
// Ermitano, Kate

//include some standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

//include OpenGL libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//include some custom code files
#include "glfunctions.h"	//include all OpenGL stuff
#include "Shader.h"			// class to compile shaders

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace glm;

//global variables to help us do things
int g_ViewportWidth = 512; int g_ViewportHeight = 512; // Default window size, in pixels
double mouse_x, mouse_y;	//variables storing mouse position
const vec3 g_backgroundColor(0.0f, 0.0f, 0.0f); // background colour - a GLM 3-component vector

// uniform location variables
GLuint model_loc, view_loc, projection_loc, normal_loc, texture_loc;
GLuint light_loc, light_intensity_loc, cam_pos_loc, ambient_loc, diffuse_loc, specular_loc, shininess_loc, alpha_loc;

// if using orthographic project, and if using orbital camera settings
bool orthographic = false;
bool orbital = false;
// for debugging purposes

// camera variables
glm::vec3 cameraPos = vec3(0.0f, 5.0f, 10.0f);								// Pos: position of camera
glm::vec3 cameraTarget = vec3(0.0f, 4.0f, -2.0f);							// Center: where you wanna look at in world space
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(cameraTarget, cameraUp));
float cameraSpeed = 0.25f;
// would have created struct, but everything is declared here already, and they aren't used as often as other structs

// fps camera variables
bool firstMouse = true;
float camera_yaw = -90.0f; //yaw
float camera_pitch = 0.0f; //pitch
float lastX = 512.0f / 2.0f;
float lastY = 512.0f / 2.0f;
float fov = 90.0f; //field of view

GLuint g_simpleShader = 0;				// shader identifier
std::vector <GLuint> g_vao;				// vao vector
std::vector <GLuint> g_NumTriangles;	// num triangles vector

std::vector <std::string> objects;		// object vector
std::vector < std::vector < tinyobj::shape_t > > shapesVector; // shapes vector

std::vector <std::string> textures;		// textures vector
std::vector <GLuint> texture_ids;		// texture id vector

// for setting for loops and vector sizes
GLuint objCount;						// objects.size(), but to be called later
GLuint texCount;						// textures.size(), but to be called later

// light
glm::vec3 g_light(10.0f, 10.0f, 10.0f);
GLfloat light_intensity = 1.0f;

// variables to transform objects
GLfloat earth_rot = 0.0f;
GLfloat moon_rot = 0.0f;
GLfloat earthX = 0.0f;
GLfloat earthZ = 0.0f;
GLfloat ring_rot = 90.0f;
GLfloat coin_rot = 90.0f;
GLfloat hex_rot = 90.0f;
GLfloat saturn_rot = 90.0f;

// skybox settings
//GLuint g_simpleShader_sky = 0;			// skybox shader identifier
//GLuint g_vao_sky = 0;					// skybox vao
//GLuint g_NumTriangles_sky = 0;			// num tris skybox
//GLuint texture_id_sky = 0;				// global texture id
//GLuint model_loc_sky, view_loc_sky, projection_loc_sky;
// skybox settings no longer needed as it is stored in index 0 of vectors

// matrices for projection, view, and model
// model matrices stored in vector for mesh parenting
mat4 view_matrix, projection_matrix;
std::vector <mat4> models;

struct TransformationValues {
	// struct for transformation values
	glm::vec3 translation, rotation, scale;

	// first constructor accepts 9 float values for translate's, rotate's, scale's xyz values
	TransformationValues(float tx, float ty, float tz, float rx, float ry, float rz, float sx, float sy, float sz) : translation(tx, ty, tz), rotation(rx, ry, rz), scale(sx, sy, sz) {}

	// second constructor accepts 3 vec3 values for translate, rotate, scale vec3 values
	TransformationValues(glm::vec3 t, glm::vec3 r, glm::vec3 s) : translation(t), rotation(r), scale(s) {}
};

struct MaterialProperties {
	// struct for material properties
	glm::vec3 ambient, diffuse, specular;
	GLfloat shininess, alpha;

	// constructor accepts 3 vec3 values for ambient, diffuse, specular, then 2 Glfloat values for shininess and alpha
	MaterialProperties(glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, GLfloat shiny, GLfloat transparency) : ambient(ambi), diffuse(diff), specular(spec), shininess(shiny), alpha(transparency) {}
};

// ------------------------------------------------------------------------------------------
// Initialization of scene
// ------------------------------------------------------------------------------------------
void load()
{

	// optimized version

	// load skybox shader
	//Shader simpleShaderSky("src/shader_sky.vert", "src/shader_sky.frag");
	//g_simpleShader_sky = simpleShaderSky.program;
	// although regular shader file was redesigned to not need this

	//load regular shader
	Shader simpleShader("src/shader.vert", "src/shader.frag");
	g_simpleShader = simpleShader.program;

	// put obj file paths into a vector
	objects.push_back("assets/sphere.obj");
	objects.push_back("assets/Thread.obj");
	objects.push_back("assets/sphere.obj");
	objects.push_back("assets/sphere.obj");
	objects.push_back("assets/sphere.obj");
	objects.push_back("assets/Stellar.obj");
	objects.push_back("assets/Cloud.obj");
	objects.push_back("assets/Star.obj");
	objects.push_back("assets/Crescent.obj");
	objects.push_back("assets/Icosahedron.obj");
	objects.push_back("assets/Coin.obj");
	objects.push_back("assets/Tetrahedron.obj");
	objects.push_back("assets/Octahedron.obj");
	objects.push_back("assets/Heart.obj");
	objects.push_back("assets/Hex.obj");
	objects.push_back("assets/AmongUs.obj");
	objects.push_back("assets/plane.obj");

	objCount = objects.size();
	models.resize(objCount);
	g_vao.resize(objCount);
	g_NumTriangles.resize(objCount);

	// shapes vector getting its size based on number of objects
	for (int i = 0; i < objCount; i++)
		shapesVector.emplace_back();

	std::vector <bool> ret(objCount);
	for (int i = 0; i < objCount; i++) {
		ret[i] = tinyobj::LoadObj(shapesVector[i], objects[i].c_str());

		if (ret[i]) {
			cout << "OBJ File: " << objects[i] << " successfully loaded!\n";
		}
		else {
			cout << "OBJ File: " << objects[i] << " cannot be found or is not valid OBJ file.\n";
		}
	}

	for (int i = 0; i < objCount; i++) {
		GLuint chosen_shader = g_simpleShader;
		g_vao[i] = gl_createAndBindVAO();
		std::cout << "vao: " << g_vao[i] << "\n";

		// for skybox settings, they use regular shaders, but a component will indicate it is a skybox

		gl_createAndBindAttribute(&(shapesVector[i][0].mesh.positions[0]),
			shapesVector[i][0].mesh.positions.size() * sizeof(float),
			chosen_shader, "a_vertex", 3
		);

		gl_createIndexBuffer(&(shapesVector[i][0].mesh.indices[0]),
			shapesVector[i][0].mesh.indices.size() * sizeof(unsigned int)
		);

		gl_createAndBindAttribute(&(shapesVector[i][0].mesh.texcoords[0]),
			shapesVector[i][0].mesh.texcoords.size() * sizeof(float),
			chosen_shader, "a_uv", 2
		);

		gl_createAndBindAttribute(&(shapesVector[i][0].mesh.normals[0]),
			shapesVector[i][0].mesh.normals.size() * sizeof(float),
			chosen_shader, "a_normal", 3
		);

		gl_unbindVAO();

		//store number of triangles (use in draw())
		g_NumTriangles[i] = shapesVector[i][0].mesh.indices.size() / 3;
	}
	
	// put texture file paths into a vector
	textures.push_back("textures/milkyway.bmp");
	textures.push_back("textures/Thread.png");
	textures.push_back("textures/earth.bmp");
	textures.push_back("textures/moon.bmp");
	textures.push_back("textures/saturn.jpg");
	textures.push_back("textures/Stellar.png");
	textures.push_back("textures/Cloud.png");
	textures.push_back("textures/Star.png");
	textures.push_back("textures/Crescent.png");
	textures.push_back("textures/Icosahedron.png");
	textures.push_back("textures/Coin.png");
	textures.push_back("textures/Tetrahedron.png");
	textures.push_back("textures/Octahedron.png");
	textures.push_back("textures/Heart.png");
	textures.push_back("textures/Hex.png");
	textures.push_back("textures/AmongUs.jpg");
	textures.push_back("textures/rings.png");

	texCount = textures.size();
	texture_ids.resize(texCount);

	for (int i = 0; i < texCount; i++) {
		int width, height, numChannels;

		stbi_set_flip_vertically_on_load(true); // remove if texture is flipped
		unsigned char* pixels = stbi_load(textures[i].c_str(), &width, &height, &numChannels, 0);
		glGenTextures(1, &texture_ids[i]);
		glBindTexture(GL_TEXTURE_2D, texture_ids[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if (pixels) {
			// if-else statement created to not run into error when numChannels is different (RGB, RGBA)
			if (numChannels == 4) {
				glTexImage2D(
					GL_TEXTURE_2D, // target
					0, // level = 0 base, no mipmap
					GL_RGBA, // how the data will be stored (Grayscale, RGB, RGBA)
					width, //width of the image
					height, //height of the image
					0, // border
					GL_RGBA, // format of original data
					GL_UNSIGNED_BYTE, // type of data
					pixels
				);
				std::cout << "Successfully loaded: Texture " << textures[i].c_str() << " with a width of " << width << ", a height of " << height << ", and uses 4 channels." << std::endl;
			}
			else if (numChannels == 3) {
				glTexImage2D(
					GL_TEXTURE_2D, // target
					0, // level = 0 base, no mipmap
					GL_RGB, // how the data will be stored (Grayscale, RGB, RGBA)
					width, //width of the image
					height, //height of the image
					0, // border
					GL_RGB, // format of original data
					GL_UNSIGNED_BYTE, // type of data
					pixels
				);
				std::cout << "Successfully loaded: Texture " << textures[i].c_str() << " with a width of " << width << ", a height of " << height << ", and uses 3 channels." << std::endl;
			}
			else {
				std::cout << "Failed to load: Texture " << textures[i].c_str() << " with a width of " << width << ", a height of " << height << ", and uses " << numChannels << " channels. (error: channel)" << std::endl;
			}

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "Failed to load: Texture: " << textures[i].c_str() << " with a width of " << width << ", a height of " << height << ", and uses " << numChannels << " channels. (error: pixels)" << std::endl;
		}
		stbi_image_free(pixels);

		texture_loc = glGetUniformLocation(g_simpleShader, "u_texture");
		glUniform1i(texture_loc, i);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture_ids[i]);
	}

}

void renderObject(int index, mat4 model_parent, TransformationValues transform, MaterialProperties material);
// ^ to tell the program the this functions exists below

// ------------------------------------------------------------------------------------------
// This function actually draws to screen and called non-stop, in a loop
// ------------------------------------------------------------------------------------------
void draw()
{
	glClearColor(g_backgroundColor.x, g_backgroundColor.y, g_backgroundColor.z, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// camera settings
	// 
	// remove orthographic and orbital if there's time
	// but both are also useful during testing and debugging...

	float radius = 5.0f;
	float camX = sin(glfwGetTime()) * radius;
	float camZ = cos(glfwGetTime()) * radius;

	if (!orbital) {
		view_matrix = glm::lookAt(
			cameraPos,		//eye, where the camera is
			cameraPos+cameraTarget,	//center, where the camera is looking at
			cameraUp		//up, roll of pitch-yaw-roll
		);

		view_loc = glGetUniformLocation(g_simpleShader, "u_view");
		glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}
	else {
		view_matrix = glm::lookAt(
			glm::vec3(camX, 4.0f, camZ), //camX, 0, camZ
			glm::vec3(0.0f, 4.0f, 0.0f), //0,0,0
			glm::vec3(0.0f, 1.0f, 0.0f)  //0,1,0
		);

		view_loc = glGetUniformLocation(g_simpleShader, "u_view");
		glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}

	if (!orthographic) {

		projection_loc = glGetUniformLocation(g_simpleShader, "u_projection");
		projection_matrix = perspective(
			fov, // field of view
			1.0f, // aspect ratio 1:1
			0.1f, // near plane (distance from camera), very low number
			50.0f // far plane (distance from camera), relatively big but not too big
		);
		// on top of other code so that all vao have same projection

		glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	}
	else {
		projection_loc = glGetUniformLocation(g_simpleShader, "u_projection");
		projection_matrix = ortho(
			-5.0f,
			5.0f,
			-5.0f,
			5.0f,
			-10.0f, // near plane
			10.0f // far plane
		);
		glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	}

	// skybox settings activated first

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glUseProgram(g_simpleShader);

	// skybox functions
	// skybox is index 0 for models[], texture_ids[], g_vao[], g_NumTriangles[]

	model_loc = glGetUniformLocation(g_simpleShader, "u_model");
	texture_loc = glGetUniformLocation(g_simpleShader, "u_texture");
	alpha_loc = glGetUniformLocation(g_simpleShader, "u_alpha");

	models[0] = translate(mat4(1.0f), cameraPos);

	// send values to shader
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(models[0]));
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	glUniform1i(texture_loc, 0);
	glUniform1f(alpha_loc, -1.0f);			// when using g_simpleShader (not g_simpleShader_sky), alpha = -1.0f signifies skybox settings
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_ids[0]);
	gl_bindVAO(g_vao[0]);
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles[0], GL_UNSIGNED_INT, 0);



	// skybox setup done, now drawing other objects

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_2D);

	// activate shader
	// glUseProgram(g_simpleShader);
	// no need to reactivate because merged shader programs already
	
	// funtion takes the following parameters:
	// index,
	// model_parent,
	// TransformationValues (translate, rotate, scale),
	// MaterialProperties (ambient, diffuse, specular, shininess, alpha)

	// index 0 not here because index 0 is skybox

	// thread
	renderObject(1,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 6.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.05f, 0.7f, 0.05f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			100.0f,
			1.0f
		)
	);
	
	// earth
	renderObject(2,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(earthX, 0.0f, earthZ),
			glm::vec3(0.0f, earth_rot, 0.0f),
			glm::vec3(0.5f, 0.5f, 0.5f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			5.0f,
			1.0f
		)
	);
	
	// moon
	renderObject(3,
		models[2],					// parent model: earth
		TransformationValues(
			glm::vec3(0.0f, 0.0f, -3.0f),
			glm::vec3(0.0f, moon_rot, 0.0f),
			glm::vec3(0.125f, 0.125f, 0.125f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			10.0f,
			1.0f
		)
	);

	// saturn
	renderObject(4,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 8.0f, 0.0f),
			glm::vec3(0.0f, saturn_rot, 0.0f),
			glm::vec3(0.4f, 0.4f, 0.4f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			20.0f,
			1.0f
		)
	);

	// stellar (stellated dodecahedron)
	renderObject(5,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.15f, 0.15f, 0.15f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			20.0f,
			1.0f
		)
	);

	// cloud
	renderObject(6,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 2.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.125f, 0.125f, 0.125f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			20.0f,
			1.0f
		)
	);

	// star
	renderObject(7,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 3.0f, 0.0f),
			glm::vec3(90.0f, 0.0f, 0.0f),
			glm::vec3(0.5f, 0.5f, 0.5f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			1.0f,
			1.0f
		)
	);

	// crescent
	renderObject(8,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 4.0f, 0.0f),
			glm::vec3(90.0f, 90.0f, 0.0f),
			glm::vec3(0.5f, 0.5f, 0.5f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			10.0f,
			1.0f
		)
	);

	// icosahedron
	renderObject(9,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 5.0f, 0.0f),
			glm::vec3(90.0f, 0.0f, 0.0f),
			glm::vec3(0.2f, 0.2f, 0.2f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			10.0f,
			1.0f
		)
	);

	// coin
	renderObject(10,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 6.0f, 0.0f),
			glm::vec3(90.0f, 0.0f, coin_rot),
			glm::vec3(0.5f, 0.5f, 0.5f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			10.0f,
			1.0f
		)
	);

	// tetrahedron
	renderObject(11,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 7.0f, 0.0f),
			glm::vec3(90.0f, 90.0f, 90.0f),
			glm::vec3(0.5f, 0.5f, 0.5f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			10.0f,
			1.0f
		)
	);

	// octahedron
	renderObject(12,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 9.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.2f, 0.2f, 0.2f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			100.0f,
			1.0f
		)
	);

	// heart
	renderObject(13,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 10.0f, 0.0f),
			glm::vec3(90.0f, 0.0f, 0.0f),
			glm::vec3(0.2f, 0.2f, 0.2f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			20.0f,
			1.0f
		)
	);

	// hex
	renderObject(14,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(0.0f, 11.0f, 0.0f),
			glm::vec3(90.0f, hex_rot, 0.0f),
			glm::vec3(0.25f, 0.25f, 0.25f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			25.0f,
			1.0f
		)
	);

	// among us
	renderObject(15,
		mat4(0.0f),
		TransformationValues(
			glm::vec3(3.0f, 6.5f, 1.0f),
			glm::vec3(0.5f, 1.0f, 0.0f),
			glm::vec3(0.008f, 0.008f, 0.008f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			20.0f,
			1.0f
		)
	);

	// settings for alpha map usage

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	// rings
	renderObject(16,
		models[4],					// parent model: saturn
		TransformationValues(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(ring_rot, 0.0f, 0.0f),
			glm::vec3(4.0f, 4.0f, 4.0f)
		),
		MaterialProperties(
			glm::vec3(0.1f, 0.1f, 0.1f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			20.0f,
			-1.0f					// use -1.0f for alpha maps (just like skybox)
		)
	);
	
	// object animations below
	ring_rot += 0.005;
	coin_rot += 0.01;
	hex_rot += 0.025;
	saturn_rot += 0.025;
	earth_rot += 0.01;

	// fps camera updates with code below

	cameraTarget = glm::normalize(
		vec3(
			cos(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch)),
			sin(glm::radians(camera_pitch)),
			sin(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch))
		)
	);

	cameraRight = glm::normalize(glm::cross(cameraTarget, cameraUp));

	cameraTarget = glm::normalize(
		vec3(
			cos(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch)),
			sin(glm::radians(camera_pitch)),
			sin(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch))
		)
	);

	cameraRight = glm::normalize(glm::cross(cameraTarget, cameraUp));



}

// ------------------------------------------------------------------------------------------
// This function is called to render an object to screen
// ------------------------------------------------------------------------------------------
void renderObject(int index, mat4 model_parent, TransformationValues transform, MaterialProperties material)
{
	// activate shader
	glUseProgram(g_simpleShader);

	// get uniform locations
	texture_loc = glGetUniformLocation(g_simpleShader, "u_texture");
	light_loc = glGetUniformLocation(g_simpleShader, "u_light");
	light_intensity_loc = glGetUniformLocation(g_simpleShader, "u_light_intensity");
	cam_pos_loc = glGetUniformLocation(g_simpleShader, "u_cam_pos");
	ambient_loc = glGetUniformLocation(g_simpleShader, "u_ambient");
	diffuse_loc = glGetUniformLocation(g_simpleShader, "u_diffuse");
	specular_loc = glGetUniformLocation(g_simpleShader, "u_specular");
	shininess_loc = glGetUniformLocation(g_simpleShader, "u_shininess");
	alpha_loc = glGetUniformLocation(g_simpleShader, "u_alpha");
	model_loc = glGetUniformLocation(g_simpleShader, "u_model");
	normal_loc = glGetUniformLocation(g_simpleShader, "a_normal");

	// bind vao
	gl_bindVAO(g_vao[index]);

	// render textures
	glUniform1i(texture_loc, index);
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, texture_ids[index]);

	// object transformations
	models[index] = translate(mat4(1.0f), vec3(transform.translation.x, transform.translation.y, transform.translation.z)) *
		rotate(mat4(1.0f), transform.rotation.x, vec3(1.0f, 0.0f, 0.0f)) *
		rotate(mat4(1.0f), transform.rotation.y, vec3(0.0f, 1.0f, 0.0f)) *
		rotate(mat4(1.0f), transform.rotation.z, vec3(0.0f, 0.0f, 1.0f)) *
		scale(mat4(1.0f), vec3(transform.scale.x, transform.scale.y, transform.scale.z));

	// perform parenting if model has one
	if (model_parent != mat4(0.0f)) {
		models[index] = model_parent * models[index];
	}

	// send transformations and normals to shader
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(models[index]));
	glm::mat4 normal_matrix = glm::transpose(glm::inverse(models[index]));
	glUniformMatrix4fv(normal_loc, 1, GL_FALSE, glm::value_ptr(normal_matrix));

	// send material properties to shader
	glUniform3f(light_loc, g_light.x, g_light.y, g_light.z);
	glUniform1f(light_intensity_loc, light_intensity);
	glUniform3f(cam_pos_loc, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(ambient_loc, material.ambient.x, material.ambient.y, material.ambient.z);
	glUniform3f(diffuse_loc, material.diffuse.x, material.diffuse.y, material.diffuse.z);
	glUniform3f(specular_loc, material.specular.x, material.specular.y, material.specular.z);
	glUniform1f(shininess_loc, material.shininess);
	glUniform1f(alpha_loc, material.alpha);

	// draw to screen!
	glDrawElements(GL_TRIANGLES, 3 * g_NumTriangles[index], GL_UNSIGNED_INT, 0);

}

// ------------------------------------------------------------------------------------------
// This function is called every time you press a screen
// ------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//quit
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);
	//reload
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		load();
	if (key == GLFW_KEY_W && action == GLFW_PRESS && !orbital) {
		cameraPos += cameraTarget * cameraSpeed;
		cout << "pressed w button, moving camera forward" << endl;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS && !orbital) {
		cameraPos -= cameraTarget * cameraSpeed;
		cout << "pressed s button, moving camera backward" << endl;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS && !orbital) {
		cameraPos += cameraRight * cameraSpeed;
		cout << "pressed d button, moving camera to the right" << endl;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS && !orbital) {
		cameraPos -= cameraRight * cameraSpeed;
		cout << "pressed a button, moving camera to the left" << endl;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		g_light.x += 1.0f;
		cout << "pressed right button, g_light.x = " << g_light.x << endl;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		g_light.x -= 1.0f;
		cout << "pressed left button, g_light.x = " << g_light.x << endl;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		g_light.z += 1.0f;
		cout << "pressed down button, g_light.z = " << g_light.z << endl;
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		g_light.z -= 1.0f;
		cout << "pressed up button, g_light.z = " << g_light.z << endl;
	}
	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
		g_light.y += 1.0f;
		cout << "pressed right bracket button, g_light.y = " << g_light.y << endl;
	}
	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
		g_light.y -= 1.0f;
		cout << "pressed left bracket button, g_light.y = " << g_light.y << endl;
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS) {
		light_intensity -= 1.0f;
		cout << "pressed n button, light intensity decreases" << endl;
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		light_intensity += 1.0f;
		cout << "pressed m button, light intensity increases" << endl;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		//toggle persp / ortho
		if (orthographic) {
			orthographic = false;
		}
		else {
			orthographic = true;
		}
		cout << "pressed p button, orthographic = " << orthographic << endl;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS && !orbital) {
		cameraPos += cameraUp * cameraSpeed;
		cout << "pressed q button, moving camera upward" << endl;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS && !orbital) {
		cameraPos -= cameraUp * cameraSpeed;
		cout << "pressed z button, moving camera downward" << endl;
	}
	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		cout << "pressed t button, glfwGetTime() = " << glfwGetTime() << endl;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		if (orbital) {
			orbital = false;
		}
		else {
			orbital = true;
		}
		cout << "pressed o button, orbital = " << orbital << endl;
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		earthZ += -0.25f;
		cout << "pressed i button, earthZ = " << earthZ << endl;
	}
	if (key == GLFW_KEY_K && action == GLFW_PRESS) {
		earthZ += 0.25f;
		cout << "pressed k button, earthZ = " << earthZ << endl;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		earthX += -0.25f;
		cout << "pressed j button, earthX = " << earthX << endl;
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		earthX += 0.25f;
		cout << "pressed l button, earthX = " << earthX << endl;
	}
}

// ------------------------------------------------------------------------------------------
// This function is called every time you click the mouse
// ------------------------------------------------------------------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        cout << "Left mouse down at" << mouse_x << ", " << mouse_y << endl;
		//xAxis = (mouse_x - 255) / 256.0f;
		//yAxis = -(mouse_y - 255) / 256.0f;
		// ^ old code about clicking to screen to move objects to that position
    }
}

// ------------------------------------------------------------------------------------------
// This function is called every time the screen detects the cursor's presence
// ------------------------------------------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (!orbital) {

		lastX = 256;
		lastY = 256;

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		xoffset *= 0.001f;
		yoffset *= 0.001f;

		camera_yaw += xoffset;
		camera_pitch += yoffset;

		if (camera_pitch > 89.0f)
			camera_pitch = 89.0f;
		if (camera_pitch < -89.0f)
			camera_pitch = -89.0f;

		cameraTarget = glm::normalize(
			vec3(
				cos(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch)),
				sin(glm::radians(camera_pitch)),
				sin(glm::radians(camera_yaw)) * cos(glm::radians(camera_pitch))
			)
		);
	}
}

// ------------------------------------------------------------------------------------------
// This function is called every time the mouse is scrolled
// ------------------------------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= (float)yoffset;
	if (fov < 20.0f)
		fov = 20.0f;
	if (fov > 160.0f)
		fov = 160.0f;
	cout << "fov = " << fov << endl;
}

int main(void)
{
	//setup window and other stuff, defined in glfunctions.cpp
	GLFWwindow* window;
	if (!glfwInit())return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(g_ViewportWidth, g_ViewportHeight, "Hello OpenGL!", NULL, NULL);
	if (!window) {glfwTerminate();	return -1;}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

	//input callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glClearColor(g_backgroundColor.x, g_backgroundColor.y, g_backgroundColor.z, 1.0f);

	//load all the resources
	load();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
		draw();

        // Swap front and back buffers
        glfwSwapBuffers(window);
        
        // Poll for and process events
        glfwPollEvents();
        
        //mouse position must be tracked constantly (callbacks do not give accurate delta)
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
    }

    //terminate glfw and exit
    glfwTerminate();
    return 0;
}


