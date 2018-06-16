#ifndef STRUCTS_H
#define STRUCTS_H

#include <fstream>
#include <iostream>
#include <tuple>
#include <memory>
#include <cmath>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <chrono>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <pthread.h>
#include <unordered_map>


namespace help {

//typedef std::tuple<int,int,int> t3int;
//typedef std::tuple<float,float,float> t3float;
//template<typename T> using t3 = std::tuple<T,T,T>


template<typename T>
class Vec3 {
    public:
        Vec3() : x(0), y(0), z(0) {};
        Vec3(T k) : x(k), y(k), z(k) {};
        Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {};
        template<typename U> Vec3<T>(Vec3<U> v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)) {};
        Vec3 operator * (const T &r) const { return Vec3(x * r, y * r, z * r); }
        Vec3 operator * (const Vec3 &v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
        Vec3 operator - (const Vec3 &v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
        Vec3 operator + (const Vec3 &v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
        Vec3 operator / (const Vec3 &v) const { return Vec3(x / v.x, y / v.y, z / v.z); }
        Vec3 operator / (const T &v) const { return Vec3(x / v, y / v, z / v); }
        Vec3 operator - () const { return Vec3(-x, -y, -z); }
        Vec3& operator += (const Vec3 &v) { x += v.x, y += v.y, z += v.z; return *this; }
        bool operator > (const T &v) const { return ((x > v) || (y > v) || (z > v));}
        bool operator > (const Vec3 &v) const { return ((x > v.x) && (y > v.y) && (z > v.z)); }
        bool operator < (const T &v) const { return ((x < v) || (y < v) || (z < v));}
        bool operator < (const Vec3 &v) const { return ((x < v.x) && (y < v.y) && (z < v.z)); }
        bool operator == (const Vec3 &v) const { return (v.x == x) && (v.y == y) && (v.z && z); }
        bool operator == (const T &v) const { return (x == v) && (y == v) && (z == v); }
        bool operator != (const Vec3 &v) { return !((v.x == x) && (v.y == y) && (v.z && z)); }
        bool operator != (const T& v) { return !((x == v) && (y == v) && (z == v)); }
        void normalize() {
            T l2 = (x*x) + (y*y) + (z*z);
            if (l2 > 0) {
                T invL = 1 / std::sqrt(l2);
                x *= invL;
                y *= invL;
                z *= invL;
            }
        };
        std::string toString() {
            return "(" + boost::lexical_cast<std::string>(x) + "," + boost::lexical_cast<std::string>(y)
            + "," + boost::lexical_cast<std::string>(z) + ")";
        }
        bool between(const Vec3<T> low, const Vec3<T> high, const T kEpsilon) const {
            if (low.x - x > kEpsilon || x - high.x > kEpsilon) return false;
            if (low.y - y > kEpsilon || y - high.y > kEpsilon) return false;
            if (low.z - z > kEpsilon || z - high.z > kEpsilon) return false;
            return true;
        }
        void cap(const T &low,const T &high) {
            if (x < low) x = low;
            if (x > high) x = high;
            if (y < low) y = low;
            if (y > high) y = high;
            if (z < low) z = low;
            if (z > high) z = high;
        }
        void cap(const Vec3<T> &low, const Vec3<T> &high) {
            if (low.x > x) x = low.x;
            if (low.y > y) y = low.y;
            if (low.z > z) z = low.z;
            if (high.x < x) x = high.x;
            if (high.y < y) y = high.y;
            if (high.z < z) z = high.z;
        }
        Vec3<T> crossProduct(const Vec3<T> &v) const {
            return Vec3<T>(y * v.z - z * v.y,
                            z * v.x - x * v.z,
                            x * v.y - y * v.x);
        }
        T dotProduct(const Vec3<T> &v) const {
            return x * v.x + y * v.y + z * v.z;
        }
        T distance(const Vec3<T> &v) const {
            return (x * v.x) * (x * v.x) + (y * v.y) * (y * v.y) + (z * v.z) * (z * v.z);
        }
        T max() const {
            return std::max(x, std::max(y,z));
        }
        T min() const {
            return std::min(x, std::min(y,z));
        }
        Vec3<T> abs() const {
            return Vec3<T>(std::fabs(x),std::fabs(y),std::fabs(z));
        }

        friend Vec3 operator * (const float &r, const Vec3 &v) { return Vec3(v.x * r, v.y * r, v.z * r); }
        friend std::ostream & operator << (std::ostream &os, const Vec3 &v) { return os << v.x << ", " << v.y << ", " << v.z; }
        virtual ~Vec3() {};
        T x, y, z;
};

template<typename T>
class Vec2 {
    public:
        Vec2() : x(0), y(0) {};
        Vec2(T k) : x(k), y(k) {};
        Vec2(T xx, T yy) : x(xx), y(yy) {};
        Vec2 operator * (const float &r) const { return Vec2(x * r, y* r); }
        Vec2 operator * (const Vec2 &v) const { return Vec2(x * v.x, y * v.y); }
        Vec2 operator - (const Vec2 &v) const { return Vec2(x - v.x, y - v.y); }
        Vec2 operator + (const Vec2 &v) const { return Vec2(x + v.x, y + v.y); }
        Vec2 operator - () const { return Vec2(-x, -y); }
        Vec2& operator += (const Vec2 &v) { x += v.x, y += v.y; return *this; }
        void normalize() {
            T l2 = (x*x) + (y*y);
            if (l2 > 0) {
                T invL = 1 / std::sqrt(l2);
                x *= invL;
                y *= invL;
            }
        };
        std::string toString() {
            return "(" + boost::lexical_cast<std::string>(x) + "," + boost::lexical_cast<std::string>(y)
            + ")";
        }
        friend Vec2 operator * (const float &r, const Vec2 &v) { return Vec2(v.x * r, v.y * r); }
        friend std::ostream & operator << (std::ostream &os, const Vec2 &v) { return os << v.x << ", " << v.y; }
        virtual ~Vec2() {};
        T x, y; 
};

enum nodeTypes {
    TransformNode,
    SpotLightNode,
    PointLightNode,
    DirectionalLightNode,
    ShapeNode,
    SphereNode,
    BoxNode,
    TriangleSetNode,
    CoordinateNode,
    ColorNode,
    AppearanceNode,
    MaterialNode,
    ViewpointNode,
    BackgroundNode,
    NormalNode,
    IgnoreNode
};

enum RayType {PrimaryRay, ShadowRay};

enum class Shapes {UNIDENTFIED=0,BOX=1,SPHERE=2,CONE=3,CYLINDER=4,PYRAMID=5,DOME=6, 
        CAMERA=7, TRIANGLEMESH=8, LIGHT=9};

static std::string typeToString(Shapes type) {
    std::string ret = "";
    switch (type) {
        case Shapes::UNIDENTFIED:
            ret = "UNIDENTFIED";
            break;
        case Shapes::BOX:
            ret = "BOX";
            break;
        case Shapes::SPHERE:
            ret = "SPHERE";
            break;
        case Shapes::CONE:
            ret = "CONE";
            break;
        case Shapes::CYLINDER:
            ret = "CYLINDER";
            break;
        case Shapes::PYRAMID:
            ret = "PYRAMID";
            break;
        case Shapes::DOME:
            ret = "DOME";
            break;
        case Shapes::CAMERA:
            ret = "CAMERA";
            break;
        case Shapes::TRIANGLEMESH:
            ret = "TRIANGLEMESH";
            break;
        case Shapes::LIGHT:
            ret = "LIGHT";
            break;
        default:
            ret = "default";
            break; 
    }
    return ret;
}

static const char* ShapeStrings[] = {"UNIDENTFIED", "BOX", "SPHERE", "CONE", "CYLINDER", "PYRAMID", "DOME", "CAMERA", "TRIANGLEMESH", "LIGHT"};

enum LightTypes {SPOTLIGHT, POINTLIGHT, DIRECTIONALLIGHT}; 

enum octreeType {REGULAR, NONUNIFORM};

class Light {
    public:
        Light(const LightTypes t, const float &i = 1) : intensity(i), type(t) {
            direction = Vec3<float>(0,0,-1);
            attenuation = Vec3<float>(1,0,0);
        }
        virtual ~Light() {}
        //For all
        const LightTypes type;
        float intensity = 1.0;
        float ambientIntensity = 0.2;
        
        //For Spotlights and DirectionalLight
        Vec3<float> direction;
        
        //For Spotlights and Pointlights
        Vec3<float> attenuation;
        float radius = 100;

        //For Spotlights
        float beamWidth =   1.57079632; //  == Pi/2
        float cutOffAngle = 0.78539816; //  == Pi/4

        bool on = true;
        std::string toString() {
            std::string ret = "";
            switch (type) {
                case SPOTLIGHT:
                    ret += "\tSpotLight:";
                    ret += "\nBeam Width: " + boost::lexical_cast<std::string>(beamWidth);
                    ret += "\ncutOffAngle: " + boost::lexical_cast<std::string>(cutOffAngle);
                    ret += "\nRadius: " + boost::lexical_cast<std::string>(radius);
                    ret += "\nDirection: " + direction.toString();
                break;
                case POINTLIGHT:
                    ret += "\tPointLight:";
                    ret += "\nRadius: " + boost::lexical_cast<std::string>(radius);
                break;
                case DIRECTIONALLIGHT:
                    ret += "\tDirectionalLight:";
                    ret += "\nDirection: " + direction.toString();
                break;
            }
            return ret + "\nIntensity " + boost::lexical_cast<std::string>(intensity);  
        }
};

template <typename T>
struct RAY_STRUCT {
    RAY_STRUCT() {
        point = Vec3<T>();
        nvector = Vec3<T>();
        x = 0;
        y = 0;
    }
    RAY_STRUCT(Vec3<T> p, Vec3<T> v, int ix = 0, int iy = 0) {
        point = p;
        nvector = v;
        x = ix;
        y = iy;
    }
    Vec3<T> point;
    Vec3<T> nvector;
    int x, y;
};

struct BOX_STRUCT {
    BOX_STRUCT() {
        width = Vec3<float>();
    }
    Vec3<float> width;
};

struct SPHERE_STRUCT {
    SPHERE_STRUCT() {
        radius = 0;
        radius2 = 0;
    }
    float radius;
    float radius2;
};

struct CAMERA_STRUCT {
    //Aperture? Resolution?
    CAMERA_STRUCT() {
        xRes = 0;
        yRes = 0;
        fov = 0.0;
    }
    int xRes;
    int yRes;
    float fov;
};

struct TRIANGLEMESH_STRUCT {
    TRIANGLEMESH_STRUCT() {
        numtri = 0;
        position = nullptr;
        vertexNormals = nullptr;
        faceNormals = nullptr;
        vertexIndexArray = nullptr;
    }
    int numtri;
    std::shared_ptr<std::vector<Vec3<float>>> position;
    std::shared_ptr<std::vector<Vec3<float>>> vertexNormals;
    std::shared_ptr<std::vector<Vec3<float>>> faceNormals;
    std::shared_ptr<std::vector<int>> vertexIndexArray;
    Vec3<float> width;
    //Vec2<float> [] triangle texture coordinates?
    bool smoothShading = true;
    std::string toString(std::string tabs = "") {
        std::string ret = "\n";
        ret += tabs + "Width: (" + width.toString() + "\n";
        if(vertexIndexArray != nullptr) {
            ret += tabs +"TrianglesIndex Size: " + boost::lexical_cast<std::string>(vertexIndexArray->size()) + "\n";
        }
        if (vertexIndexArray->size() < 22) {
            if(position != nullptr) {
                ret += tabs + "Coords of triangles: \n";
                for (int i = 0; i < vertexIndexArray->size() / 3; i++) {
                    ret += tabs + boost::lexical_cast<std::string>(i) + ": " + (*position)[(*vertexIndexArray)[i*3]].toString() + " " + (*position)[(*vertexIndexArray)[i*3+1]].toString() + " " + (*position)[(*vertexIndexArray)[i*3+2]].toString() + "\n";
                }
            }
            if(vertexNormals != nullptr && smoothShading) {
                ret += tabs + "Vertex Normals: \n";
                for (int i = 0; i < vertexNormals->size(); i++) {
                    ret += tabs + boost::lexical_cast<std::string>(i) + ": " + (*vertexNormals)[i].toString() + "\n";
                } 
            }
            if(faceNormals != nullptr && !smoothShading) {
                ret += tabs + "Face normals: \n";
                for (int i = 0; i < faceNormals->size(); i++) {
                    ret += tabs + (*faceNormals)[i].toString()+ "\n";
                }
            }
        }
        return ret;
    }
};

union Shapesdata {
    BOX_STRUCT *box;
    SPHERE_STRUCT *sphere;
    CAMERA_STRUCT *camera;
    TRIANGLEMESH_STRUCT *triangles;
    Light *light;
};

typedef unsigned char  BYTE; // 1 byte
typedef unsigned short  WORD; // 2 bytes
typedef unsigned long  DWORD; // 4 bytes

typedef struct tagBITMAPINFOHEADER {
    DWORD    biSize;            // size of the structure
    DWORD    biWidth;           // image width
    DWORD    biHeight;          // image height
    WORD     biPlanes;          // bitplanes
    WORD     biBitCount;       // resolution
    DWORD    biCompression;     // compression
    DWORD    biSizeImage;       // size of the image
    DWORD    biXPelsPerMeter;   // pixels per meter X
    DWORD    biYPelsPerMeter;   // pixels per meter Y
    DWORD    biClrUsed;         // colors used
    DWORD    biClrImportant;    // important colors
} BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER {
    WORD    bfType;        // must be 'BM'
    DWORD   bfSize;        // size of the whole .bmp file
    WORD    bfReserved1;   // must be 0
    WORD    bfReserved2;   // must be 0
    DWORD   bfOffBits;
} BITMAPFILEHEADER;

template<typename T>
class Matrix44 {
    public:
        T x[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
        Matrix44() {}
        Matrix44(T a, T b, T c, T d, T e, T f, T g, T h,
                T i, T j, T k, T l, T m, T n, T o, T p) {
            x[0][0] = a;
            x[0][1] = b;
            x[0][2] = c;
            x[0][3] = d;
            x[1][0] = e;
            x[1][1] = f;
            x[1][2] = g;
            x[1][3] = h;
            x[2][0] = i;
            x[2][1] = j;
            x[2][2] = k;
            x[2][3] = l;
            x[3][0] = m;
            x[3][1] = n;
            x[3][2] = o;
            x[3][3] = p;
        }

        const T* operator [] (int i) const {return x[i];}
        T* operator [] (int i) {return x[i];}

        Matrix44 operator * (const Matrix44& v) const {
            Matrix44 tmp;
            multiply(*this, v, tmp);
            return tmp;
        }

        static void multiply(const Matrix44<T> &a, const Matrix44& b, Matrix44& c) {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    c[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];
                }
            }
        }

        Matrix44 transposed() const {
            Matrix44 t;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    t[i][j] = x[j][i];
                }
            }
            return t;
        }

        Matrix44& transpose() {
            Matrix44 t;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    t[i][j] = x[j][i];
                }
            }
            *this = t;
            return *this;
        }

        template<typename S>
        void multVecMatrix(const Vec3<S> &src, Vec3<S> &dst) const {
            S a, b, c, w;
        
            a = src.x * x[0][0] + src.y * x[1][0] + src.z * x[2][0] + x[3][0];
            b = src.x * x[0][1] + src.y * x[1][1] + src.z * x[2][1] + x[3][1];
            c = src.x * x[0][2] + src.y * x[1][2] + src.z * x[2][2] + x[3][2];
            w = src.x * x[0][3] + src.y * x[1][3] + src.z * x[2][3] + x[3][3];
        
            dst.x = a / w;
            dst.y = b / w;
            dst.z = c / w;
        }

        template<typename S>
        void multDirMatrix(const Vec3<S> &src, Vec3<S> &dst) const {
            S a, b, c;
        
            a = src[0] * x[0][0] + src[1] * x[1][0] + src[2] * x[2][0];
            b = src[0] * x[0][1] + src[1] * x[1][1] + src[2] * x[2][1];
            c = src[0] * x[0][2] + src[1] * x[1][2] + src[2] * x[2][2];
        
            dst.x = a;
            dst.y = b;
            dst.z = c;
        }

        Matrix44 inverse() const {
            int i, j, k;
            Matrix44 s;
            Matrix44 t (*this);
        
            // Forward elimination
            for (i = 0; i < 3 ; i++) {
                int pivot = i;
            
                T pivotsize = t[i][i];
            
                if (pivotsize < 0)
                    pivotsize = -pivotsize;
                
                for (j = i + 1; j < 4; j++) {
                    T tmp = t[j][i];         
                    if (tmp < 0)
                        tmp = -tmp;
                        
                    if (tmp > pivotsize) {
                        pivot = j;
                        pivotsize = tmp;
                    }
                }
            
                if (pivotsize == 0) {
                    // Cannot invert singular matrix
                    return Matrix44();
                }
            
                if (pivot != i) {
                    for (j = 0; j < 4; j++) {
                        T tmp;
                        
                        tmp = t[i][j];
                        t[i][j] = t[pivot][j];
                        t[pivot][j] = tmp;
                    
                        tmp = s[i][j];
                        s[i][j] = s[pivot][j];
                        s[pivot][j] = tmp;
                    }
                }
            
                for (j = i + 1; j < 4; j++) {
                    T f = t[j][i] / t[i][i];
                
                    for (k = 0; k < 4; k++) {
                        t[j][k] -= f * t[i][k];
                        s[j][k] -= f * s[i][k];
                    }
                }
            }
        
        // Backward substitution
            for (i = 3; i >= 0; --i) {
                T f;
            
                if ((f = t[i][i]) == 0) {
                // Cannot invert singular matrix
                    return Matrix44();
                }
            
                for (j = 0; j < 4; j++) {
                    t[i][j] /= f;
                    s[i][j] /= f;
                }
            
                for (j = 0; j < i; j++) {
                    f = t[j][i];
                
                    for (k = 0; k < 4; k++) {
                        t[j][k] -= f * t[i][k];
                        s[j][k] -= f * s[i][k];
                    }
                }
            }
            
            return s;
        }

        const Matrix44<T>& invert() {
            *this = inverse();
            return *this;
        }

        friend std::ostream& operator << (std::ostream &s, const Matrix44 &m) {
            std::ios_base::fmtflags oldFlags = s.flags();
            int width = 12; // total with of the displayed number
            s.precision(5); // control the number of displayed decimals
            s.setf (std::ios_base::fixed);
            
            s << "[" << std::setw (width) << m[0][0] <<
                 " " << std::setw (width) << m[0][1] <<
                 " " << std::setw (width) << m[0][2] <<
                 " " << std::setw (width) << m[0][3] << "\n" <<
                
                 " " << std::setw (width) << m[1][0] <<
                 " " << std::setw (width) << m[1][1] <<
                 " " << std::setw (width) << m[1][2] <<
                 " " << std::setw (width) << m[1][3] << "\n" <<
                
                 " " << std::setw (width) << m[2][0] <<
                 " " << std::setw (width) << m[2][1] <<
                 " " << std::setw (width) << m[2][2] <<
                 " " << std::setw (width) << m[2][3] << "\n" <<
                
                 " " << std::setw (width) << m[3][0] <<
                 " " << std::setw (width) << m[3][1] <<
                 " " << std::setw (width) << m[3][2] <<
                 " " << std::setw (width) << m[3][3] << "]";
            
            s.flags (oldFlags);
            return s;
        }

};

}
#endif //
