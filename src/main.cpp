/*Uses the Function p = u + vt to determine the color of a pixel after shooting out a ray*/
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

// Include GLM
#include <glm/ext.hpp>

typedef std::chrono::high_resolution_clock Clock;
using namespace glm;

int raycount = 0;
int reflectcount = 0;
int refractcount = 0;
float d;
int n; // N*N will give you the size of the image

const vec3 cam(0.0, 0.0, 1.0);
vec3 backgroundColor(0.0,0.0,0.0);
vec3 ambiant(0.0);
const double defaultrefraction = 1.000295;

vec3 u = cam;
vec3 v(0.0, 0.0, 0.0);
vec3 p;

float pw;
float ph;

struct Material {
	vec3 diffuse;
	vec3 specular;
	vec3 refraction;
	double index;
	float emissivity;

	Material() {
		diffuse = vec3(1.0);
		specular = vec3(1.0);
		refraction = vec3(0.0);
		index = defaultrefraction;
		emissivity = 0;
	}

	void set(double dr, double dg, double db, double sr, double sg, double sb, double p) {
		diffuse = vec3(dr, dg, db);
		specular = vec3(sr, sg, sb);
		emissivity = p;
	}

	void refrac(double r, double g, double b, double i) {
		refraction = vec3(r, g, b);
		index = i;
	}

	void print() const{
		printf("Material: \nDiffuse: %f, %f, %f\nSpecular: %f, %f, %f\nRefraction: %f, %f, %f, %f \nEmissivity: %f\n\n", diffuse.x, diffuse.y, diffuse.z, specular.x, specular.y, specular.z,refraction.x, refraction.y, refraction.z, index, emissivity);
	}
};

struct Object { // this could be turned into multiple types of objects by changing the calcintersection for any particular object
	dmat4 Xform;
	dmat4 Xfmi;
	Material mat;

	Object() {
		Xform = dmat4(1.0);
		Xfmi = dmat4(1.0);
	}

	Object(dmat4 xform, dmat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

	virtual ~Object() {};

	virtual double* calcIntersection(dvec3, dvec3, double&) { return NULL; };

	void print() const{
		printf("Object: \nXform:\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\nXfmi: \n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n", Xform[0][0], Xform[1][0], Xform[2][0], Xform[3][0], Xform[0][1], Xform[1][1], Xform[2][1], Xform[3][1], Xform[0][2], Xform[1][2], Xform[2][2], Xform[3][2], Xform[0][3], Xform[1][3], Xform[2][3], Xform[3][3],
			Xfmi[0][0], Xfmi[1][0], Xfmi[2][0], Xfmi[3][0], Xfmi[0][1], Xfmi[1][1], Xfmi[2][1], Xfmi[3][1], Xfmi[0][2], Xfmi[1][2], Xfmi[2][2], Xfmi[3][2], Xfmi[0][3], Xfmi[1][3], Xfmi[2][3], Xfmi[3][3]);
		mat.print();
	}
};

struct Sphere : virtual public Object{
	Sphere() {
		Xform = dmat4(1.0);
		Xfmi = dmat4(1.0);
	}

	Sphere(dmat4 xform, dmat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

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
	}

	double* calcIntersection(dvec3 u, dvec3 v, double& ret) override {
		double space;//is allocated memory to point t to
		double* t = NULL;

		t = quadratic(dot(v, v), 2.0 * dot(u, v), dot(u, u) - 1.0, &space);

		if (!t)
			return NULL;
		if (*t < 0.0)//this determines if it is behind the start of the ray
			return NULL;

		ret = *t;

		return &ret;
	}
};

struct Cylinder : virtual public Object {
	Cylinder() {
		Xform = mat4(1.0);
		Xfmi = mat4(1.0);
	}

	Cylinder(mat4 xform, mat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

	double* quadratic(double A, double B, double C, double* ret) {//we pass a pointer in so this has some initlized memory in the calling function to set the final value too.
		return NULL;
	}

	double* calcIntersection(dvec3 u, dvec3 v, double& ret) override {
		return NULL;
	}
};

struct Cone : virtual public Object {
	Cone() {
		Xform = mat4(1.0);
		Xfmi = mat4(1.0);
	}

	Cone(mat4 xform, mat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

	double* quadratic(double A, double B, double C, double* ret) {//we pass a pointer in so this has some initlized memory in the calling function to set the final value too.
		return NULL;
	}

	double* calcIntersection(dvec3 u, dvec3 v, double& ret) override {
		return NULL;
	}
};

struct Paraboloid : virtual public Object {
	Paraboloid() {
		Xform = mat4(1.0);
		Xfmi = mat4(1.0);
	}

	Paraboloid(mat4 xform, mat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

	double* quadratic(double A, double B, double C, double* ret) {//we pass a pointer in so this has some initlized memory in the calling function to set the final value too.
		return NULL;
	}

	double* calcIntersection(dvec3 u, dvec3 v, double& ret) override {
		return NULL;
	}
};

struct Hyperboloid : virtual public Object {
	Hyperboloid() {
		Xform = mat4(1.0);
		Xfmi = mat4(1.0);
	}

	Hyperboloid(mat4 xform, mat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

	double* quadratic(double A, double B, double C, double* ret) {//we pass a pointer in so this has some initlized memory in the calling function to set the final value too.
		return NULL;
	}

	double* calcIntersection(dvec3 u, dvec3 v, double& ret) override {
		return NULL;
	}
};

struct Torus : virtual public Object {
	Torus() {
		Xform = mat4(1.0);
		Xfmi = mat4(1.0);
	}

	Torus(mat4 xform, mat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

	double* quadratic(double A, double B, double C, double* ret) {//we pass a pointer in so this has some initlized memory in the calling function to set the final value too.
		return NULL;
	}

	double* calcIntersection(dvec3 u, dvec3 v, double& ret) override {
		return NULL;
	}
};

struct Triangle : virtual public Object {
	Triangle() {
		Xform = mat4(1.0);
		Xfmi = mat4(1.0);
	}

	Triangle(mat4 xform, mat4 xfmi, Material m) {
		Xform = xform;
		Xfmi = xfmi;
		mat = m;
	}

	double* quadratic(double A, double B, double C, double* ret) {//we pass a pointer in so this has some initlized memory in the calling function to set the final value too.
		return NULL;
	}

	double* calcIntersection(dvec3 u, dvec3 v, double& ret) override {
		return NULL;
	}
};

struct Ray { //Stores data for Ray
	dvec4 start;//is the Eye
	dvec4 dir;
	Material mat;//this stores the return material
	double currentRefraction;
	bool isnorm;//this flag is false when using shadow rays

	Ray(vec3 vec) {
		start = dvec4(vec, 1.0);
		dir = dvec4(0.0, 0.0, 0.0, 0.0);
		isnorm = false;
		currentRefraction = defaultrefraction;
	}

	Ray(dvec3 s, dvec3 d) {
		start = dvec4(s, 1.0);
		dir = dvec4(d, 0.0);
		isnorm = false;
		currentRefraction = defaultrefraction;
	}

	Ray(dvec3 s, dvec3 d, bool isn) {
		start = dvec4(s, 1.0);
		dir = dvec4(d, 0.0);
		isnorm = isn;
		currentRefraction = defaultrefraction;
	}

	Ray(dvec3 s, dvec3 d, vec3 dif, vec3 spec) {
		start = dvec4(s, 1.0);
		dir = dvec4(d, 0.0);
		mat.diffuse = dif;
		mat.specular = spec;
		isnorm = false;
		currentRefraction = defaultrefraction;
	}

	void setTrue() {
		isnorm = true;
	}

	void normalize() {
		dir = dvec4(glm::normalize(dvec3(dir)), 0.0);
		isnorm = true;
	}

	void print() const{
		printf("Ray:\nStart: %f, %f, %f, %f\nDirection: %f, %f, %f, %f\n", start.x, start.y, start.z, start.w, dir.x, dir.y, dir.z, dir.w);
		mat.print();
	}
};

struct Hitpoint { //stores our hitpoint,normal at the hitpoint, and material(?)
	dvec3 hitpoint;
	dvec3 normal;

	Hitpoint() {
		hitpoint = dvec3(0.0);
		normal = dvec3(0.0);
	}

	Hitpoint(dvec3 h, dvec3 n) {
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

std::vector<Object*> objects;

float dot(const vec3 left, const vec3 right) {
	return left.x * right.x + left.y * right.y + left.z * right.z;
};

float vectorMag(const vec3* vec) {
	if (!vec)
		return 0.0;
	return abs(sqrt(pow(vec->x, 2) + pow(vec->y, 2) + pow(vec->z, 2)));
};

vec3* reflection(Hitpoint* hitpoint, Ray* ray) {
	/*used to find the color that should be here if it is a reflective object*/
	dvec3 start = hitpoint->hitpoint + (double(0.01) * hitpoint->normal);
	reflectcount++;

	if (reflectcount > 20) {//resets the count after we have gone through a previous thing
		reflectcount = 0;
	}

	if (!ray->isnorm)
		ray->normalize();
	
	Ray* temp = new Ray(start, dvec3(ray->dir) - 2 * dot(hitpoint->normal, dvec3(ray->dir)) * hitpoint->normal, true);

	vec3* Color = new vec3(ray->mat.specular * trace(temp));

	if (vectorMag(Color) < .01) {//this resets the count after we have gone through it
		reflectcount = 0;
		return NULL;
	}

	return Color;
};

vec3* refraction(Hitpoint* hitpoint, Ray* ray){
	//used to find the color that should be here if it is a rafractive object
	dvec3 start = hitpoint->hitpoint;//we move the hitpoint into the object to guarentee that the first hit is not with the object itself
	double ni = dot(hitpoint->normal, -ray->dir); 
	double eta = 0;

	if (ray->currentRefraction == defaultrefraction)//outside
		eta = ray->currentRefraction / ray->mat.index;
	else//inside
		eta = ray->currentRefraction / defaultrefraction;

	refractcount++;

	if (refractcount > 10) {//this resets the count after we have gone through it
		refractcount = 0;
	}

	double rad = 1 - (eta * eta)*(1 - (ni * ni));
	if (rad < 0) {//total internal refraction
		return NULL;
	}

	//print(start);

	Ray* temp = new Ray(start, eta*ni - sqrt(rad)*hitpoint->normal - eta * dvec3(ray->dir), true);
	//print(eta*ni - sqrt(rad)*hitpoint->normal + eta * dvec3(ray->dir));

	if (ray->currentRefraction == defaultrefraction)
		temp->currentRefraction = ray->mat.index;
	else
		temp->currentRefraction = defaultrefraction;

	vec3* Color = new vec3(ray->mat.refraction * trace(temp));

	if (vectorMag(Color) < .01) {
		refractcount = 0;
		return NULL;
	}

	return Color;
};

vec3 PhongIllumination(const Hitpoint* point,const Ray* ray,const Light* light) {//need to fix this, currently ambiant is added for each light, incorrect
	/*Calculate the phong illumination model*/
	
	dvec3 lightDir = normalize(dvec3(light->position) - point->hitpoint);
	dvec3 reflectDir = -lightDir - 2 * dot(point->normal, -lightDir)*point->normal;
	dvec3 viewDir = normalize(dvec3(ray->start) - point->hitpoint);

	float lambertian = max(dot(lightDir, point->normal), double(0.0));
	float specular = 0.0;

	if (lambertian > 0.0) {
		float specAngle = max(dot(reflectDir, viewDir), double(0.0));
		specular = pow(specAngle, ray->mat.emissivity);
	}
	return light->color * vec3((lambertian * ray->mat.diffuse) + (specular * ray->mat.specular));
};

bool Shadow(Hitpoint* point,const Ray* ray,const Light* light) {
	/*Check between the object and the light to see if this new ray intersects another object*/
	Ray lightRay(point->hitpoint + .01 * point->normal, dvec3(light->position) - point->hitpoint);
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
		if (!Shadow(hitpoint, ray, &light))
			Color = Color + PhongIllumination(hitpoint, ray, &light);
	}

	if (ray->mat.emissivity >= 0) {
		if (reflectcount <= 20) {
			vec3* space = reflection(hitpoint, ray);
			if(space)
				Color = Color + *space;
		}
	}

	if (ray->mat.index > defaultrefraction){
		if (refractcount <= 10) {
			vec3* space = refraction(hitpoint, ray);
			if (space) //This checks to see that something did not happen in rafract to not return a value
				Color = Color + *space;
			else
				refractcount = 0;
		}
	}
		

	if (Color.x > 1.5 || Color.y > 1.5 || Color.z > 1.5) { //std::cerr << "Error: light over 150% at some pixel\n"; print(Color);
	}
	return Color;
};

Hitpoint* closestIntersection(Ray *ray, Hitpoint* ret) {// return the intersection point, surface normal, surface, surface attributes, etc.
	raycount++;//this is here because shadow doenst call trace, we dont care about the color, but it does technically shoot a ray.
	double t;//some initlized memory to point our pointers to, has to be here because of scope
	double* current = NULL;
	double* smallest = NULL;
	
	for (auto temp = objects.begin(); temp < objects.end(); temp++) {
		Object* object = *temp;
		current = object->calcIntersection(vec3(object->Xfmi * ray->start), vec3(object->Xfmi * ray->dir), t);//we "downcast" these vectors so that we can take the proper dot product of them
		if (current) {//We hit an object
			if (!ray->isnorm) {//if we are checking a shadow ray
				if (*current < 1) return ret = new Hitpoint;//if we hit an object, just return any value
			}
			else {//for any other ray, we need to know what we hit
				if (!smallest) { //If this is the first time we intersect an object
					smallest = new double;
					*smallest = *current;
					ret = new Hitpoint;
					ret->hitpoint.x = ray->start.x + (*smallest * ray->dir.x);
					ret->hitpoint.y = ray->start.y + (*smallest * ray->dir.y);
					ret->hitpoint.z = ray->start.z + (*smallest * ray->dir.z);
					ret->normal = vec3(transpose(object->Xfmi) * vec4(vec3(object->Xfmi * vec4(ret->hitpoint, 1.0)), 0.0));
					ray->mat = object->mat;
				} 
				else {
					if (*current < *smallest) { //If the new intersection is less then the current stored, change the current
						*smallest = *current;
						ret->hitpoint.x = ray->start.x + (*smallest * ray->dir.x);
						ret->hitpoint.y = ray->start.y + (*smallest * ray->dir.y);
						ret->hitpoint.z = ray->start.z + (*smallest * ray->dir.z);
						ret->normal = dvec3(transpose(object->Xfmi) * dvec4(dvec3(object->Xfmi * dvec4(ret->hitpoint, 1.0)), 0.0));
						ray->mat = object->mat;
					}
				}
			}
		}
	}
	//ray->mat.print();
	//std::cout << ray->currentRefraction << "\n";

	if (!smallest)//nothing was hit
		return NULL;
	ret->normal = normalize(ret->normal);
	ret->hitpoint = ret->hitpoint + (double(0.01) * ret->normal);
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

	vector<dmat4> CTM;
	vector<dmat4> CIM;
	vector<Material> MAT;

	CTM.push_back(dmat4(1.0));
	CIM.push_back(dmat4(1.0));
	MAT.push_back(Material());

	for (auto i = tokens.begin(); i < tokens.end(); i++) {
		//std::cout << *i << " ";
		if ("#" == *i);// { std::cout << "\n"; }
		else if ("view" == *i) {
			n = stoi(*(i + 1));
			d = stof(*(i + 2));
			//printf("%i %f\n", n,d);
			i += 2;
		}
		else if ("scale" == *i) {
			double arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stod(*(i + j + 1));
			}
			//printf("%f %f %f\n", arr[0], arr[1], arr[2]);
			CTM[level] = CTM[level] * scale(dmat4(1.0), dvec3(arr[0], arr[1], arr[2]));
			CIM[level] = inverse(scale(dmat4(1.0), dvec3(arr[0], arr[1], arr[2]))) * CIM[level];
			//print(CIM[level]);
			i += 3;
		}
		else if ("move" == *i) {
			double arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stod(*(i + j + 1));
			}
			//printf("%f %f %f\n", arr[0], arr[1], arr[2]);
			CTM[level] = CTM[level] * translate(dmat4(1.0), dvec3(arr[0], arr[1], arr[2]));
			CIM[level] = inverse(translate(dmat4(1.0), dvec3(arr[0], arr[1], arr[2]))) * CIM[level];
			//print(CIM[level]);
			i += 3;
		}
		else if ("rotate" == *i) {
			double arr[4];
			for (int j = 0; j < 4; j++) {
				arr[j] = stod(*(i + j + 1));
			}
			//printf("%f %f %f %f\n", arr[0], arr[1], arr[2], arr[3]);
			CTM[level] = CTM[level] * rotate(dmat4(1.0),radians(arr[0]), dvec3(arr[1], arr[2], arr[3]));
			CIM[level] = inverse(rotate(dmat4(1.0), radians(arr[0]), dvec3(arr[1], arr[2], arr[3]))) * CIM[level];
			//print(CIM[level]);
			i += 4;
		}
		else if ("sphere" == *i) {
			Sphere* object = new Sphere(CTM[level], CIM[level], MAT[level]);
			//object->print();
			objects.push_back(object); //this needs to call some sort of new operator to allocate memory for the thingy
			//cout << "\n";
		}
		else if ("cylinder" == *i) {
			Cylinder* object = new Cylinder(CTM[level], CIM[level], MAT[level]);
			//object->print();
			objects.push_back(object); //this needs to call some sort of new operator to allocate memory for the thingy
			//cout << "\n";
		}
		else if ("cone" == *i) {
			Cone* object = new Cone(CTM[level], CIM[level], MAT[level]);
			//object->print();
			objects.push_back(object); //this needs to call some sort of new operator to allocate memory for the thingy
			//cout << "\n";
		}
		else if ("paraboloid" == *i) {
			Paraboloid* object = new Paraboloid(CTM[level], CIM[level], MAT[level]);
			//object->print();
			objects.push_back(object); //this needs to call some sort of new operator to allocate memory for the thingy
			//cout << "\n";
		}
		else if ("hyperboloid" == *i) {
			Hyperboloid* object = new Hyperboloid(CTM[level], CIM[level], MAT[level]);
			//object->print();
			objects.push_back(object); //this needs to call some sort of new operator to allocate memory for the thingy
			//cout << "\n";
		}
		else if ("torus" == *i) {
			Torus* object = new Torus(CTM[level], CIM[level], MAT[level]);
			//object->print();
			objects.push_back(object); //this needs to call some sort of new operator to allocate memory for the thingy
			//cout << "\n";
		}
		else if ("triangle" == *i) {
			Triangle* object = new Triangle(CTM[level], CIM[level], MAT[level]);
			//object->print();
			objects.push_back(object); //this needs to call some sort of new operator to allocate memory for the thingy
			//cout << "\n";
		}
		else if ("light" == *i) {
			float arr[6];
			for (int j = 0; j < 6; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			//printf("%f %f %f %f %f %f\n", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
			lights.push_back(Light(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]));
			i += 6;
		}
		else if ("background" == *i) {
			float arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			//printf("%f %f %f\n", arr[0], arr[1], arr[2]);
			backgroundColor = vec3(arr[0], arr[1], arr[2]);
			i += 3;
		}
		else if ("ambient" == *i) {
			float arr[3];
			for (int j = 0; j < 3; j++) {
				arr[j] = stof(*(i + j + 1));
			}

			//printf("%f %f %f\n", arr[0], arr[1], arr[2]);
			ambiant = vec3(arr[0], arr[1], arr[2]);
			i += 3;
		}
		else if ("refraction" == *i) {
			float arr[4];
			for (int j = 0; j < 4; j++) {
				arr[j] = stod(*(i + j + 1));
			}

			//printf("%f %f %f %f\n", arr[0], arr[1], arr[2], arr[3]);
			MAT[level].refrac(arr[0], arr[1], arr[2], arr[3]);
			i += 4;
		}
		else if ("material" == *i) {
			float arr[7];
			for (int j = 0; j < 7; j++) {
				arr[j] = stof(*(i + j + 1));
			}
			//printf("%f %f %f %f %f %f %f\n", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6]);
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
			
			//std::cout << "\n";
		}
		else if ("groupend" == *i) {//should move back up one level of the scene graph
			if (level == 0) cerr << "Error: called groupend without calling group";
			else
				level--;
			//std::cout << "\n";
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
	//std::cout << "Allocated Pixels array\n";

	pw = 2 * d / n;
	ph = 2 * d / n;

	Ray current(u);
	//printVec(&u);
	//printVec(&v);
	auto t1 = Clock::now();
	for (int j = n-1; j >= 0; j--) {
		v.y = d - ph * j - ph / 2;//calculate midpoint for row
		//std::cout << v.y << "\n";
		for (int i = n-1; i >= 0; i--) {
			v.x = d - ph * i - ph / 2;//calculate midpoint for column
			//std::cout << v.x << "\n";
			current.dir = dvec4(v - u, 0.0);
			current.currentRefraction = defaultrefraction;//this is to make sure it is the right value
			current.normalize();
			vec3 Color = trace(&current);
			Pixels[i][j][0] = Color.r;
			Pixels[i][j][1] = Color.g;
			Pixels[i][j][2] = Color.b;
		}
	}
	auto t2 = Clock::now();
	std::cout << "Sucessfully shot all rays " << raycount << "\n";
	int count = 0;

	unsigned char r, g, b;
	std::ofstream picfile;
	picfile.open("out.ppm",std::ofstream::binary);
	picfile << "P6# " << n << "x" << n << " Raytracer output\n" << n << " " << n << "\n255\n";
	// For each pixel
	for (int j = 0; j <= n-1; j++) {// Y is flipped!
		for (int i = n-1; i >= 0; i--) {
			r = int(clamp(Pixels[i][j][0], float(0), float(1.0)) * 255);
			g = int(clamp(Pixels[i][j][1], float(0), float(1.0)) * 255);
			b = int(clamp(Pixels[i][j][2], float(0), float(1.0)) * 255);
			picfile << r << g << b;
			//std::cout << int(Pixels[i][j][0] * 255) << " " << int(Pixels[i][j][1] * 255) << " " << int(Pixels[i][j][2] * 255) << " \n";
			// Remember though that this is a number between 0 and 255
			// so might have to convert from 0-1.
			count++;
		}
	}

	std::cout << "Sucessfully wrote to the PPM " << count << "\n";

	std::cout << "Delta t2-t1: " 
		<< std::chrono::duration_cast<std::chrono::minutes>(t2 - t1).count()
		<< " minutes, "
		<< std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
		<< " seconds, "
		<< std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
		<< " nanoseconds" << std::endl;

	picfile.close();
	return 0;
}
