/*Uses the Function p = u + vt to determine the color of a pixel after shooting out a ray
ran into problems with pointers pointing to uninitlized memory and trying to set a value there*/
//https://stackoverflow.com/questions/1986378/how-to-set-up-quadratic-equation-for-a-ray-sphere-intersection
//https://stackoverflow.com/questions/20028396/how-could-declaring-a-large-2d-array-crash-a-program
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
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
vec4 defaultrefraction = vec4(vec3(0.0), 1.0002962);

vec3 u = cam;
vec3 v(0.0, 0.0, 0.0);
vec3 p;

float pw;
float ph;

struct Material {
	vec3 diffuse;
	vec3 specular;
	vec4 refraction;
	float emissivity;

	Material() {
		diffuse = vec3(double(0.0));
		specular = vec3(double(0.0));
		refraction = vec4(double(0.0));
		emissivity = 0;
	}

	void set(double dr, double dg, double db, double sr, double sg, double sb, double p) {
		diffuse = vec3(dr, dg, db);
		specular = vec3(sr, sg, sb);
		emissivity = p;
	}

	void refrac(double r, double g, double b, double i) {
		refraction = vec4(r, g, b, i);
	}

	void print() {
		printf("Material: \nDiffuse: %f, %f, %f\nSpecular: %f, %f, %f\nEmissivity: %f\n\n", diffuse.x, diffuse.y, diffuse.z, specular.x, specular.y, specular.z, emissivity);
	}
};

struct Object {
	mat4 Xform;
	mat4 Xfmi;
	Material mat;

	Object() {
		Xform = mat4(1.0);
		Xfmi = mat4(1.0);
	}

	Object(mat4 xform, mat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

	Object(mat4 *xform, mat4 *xfmi, Material *m) {
		Xform = *xform;
		Xfmi = *xfmi;
		mat = *m;
	}

	void print() {
		printf("Object: \nXform:\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\nXfmi: \n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n", Xform[0][0], Xform[1][0], Xform[2][0], Xform[3][0], Xform[0][1], Xform[1][1], Xform[2][1], Xform[3][1], Xform[0][2], Xform[1][2], Xform[2][2], Xform[3][2], Xform[0][3], Xform[1][3], Xform[2][3], Xform[3][3]);
		mat.print();
	}
};

struct Ray { //Stores data for Ray
	vec4 start;//is the Eye
	vec4 dir;
	Material mat;//does this make sense? should the Ray store full material data or just some color information?

	Ray(vec3 vec) {
		start = vec4(vec, 1.0);
		dir = vec4(0.0, 0.0, 0.0, 0.0);
	}

	Ray(vec3 s, vec3 d) {
		start = vec4(s, 1.0);
		dir = vec4(d, 0.0);
	}

	void normalize() {
		dir = vec4(glm::normalize(vec3(dir)), 0.0);
	}

	void print() {
		printf("Ray:\nStart: %f, %f, %f\nDirection: %f, %f, %f\n", start.x, start.y, start.z, dir.x, dir.y, dir.z);
		mat.print();
	}
};

struct Light { //Stores our light data, its position in world space and the color of light.
	vec3 position;
	vec3 color;

	Light(double r, double g, double b, double x, double y, double z) {
		position = vec3(x, y, z);
		color = vec3(r, g, b);
	}

	void print() {
		printf("Light: \nPosition: %f, %f, %f\nColor: %f, %f, %f\n", position.x, position.y, position.z, color.x, color.y, color.z);
	}
};


double* quadratic(double, double, double, double*);
vec3* calcIntersection(vec3, vec3, vec3*);
vec3* closestIntersection(Ray);

void print(const vec3 p) {
	printf("Vec3: %f, %f, %f\n", p.r, p.g, p.b);
};

void print(const vec4 p) {
	printf("Vec4: %f, %f, %f, %f\n", p.x, p.y, p.z, p.w);
};

void print(const mat4 Xform) {
	printf("Mat4:\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n", Xform[0][0], Xform[1][0], Xform[2][0], Xform[3][0], Xform[0][1], Xform[1][1], Xform[2][1], Xform[3][1], Xform[0][2], Xform[1][2], Xform[2][2], Xform[3][2], Xform[0][3], Xform[1][3], Xform[2][3], Xform[3][3]);
};

std::vector<Light> lights;

std::vector<Object> objects;

vec3 PhongIllumination(vec3 point, Ray ray, Light light, Object object) {
	/*Calculate the phong illumination model*/

	//ray.print();
	//object.print();
	//light.print();

	vec4 vertPos4 = object.Xfmi * vec4(point, 1.0);
	vec3 vertPos = vec3(vertPos4) / vertPos4.w;
	vec3 normalInterp = vec3(transpose(object.Xfmi) * vec4(point, 0.0));//this should take an arbitrary normal, but for now take the normal of a shpere aka the point you hit it

	vec3 normal = normalize(normalInterp);
	vec3 lightDir = normalize(light.position - vertPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(-vertPos);

	float lambertian = max(dot(lightDir, normal), float(0.0));
	float specular = 0.0;

	if (lambertian > 0.0) {
		double specAngle = max(dot(reflectDir, viewDir), float(0.0));
		specular = pow(specAngle, ray.mat.emissivity);
		printf("Phong values: \nLambertian: %f\nSpecular: %f\n", lambertian, specular);
	}

	return vec3(ambiant + lambertian * ray.mat.diffuse + specular * ray.mat.specular);
};

bool Shadow(vec3 point, Ray ray, Light light) {
	/*Check between the object and the light to see if this new ray intersects another object*/
	Ray lightRay(point, light.position);
	vec3* test = closestIntersection(lightRay);
	if (!test || !(test < 1.0))
		return true;
	return false;
};

// Ray reflect(vec3 point, Ray ray){//used to find the color that should be here if it is a reflective object};
// Ray rafraction(vec3 point, Ray ray){//used to find the color that should be here if it is a rafractive object};

vec3 Shade(vec3 point, Ray ray, Object object) {
	/*Used to discover what color a particular pixel should be*/
	vec3 Color = backgroundColor;
	for (const Light &light : lights){
		if (!Shadow(point, ray, light))
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

	/*if (rad > 0.0)
		std::cout << rad << "\n";*/

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

vec3* calcIntersection(vec3 u, vec3 v, vec3* ret){
	double space;//is allocated memory to point t to
	double* t = NULL;

	//print(u);
	//print(v);

	t = quadratic(dot(v,v), 2.0 * dot(u,v), dot(u,u) - 1.0, &space);
	
	if (!t)
		return NULL;
	if (*t < 0.0)
		return NULL;
	
	vec3 temp;
	
	temp.x = u.x + *t * v.x;
	temp.y = u.y + *t * v.y;
	temp.z = u.z + *t * v.z;

	*ret = temp;

	return ret;
};

vec3* closestIntersection(Ray *ray) {// return the intersection point, surface normal, surface, surface attributes, etc.
	vec3 space;//some initlized memory to point our pointers to, has to be here because of scope
	vec3* current = NULL;
	vec3* smallest = NULL;

	for (const Object &object : objects){
		//print(object.Xfmi);
		current = calcIntersection(vec3(object.Xfmi * ray->start), vec3(ray->dir), &space);//we "downcast" these vectors so that we can take the proper dot product of them
		if(current) {
			if (!smallest) { //If this is the first time we intersect an object
				smallest = current;
			}
			else if (current->x < smallest->x ) { //If the new intersection is less then the current stored, change the current
				smallest = current;
				//std::cout << "Got here\n";
			}
		}
	}

	if (!smallest)
		return NULL;
	
	ray->mat = ret->mat;
	//ray->mat.print();
	return smallest;
};

vec3 trace(Ray ray) {
	//print(ray.dir);
	vec3* intersection = closestIntersection(&ray);
	if (intersection)
		return Shade(*intersection, ray);
	return backgroundColor;
};

void processInput() {
	using namespace std;
	d = 5;
	n = 10;
	Xsize = n;
	Ysize = n;

	int count = 1;
	
	Object groupobject;
	////groupobject.print();
	std::vector<mat4> inversestack;
	
	std::string line, token; 
	std::vector<std::string> tokens;

	size_t pos = 0;

	while (std::getline(std::cin, line, '\n')) {
		std::istringstream check;
		check.str(line);
		for (token; std::getline(check, token, ' '); ) {
			if (token[0] == '#') {
				tokens.push_back("#");
				check.str("");
			}
			else if (token.length() == 0) {//removes spaces, turns out a string with only space is considered length of zero?
				//check.str(token.substr());
				//cout << "visited here";
			}
			else {
				tokens.push_back(token);
				//cout << token.c_str() << endl;
			}
			//cout << token.c_str() << endl;
		}
	}

	for (auto i = tokens.begin(); i < tokens.end(); i++) {
		std::cout << *i << " ";
		if ("#" == *i){ std::cout << "\n"; }
		else if ("view" == *i) {
			n = std::stoi(*(i + 1));
			d = std::stof(*(i + 2));
			printf("%i %f\n", n,d);
			Xsize = n;
			Ysize = n;
			i += 2;
		}
		else if ("scale" == *i) {
			float arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f\n", arr[0], arr[1], arr[2]);
			groupobject.Xform = scale(groupobject.Xform, vec3(arr[0], arr[1], arr[2]));
			inversestack.push_back(inverse(scale(mat4(1.0), vec3(arr[0], arr[1], arr[2]))));
			i += 3;
		}
		else if ("move" == *i) {
			float arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f\n", arr[0], arr[1], arr[2]);
			groupobject.Xform = translate(groupobject.Xform, vec3(arr[0], arr[1], arr[2]));
			inversestack.push_back(inverse(translate(mat4(1.0), vec3(arr[0], arr[1], arr[2]))));
			//print(vec3(x, y, z));
			//print(groupobject.Xform);
			//print(groupobject.Xfmi);
			i += 3;
		}
		else if ("rotate" == *i) {
			float arr[4];
			for (int j = 0; j < 4; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f %f\n", arr[0], arr[1], arr[2], arr[3]);
			groupobject.Xform = rotate(groupobject.Xform, arr[0], vec3(arr[1], arr[2], arr[3]));
			inversestack.push_back(inverse(rotate(mat4(1.0), arr[0], vec3(arr[1], arr[2], arr[3]))));
			i += 4;
		}
		else if ("sphere" == *i) {
			mat4 temp(1.0);
			for (auto object = inversestack.rbegin(); object != inversestack.rend(); object++) {
				temp = temp * *object;
			}
			groupobject.Xfmi = temp;

			objects.push_back(groupobject); //this needs to call some sort of new operator to allocate memory for the thingy
			//objects[0].print();
			cout << "\n";
		}
		else if ("light" == *i) {
			float arr[6];
			for (int j = 0; j < 6; j++) {
				//cout << *(i + j + 1) << " ";
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f %f %f %f\n", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
			lights.push_back(Light(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]));
			i += 6;
		}
		else if ("background" == *i) {
			float arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f\n", arr[0], arr[1], arr[2]);
			backgroundColor = vec3(arr[0], arr[1], arr[2]);
			i += 3;
		}
		else if ("ambient" == *i) {
			float arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f\n", arr[0], arr[1], arr[2]);
			ambiant = vec3(arr[0], arr[1], arr[2]);
			i += 3;
		}
		else if ("refraction" == *i) {
			float arr[4];
			for (int j = 0; j < 4; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f %f\n", arr[0], arr[1], arr[2], arr[3]);
			groupobject.mat.refrac(arr[0], arr[1], arr[2], arr[3]);
			i += 4;
		}
		else if ("material" == *i) {
			float arr[7];
			for (int j = 0; j < 7; j++) {
				arr[j] = stof(*(i + j + 1));
			}
			printf("%f %f %f %f %f %f %f\n", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6]);
			groupobject.mat.set(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6]);
			i += 7;
		}
		else if ("group" == line) {//when we find were at a new group, reset the current state
			Object* temp = new Object();
			groupobject = *temp;
			inversestack.clear();
			std::cout << "\n";
		}
		else if ("groupend" == line) {//when we end the group, reset the current state
			Object* temp = new Object();
			groupobject = *temp;
			inversestack.clear();
			std::cout << "\n";
		}
		else
			cout << "\nError: could not read line " << count << " in file. Line: " << *i << "\n";
		count++;
	}
		
	std::cout << "Sucessfully read the file\n";
};

int main() {
	
	processInput();
	
	std::vector<std::vector<vec3>> Pixels(n, std::vector<vec3>(n));

	pw = 2 * d / n;
	ph = 2 * d / n;

	Ray current(u);
	//printVec(&u);
	//printVec(&v);
	for (float j = n-1; j >= 0; j--) {
		v.y = d - ph * j - ph / 2;//calculate midpoint for row
		//std::cout << v.y << "\n";
		for (float i = 0; i <= n - 1; i++) {
			v.x = d - ph * i - ph / 2;//calculate midpoint for column
			//std::cout << v.x << "\n";
			current.dir = vec4(v - u, 0.0);
			//print(v);
			current.normalize();
			//print(current.dir);
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
	for (int j = Ysize; j >= 0; j--) {     // Y is flipped!
		for (int i = 0; i < Xsize; i++) {
			r = Pixels[i][j].r * 255;
			g = Pixels[i][j].g * 255;
			b = Pixels[i][j].b * 255;
			fprintf(picfile, "%c%c%c", r, g, b);
			//if (i == Xsize - 1)//use to dertimaine if your actually writing to the file in the case of large files
			//	std::cout << "row\n";
			//print(Pixels[i][j]);
			// Remember though that this is a number between 0 and 255
			// so might have to convert from 0-1.
		}
	}
	fclose(picfile);
	return 0;
}
