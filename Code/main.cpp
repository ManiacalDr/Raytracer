/*Uses the Function p = u + vt to determine the color of a pixel after shooting out a ray
ran into problems with pointers pointing to uninitlized memory and trying to set a value there*/
//https://stackoverflow.com/questions/1986378/how-to-set-up-quadratic-equation-for-a-ray-sphere-intersection
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>

// Include GLM
#include <glm/ext.hpp>

using namespace glm;

float d;
int n;
int Xsize; // N*N will give you the size of the image
int Ysize;

const vec3 cam(0.0, 0.0, 1.0);
vec3 backgroundColor(0.0,0.0,0.0);
vec3 ambiant(0.6, 0.6, 0.6);

vec3 u = cam;
vec3 v(0.0, 0.0, 0.0);
vec3 p;

float pw;
float ph;

void print(const vec3* p) {
	printf("Vec3: %f, %f, %f\n", p->r, p->g, p->b);
};

void print(const vec4* p) {
	printf("Vec4: %f, %f, %f, %f\n", p->x, p->y, p->z, p->w);
};

void print(const mat4* Xform) {
	printf("Mat4:\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n", Xform[0][0], Xform[1][0], Xform[2][0], Xform[0][1], Xform[1][1], Xform[2][1], Xform[0][2], Xform[1][2], Xform[2][2]);
};

struct Material {
	vec3 diffuse;
	vec3 specular;
	vec4 refraction;
	float emissivity;

	Material() {
		diffuse = vec3(0.0);
		specular = vec3(0.0);
		refraction = vec4(vec3(0.0), 1.0002962);
		emissivity = 0;
	}

	void set(float dr, float dg, float db, float sr, float sg, float sb, float p) {
		diffuse = vec3(dr,dg,db);
		specular = vec3(sr,sg,sb);
		emissivity = p;
	}

	void refrac(float r, float g, float b, float i) {
		refraction = vec4(r, g, b, i);
	}

	void print(){
		printf("Material: \nDiffuse: %f, %f, %f\nSpecular: %f, %f, %f\nEmissivity: %f\n\n", diffuse.x, diffuse.y, diffuse.z, specular.x, specular.y, specular.z, emissivity);
	}
};

struct Object {
	mat4 Xform;
	mat4 Xfmi;
	Material mat;

	Object(){
		Xform = mat4(1.0);
		Xfmi = mat4(1.0);
	}

	Object(mat4 xform, mat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

	void print() {
		printf("Object: \nXform:\n\t%f, %f, %f\n\t%f, %f, %f\n\t%f, %f, %f\nXfmi: \n\t%f, %f, %f\n\t%f, %f, %f\n\t%f, %f, %f\n", Xform[0][0], Xform[1][0], Xform[2][0], Xform[0][1], Xform[1][1], Xform[2][1], Xform[0][2], Xform[1][2], Xform[2][2],
			Xfmi[0][0], Xfmi[1][0], Xfmi[2][0], Xfmi[0][1], Xfmi[1][1], Xfmi[2][1], Xfmi[0][2], Xfmi[1][2], Xfmi[2][2]);
		mat.print();
	}
};

struct Ray { //Stores data for Ray
	vec4 start;//is the Eye
	vec4 dir;
	Material mat;
	bool isNorm; //used to store if the Ray is normalized, for shadow rays we will not normalize

	Ray(vec3 vec){
		start = vec4(vec,0.0);
		dir = vec4(0.0,0.0,0.0,0.0);
		isNorm = false;
	}

	void normalize() {
		dir = vec4(glm::normalize(vec3(dir)), 0.0);
		isNorm = true;
	}

	void print() {
		printf("Ray:\nStart: %f, %f, %f\nDirection: %f, %f, %f\n", start.x, start.y, start.z, dir.x, dir.y, dir.z);
		mat.print();
		std::cout << isNorm << "\n\n";
	}
};

struct Light { //Stores our light data, its position in world space and the color of light.
	vec3 position;
	vec3 color;

	Light(float r, float g, float b, float x, float y, float z){
		position = vec3(x, y, z);
		color = vec3(r, g, b);
	}

	void print() {
		printf("Light: \nPosition: %f, %f, %f\nColor: %f, %f, %f\n", position.x, position.y, position.z, color.x, color.y, color.z);
	}
};

std::vector<Light> lights;

std::vector<Object> objects;

vec3 PhongIllumination(vec3 point, Ray ray, Light light, Object object) {
	/*Calculate the phong illumination model*/

	//ray.print();
	//object.print();
	//light.print();

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
	return normalize(Color);
};

double* quadratic(double A, double B, double C, double* ret) {//we pass a pointer in so this has some initlized memory in the calling function to set the final value too.

	//std::cout << A << ", " << B << ", " << C << "\n";

	double rad = (B * B) - (4 * A * C);
	double t1;

	//if (rad > 0.0)
	//	std::cout << rad << "\n";

	if (rad <= 0.0)
	{
		if (ret) 
			return NULL;
	}

	double rootrad = sqrt(rad);

	if (B > 0)
		t1 = (-B - rootrad) / (2 * A);
	else
		t1 = (-B + rootrad) / (2 * A);
	double t2 = C / (A*t1);

	*ret = t2;//what was the point of this again?
	return ret;
};

vec3* calcIntersection(Ray *ray, vec3* ret){
	double space;
	double* t = NULL;
	t = quadratic(dot(ray->dir,ray->dir), 2.0 * dot(ray->start,ray->dir), dot(ray->start,ray->start) - 1.0, &space);

	if (!t)
		return NULL;
	vec3 temp;

	temp.x = ray->start.x + *t * ray->dir.x;
	temp.y = ray->start.y + *t * ray->dir.y;
	temp.z = ray->start.z + *t * ray->dir.z;

	*ret = temp;

	return ret;
};

vec3* closestIntersection(Ray *ray, Object *ret) {// return the intersection point, surface normal, surface, surface attributes, etc.
	vec3 space;//some initlized memory to point our pointers to, has to be here because of scope
	vec3* current = NULL;
	vec3* thing = NULL;

	for (const Object &object : objects){
		ray->dir = ray->dir * object.Xform;
		thing = calcIntersection(ray, &space);
		if(thing) {
			if (!current) { //If this is the first time we intersect an object
				current = thing;
				*ret = object;
			}
			else if (thing->x < current->x ) { //If the new intersection is less then the current stored, change the current
				current = thing;
				*ret = object;
				//std::cout << "Got here\n";
			}
		}
	}

	if (!current)
		return NULL;
	
	ray->mat = ret->mat;
	//ray->mat.print();
	return current;
};

vec3 trace(Ray ray) {
	Object object;
	vec3* intersection = closestIntersection(&ray, &object);
	if (intersection)
		return Shade(*intersection, ray, object);
	return backgroundColor;
};

int main() {
	d = 5;
	n = 10;
	Xsize = n;
	Ysize = n;
	
	int count = 1;

	std::string line;
	Object groupobject;
	std::vector<mat4> inversestack;
	while (std::getline(std::cin, line, ' ')) {
		std::cout << line << "\n";
		if ("#" == line);
		else if ("view" == line) {
			std::cin >> n >> d;
			Xsize = n;
			Ysize = n;
		}
		else if ("scale" == line) {
			float x, y, z;
			std::cin >> x >> y >> z;
			groupobject.Xform = scale(groupobject.Xform, vec3(x, y, z));
			inversestack.push_back(inverse(scale(mat4(1.0), vec3(x, y, z))));
		}
		else if ("move" == line){
			float x, y, z;
			std::cin >> x >> y >> z;
			groupobject.Xform = translate(groupobject.Xform, vec3(x, y, z));
			inversestack.push_back(inverse(translate(mat4(1.0), vec3(x, y, z))));
			print(&groupobject.Xform);
			print(&groupobject.Xfmi);
		}
		else if ("rotate" == line){
			float theta, x, y, z;
			std::cin >> theta >> x >> y >> z;
			groupobject.Xform = rotate(groupobject.Xform, theta, vec3(x, y, z));
			inversestack.push_back(inverse(rotate(mat4(1.0), theta, vec3(x, y, z))));
		}
		else if ("sphere" == line) { 
			mat4 temp(1.0);
			for (auto object = inversestack.rbegin(); object != inversestack.rend(); object++) {
				temp = temp * *object;
			}
			groupobject.Xfmi = temp;
			objects.push_back(groupobject); 
			groupobject.print();
		}
		else if ("light" == line) {
			float r, g, b, x, y, z;
			std::cin >> r >> g >> b >> x >> y >> z;
			lights.push_back(Light(r, g, b, x, y, z));
		}
		else if ("background" == line) {
			float r, g, b;
			std::cin >> r >> g >> b;
			backgroundColor = vec3(r, g, b);
		}
		else if ("ambient" == line) {
			float r, g, b;
			std::cin >> r >> g >> b;
			ambiant = vec3(r, g, b);
		}
		else if ("refraction" == line) {
			float r, g, b, i;
			std::cin >> r >> g >> b >> i;
			groupobject.mat.refrac(r, g, b, i);
		}
		else if ("material" == line) {
			float dr, dg, db, sr, sg, sb, p;
			std::cin >> dr >> dg >> db >> sr >> sg >> sb >> p;
			groupobject.mat.set(dr, dg, db, sr, sg, sb, p);
		}
		else if ("group" == line){}
		else if ("groupend" == line){}
		else
			printf("\nError: could not read line %i in file. Line: %s\n", count, line);
		std::getline(std::cin, line, '\n');
		count++;
	}

	vec3 Pixels[n][n];
	pw = 2 * d / float(n);
	ph = 2 * d / float(n);


	Ray current(u);
	//printVec(&u);
	//printVec(&v);
	for (float j = 0; j <= n - 1; j++) {
		v.y = d - ph * j - ph / 2;//calculate midpoint for row
		//std::cout << v.y << "\n";
		for (float i = 0; i <= n - 1; i++) {
			v.x = d - ph * i - ph / 2;//calculate midpoint for column
			//std::cout << v.x << "\n";
			current.dir = vec4(v - u, 0.0);
			current.normalize();
			//print(&current.dir);
			//print(&current.start);
			Pixels[int(i)][int(j)] = trace(current);
		}
	}
	

	unsigned char r, g, b;
	FILE *picfile;
	picfile = fopen("out.ppm", "w");
	fprintf(picfile, "P6\n# %dx%d Raytracer output\n%d %d\n255\n",
		Xsize, Ysize, Xsize, Ysize);
	// For each pixel
	for (int j = Ysize - 1; j >= 0; j--) {     // Y is flipped!
		for (int i = 0; i <= Xsize - 1; i++) {
			r = Pixels[i][j].r * 255;
			g = Pixels[i][j].g * 255;
			b = Pixels[i][j].b * 255;
			fprintf(picfile, "%c%c%c", r, g, b);
			//print(&Pixels[i][j]);
			// Remember though that this is a number between 0 and 255
			// so might have to convert from 0-1.
		}
	}
	fclose(picfile);
	return 0;
}
