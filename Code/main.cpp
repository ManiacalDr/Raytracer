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
int n; // N*N will give you the size of the image

const vec3 cam(0.0, 0.0, 1.0);
vec3 backgroundColor(0.0,0.0,0.0);
vec3 ambiant(0.0);
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

	void print() const{
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

	void print() const{
		printf("Object: \nXform:\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\nXfmi: \n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n", Xform[0][0], Xform[1][0], Xform[2][0], Xform[3][0], Xform[0][1], Xform[1][1], Xform[2][1], Xform[3][1], Xform[0][2], Xform[1][2], Xform[2][2], Xform[3][2], Xform[0][3], Xform[1][3], Xform[2][3], Xform[3][3],
			Xfmi[0][0], Xfmi[1][0], Xfmi[2][0], Xfmi[3][0], Xfmi[0][1], Xfmi[1][1], Xfmi[2][1], Xfmi[3][1], Xfmi[0][2], Xfmi[1][2], Xfmi[2][2], Xfmi[3][2], Xfmi[0][3], Xfmi[1][3], Xfmi[2][3], Xfmi[3][3]);
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

	void print() const{
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

	void print() const{
		printf("Light: \nPosition: %f, %f, %f\nColor: %f, %f, %f\n", position.x, position.y, position.z, color.x, color.y, color.z);
	}
};


double* quadratic(double, double, double, double*);
vec3* calcIntersection(vec3, vec3, vec3*);
vec3* closestIntersection(Ray*, Object*);//get rid of this, transform the returned hit insted of having to do this weird transfering of the object.
vec3* closestIntersection(Ray*);
vec3 trace(Ray*);

void print(const vec3 p) {
	printf("Vec3: %f, %f, %f\n", p.r, p.g, p.b);
};

void print(const vec4 p) {
	printf("Vec4: %f, %f, %f, %f\n", p.x, p.y, p.z, p.w);
};

void print(const mat4 Xform) {
	printf("Mat4:\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n", Xform[0][0], Xform[1][0], Xform[2][0], Xform[3][0], Xform[0][1], Xform[1][1], Xform[2][1], Xform[3][1], Xform[0][2], Xform[1][2], Xform[2][2], Xform[3][2], Xform[0][3], Xform[1][3], Xform[2][3], Xform[3][3]);
};

double vectorMag(const vec3* vec) {
	if (!vec) {
		return 0.0;
	}
	return abs(sqrt(pow(vec->x, 2) + pow(vec->y, 2) + pow(vec->z, 2)));
};

std::vector<Light> lights;

std::vector<Object> objects;

vec3 PhongIllumination(const vec3 point,const Ray* ray,const Light* light,const Object* object) {
	/*Calculate the phong illumination model*/

	//ray.print();
	//object.print();
	//light->print();

	vec4 vertPos4 = object->Xfmi * vec4(point, 1.0);
	vec3 vertPos = vec3(vertPos4) / vertPos4.w;
	vec3 normalInterp = vec3(transpose(object->Xfmi) * vec4(point, 0.0));//this should take an arbitrary normal, but for now take the normal of a shpere aka the point you hit it

	vec3 normal = normalize(normalInterp);
	vec3 lightDir = normalize(light->position - vertPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(-vertPos);

	float lambertian = max(dot(lightDir, normal), float(0.0));
	float specular = 0.0;

	if (lambertian > 0.0) {
		double specAngle = max(dot(reflectDir, viewDir), float(0.0));
		specular = pow(specAngle, ray->mat.emissivity);
		//printf("Phong values: \nLambertian: %f\nSpecular: %f\n", lambertian, specular);
	}

	return vec3(ambiant + lambertian * ray->mat.diffuse + specular * ray->mat.specular);
};

bool Shadow(vec3 point,const Ray* ray,const Light* light) {
	/*Check between the object and the light to see if this new ray intersects another object*/
	Ray lightRay(point, light->position - point);
	vec3* test = closestIntersection(&lightRay);
	if (test && vectorMag(test) < 1.0)
		return true;
	return false;
};

Ray reflect(vec3* hitpoint, Ray* ray){
	/*used to find the color that should be here if it is a reflective object*/
	vec3 normal = normalize(*hitpoint);
	return Ray(*hitpoint, vec3(ray->dir) - 2*dot(vec3(ray->dir), normal)*normal);
};

Ray rafraction(vec3* hitpoint, Ray* ray){
	/*used to find the color that should be here if it is a rafractive object*/
	vec3 normal = normalize(*hitpoint);
	double ni = dot(n, -r.d); 
	double eta = current_index / new_index; 
	return Ray((x, eta*ni - sqrt(1 - eta * eta*(1 - ni * ni)))*n + eta * r.d);
	return Ray(*hitpoint, vec3(ray->dir) - 2 * dot(vec3(ray->dir), normal)*normal);
};

vec3 Shade(vec3 hitpoint, Ray* ray, Object* object) {
	/*Used to discover what color a particular pixel should be*/
	//std::cout << "Registered hit\n";
	vec3 Color = backgroundColor;
	for (const Light &light : lights){
		if (Shadow(hitpoint, ray, &light))
			Color += PhongIllumination(hitpoint, ray, &light, object);
	}
	Ray space = reflect(&hitpoint, ray);
	if (object->mat.emissivity == 0) Color += trace(&space);
	if refractive Color += trace(refraction(point, ray));
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
	t = quadratic(dot(v, v), 2.0 * dot(u, v), dot(u, u) - 1.0, &space);
	//print(u);
	//print(v);
	
	if (!t)
		return NULL;
	//std::cout << *t << " \n";
	if (*t < 0.0)
		return NULL;
	
	vec3 temp;
	
	temp.x = u.x + *t * v.x;
	temp.y = u.y + *t * v.y;
	temp.z = u.z + *t * v.z;

	*ret = temp;

	return ret;
};

vec3* closestIntersection(Ray *ray, Object *ret) {// return the intersection point, surface normal, surface, surface attributes, etc.
	vec3 space;//some initlized memory to point our pointers to, has to be here because of scope
	vec3* current = NULL;
	vec3* smallest = NULL;

	for (const Object &object : objects){
		//print(object.Xfmi);
		//object.print();
		current = calcIntersection(vec3(object.Xfmi * ray->start), vec3(ray->dir), &space);//we "downcast" these vectors so that we can take the proper dot product of them
		if(current) {
			if (!smallest) { //If this is the first time we intersect an object
				smallest = current;
				*ret = object;
			}
			else if (vectorMag(current) < vectorMag(smallest)) { //If the new intersection is less then the current stored, change the current
				smallest = current;
				*ret = object;
				std::cout << "Got here\n";
			}
		}
	}

	if (!smallest)
		return NULL;
	
	ray->mat = ret->mat;
	//ray->mat.print();
	return smallest;
};

vec3* closestIntersection(Ray *ray) {// return the intersection point, surface normal, surface, surface attributes, etc.
	vec3 space;//some initlized memory to point our pointers to, has to be here because of scope
	vec3* current = NULL;
	vec3* smallest = NULL;

	for (const Object &object : objects) {
		//print(object.Xfmi);
		current = calcIntersection(vec3(object.Xfmi * ray->start), vec3(ray->dir), &space);//we "downcast" these vectors so that we can take the proper dot product of them
		if (current) {
			if (!smallest) { //If this is the first time we intersect an object
				smallest = current;
				ray->mat = object.mat;
			} 
			else if (vectorMag(current) < vectorMag(smallest)) { //If the new intersection is less then the current stored, change the current
				smallest = current;
				ray->mat = object.mat;
				//std::cout << "Got here\n";
			}
		}
	}

	if (!smallest)
		return NULL;

	//ray->mat.print();
	return smallest;
};

vec3 trace(Ray* ray) {
	Object object;
	vec3* intersection = closestIntersection(ray, &object);
	if (intersection)
		return Shade(*intersection, ray, &object);
	return backgroundColor;
};

void processInput() {
	using namespace std;
	
	string line, token; 
	vector<string> tokens;

	while (getline(std::cin, line, '\n')) {
		istringstream check;
		check.str(line);
		for (token; getline(check, token, ' '); ) {
			if (token[0] == '#') {
				tokens.push_back("#");
				check.str("");
			}
			else if (token.length() == 0) {//removes spaces, turns out a string with only space is considered length of zero?
				//check.str(token.substr());
			}
			else {
				tokens.push_back(token);
				//cout << token.c_str() << endl;
			}
			//cout << token.c_str() << endl;
		}
	}

	d = 5;
	n = 10;

	int count = 1;

	bool ingroup = false;

	Object groupobject;
	std::vector<mat4> stack;

	for (auto i = tokens.begin(); i < tokens.end(); i++) {
		std::cout << *i << " ";
		if ("#" == *i){ std::cout << "\n"; }
		else if ("view" == *i) {
			n = stoi(*(i + 1));
			d = stof(*(i + 2));
			printf("%i %f\n", n,d);
			i += 2;
		}
		else if ("scale" == *i) {
			float arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f\n", arr[0], arr[1], arr[2]);

			stack.push_back(scale(mat4(1.0), vec3(arr[0], arr[1], arr[2])));

			i += 3;
		}
		else if ("move" == *i) {
			float arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f\n", arr[0], arr[1], arr[2]);

			stack.push_back(translate(mat4(1.0), vec3(arr[0], arr[1], arr[2])));
			
			i += 3;
		}
		else if ("rotate" == *i) {
			float arr[4];
			for (int j = 0; j < 4; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			printf("%f %f %f %f\n", arr[0], arr[1], arr[2], arr[3]);

			stack.push_back(rotate(mat4(1.0), arr[0], vec3(arr[1], arr[2], arr[3])));

			i += 4;
		}
		else if ("sphere" == *i) {
			mat4 temp = groupobject.Xfmi;
			groupobject.Xfmi = mat4(1.0);//set it to a new matrix
			for (auto object = stack.begin(); object != stack.end(); object++) {//this multiples from clostest to the top
				groupobject.Xfmi = groupobject.Xfmi * *object;
			}
			groupobject.Xfmi *= temp;
			temp = groupobject.Xform;//were going to save this for a moment
			groupobject.Xform = mat4(1.0);
			for (auto object = stack.rbegin(); object != stack.rend(); object++) {//this multiples from clostest to the top
				groupobject.Xform = groupobject.Xform * inverse(*object);
			}
			groupobject.Xform *= temp;
			//groupobject.print();
			stack.clear();//we clear because the object is already transformed, if we did't do this the next sphere would be transformed by what is in the stack again
			objects.push_back(groupobject); //this needs to call some sort of new operator to allocate memory for the thingy
			cout << "\n";
		}
		else if ("light" == *i) {
			float arr[6];
			for (int j = 0; j < 6; j++) {
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
		else if ("group" == *i) {
			if (ingroup) { cerr << "\nError: called group while still in group.\n"; }
			if (!stack.empty()) {
			mat4 temp = groupobject.Xform;//were going to save this for a moment
			groupobject.Xform = mat4(1.0);//set it to a new matrix
			for (auto object = stack.rbegin(); object != stack.rend(); object++) {//this multiples from clostest to the top
				groupobject.Xform = groupobject.Xform * *object;
			}
			groupobject.Xform *= temp;//this transforms it by whatever was done before the group
		}
			stack.clear();
			ingroup = true;
			std::cout << "\n";
		}
		else if ("groupend" == *i) {
			if(!ingroup) { cerr << "\nError: called groupend while not in a group.\n"; }
			if (!stack.empty()) {//instead of reseting the state, we return it to what it was before group was called, whatever that was
				for (auto object = stack.rbegin(); object != stack.rend(); object++) {
					groupobject.Xfmi = groupobject.Xfmi * *object;//the opossite of inverse in not inverse
				}
				for (auto object = stack.begin(); object != stack.end(); object++) {
					groupobject.Xform = groupobject.Xform * inverse(*object);//inverts whatever is there
				}
			}

			stack.clear();
			ingroup = false;
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

	//Pixels[n][n][3]
	float** Pixels[n];
	for (int i = 0; i < n; i++) {
		Pixels[i] = new float*[n];
		for (int j = 0; j < n; j++) {
			Pixels[i][j] = new float[3];
		}
	}
	std::cout << "Allocated Pixels array\n";

	pw = 2 * d / n;
	ph = 2 * d / n;

	Ray current(u);
	//printVec(&u);
	//printVec(&v);
	int count = 0;
	for (int j = 0; j <= n - 1; j++) {
		v.y = d - ph * j - ph / 2;//calculate midpoint for row
		//std::cout << v.y << "\n";
		for (int i = 0; i <= n - 1; i++) {
			v.x = d - ph * i - ph / 2;//calculate midpoint for column
			//std::cout << v.x << "\n";
			current.dir = vec4(v - u, 0.0);
			//print(v);
			current.normalize();
			//print(current.dir);
			//print(&current.start);
			vec3 Color = trace(&current);
			Pixels[i][j][0] = Color.r;
			Pixels[i][j][1] = Color.g;
			Pixels[i][j][2] = Color.b;
			count++;
		}
	}
	std::cout << "Sucessfully shot all rays " << count << "\n";
	count = 0;

	unsigned char r, g, b;
	FILE *picfile;
	picfile = fopen("out.ppm", "w");
	fprintf(picfile, "P6# %dx%d Raytracer output\n%d %d\n255\n",
			n,n,n,n);
	// For each pixel
	for (int j = n - 1; j >= 0; j--) {// Y is flipped!
		for (int i = 0; i < n; i++) {
			r = int(Pixels[i][j][0] * 255);
			g = int(Pixels[i][j][1] * 255);
			b = int(Pixels[i][j][2] * 255);
			fprintf(picfile, "%c%c%c", r, g, b);
			//if (i == Xsize - 1)//use to dertimaine if your actually writing to the file in the case of large files
			//	std::cout << "row\n";
			//std::cout << int(Pixels[i][j][0] * 255) << " " << int(Pixels[i][j][1] * 255) << " " << int(Pixels[i][j][2] * 255) << " \n";
			// Remember though that this is a number between 0 and 255
			// so might have to convert from 0-1.
			count++;
		}
		//fprintf(picfile, "\n");
	}

	std::cout << "Sucessfully wrote to the PPM " << count << "\n";
	fclose(picfile);
	return 0;
}

