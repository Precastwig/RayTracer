#ifndef SHAPE_H
#define SHAPE_H
#include <structs.h>

using namespace help;

class Shape {
    public:
        Shape(std::shared_ptr<Vec3<float>> sPosition, std::shared_ptr<Vec3<float>> sColour, std::shared_ptr<Vec3<float>> sRotation , Shapes type);
        Shape(Shapes type);
        Shape();
        void nullShapedata();
        virtual ~Shape();

        //Setters
        void setBoxdata(float xsize, float ysize, float zsize);
        void setSpheredata(float radius);
        void setCameradata(int x, int y, float f);
        void setLightdata(float i);
        void setSpotlightdata(Vec3<float> at, Vec3<float> dir, float rad, float bw, float coa);
        void setDirectionallightdata(Vec3<float> dir);
        void setPointlightdata(Vec3<float> at, float rad);
        void setTriangleIndexes(int numtri, std::shared_ptr<std::vector<int>> vertexIndexArray);
        void setTriangleCoords(std::shared_ptr<std::vector<Vec3<float>>> position);
        void setTriwidth();
        void makeTriangleNormals(bool smoothshading);
        void addVertexNormals(std::shared_ptr<std::vector<Vec3<float>>> norms);

        void setPosition(std::shared_ptr<Vec3<float>> newpos);
        void setPosition(float x, float y, float z);
        void movePosition(std::shared_ptr<Vec3<float>> movement);

        void setRotation(std::shared_ptr<Vec3<float>> newrot);
        void setRotation(float x, float y, float z);
        void moveRotation(std::shared_ptr<Vec3<float>> movement);

        void setDColour(std::shared_ptr<Vec3<float>> newcol);
        void setDColour(float r, float g, float b);
        void setSColour(std::shared_ptr<Vec3<float>> newcol);
        void setSColour(float r, float g, float b);
        void setShininess(float s);
        void setTransparency(float t);

        void setType(Shapes type);
        void setType(LightTypes type);
        void setShadingType(bool smooth);

        //Check for intersection
        bool intersect(RAY_STRUCT<float> ray, float &t, std::shared_ptr<int> triIndex, Vec2<float> &uv);
        bool intersectSphere(RAY_STRUCT<float> ray, float &t);
        bool intersectTriangleMesh(RAY_STRUCT<float> ray, float &t, std::shared_ptr<int> triIndex, Vec2<float> &uv);
        bool intersectTriangle(RAY_STRUCT<float> ray, Vec3<float> p1, Vec3<float> p2, Vec3<float> p3, float &t, float &u, float &v);
        bool intersectLight(RAY_STRUCT<float> ray, float &t);

        //Light functions
        //void illuminate(const Vec3<float> &P, Vec3<float> &lightDir, Vec3<float> &lightIntensity, float &);

        //Getters
        void getSurfaceData(RAY_STRUCT<float> ray, Vec3<float> pointHit, int &triIndex, Vec2<float> &uv, Vec3<float> &hitNormal);
        std::string toString(std::string tabs = "");
        Shapes getType();
        bool getShadingType();
        BOX_STRUCT getBoxdata();
        SPHERE_STRUCT getSpheredata();
        CAMERA_STRUCT getCameradata();
        int getNTris();
        Light getLightdata();
        Vec3<float> getWidth();
        std::shared_ptr<Vec3<float>> getPos();
        std::shared_ptr<Vec3<float>> getDColour();
        std::shared_ptr<Vec3<float>> getSColour();
        float getShininess();
        float getTransparency();
    private:
        void clearShapedata();
        void initShapedata();
        void initShapedata(LightTypes l);
        std::vector<int> getTriangleIndexes(int x);
        std::shared_ptr<Vec3<float>> rotation;
        std::shared_ptr<Vec3<float>> position;
        std::shared_ptr<Vec3<float>> diffuseColour;
        std::shared_ptr<Vec3<float>> specularColour;
        float shininess;
        float transparency;
        Shapes shape;
        Shapesdata shapedata;
};

#endif // SHAPE_H
