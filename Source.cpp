#include "glsl.h"
#include <GL\glew.h>
#include <GL\freeglut.h>
#include "soil\SOIL.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

cwc::glShaderManager SM;
cwc::glShader * shader;



//Двумерный вектор
struct Vector2D {
	Vector2D() {
		x = y = 0;
	}

	Vector2D(GLfloat x, GLfloat y) {
		this->x = x;
		this->y = y;
	}
	GLfloat x;
	GLfloat y;
};

//Трехмерный вектор
struct Vector3D {
	Vector3D() {
		x = y = z = 0;
	}

	Vector3D(GLfloat x, GLfloat y, GLfloat z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	GLfloat x;
	GLfloat y;
	GLfloat z;
};

//Точечный источник света
struct PointLight {
	bool active;
	Vector3D position;

	Vector3D ambient;
	Vector3D diffuse;
	Vector3D specular;
} point_light, point_light2;

//Прожектор
struct SpotLight {
	bool active;
	Vector3D position;
	Vector3D direction;

	Vector3D ambient;
	Vector3D diffuse;
	Vector3D specular;

	GLfloat cutoff;
} spot_light;

//Направленный
struct DirectionalLight {
	bool active;
	Vector3D direction;
	//Vector3D position;

	Vector3D ambient;
	Vector3D diffuse;
	Vector3D specular;
} directional_light;

struct Material {
	Vector3D ambient;
	Vector3D diffuse;
	Vector3D specular;
};

//Вершина модели
struct Vertex {
	Vector3D v;
	Vector2D vt;
	Vector3D vn;
};

//Информация об объекте
class Mesh {
public:
 	Mesh() {
		texture = 0;
		size = 1.0f;
	}


	//Загрузка из файла
	void loadModel(const char* fileName) {
		ifstream file;
		file.open(fileName);
		cout << endl << fileName << " | Opened: " << file.is_open() << endl << endl;

		if (file.is_open()) 
		{
			char line[64];
			vector<Vector3D> v_list;
			vector<Vector2D> vt_list;
			vector<Vector3D> vn_list;

			while (file)
			{
				file.getline(line, 64);
				if (strlen(line))
				{
					stringstream ss(line);
					string line_code; ss >> line_code;

					if (line_code == "v")
					{
						Vector3D v;
						GLfloat coord;
						ss >> coord; v.x = coord;
						ss >> coord; v.y = coord;
						ss >> coord; v.z = coord;
						v_list.push_back(v);
					}

					if (line_code == "vt")
					{
						Vector2D vt;
						GLfloat coord;
						ss >> coord; vt.x = coord;
						ss >> coord; vt.y = coord;
						vt_list.push_back(vt);
					}

					if (line_code == "vn")
					{
						Vector3D vn;
						GLfloat coord;
						ss >> coord; vn.x = coord;
						ss >> coord; vn.y = coord;
						ss >> coord; vn.z = coord;
						vn_list.push_back(vn);
					}


					if (line_code == "f")
					{

						GLuint ind;
						for (int i = 0; i < 3; i++)
						{
							Vertex _vertex;
							string s;
							ss >> s;
							replace(s.begin(), s.end(), '/', ' ');
							stringstream ss(s);
							ss >> ind; _vertex.v = v_list[ind - 1];
							ss >> ind; _vertex.vt = vt_list[ind - 1];
							ss >> ind; _vertex.vn = vn_list[ind - 1];
							vertexes.push_back(_vertex);
						}

					}
				}
			}
			file.close();
		}
	}

	void loadTexture(const char* fileName){
		texture = SOIL_load_OGL_texture(fileName, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

		if (texture == 0) return;

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void draw(cwc::glShader *shader) {
		glPushMatrix();

		glRotatef(rotation.x, 1, 0, 0);
		glRotatef(rotation.y, 0, 1, 0);
		glRotatef(rotation.z, 0, 0, 1);

		glTranslatef(position.x, position.y, position.z);

		glRotatef(model_rotation.x, 1, 0, 0);
		glRotatef(model_rotation.y, 0, 1, 0);
		glRotatef(model_rotation.z, 0, 0, 1);
		glScalef(size, size, size);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		shader->setUniform1i("texture_image", 0);

		shader->setUniform3f("material_ambient", material.ambient.x, material.ambient.y, material.ambient.z);
		shader->setUniform3f("material_diffuse", material.diffuse.x, material.diffuse.y, material.diffuse.z);
		shader->setUniform3f("material_specular", material.specular.x, material.specular.y, material.specular.z);

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < vertexes.size(); i++) {
			glNormal3f(vertexes[i].vn.x, vertexes[i].vn.y, vertexes[i].vn.z);
			glTexCoord2f(vertexes[i].vt.x, vertexes[i].vt.y);
			glVertex3f(vertexes[i].v.x, vertexes[i].v.y, vertexes[i].v.z);
		}
		glEnd();

		glDisable(GL_TEXTURE_2D);

		glPopMatrix();
	}

	void setMaterial(Material material) {
		this->material = material;
	}

	void setPosition(Vector3D position) {
		this->position = position;
	}

	void setRotation(Vector3D rotation) {
		this->rotation = rotation;
	}

	void setModelRotation(Vector3D model_rotation) {
		this->model_rotation = model_rotation;
	}

	void setSize(GLfloat size) {
		this->size = size;
	}

	Vector3D getPosition() {
		return position;
	}

	Vector3D getRotation() {
		return rotation;
	}

	Vector3D getModelRotation() {
		return model_rotation;
	}

	GLfloat getSize() {
		return size;
	}

	float orbitAngle = 0, orbitSpeed = 0, orbitDist = 0;

private:
	Vector3D position, rotation, model_rotation;
	GLfloat size;
	Material material;
	vector<Vertex> vertexes;
	GLuint texture;
};

//Список всех объектов
vector<Mesh*> objects;

//Камера
Vector3D camera_position;
Vector3D camera_rotation;

Vector3D OrbitPosition(float angle, float radius, Vector3D origin = Vector3D(0,0,0))
{
	return Vector3D(origin.x + radius * sin(angle / 6.24f), origin.y + 0, origin.z + radius * cos(angle / 6.24f));
}

Vector3D difference(Vector3D v1, Vector3D v2)
{
	return Vector3D(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

//Рендер
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glPushMatrix();
	glRotatef(camera_rotation.x, 1, 0, 0);
	glRotatef(camera_rotation.y, 0, 1, 0);
	glRotatef(camera_rotation.z, 0, 0, 1);
	glTranslatef(camera_position.x, camera_position.y, camera_position.z);

	if (shader) {
		shader->begin() ;

		GLfloat matrix[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
		shader->setUniformMatrix4fv("light_model_matrix", 1, GL_FALSE, &matrix[0]);

		shader->setUniform1i("point_active", point_light.active);
		if (point_light.active) {
			shader->setUniform3f("point_position", point_light.position.x, point_light.position.y, point_light.position.z);
			shader->setUniform3f("point_ambient", point_light.ambient.x, point_light.ambient.y, point_light.ambient.z);
			shader->setUniform3f("point_diffuse", point_light.diffuse.x, point_light.diffuse.y, point_light.diffuse.z);
			shader->setUniform3f("point_specular", point_light.specular.x, point_light.specular.y, point_light.specular.z);
		}

		if (point_light2.active) {
			shader->setUniform3f("point_position", point_light2.position.x, point_light2.position.y, point_light2.position.z);
			shader->setUniform3f("point_ambient", point_light2.ambient.x, point_light2.ambient.y, point_light2.ambient.z);
			shader->setUniform3f("point_diffuse", point_light2.diffuse.x, point_light2.diffuse.y, point_light2.diffuse.z);
			shader->setUniform3f("point_specular", point_light2.specular.x, point_light2.specular.y, point_light2.specular.z);
		}

		shader->setUniform1i("spot_active", spot_light.active);
		if (spot_light.active) {
			shader->setUniform3f("spot_position", spot_light.position.x, spot_light.position.y, spot_light.position.z);
			shader->setUniform3f("spot_direction", spot_light.direction.x, spot_light.direction.y, spot_light.direction.z);
			shader->setUniform3f("spot_ambient", spot_light.ambient.x, spot_light.ambient.y, spot_light.ambient.z);
			shader->setUniform3f("spot_diffuse", spot_light.diffuse.x, spot_light.diffuse.y, spot_light.diffuse.z);
			shader->setUniform3f("spot_specular", spot_light.specular.x, spot_light.specular.y, spot_light.specular.z);
			shader->setUniform1f("spot_cutoff", spot_light.cutoff);
		}

		shader->setUniform1i("directional_active", directional_light.active);
		if (directional_light.active) {
			shader->setUniform3f("directional_direction", directional_light.direction.x, directional_light.direction.y, directional_light.direction.z);
			shader->setUniform3f("directional_ambient", directional_light.ambient.x, directional_light.ambient.y, directional_light.ambient.z);
			shader->setUniform3f("directional_diffuse", directional_light.diffuse.x, directional_light.diffuse.y, directional_light.diffuse.z);
			shader->setUniform3f("directional_specular", directional_light.specular.x, directional_light.specular.y, directional_light.specular.z);

			//shader->setUniform3f("directional_position", directional_light.position.x, directional_light.position.y, directional_light.position.z);
		}

		

		Mesh *teapt = objects.at(2);
		if (teapt) {
			teapt->orbitAngle += teapt->orbitSpeed;
			teapt->setPosition(OrbitPosition(teapt->orbitAngle, teapt->orbitDist));
			Vector3D model_rotation = teapt->getModelRotation();
			model_rotation.y -= 0.05;
			model_rotation.x -= 0.08;
			teapt->setModelRotation(model_rotation);
		}

		Mesh *planet = objects.at(3);
		if (planet) {
			planet->orbitAngle += planet->orbitSpeed;
			planet->setPosition(OrbitPosition(planet->orbitAngle, planet->orbitDist));
			Vector3D model_rotation = planet->getModelRotation();
			planet->setModelRotation(model_rotation);
		}

		Mesh *satelite = objects.at(0);
		if (satelite) {
			satelite->orbitAngle += satelite->orbitSpeed;
			satelite->setPosition(OrbitPosition(satelite->orbitAngle, satelite->orbitDist, teapt->getPosition()));
			point_light2.position = OrbitPosition(satelite->orbitAngle, satelite->orbitDist, teapt->getPosition());


			satelite->setPosition(Vector3D(0,0,0));

			Vector3D model_rotation = satelite->getModelRotation();
			satelite->setModelRotation(model_rotation);
		}

		planet = objects.at(4);
		if (planet) {
			planet->orbitAngle += planet->orbitSpeed;
			planet->setPosition(OrbitPosition(planet->orbitAngle, planet->orbitDist));
		}

		for (int i = 0; i < 20; i++)
		{
			planet = objects.at(5 + i);
			if (planet) {
				planet->orbitAngle += planet->orbitSpeed;
				planet->setPosition(OrbitPosition(planet->orbitAngle, planet->orbitDist));
				Vector3D model_rotation = planet->getModelRotation();
				model_rotation.y -= planet->orbitSpeed;
				model_rotation.x += planet->orbitSpeed;
				planet->setModelRotation(model_rotation);
			}
		}

		for (auto i : objects)
			i->draw(shader);

		shader->end();
	}

	//glColor3f(1.0f, 1.0f, 1.0f);
	glColor3f(1.0f, 0.8f, 0.8f);
	if (point_light.active) {
		glPushMatrix();
		glTranslatef(point_light.position.x, point_light.position.y, point_light.position.z);
		glutSolidSphere(0.25, 10, 10);
		glPopMatrix();
	}

	glColor3f(0, 0.8f, 0.8f);
	if (point_light2.active) {
		glPushMatrix();
		glTranslatef(point_light2.position.x, point_light2.position.y, point_light2.position.z);
		glutSolidSphere(0.02, 10, 10);
		glPopMatrix();
	}

	glColor3f(1.0f, 0.8f, 0.8f);
	if (spot_light.active) {
		glPushMatrix();

		glTranslatef(spot_light.position.x, spot_light.position.y, spot_light.position.z);
		glutSolidSphere(0.1, 10, 10);
		glPopMatrix();
	}

	if (directional_light.active) {
		glPushMatrix();
		glTranslatef(directional_light.direction.x, directional_light.direction.y, directional_light.direction.z);
		//glTranslatef(directional_light.position.x, directional_light.position.y, directional_light.position.z);
		glColor3f(1.0f, 1.0f, 0.0f);
		glutSolidSphere(0.1, 10, 10);
		glPopMatrix();
	}

	glPopMatrix();
	glutSwapBuffers();
	glutPostRedisplay();
}

//Инициалзация
void init()
{
	//glClearColor(0, 0, 0, 0);
	glClearColor(0.005, 0, 0.1 , 0); //белый фон
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	//Создание объектов
	Material material;
	material.ambient = Vector3D(0.1, 0.1, 0.1);
	material.diffuse = Vector3D(1, 0, 1);
	material.specular = Vector3D(1, 1, 1);

	Mesh* object0 = new Mesh();
	object0->loadModel("ball.obj");
	object0->loadTexture("satelite.png");
	object0->setMaterial(material);
	object0->setSize(0.0003f);

	object0->orbitDist = 0.2f;
	object0->orbitSpeed = 0.05f;
	objects.push_back(object0);

	Mesh* object1 = new Mesh();
	object1->loadModel("ball.obj");
	object1->loadTexture("box.png");
	object1->setMaterial(material);
	object1->setSize(0.0005f);
	objects.push_back(object1);

	Mesh* object2 = new Mesh();
	object2->loadModel("teapot.obj");
	object2->loadTexture("marble.png");
	object2->setMaterial(material);
	object2->setSize(0.2f);

	object2->orbitDist = 1.0f;
	object2->orbitSpeed = 0.01f;

	objects.push_back(object2);

	Mesh* object3 = new Mesh();
	object3->loadModel("ball.obj");
	object3->loadTexture("carpet.png");
	object3->setMaterial(material);
	object3->setPosition(Vector3D(-2.5f, 0, 0));
	object3->setSize(0.002f);

	object3->orbitAngle = 300.0f;
	object3->orbitDist = 3.0f;
	object3->orbitSpeed = 0.02f;

	objects.push_back(object3);

	Mesh* object4 = new Mesh();
	object4->loadModel("ball.obj");
	object4->loadTexture("marble.png");
	object4->setMaterial(material);

	object4->setPosition(Vector3D(-2.5f, 0, 0));
	object4->setSize(0.005f);

	object4->orbitAngle = 20.0f;
	object4->orbitDist = 5.5f;
	object4->orbitSpeed = 0.001f;
	objects.push_back(object4);


	Mesh* asteroids = new Mesh[20];
	for (int i = 0; i < 20; i++)
	{
		asteroids[i] = Mesh();
		asteroids[i].loadModel("cube.obj");
		asteroids[i].loadTexture("marble.png");
		asteroids[i].setMaterial(material);

		float rnd = (rand() % 100) / 1000.0f;
		asteroids[i].setSize(rnd);

		rnd *= 360.0f;
		asteroids[i].setRotation(Vector3D(rnd, 2 * rnd, -rnd));

		asteroids[i].orbitAngle = rand() % 360;
		asteroids[i].orbitDist = 1.5f + ((rand() % 1000) / 1000.0f);
		asteroids[i].orbitSpeed = (rand() % 50) / 1000.0f;
		objects.push_back(asteroids + i);
	}

	//Настройка освещения
	point_light.active = true;
	point_light.position = Vector3D(0, 0, 0);
	
	//point_light.ambient = Vector3D(0.3, 0.3, 0.3);
	//point_light.diffuse = Vector3D(1.0, 1.0, 1.0);
	point_light.ambient = Vector3D(1.0, 1.0, 1.0);
	point_light.diffuse = Vector3D(1.0, 1.0, 1.0);
	point_light.specular = Vector3D(1.0, 1.0, 1.0);


	point_light2.active = true;
	point_light2.position = Vector3D(0, 0, 0);

	//point_light.ambient = Vector3D(0.3, 0.3, 0.3);
	//point_light.diffuse = Vector3D(1.0, 1.0, 1.0);
	point_light2.ambient = Vector3D(1.0, 1.0, 1.0);
	point_light2.diffuse = Vector3D(1.0, 1.0, 1.0);
	point_light2.specular = Vector3D(1.0, 1.0, 1.0);

	
	spot_light.active = false;

	spot_light.position = Vector3D(0, 1, 0.3);
	spot_light.direction = Vector3D(0, 0, 1);
	//spot_light.ambient = Vector3D(0.0, 0.0, 0.0);
	//spot_light.diffuse = Vector3D(0.0, 1.0, 0.0);
	spot_light.ambient = Vector3D(0.0, 1.0, 0.0);
	spot_light.diffuse = Vector3D(0.0, 1.0, 1.0);
	spot_light.cutoff = 30;

	directional_light.active = false;
	//directional_light.position = Vector3D(0, 0, -1);
	directional_light.direction = Vector3D(0, 5, 0);
	directional_light.ambient = Vector3D(1, 0, 0);
	//directional_light.diffuse = Vector3D(1.0, 0.0, 0.0);
	directional_light.diffuse = Vector3D(1.0, 0.0, 1.0);

	//Загрузка шейдеров
	shader = SM.loadfromFile("vertexshader.vert", "fragmentshader.frag");
	//shader = SM.loadfromFile("vertex2.shader", "fragment2.shader");
	if (shader == 0)
		std::cout << "Error (shaders are not loaded)\n";
}

void resize(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
		
	glViewport(0, 0, w, h);

	gluPerspective(45, (float)w / (float)h, 1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 5.0f, 0.0, 0.0, -1.0, 0.0f, 1.0f, 0.0f);
}

void specialKey(int key, int x, int y)
{
	int mod = glutGetModifiers();
	switch (key)
	{
	case GLUT_KEY_UP:
		if (mod == GLUT_ACTIVE_SHIFT)
			camera_position.y += 0.05f;
		else if (mod == GLUT_ACTIVE_CTRL)
			camera_position.z -= 0.05f;
		else
			camera_rotation.x += 5.0f;
		break;
	case GLUT_KEY_DOWN:
		if (mod == GLUT_ACTIVE_SHIFT)
			camera_position.y -= 0.05f;
		else if (mod == GLUT_ACTIVE_CTRL)
			camera_position.z += 0.05f;
		else
			camera_rotation.x -= 5.0f;
		break;
	case GLUT_KEY_RIGHT:
		if (mod == GLUT_ACTIVE_SHIFT)
			camera_position.x += 0.05f;
		else
			camera_rotation.y += 5.0f;
		break;
	case GLUT_KEY_LEFT:
		if (mod == GLUT_ACTIVE_SHIFT)
			camera_position.x -= 0.05f;
		else
			camera_rotation.y -= 5.0f;
		break;
	
	case GLUT_KEY_F1:
		point_light.position.x += mod == GLUT_ACTIVE_SHIFT ?  0.05f : -0.05f;
		break;
	case GLUT_KEY_F2:
		point_light.position.y += mod == GLUT_ACTIVE_SHIFT ? 0.05f : -0.05f;
		break;
	case GLUT_KEY_F3:
		point_light.position.z += mod == GLUT_ACTIVE_SHIFT ? 0.05f : -0.05f;
		break;
	
	case GLUT_KEY_F4:
		spot_light.position.x += mod == GLUT_ACTIVE_SHIFT ? 0.05f : -0.05f;
		break;
	case GLUT_KEY_F5:
		spot_light.position.y += mod == GLUT_ACTIVE_SHIFT ? 0.05f : -0.05f;
		break;
	case GLUT_KEY_F6:
		spot_light.position.z += mod == GLUT_ACTIVE_SHIFT ? 0.05f : -0.05f;
		break;

	case GLUT_KEY_F7:
		directional_light.direction.x += mod == GLUT_ACTIVE_SHIFT ? 0.05f : -0.05f;
		break;
	case GLUT_KEY_F8:
		directional_light.direction.y += mod == GLUT_ACTIVE_SHIFT ? 0.05f : -0.05f;
		break;
	case GLUT_KEY_F9:
		directional_light.direction.z += mod == GLUT_ACTIVE_SHIFT ? 0.05f : -0.05f;
		break;
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key)
	{
	case '1': point_light.active = !point_light.active;  break;
	case '2': point_light2.active = !point_light2.active;  break;
	case '3': spot_light.active = !spot_light.active;  break;
	case '4': directional_light.active = !directional_light.active;  break;
	}
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(800, 600);
	glutInitWindowPosition(200, 150);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Individual task 3");
	
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKey);

	init();

	glutMainLoop();

	return 0;
}
