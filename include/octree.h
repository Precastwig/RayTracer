#ifndef OCTREE_H
#define OCTREE_H
#include <structs.h>
#include <shape.h>
#include <nlopt.hpp>

using namespace help;

struct IsectInfo {
    std::shared_ptr<Shape> hitObject = nullptr;
    float tNear = std::numeric_limits<float>::infinity();
    Vec2<float> uv;
    int index = 0;
    unsigned long octreeName = 0;
};

struct SAHInfo {
    double SAInterior = 0;
    double SALeaf = 0;
    double SALeafObj = 0; 
};

typedef struct {
    Vec3<double> midpoint;
    Vec3<double> nWidth;
    Vec3<double> pWidth;
    std::vector<std::shared_ptr<Shape>> *objects;
} mps_data;

class Octree
{
    public:
        Octree(unsigned long id, Vec3<float> mid, Vec3<float> widthp, Vec3<float> widthn, bool setsubdivided, Octree* r, std::shared_ptr<float> svw, std::shared_ptr<std::unordered_map<unsigned long, Octree*>> all);
        Octree(unsigned long id, Octree* r, std::shared_ptr<float> svw);
        //Octree(int id, std::shared_ptr<Point> mid, float xwidth, float ywidth, float zwidth);
        virtual ~Octree();
        std::string toString(std::string tabs = "");
        bool trace(const RAY_STRUCT<float> &ray, IsectInfo &isect, RayType type = PrimaryRay);
        void setParent(Octree* sparent);
        void setMidpoint(Vec3<float> nmid);
        void addChild(Octree* leaf, int position);
        void addShape(std::shared_ptr<Shape> shape);
        void optimize(octreeType type);
        void resize(octreeType type = REGULAR);
        //Getters
        int getName() { return this->name; };
        std::vector<std::shared_ptr<Shape>> getObjects() {
            if (!subdivided) return objects; 
            else std::cerr << "Error, tried to get objects of a subdivided node\n";
        };
        std::vector<std::shared_ptr<Shape>> getLights() { return lights; };
        Octree* findleaf(const Vec3<float> &p, RayType type = PrimaryRay);
        Octree* findOctree(unsigned long name);
        void collectSAHInfo(SAHInfo &inf);
        int countVoxels();
        std::shared_ptr<float> smallestVoxelWidth;
        int depth = 1;
        int voxels = 1;
        Vec3<float> midpoint;
        unsigned long name;
        bool subdivided;
        std::vector<Octree*> leaves;
        std::shared_ptr<std::unordered_map<unsigned long, Octree*>> allOctrees;
        Octree* root;
        const float kEpsilon = 0.0001;
        // int TYPE_TRAVERSE = 0;
        // int TRI_COST = 10;
        // int SPHERE_COST = 10;
    protected:
    private:
        std::shared_ptr<Vec3<float>> computeNewCentre();
        std::vector<Vec3<float>> pBetween(Vec3<float> p1, Vec3<float> p2, bool reduced = true);
        std::shared_ptr<float> intersectsWith(const RAY_STRUCT<float> &ray);
        static double mps(unsigned n, const double *x, double *grad, void *my_func_data);
        float midpointScore(Vec3<float> mp);
        float SAH(octreeType type);
        Vec3<float> pWidth;
        Vec3<float> nWidth;
        std::vector<std::shared_ptr<Shape>> objects;
        std::vector<std::shared_ptr<Shape>> lights;
        // std::vector<std::shared_ptr<Shape>> objandlight;
        Octree* next(const RAY_STRUCT<float> &ray, RayType type);
        void svwUpdate();
        void calculateCosts(octreeType type);
        void split(octreeType type);
        void unsplit();
        nlopt::opt optimiser;
        nlopt::opt lopt;
        float surfaceArea;
        float intersectCost;
        float traverseLCost;
        float traverseICost;
};



#endif // OCTREE_H