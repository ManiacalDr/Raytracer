/*Uses the Function p = u + vt to determine the color of a pixel after shooting out a ray*/
//https://stackoverflow.com/questions/1986378/how-to-set-up-quadratic-equation-for-a-ray-sphere-intersection
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLM
#include <glm/ext.hpp>

using namespace glm;

const float d = 1;
const int N = 2;
const int Xsize = N; // N*N will give you the size of the image
const int Ysize = N;

const vec3 cam(0.0, 0.0, 1.0);
const vec3 backgroundColor(0.0,1.0,0.0);
const vec3 ambiant(0.5, 0.5, 0.5);

vec3 u = cam;
vec3 v(0.0, 0.0, 0.0);
vec3 p;

double* t;

float pw = 2*d / float(N);
float ph = 2*d / float(N);

struct Material {
	vec3 diffuse;
	vec3 specular;
	float emissivity;
};

struct Object {
	mat4 Xform;
	mat4 Xfmi;
	Material material;
};

struct Ray { //Stores data for Ray
	vec3 start;
	vec3 dir;
	Material mat;
	bool isNorm; //used to store if the Ray is normalized, for shadow rays we will not normalize

	Ray(vec3 vec){
		start = vec;
		dir = vec3(0.0,0.0,0.0);
		isNorm = false;
	}

	void normalize() {
		dir = glm::normalize(dir);
		isNorm = true;
	}
};

struct Light { //Stores our light data, its position in world space and the color of light.
	vec3 position;
	vec3 color;
};

Light lights[] = { {{10,10,10},{1.0,1.0,1.0}} };

Object objects[] = { {mat4(1.0), mat4(1.0), {{0.0,1.0,0.0},{1.0,1.0,1.0}, 4}} };

vec3 Pixels[Xsize][Ysize];

void printVec(const vec3* p) {
	printf("vec3: %f, %f, %f\n", p->x, p->y, p->z);
};

void makePPM() {
	FILE *picfile;
	picfile = fopen("out.ppm", "w");
	fprintf(picfile, "P6\n%d %d\n255\n",
		Xsize, Ysize, Xsize, Ysize);
	// For each pixel
	for (int j = Ysize - 1; j >= 0; j--) {     // Y is flipped!
		for (int i = 0; i <= Xsize - 1; i++) {
			fprintf(picfile, "%c%c%c", char((Pixels[i][j]).r * 255), char((Pixels[i][j]).g * 255), char((Pixels[i][j]).b * 255));
			printVec(&Pixels[i][j]);
			// Remember though that this is a number between 0 and 255
			// so might have to convert from 0-1.
		}
	}
	fclose(picfile);
};

vec3 PhongIllumination(vec3 point, Ray ray, Light light, Object object) {
	/*Calculate the phong illumination model*/

	vec4 vertPos4 = object.Xform * vec4(point, 1.0);
	vec3 vertPos = vec3(vertPos4) / vertPos4.w;
	vec3 normalInterp = vec3(object.Xfmi * vec4(point, 0.0));//this should take an arbitrary normal, but for now take the normal of a shpere aka the point you hit it

	vec3 normal = normalize(normalInterp);
	vec3 lightDir = normalize(light.position - vertPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(-vertPos);

	float lambertian = max(dot(lightDir, normal), float(0.0));
	float specular = 0.0;

	if (lambertian > 0.0) {
		float specAngle = max(dot(reflectDir, viewDir), float(0.0));
		specular = pow(specAngle, ray.mat.emissivity);
	}

	return vec3(ambiant + lambertian * ray.mat.diffuse + specular * ray.mat.specular);
};

//bool Shadow(vec3 point, Ray ray, Light light) {
//	/*Check between the object and the light to see if this new ray intersects another object*/
//	if (there is object between)
//		return true;
//	return false;
//};

// Ray reflect(vec3 point, Ray ray){//used to find the color that should be here if it is a reflective object};
// Ray rafraction(vec3 point, Ray ray){//used to find the color that should be here if it is a rafractive object};

vec3 Shade(vec3 point, Ray ray, Object object) {
	/*Used to discover what color a particular pixel should be*/
	vec3 Color = backgroundColor;
	for (const Light &light : lights){
		//if !Shadow(point, ray, light)
			Color += PhongIllumination(point, ray, light, object);
	}
	//if specularMaterial Color += trace(reflect(point, ray));
	//if refractive Color += trace(refraction(point, ray));
	return Color;
};

double* quadratic(double A, double B, double C) {
	double rad = B * B - 4 * A*C;
	double t1;

	if (rad <= 0.0)
		return NULL;

	double rootrad = sqrt(rad);

	if (B > 0)
		t1 = (-B - rootrad) / (2 * A);
	else
		t1 = (-B + rootrad) / (2 * A);

	double temp = C / (A*t1);
	return &temp;//what was the point of this again?
};

vec3* calcIntersection(Ray *ray){
	t = quadratic(dot(ray->dir,ray->dir), 2 * dot(ray->start,ray->dir), dot(ray->start,ray->start) - 1);

	if (t == NULL)
		return NULL;
//std::cout << "Got here\n";
	vec3* temp;

	temp->x = ray->start.x + *t * ray->dir.x;//don
	temp->y = ray->start.y + *t * ray->dir.y;
	temp->z = ray->start.z + *t * ray->dir.z;
	
	return temp;
};

vec3* closestIntersection(Ray *ray, Object *object) {// return the intersection point, surface normal, surface, surface attributes, etc.
	Object closestObject;
	vec3* current = NULL;
	vec3* thing = NULL;

	for (const Object &object : objects){
		//std::cout << "Got here\n";
		thing = calcIntersection(ray);
		//std::cout << "Got here\n";
		if(thing != NULL) {
			if (&current == NULL) { //If this is the first time we intersect an object
				current = thing;
				closestObject = object;
			}
			else if (thing->x < current->x ) { //If the new intersection is less then the current stored, change the current
				current = thing;
				closestObject = object;
			}
		}
	}

	if (thing == NULL || current == NULL)
		return NULL;
	
	object = &closestObject;
	ray->mat = object->material;
	return current;
};

vec3 trace(Ray ray) {
	Object object;
	vec3* intersection = closestIntersection(&ray, &object);
	if (intersection != NULL)
		return Shade(*intersection, ray, object);
	return backgroundColor;
};

void RayCast() {
	Ray current(u);
	//printVec(&u);
	//printVec(&v);
	for (float j = 0; j <= N - 1; j++) {
		v.y = d - ph * j - ph / 2;//calculate midpoint for row
		//std::cout << v.y << "\n";
		for (float i = 0; i <= N - 1; i++) {
			v.x = d - ph * i - ph / 2;//calculate midpoint for column
			//std::cout << v.x << "\n";
			current.dir = v;

			printVec(&current.dir);
			//printVec(&current.start);
			Pixels[int(i)][int(j)] = trace(current);
		}
	}
};

int main() {
	RayCast();
	makePPM();
	return 0;
}
