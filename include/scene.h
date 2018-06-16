#ifndef SCENE_H
#define SCENE_H
#include <structs.h>
#include <shape.h>
#include <octree.h>

using namespace help;

class Scene
{
    public:
        Scene(std::string filename, octreeType optimisation = REGULAR);
        void printOctree();
        std::shared_ptr<Vec3<float>> computeRay(int i, int j);
        void parseX3D(octreeType optimisation = REGULAR);
        virtual ~Scene();
        Octree* tree = nullptr;
    protected:
    private:
        bool normalsInFile = false;
        void parseNode(boost::property_tree::ptree::value_type const &node, std::shared_ptr<Shape> output);
        void parseShape(boost::property_tree::ptree::value_type const &node, std::shared_ptr<Shape> output);
        bool trace(const RAY_STRUCT<float> &ray, IsectInfo &isectinfo, RayType type = PrimaryRay);
        nodeTypes stringToNode(std::string in);
        template <typename T> std::shared_ptr<Vec3<T>> strToTuple(std::string str);
        template <typename T> std::vector<T> strToVector(std::string str);
        template <typename T> std::vector<Vec3<T>> strToVec3Vector(std::string str);
        std::shared_ptr<Vec3<float>> skycolour;
        std::shared_ptr<Vec3<float>> groundcolour;
        boost::property_tree::ptree peetree;
        std::shared_ptr<Shape> sceneCamera;
        std::vector<std::shared_ptr<Shape>> lights;
        const float BIAS = 0.04;
};

template <typename T>
std::shared_ptr<Vec3<T>> Scene::strToTuple(std::string str) {
    std::vector<std::string> parts;
    T conv[3];
    boost::algorithm::split(parts, str, boost::algorithm::is_any_of(" "));
    for (int i = 0; i < 3; i++) {
        try {
            if (parts[i] != "")
                conv[i] = boost::lexical_cast<T>(parts[i]);
        } catch (boost::bad_lexical_cast &) {
            std::cerr << "Boost cast error in strToTuple: " << str << std::endl;
        }
    }

    return std::make_shared<Vec3<T>>(conv[0],conv[1], conv[2]);
}

template<typename T>
std::vector<T> Scene::strToVector(std::string str) {
    std::vector<std::string> parts;
    std::vector<T> ret;
    boost::algorithm::split(parts, str, boost::algorithm::is_any_of(" "));
    ret.resize(parts.size());
    for(int i = 0; i < parts.size(); i++) {
        try {
            if (parts[i] != "")
                ret[i] = boost::lexical_cast<T>(parts[i]);
        } catch (boost::bad_lexical_cast &) {
            std::cerr << "Boost cast error in strToVector: " << ret[i] << std::endl;
        }
    }
    return ret;
}

template<typename T>
std::vector<Vec3<T>> Scene::strToVec3Vector(std::string str) {
    std::vector<std::string> parts;
    std::vector<Vec3<T>> ret;
    //Vec3<T> temp;
    boost::algorithm::split(parts, str, boost::algorithm::is_any_of(" "));
    ret.resize(parts.size()/3);
    for(int i = 0; i < ret.size(); i++) {
        if (parts[i] != "")
            ret[i] = Vec3<T>(boost::lexical_cast<T>(parts[i*3]),
                            boost::lexical_cast<T>(parts[i*3+1]),
                            boost::lexical_cast<T>(parts[i*3+2]));
    }
    return ret;
}


#endif // SCENE_H
