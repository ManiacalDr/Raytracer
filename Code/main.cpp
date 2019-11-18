/*Uses the Function p = u + vt to determine the color of a pixel after shooting out a ray
ran into problems with pointers pointing to uninitlized memory and trying to set a value there*/
//https://stackoverflow.com/questions/1986378/how-to-set-up-quadratic-equation-for-a-ray-sphere-intersection
//https://stackoverflow.com/questions/20028396/how-could-declaring-a-large-2d-array-crash-a-program
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

// Include GLM
#include <glm/ext.hpp>

using namespace glm;

int raycount = 0;
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

struct Object { // this could be turned into multiple types of objects by changing the calcintersection for any particular object
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
	bool isnorm;

	Ray(vec3 vec) {
		start = vec4(vec, 1.0);
		dir = vec4(0.0, 0.0, 0.0, 0.0);
		isnorm = false;
	}

	Ray(vec3 s, vec3 d) {
		start = vec4(s, 1.0);
		dir = vec4(d, 0.0);
		isnorm = false;
	}

	void normalize() {
		dir = vec4(glm::normalize(vec3(dir)), 0.0);
		isnorm = true;
	}

	void print() const{
		printf("Ray:\nStart: %f, %f, %f\nDirection: %f, %f, %f\n", start.x, start.y, start.z, dir.x, dir.y, dir.z);
		mat.print();
	}
};

struct Hitpoint { //stores our hitpoint,normal at the hitpoint, and material(?)
	vec3 hitpoint;
	vec3 normal;

	Hitpoint() {
		hitpoint = vec3(0.0);
		normal = vec3(0.0);
	}

	Hitpoint(vec3 h, vec3 n) {
		hitpoint = h;
		normal = n;
	}

	Hitpoint(const Hitpoint& hit) {
		hitpoint = hit.hitpoint;
		hitpoint = hit.normal;
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
double* calcIntersection(vec3, vec3, double&);
Hitpoint* closestIntersection(Ray*, Hitpoint*);
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

std::vector<Light> lights;

std::vector<Object> objects;

double vectorMag(const vec3 vec) {
	return abs(sqrt(pow(vec.x, 2) + pow(vec.y, 2) + pow(vec.z, 2)));
};

Ray* reflect(Hitpoint* hitpoint, Ray* ray) {
	/*used to find the color that should be here if it is a reflective object*/
	static int count = 0;

	vec3 start = hitpoint->hitpoint + float(1.01) * hitpoint->normal;

	if (count > 20) {
		count = 0;
		return NULL;
	}

	count++;

	if (!ray->isnorm)
		ray->normalize();

	Ray* temp = new Ray(start, -vec3(ray->dir) - 2 * dot(hitpoint->normal, -vec3(ray->dir)) * hitpoint->normal);
	return temp;
};

//Ray rafraction(vec3* hitpoint, Ray* ray){//Need to fix the Math in this one
//	/*used to find the color that should be here if it is a rafractive object*/
//	vec3 normal = normalize(*hitpoint);
//	double ni = dot(n, -r.d); 
//	double eta = current_index / new_index; 
//	return Ray((x, eta*ni - sqrt(1 - eta * eta*(1 - ni * ni)))*n + eta * r.d);
//	return Ray(*hitpoint, vec3(ray->dir) - 2 * dot(vec3(ray->dir), normal)*normal);
//};

vec3 PhongIllumination(const Hitpoint* point,const Ray* ray,const Light* light) {//need to fix this, currently ambiant is added for each light, incorrect
	/*Calculate the phong illumination model*/

	//ray->print();
	//light->print();
	
	vec3 lightDir = normalize(light->position - point->hitpoint);
	vec3 reflectDir = -lightDir - 2 * dot(point->normal, -lightDir)*point->normal;
	vec3 viewDir = normalize(vec3(ray->start) - point->hitpoint);

	double lightdist = vectorMag(light->position - point->hitpoint);
	float lambertian = max(dot(lightDir, point->normal), float(0.0));
	float specular = 0.0;
	float lightatt = min(1/(lightdist*.02) , 1.0);

	if (lambertian > 0.0) {
		//lambertian = 0;
		float specAngle = max(dot(reflectDir,float(1.001) * viewDir), float(0.0));
		specular = pow(specAngle, ray->mat.emissivity);
	}
	return lightatt * light->color * vec3((lambertian * ray->mat.diffuse) + (specular * ray->mat.specular));
};

bool Shadow(Hitpoint* point,const Ray* ray,const Light* light) {
	/*Check between the object and the light to see if this new ray intersects another object*/
	Ray lightRay(point->hitpoint, light->position - point->hitpoint);
	Hitpoint* test = closestIntersection(&lightRay, test);
	if(test) {//if we return null, either nothing was hit, or all other objects are past the light, not casting a shadow
		return true;
	}
	return false;
};

vec3 Shade(Hitpoint* hitpoint, Ray* ray) {
	/*Used to discover what color a particular pixel should be*/
	vec3 Color = ambiant * ray->mat.diffuse;

	for (const Light &light : lights){
		if (!Shadow(hitpoint, ray, &light));
			Color = Color + PhongIllumination(hitpoint, ray, &light);
	}
	Ray* space = reflect(hitpoint, ray);
	if (!space) return Color;
	if (ray->mat.emissivity > 0) Color += ray->mat.specular * trace(space);
	//if refractive Color += trace(refraction(point, ray)); 376030

	if (Color.x > 1.5 || Color.y > 1.5 || Color.z > 1.5) { std::cerr << "Error: light over 150% at some pixel\n"; }// print(Color); Color = vec3(1.0); }
	return Color;
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

double* calcIntersection(vec3 u, vec3 v, double& ret){
	double space;//is allocated memory to point t to
	double* t = NULL;
	t = quadratic(dot(v, v), 2.0 * dot(u, v), dot(u, u) - 1.0, &space);
	//print(u);
	//print(v);
	
	if (!t)
		return NULL;
	//std::cout << *t << " \n";
	if (*t < 0.0)//this determines if it is behind the start of the ray
		return NULL;
	
	ret = *t;

	return &ret;
};

Hitpoint* closestIntersection(Ray *ray, Hitpoint* ret) {// return the intersection point, surface normal, surface, surface attributes, etc.
	raycount++;//this is here because shadow doenst call trace, we dont care about the color, but it does technically shoot a ray.
	double t;//some initlized memory to point our pointers to, has to be here because of scope
	double* current = NULL;
	double* smallest = NULL;
	for (const Object &object : objects) {
		current = calcIntersection(vec3(object.Xfmi * ray->start), vec3(object.Xfmi * ray->dir), t);//we "downcast" these vectors so that we can take the proper dot product of them
		if (current) {//We hit an object
			if (!ray->isnorm) {//if we are checking a shadow ray
				if (*current < 1) return ret = new Hitpoint;//if we hit an object, just return any value
			}
			else {//for any other ray, we need to know what we hit
				if (!smallest) { //If this is the first time we intersect an object
					smallest = new double;
					*smallest = *current;
					ret = new Hitpoint;
					ret->hitpoint.x = ray->start.x + *smallest * ray->dir.x;
					ret->hitpoint.y = ray->start.y + *smallest * ray->dir.y;
					ret->hitpoint.z = ray->start.z + *smallest * ray->dir.z;
					ret->normal = vec3(transpose(object.Xfmi) * (object.Xfmi * vec4(ret->hitpoint, 1.0)));
					ray->mat = object.mat;
				} 
				else {
					if (*current < *smallest) { //If the new intersection is less then the current stored, change the current
						*smallest = *current;
						ret->hitpoint.x = ray->start.x + *smallest * ray->dir.x;
						ret->hitpoint.y = ray->start.y + *smallest * ray->dir.y;
						ret->hitpoint.z = ray->start.z + *smallest * ray->dir.z;
						ret->normal = vec3(transpose(object.Xfmi) * vec4(vec3(object.Xfmi * vec4(ret->hitpoint, 1.0)),0.0));
						ray->mat = object.mat;
					}
				}
			}
		}
	}
	if (!smallest)//nothing was hit
		return NULL;
	ret->normal = normalize(ret->normal);
	//print(ret->hitpoint);
	//print(ret->normal);
	return ret;
};

vec3 trace(Ray* ray) {
	Hitpoint* intersection = closestIntersection(ray, intersection);
	if (intersection)
		return Shade(intersection, ray);
	return backgroundColor;
};

void processInput(const std::string& file) {
	using namespace std;
	
	fstream input;
	input.open(file);
	string line, token; 
	vector<string> tokens;

	while (getline(input, line, '\n')) {
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
	cout << "Sucessfully parsed the file\n";
	d = 5;
	n = 10;

	int count = 1;
	int level = 0;//defines what level of the scene graph we are currently in

	bool ingroup = false;

	Object* groupobject;
	vector<mat4> CTM;
	vector<mat4> CIM;
	vector<Material> MAT;

	CTM.push_back(mat4(1.0));
	CIM.push_back(mat4(1.0));
	MAT.push_back(Material());

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
			CTM[level] = CTM[level] * scale(mat4(1.0), vec3(arr[0], arr[1], arr[2]));
			CIM[level] = inverse(scale(mat4(1.0), vec3(arr[0], arr[1], arr[2]))) * CIM[level];
			//print(CIM[level]);
			i += 3;
		}
		else if ("move" == *i) {
			float arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stof(*(i + j + 1));
			}
			printf("%f %f %f\n", arr[0], arr[1], arr[2]);
			CTM[level] = CTM[level] * translate(mat4(1.0), vec3(arr[0], arr[1], arr[2]));
			CIM[level] = inverse(translate(mat4(1.0), vec3(arr[0], arr[1], arr[2]))) * CIM[level];
			//print(CIM[level]);
			i += 3;
		}
		else if ("rotate" == *i) {
			float arr[4];
			for (int j = 0; j < 4; j++) {
				arr[j] = stof(*(i + j + 1));
			}
			printf("%f %f %f %f\n", arr[0], arr[1], arr[2], arr[3]);
			CTM[level] = CTM[level] * rotate(mat4(1.0), arr[0], vec3(arr[1], arr[2], arr[3]));
			CIM[level] = inverse(rotate(mat4(1.0), arr[0], vec3(arr[1], arr[2], arr[3]))) * CIM[level];
			//print(CIM[level]);
			i += 4;
		}
		else if ("sphere" == *i) {
			groupobject = new Object(CTM[level], CIM[level], MAT[level]);
			//groupobject->print();
			objects.push_back(*groupobject); //this needs to call some sort of new operator to allocate memory for the thingy
			delete groupobject;
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
			MAT[level].refrac(arr[0], arr[1], arr[2], arr[3]);
			i += 4;
		}
		else if ("material" == *i) {
			float arr[7];
			for (int j = 0; j < 7; j++) {
				arr[j] = stof(*(i + j + 1));
			}
			printf("%f %f %f %f %f %f %f\n", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6]);
			MAT[level].set(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6]);
			i += 7;
		}
		else if ("group" == *i) {//should traverse to the next level of the scene graph
			level++;
			if ((level + 1) > CTM.size() || (level + 1) > CIM.size() || (level + 1) > MAT.size()) {
				//this is a more verbose way of calling this, technically one one should be necessary, there should never be one with a greater or lesser number of levels
				CTM.push_back(CTM[level - 1]);
				CIM.push_back(CIM[level - 1]);
				MAT.push_back(MAT[level - 1]);
			}
			else {
				CTM[level] = CTM[level - 1];
				CIM[level] = CIM[level - 1];
				MAT[level] = MAT[level - 1];
			}
			
			std::cout << "\n";
		}
		else if ("groupend" == *i) {//should move back up one level of the scene graph
			if (level == 0) cerr << "Error: called groupend without calling group";
			else
				level--;
			std::cout << "\n";
		}
		else
			cout << "\nError: could not read line " << count << " in file. Line: " << *i << "\n";
		count++;
		//cout << level << "\n";
	}
	
	cout << "Sucessfully read the file\n";
};

int main(int argc, char* argv[]) {
	
	if (argc < 2) {// Check the number of parameters
		// Tell the user how to run the program
		std::cerr << "Usage: " << argv[0] << " file" << std::endl;
		return 1;
	}
	std::string file(argv[1]);
	processInput(file);

	//Pixels[n][n][3]
	float** Pixels[n];//Made this way because I ran into an issue with allocating a 1000x1000 crashing the program due to large allocation request on the stack
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
	for (int j = n-1; j >= 0; j--) {
		v.y = d - ph * j - ph / 2;//calculate midpoint for row
		//std::cout << v.y << "\n";
		for (int i = n-1; i >= 0; i--) {
			v.x = d - ph * i - ph / 2;//calculate midpoint for column
			//std::cout << v.x << "\n";
			current.dir = vec4(v - u, 0.0);
			current.normalize();
			vec3 Color = trace(&current);
			Pixels[i][j][0] = Color.r;
			Pixels[i][j][1] = Color.g;
			Pixels[i][j][2] = Color.b;
		}
	}
	std::cout << "Sucessfully shot all rays " << raycount << "\n";
	int count = 0;

	unsigned char r, g, b;
	FILE *picfile;
	picfile = fopen("out.ppm", "w");
	fprintf(picfile, "P6# %dx%d Raytracer output\n%d %d\n255\n",
			n,n,n,n);
	// For each pixel
	for (int j = 0; j <= n-1; j++) {// Y is flipped!
		for (int i = n-1; i >= 0; i--) {
			r = int(clamp(Pixels[i][j][0], float(0.0), float(1.0)) * 255);
			g = int(clamp(Pixels[i][j][1], float(0.0), float(1.0)) * 255);
			b = int(clamp(Pixels[i][j][2], float(0.0), float(1.0)) * 255);
			fprintf(picfile, "%c%c%c", r, g, b);
			//std::cout << int(Pixels[i][j][0] * 255) << " " << int(Pixels[i][j][1] * 255) << " " << int(Pixels[i][j][2] * 255) << " \n";
			// Remember though that this is a number between 0 and 255
			// so might have to convert from 0-1.
			count++;
		}
	}

	std::cout << "Sucessfully wrote to the PPM " << count << "\n";
	fclose(picfile);
	return 0;
}

