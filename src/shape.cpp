#include <shape.h>

using namespace help;

Shape::Shape(std::shared_ptr<Vec3<float>> sPosition, std::shared_ptr<Vec3<float>> sColour, std::shared_ptr<Vec3<float>> sRotation , Shapes type) {
    this->position = sPosition;
    this->diffuseColour = sColour;
    this->specularColour = sColour;
    this->rotation = sRotation;
    this->shape = type;
    this->shininess = 0.0;
    this->transparency = 0.0;
	initShapedata();
}

Shape::Shape(Shapes type) {
	this->position = std::make_shared<Vec3<float>>(0,0,0);
    this->diffuseColour = std::make_shared<Vec3<float>>(0.17,0.17,0.19);
    this->specularColour = std::make_shared<Vec3<float>>(0.17,0.17,0.19);
    this->rotation = std::make_shared<Vec3<float>>(0,0,0);
    this->shape = type;
    this->shininess = 0.0;
    this->transparency = 0.0;
    initShapedata();
}

Shape::Shape() {
    this->position = std::make_shared<Vec3<float>>(0,0,0);
    this->diffuseColour = std::make_shared<Vec3<float>>(0.17,0.17,0.19);
    this->specularColour = std::make_shared<Vec3<float>>(0.17,0.17,0.19);
    this->rotation = std::make_shared<Vec3<float>>(0,0,0);
    this->shininess = 0.0;
    this->transparency = 0.0;
    this->shape = Shapes::UNIDENTFIED;
}

void Shape::setPosition(std::shared_ptr<Vec3<float>> newpos) {
    position = newpos;
}

void Shape::setPosition(float x, float y, float z) {
    this->position = std::make_shared<Vec3<float>>(x,y,z);
}

void Shape::movePosition(std::shared_ptr<Vec3<float>> movement) {
    *this->position += *movement;
}

void Shape::setRotation(float x, float y, float z) {
    this->rotation = std::make_shared<Vec3<float>>(x,y,z);
}

void Shape::setRotation(std::shared_ptr<Vec3<float>> newrot) {
    this->rotation = newrot;
}

void Shape::moveRotation(std::shared_ptr<Vec3<float>> movement) {
    *(this->rotation) += *movement;
}

void Shape::setBoxdata(float x, float y, float z) {
    if (shape == Shapes::BOX) {
        if (shapedata.box == nullptr) {
            shapedata.box = new BOX_STRUCT();
        }
        this->shapedata.box->width.x = x;
        this->shapedata.box->width.y = y;
        this->shapedata.box->width.z = z;

    } else {
        std::cerr << "ERROR: Setting box data for non box object.\n";
    }
}

void Shape::setSpheredata(float radius) {
    if (shape == Shapes::SPHERE) {
        if (shapedata.sphere == nullptr) {
            shapedata.sphere = new SPHERE_STRUCT();
        }
        this->shapedata.sphere->radius = radius;
        this->shapedata.sphere->radius2 = radius * radius;
    } else {
        std::cerr << "ERROR: Setting sphere data for non sphere object.\n";
    }
}

void Shape::setCameradata(int x, int y, float f) {
    if (shape == Shapes::CAMERA) {
        if (shapedata.camera == nullptr) {
            shapedata.camera = new CAMERA_STRUCT();
        }
        this->shapedata.camera->xRes = x;
        this->shapedata.camera->yRes = y;
        this->shapedata.camera->fov = f;
    }
}

void Shape::setLightdata(float i) {
    if (shapedata.light != nullptr) {
        this->shapedata.light->intensity = i;
    } else {
        if (shape != Shapes::LIGHT) 
            std::cerr << "Cant set light data for non light object\n";
        else 
            std::cerr << "Cant set light data for uninitialised light object\n";
    }
}

void Shape::setSpotlightdata(Vec3<float> at, Vec3<float> dir, float rad, float bw, float coa) {
    if (shape == Shapes::LIGHT) {
        if (shapedata.light != nullptr) {
            (*shapedata.light).attenuation = at;
            (*shapedata.light).direction = dir;
            (*shapedata.light).radius = rad;
            (*shapedata.light).beamWidth = std::min(bw,(float)1.57079632);
            (*shapedata.light).cutOffAngle = std::min(coa,(float)1.57079632);
        } else {
            std::cerr << "Cant set spotlight data for uninitialised light object\n";
        }
    }
}

void Shape::setDirectionallightdata(Vec3<float> dir) {
    if (shape == Shapes::LIGHT) {
        if (shapedata.light != nullptr) {
            (*shapedata.light).direction = dir;
        } else {
            std::cerr << "Cant set directionallight data for uninitialised light object\n";
        }
    }
}

void Shape::setPointlightdata(Vec3<float> at, float rad) {
    if (shape == Shapes::LIGHT) {
        if (shapedata.light != nullptr) {
            (*shapedata.light).attenuation = at;
            (*shapedata.light).radius = rad;
        } else {
            std::cerr << "Cant set pointlight data for uninitialised light object\n";
        }
    }
}

void Shape::setTriangleIndexes(int numtri, std::shared_ptr<std::vector<int>> vertexIndexArray) {
    if (shape == Shapes::TRIANGLEMESH) {
        if (shapedata.triangles == nullptr) {
            shapedata.triangles = new TRIANGLEMESH_STRUCT();
        }
        this->shapedata.triangles->numtri = numtri;
        this->shapedata.triangles->vertexIndexArray = vertexIndexArray;
    }
}

void Shape::setTriangleCoords(std::shared_ptr<std::vector<Vec3<float>>> position) {
    if (shape == Shapes::TRIANGLEMESH) {
        if (shapedata.triangles == nullptr) {
            shapedata.triangles = new TRIANGLEMESH_STRUCT();
        }
        this->shapedata.triangles->position = position;
    }
}

void Shape::addVertexNormals(std::shared_ptr<std::vector<Vec3<float>>> norms) {
    if (shape == Shapes::TRIANGLEMESH) {
        if (shapedata.triangles == nullptr) {
            shapedata.triangles = new TRIANGLEMESH_STRUCT();
        }
        if (this->shapedata.triangles->smoothShading == true)
            this->shapedata.triangles->vertexNormals = norms;
        else
            this->shapedata.triangles->faceNormals = norms;
    }
}

void Shape::makeTriangleNormals(bool smoothshading) {
    if(shape == Shapes::TRIANGLEMESH && shapedata.triangles != nullptr) {
        //Initialise some pointers and the output vector
        std::vector<Vec3<float>> faceNormalTemp;
        std::shared_ptr<std::vector<int>> indexes = this->shapedata.triangles->vertexIndexArray; 
        std::shared_ptr<std::vector<Vec3<float>>> coords = this->shapedata.triangles->position;
        faceNormalTemp.resize(this->shapedata.triangles->numtri);
        Vec3<float> A, B, C;
        for(int i = 0; i < faceNormalTemp.size(); i++) {
            A = (*coords)[(*indexes)[i*3]];
            B = (*coords)[(*indexes)[i*3+1]];
            C = (*coords)[(*indexes)[i*3+2]];
            // std::cout << A << " " << B << " " << C << std::endl;
            faceNormalTemp[i] = ((B - A).crossProduct(C - A));
            faceNormalTemp[i].normalize();
            // std::cout << faceNormalTemp[i] << std::endl;
        }
        this->shapedata.triangles->faceNormals = std::make_shared<std::vector<Vec3<float>>>(faceNormalTemp);
        // Make vertex normals
        // std::cout << "MAKING VERTEX NORMALS" << std::endl;
        std::vector<Vec3<float>> vertexNormalTemp = std::vector<Vec3<float>>();
        std::vector<int> faceNormalIndexes = std::vector<int>();
        vertexNormalTemp.resize(coords->size());
        // std::cout << vertexNormalTemp.size() << "\n";
        for(int i = 0; i < vertexNormalTemp.size(); i++) {
            faceNormalIndexes = getTriangleIndexes(i);
            Vec3<float> sum = Vec3<float>(0,0,0);
            // std::cout << "Vertex " << i << std::endl;
            for (int j = 0; j < faceNormalIndexes.size(); j++) {
                // std::cout << "\ti:" << faceNormalIndexes[j] << " : " << faceNormalTemp[faceNormalIndexes[j]] << " sum : " << sum << std::endl;;
                sum += faceNormalTemp[faceNormalIndexes[j]];
            }
            if (faceNormalIndexes.size() > 0)
                vertexNormalTemp[i] = sum / faceNormalIndexes.size();
            else 
                vertexNormalTemp[i] = sum;
            vertexNormalTemp[i].normalize();
            // std:: cout << "\t\t" << vertexNormalTemp[i] << std::endl;
        }
        this->shapedata.triangles->vertexNormals = std::make_shared<std::vector<Vec3<float>>>(vertexNormalTemp);
    }
}

std::vector<int> Shape::getTriangleIndexes(int x) {
    std::shared_ptr<std::vector<int>> indexes = this->shapedata.triangles->vertexIndexArray;
    std::vector<int> ret = std::vector<int>();
    //std::cout << indexes->size() << "\n";
    for(int i = 0; i < indexes->size(); i++) {
        if ((*indexes)[i] == x) {
            ret.push_back((int)floor(i/3));
        }
        //std::cout << i << "\n";
    }
    return ret;
}

void Shape::setDColour(float r, float g, float b) {
    this->diffuseColour = std::make_shared<Vec3<float>>(r,g,b);
}

void Shape::setDColour(std::shared_ptr<Vec3<float>> newcol) {
    this->diffuseColour = newcol;
}

void Shape::setSColour(float r, float g, float b) {
    this->specularColour = std::make_shared<Vec3<float>>(r,g,b);
}

void Shape::setSColour(std::shared_ptr<Vec3<float>> newcol) {
    this->specularColour = newcol;
}        

void Shape::setShininess(float s) {
    this->shininess = s;
}

void Shape::setTransparency(float t) {
    this->transparency = t;
}

void Shape::setType(Shapes type) {
	clearShapedata();
    this->shape = type;
    initShapedata();
}

void Shape::setType(LightTypes type) {
    clearShapedata();
    this->shape = Shapes::LIGHT;
    initShapedata(type);
}

void Shape::setShadingType(bool smooth) {
    if (this->shape == Shapes::TRIANGLEMESH) {
        this->shapedata.triangles->smoothShading = smooth;
    }
}

void Shape::clearShapedata() {
	switch (shape) {
		case Shapes::SPHERE:
	        delete shapedata.sphere;
			break;
        case Shapes::BOX:
			delete shapedata.box;
			break;
		case Shapes::CAMERA:
			delete shapedata.camera;
			break;
        case Shapes::TRIANGLEMESH:
            delete shapedata.triangles;
            break;
        case Shapes::LIGHT:
            delete shapedata.light;
		default:
			break;
	}
}

void Shape::initShapedata() {
	switch (shape) {
		case Shapes::SPHERE:
			shapedata.sphere = new SPHERE_STRUCT();
            break;
		case Shapes::BOX:
			shapedata.box = new BOX_STRUCT();
			break;
		case Shapes::CAMERA:
			shapedata.camera = new CAMERA_STRUCT();
            break;
        case Shapes::TRIANGLEMESH:
            shapedata.triangles = new TRIANGLEMESH_STRUCT();
            break;
		default:
			std::cerr << "Cannot initialise unidentified type " << (int)shape;
			break;
	}
}

void Shape::initShapedata(LightTypes l) {
    if (shape == Shapes::LIGHT)
        shapedata.light = new Light(l);
}

bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = - 0.5 * b / a;
    else {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) std::swap(x0, x1);

    return true;
}

bool Shape::intersectSphere(RAY_STRUCT<float> ray, float &t) {
    float t0, t1; // Solutions for t if the ray intesects
    //Find vector from ray to centre of circle
    Vec3<float> L = ray.point - (*this->position);
    float a = ray.nvector.dotProduct(ray.nvector);
    float b = 2 * ray.nvector.dotProduct(L);
    float c = L.dotProduct(L) - this->shapedata.sphere->radius2;
    if(!solveQuadratic(a,b,c,t0,t1)) {
        return false;
    }

    if (t0 > t1) {
        std::swap(t0,t1);
    }
    if (t0 < 0) {
        t0 = t1; //If t0 is negative then it's behind the camera so pointless
        if (t0 < 0) {
            //std::cout << "Shape is behind camera";
            return false; //If they're both behind the camera then no intersect!
        }
    }
    t = t0;
    return true;
}

bool Shape::intersectTriangle(RAY_STRUCT<float> ray, Vec3<float> p1, Vec3<float> p2, Vec3<float> p3, float &t, float &u, float &v) {
    const float kEpsilon = 0.00001;
    Vec3<float> AB = (p2) - (p1);
    // std::cout << "AB " << AB.toString() << std::endl;
    Vec3<float> AC = (p3) - (p1);
    // std::cout << "AC " << AC.toString() << std::endl;
    Vec3<float> pVec =  ray.nvector.crossProduct(AC);
    // std::cout << "pVec " << pVec.toString() << std::endl;
    float det = AB.dotProduct(pVec);
    // std::cout << "det " << det << std::endl;
    //Ray and triangle are parallel if det is close to 0
    if (fabs(det) < kEpsilon && fabs(det) > -kEpsilon) return false;
    float invDet = 1 / det; 
    // std::cout << "invDet " << invDet << std::endl;
    Vec3<float> tvec = ray.point - p1;
    // std::cout << "tvec " << tvec.toString() << std::endl;
    u = (tvec.dotProduct(pVec)) * invDet;
    // std::cout << "u " << u << std::endl;
    if (u < 0.0 || u > 1.0 ) return false;
    Vec3<float> qvec = tvec.crossProduct(AB);
    // std::cout << "qvec " << qvec << std::endl;
    v = ray.nvector.dotProduct(qvec) * invDet;
    // std::cout << "v " << v << std::endl;
    // std::cout << p1.toString() << " " << p2.toString() << " " << p3.toString() << "\n" << AB.toString() << " " << AC.toString() << std::endl;
    if (v < 0.0 || u + v > 1.0) return false;

    float tempt = AC.dotProduct(qvec) * invDet;
    if (tempt < kEpsilon) {
        return false;
    } else {
        t = tempt;
        return true;  
    }
}

bool Shape::intersectTriangleMesh(RAY_STRUCT<float> ray, float &t, std::shared_ptr<int> triIndex, Vec2<float> &uv) {
    std::shared_ptr<std::vector<Vec3<float>>> coords = this->shapedata.triangles->position;
    std::shared_ptr<std::vector<int>> indexes = this->shapedata.triangles->vertexIndexArray;
    if (coords == nullptr || indexes == nullptr ) {
        std::cerr << "coords or indexes are null" << std::endl;
    }

    Vec3<float> A;
    Vec3<float> B;
    Vec3<float> C;
    bool ret = false;
    float currentSmallestDistance = std::numeric_limits<float>::infinity();

    //std::cout << indexes->size() << std::endl; 
    //std::cout << this->shapedata.triangles->numtri << std::endl;
    //std::cout << (*indexes)[0] << std::endl;
    for (int i = 0; i < this->shapedata.triangles->numtri; i++) {
        A = (*coords)[(*indexes)[i*3]];
        B = (*coords)[(*indexes)[i*3+1]];
        C = (*coords)[(*indexes)[i*3+2]];
        float u = 0.0,v = 0.0;
        if (intersectTriangle(ray,A,B,C,t,u,v)){
            if (t < currentSmallestDistance) {
                currentSmallestDistance = t;
                ret = true;
                *triIndex = i;
                uv.x = u;
                uv.y = v;
                // if (ray.x == 950 && ray.y == 530)
                //     std::cout << "Current distance: " << currentSmallestDistance << " Index " << *triIndex << std::endl;
            }
        }
    }
    // if (ret == true && ray.x == 950 && ray.y == 530)
    //     std::cout << "\t" << currentSmallestDistance << " Index " << *triIndex << std::endl;
    t = currentSmallestDistance;
    return ret;
}

bool Shape::intersectLight(RAY_STRUCT<float> ray, float &t) {
    return false;
    //@todo stuff
}

bool Shape::intersect(RAY_STRUCT<float> ray, float &t, std::shared_ptr<int> triIndex, Vec2<float> &uv) {
    switch (shape) {
    case Shapes::SPHERE:
        return intersectSphere(ray, t);
        break;
    case Shapes::TRIANGLEMESH:
        return intersectTriangleMesh(ray, t, triIndex, uv);
        break;
    case Shapes::LIGHT:
        //return intersectLight(ray , t); 
    default:
        return false;
        break;
    }
}

void Shape::getSurfaceData(RAY_STRUCT<float> ray, Vec3<float> pointHit, int &triIndex, Vec2<float> &uv, Vec3<float> &hitNormal) {
    Vec3<float> A;
    Vec3<float> B;
    Vec3<float> C;
    std::shared_ptr<std::vector<Vec3<float>>> coords;
    std::shared_ptr<std::vector<Vec3<float>>> norms;
    std::shared_ptr<std::vector<int>> indexes;
    switch (this->shape) {
    case Shapes::SPHERE:
        //std::cout << (*this->position) << "\t" << pointHit.toString() << "\n";
        hitNormal = (pointHit - (*this->position));
        hitNormal.normalize();
        break;
    case Shapes::TRIANGLEMESH:
        indexes = this->shapedata.triangles->vertexIndexArray;
        if (!this->shapedata.triangles->smoothShading) {
            // coords = this->shapedata.triangles->position;
            // A = (*coords)[(*indexes)[triIndex*3]];
            // B = (*coords)[(*indexes)[triIndex*3+1]];
            // C = (*coords)[(*indexes)[triIndex*3+2]];
            // hitNormal = (B-A).crossProduct(C-A);
            hitNormal = (*this->shapedata.triangles->faceNormals)[triIndex];
        } else {
            //Smooth shading based on Gourad's technique (interpolating vertex normals)
            norms = this->shapedata.triangles->vertexNormals;
            A = (*norms)[(*indexes)[triIndex*3]];
            B = (*norms)[(*indexes)[triIndex*3+1]];
            C = (*norms)[(*indexes)[triIndex*3+2]];
            hitNormal = (1 - uv.x - uv.y) * A + uv.x * B + uv.y * C;
        }
        break;
    default:
        break;
    }
    hitNormal.normalize();  
}

std::string Shape::toString(std::string tabs) {
    std::string ret = "\033[1;34m";
    switch (this->shape) {
    case Shapes::BOX:
        ret += tabs + "Box:\n";
        ret += tabs + "\033[0m";
        ret += "Size: " + boost::lexical_cast<std::string>(this->shapedata.box->width.x);
        break;
    case Shapes::SPHERE:
        ret += tabs + "Sphere:\n";
        ret += tabs + "\033[0m";
        ret += "Radius: " + boost::lexical_cast<std::string>(this->shapedata.sphere->radius);
        break;
    case Shapes::CONE:
        ret += tabs + "Cone\n";
        ret += tabs + "\033[0m";
        break;
    case Shapes::CYLINDER:
        ret += tabs + "Cylinder\n";
        ret += tabs + "\033[0m";
        break;
    case Shapes::PYRAMID:
        ret += tabs + "Pyramid\n";
        ret += tabs + "\033[0m";
        break;
    case Shapes::DOME:
        ret += tabs + "Dome\n";
        ret += tabs + "\033[0m";
        break;
    case Shapes::CAMERA:
        ret += tabs + "Camera";
        ret += tabs + "\033[0m";
        break;
    case Shapes::TRIANGLEMESH:
        ret += tabs + "TriangleMesh:\n";
        ret += tabs + "\033[0m";
        ret +="Triangles: " + boost::lexical_cast<std::string>(this->shapedata.triangles->numtri);
        if(this->shapedata.triangles->position != nullptr) {
            // ret += "\nCoordinates: " + boost::lexical_cast<std::string>(this->shapedata.triangles->position->size());
        }
        if(this->shapedata.triangles->vertexNormals != nullptr) {
            // ret += "\nVertexNormals Size " + boost::lexical_cast<std::string>(this->shapedata.triangles->vertexNormals->size());
        }
        if(this->shapedata.triangles->faceNormals != nullptr) {
            // ret += "\nFaceNormals Size " + boost::lexical_cast<std::string>(this->shapedata.triangles->faceNormals->size());
        }
        ret += this->shapedata.triangles->toString(tabs);
        break;
    case Shapes::LIGHT:
        ret += tabs + "Light:\n";
        ret += "\033[0m";
        ret += tabs + this->shapedata.light->toString();
        break;
    default:
        return tabs + "Unidentified object";
    }
    ret += "\n" + tabs + "Position: " + this->position->toString() +
            " Rotation: " + this->rotation->toString() +
            "\n" + tabs + "Diffuse Colour: " + this->diffuseColour->toString() +
            " Specular Colour: " + this->specularColour->toString() +
            "\n" + tabs + "Shininess: " + boost::lexical_cast<std::string>(shininess) +
            " Width: " + getWidth().toString() +
            "\n";
    return ret;
}

Shapes Shape::getType() {
    return this->shape;
}

bool Shape::getShadingType() {
    if (this->shape == Shapes::TRIANGLEMESH) {
        return this->shapedata.triangles->smoothShading; 
    } else {
        return true;
    }
}

BOX_STRUCT Shape::getBoxdata() {
    if (this->shape == Shapes::BOX) {
        return *(this->shapedata.box);
    } else {
        std::cerr << "ERROR: Getting box data for non-box object";
    }
}

SPHERE_STRUCT Shape::getSpheredata() {
    if (this->shape == Shapes::SPHERE) {
        return *(this->shapedata.sphere);
    } else {
        std::cerr << "ERROR: Getting Sphere data for non-sphere object";
    }
}

CAMERA_STRUCT Shape::getCameradata() {
    if (this->shape == Shapes::CAMERA) {
        return *(this->shapedata.camera);
    } else {
        std::cerr << "ERROR: Getting Camera data for non-camera object";
    }
}

int Shape::getNTris() {
    if (this->shape == Shapes::TRIANGLEMESH) {
        return this->shapedata.triangles->numtri;
    } else {
        std::cerr << "ERROR: getting numtris from non trianglemesh object";
    }
}

Light Shape::getLightdata() {
    if (this->shape == Shapes::LIGHT) {
        return *(this->shapedata.light);
    } else {
        std::cerr << "ERROR: Getting Light data for non-Light object";
    }
}

void Shape::setTriwidth() {
    const float kEpsilon = 0.000001;
    Vec3<float> temp = Vec3<float>(0.0);
    Vec3<float> largest = Vec3<float>(0.0);
    for(int i = 0; i < this->shapedata.triangles->position->size(); i++) {
        temp = ((*this->shapedata.triangles->position)[i] - *position).abs();
        if (temp.x - largest.x > kEpsilon) largest.x = temp.x;
        if (temp.y - largest.y > kEpsilon) largest.y = temp.y;
        if (temp.z - largest.z > kEpsilon) largest.z = temp.z;
        // if (temp > 5 || temp < -5) std::cout << temp << "  " << i << std::endl;
    }
    this->shapedata.triangles->width = largest;
}

Vec3<float> Shape::getWidth() {
    switch (this->shape) {
    case Shapes::BOX:
        return this->shapedata.box->width;
        break;
    case Shapes::SPHERE:
        return this->shapedata.sphere->radius;
        break;
    case Shapes::TRIANGLEMESH:
    	return this->shapedata.triangles->width;
	    break;
    default:
        return Vec3<float>(0.0);
    }
}

std::shared_ptr<Vec3<float>> Shape::getPos() {
    return position;
}

std::shared_ptr<Vec3<float>> Shape::getDColour() {
    return diffuseColour;
}

std::shared_ptr<Vec3<float>> Shape::getSColour() {
    return specularColour;
}

float Shape::getShininess() {
    return shininess;
}

float Shape::getTransparency() {
    return transparency;
}

Shape::~Shape() {
    clearShapedata();
}
