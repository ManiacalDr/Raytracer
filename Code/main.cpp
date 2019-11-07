/*Uses the Function p = u + vt to determine the color of a pixel after shooting out a ray*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLM
#include <glm/ext.hpp>

using namespace glm;

const int d = 1;
const int N = 2;
const int Xsize = N; // N*N will give you the size of the image
const int Ysize = N;

vec3 cam(0.0, 0.0, 1.0);
vec3 backgroundColor(1.0,0.0,0.0);
vec3 ambiant(0.5, 0.5, 0.5);

vec3 u = cam;
vec3 v(0.0,0.0,0.0);
vec3 p;
double t;

float pw = 2*d / N;
float ph = 2*d / N;

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

	Ray(vec3 u){
		start = u;
		dir = v;
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

Light lights[] = { {{10,10,10},{1.0,1.0,1.0}}, };

Object objects[] = { {mat4(1.0), mat4(1.0), {{0.0,1.0,0.0},{1.0,1.0,1.0}, 4}}, };

vec3 Pixels[Xsize][Ysize];

void makePPM() {
	FILE *picfile;
	picfile = fopen("out.ppm", "w");
	fwpintf(fd, "P6\n# %dx%d Raytracer output\n%d %d\n255\n",
		Xsize, Ysize, Xsize, Ysize);
	// For each pixel
	for (int j = Ysize; j >= 0; j--)     // Y is flipped!
		for (int i = 0; i < Xsize; i++) {
			fprintf(fd, "%c%c%c", Pixels[i][j].r * 255, Pixels[i][j].g * 255, Pixels[i][j].b * 255);
			// Remember though that this is a number between 0 and 255
			// so might have to convert from 0-1.
		}
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

	float lambertian = max(dot(lightDir, normal), 0.0);
	float specular = 0.0;

	if (lambertian > 0.0) {
		float specAngle = max(dot(reflectDir, viewDir), float(0.0);
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

double* quadratic(vec3 A, vec3 B, vec3 C) {
	double rad = B * B - 4 * A*C;

	if (rad <= 0.0)
		return NULL;

	rootrad = sqrt(rad);

	if (B > 0)
		return (-b - rootrad) / (2 * a);
	else
		return (-b + rootrad) / (2 * a);

	double t2 = c / (a*t1);//what was the point of this again?
};

vec3* calcIntersection(Ray *ray){
	t = quadratic(ray->dir*ray->dir, 2 * ray->start*ray->dir, ray->start*ray->start - 1);

	if (t == NULL)
		return NULL;

	return ray->start + ray->dir*t;
}

vec3 closestIntersection(Ray *ray, Object *object) {// return the intersection point, surface normal, surface, surface attributes, etc.
	Object closestObject;
	vec3* current = NULL;
	vec3* thing = NULL;

	for (const Object &object : objects){
		thing = calcIntersection(ray);
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

	object = &closestObject;
	return *current;
};

vec3 trace(Ray ray) {
	Object object;
	vec3 intersection = closestIntersection(&ray, &object);
	if (&intersection != NULL)
		return Shade(intersection, ray, object);
	return backgroundColor;
};

void RayCast() {
	Ray current(u);
	for (int dy = 0; dy <= N - 1; dy++) {
		v.y = d - ph * dy - ph / 2;//calculate midpoint for row
		for (int dx = 0; dx <= N - 1; dx++) {
			v.x = d - pw * dx - pw / 2;//calculate midpoint for column
			current.dir = v;
			Pixels[dx][dy] = trace(current);
		}
	}
};

int main() {

	RayCast();
	makePPM();
	return 0;
}
