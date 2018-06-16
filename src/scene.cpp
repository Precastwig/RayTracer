#include <scene.h>

using namespace help;

Scene::Scene(std::string filename, octreeType optimisation) {
    std::filebuf fb;
    std::shared_ptr<float> kk = std::make_shared<float>(0);
    *kk = std::numeric_limits<float>::infinity();
    tree = new Octree(0, nullptr, kk);
    tree->setParent(tree);
    skycolour = std::make_shared<Vec3<float>>(0);
    groundcolour = std::make_shared<Vec3<float>>(0);
    sceneCamera = nullptr;
    if (fb.open(filename, std::ios::in)) {
        std::istream is(&fb);
        boost::property_tree::read_xml(is , peetree);
        std::cout << "File loaded correctly" << std::endl;
    } else {
        std::cerr << "That file name does not exist" << std::endl;
    }
}

Scene::~Scene() {
    delete tree;
}

std::shared_ptr<Vec3<float>> Scene::computeRay(int x, int y) {
    CAMERA_STRUCT cam = this->sceneCamera->getCameradata();
    if (x < cam.xRes && x >= 0
        && y < cam.yRes && y >= 0) {
        // The output colour
        Vec3<float> output;
        // First find the ray's parametric equation, this is a point and a vector
        RAY_STRUCT<float> ray;
        ray.point = *(this->sceneCamera->getPos());
        //The point is just the cameras position

        float aspectRatio = (float)cam.xRes / (float)cam.yRes;
        float scale = std::tan(cam.fov / 2.0);

        float pixelX = (2 * ((x + 0.5) / (float)cam.xRes) - 1) * aspectRatio * scale;
        float pixelY = (1 - 2 * (y + 0.5) / (float)cam.yRes) * scale;
        ray.nvector = Vec3<float>(pixelX,pixelY,-1);
        ray.nvector.normalize();

        // Set default background colour based on y direction of ray
        float perc = (ray.nvector.y + 1) / 2;
        output = (perc * *this->skycolour) + ((1 - perc) * *this->groundcolour);
        // Now compute intersection with objects
        ray.x = x;
        ray.y = y;
        IsectInfo isect;
        // Check if the ray hits any of the objects in the tree
        if (tree->trace(ray,isect)) {
            // If it does find the point in which it hit the object and the hit normal
            Vec3<float> hitpoint = ray.point + (ray.nvector * isect.tNear);
            // if (x == 950 && y == 530) {
            //     std::cout << "\tisect.tnear " << isect.tNear << std::endl; 
            // }
            Vec3<float> hitNormal;
            isect.hitObject->getSurfaceData(ray, hitpoint, isect.index, isect.uv, hitNormal);
         
            // Work out colour by looping through all the lights in the scene and adding its contribution to the output colour
            std::vector<std::shared_ptr<Shape>> lights = tree->getLights();
            output = Vec3<float>(0,0,0);
            for (int i = 0; i < lights.size(); i++) {
                // Get the values of the current light we're working on
                Light ld = lights[i]->getLightdata();
                // Get the position of the light
                Vec3<float> lpos = *(lights[i]->getPos());

                // Work out the direction vector from the camera to the object
                Vec3<float> L = Vec3<float>(0.0);
                if (ld.type == DIRECTIONALLIGHT) L = -ld.direction; 
                    else L = (lpos - hitpoint);
                L.normalize();

                IsectInfo isectShad;
                // Check if the light reaches our hitpoint
                bool reflect;
                Octree *oreached = tree->findleaf(hitpoint + (hitNormal * BIAS), ShadowRay);
                if (oreached == nullptr) {
                    // if (x == 989 && y == 513)
                    std::cerr << "Should never happen, but continuing, will have specks on image" << std::endl;
                    Octree *reached = tree->findleaf(hitpoint, ShadowRay);
                    if (reached == nullptr)
                        std::cerr << "Backup failed: " << hitpoint << " " << x << "," << y << std::endl;   
                    reflect = !reached->trace(RAY_STRUCT<float>(hitpoint,L,x,y),isectShad, ShadowRay);
                } else {
                    reflect = !oreached->trace(RAY_STRUCT<float>(hitpoint + (hitNormal * BIAS),L,x,y), isectShad, ShadowRay);
                }

                if (!reflect && ld.type == POINTLIGHT) {
                    Vec3<float> pos = isectShad.tNear * L + hitpoint;
                    Vec3<float> min = hitpoint;
                    Vec3<float> max = lpos;
                    if (min.x > max.x) std::swap(min.x,max.x);
                    if (min.y > max.y) std::swap(min.y,max.y);
                    if (min.z > max.z) std::swap(min.z,max.z);

                    if (pos.between(min,max,0.0001))
                        reflect = false;
                    else {
                        reflect = true;
                        // std::cout << "Mmm?" << std::endl;
                    }
                }

                if (reflect) {
                    // Work out the contribution of the light, this is split into ambient, diffuse and specular lighting
                    float spotlightFactor = 0;
                    if (ld.type == SPOTLIGHT) {
                        float spotAngle = acos(std::max((float)0, (float)(-L).dotProduct(ld.direction))); 
                        if (spotAngle >= ld.cutOffAngle) spotlightFactor = 0; 
                        else {
                            if (spotAngle <= ld.beamWidth) spotlightFactor = 1;
                            else {
                                spotlightFactor = (spotAngle - ld.cutOffAngle) / (ld.beamWidth - ld.cutOffAngle);
                            }
                        }
                    } else {
                        spotlightFactor = 1;
                    }
                    // spotlightFactor = 1;
                    float attenuation = 1;
                    if (ld.type != DIRECTIONALLIGHT) {
                        float lp = lpos.distance(hitpoint); 
                        attenuation = 1 / (std::max((float)1, ld.attenuation.x + ld.attenuation.y * hitpoint.distance(ray.point) + ld.attenuation.z * lp * lp ));
                    }

                    // Ambient takes the lights ambient intensity times the colour of the object
                    Vec3<float> ambient = ld.ambientIntensity * *isect.hitObject->getDColour();

                    // Diffuse takes the light intensity times the colour of the object times the dot product of our hitnormal and our vector from light to object
                    Vec3<float> diffuse = ld.intensity * *isect.hitObject->getDColour() * (std::max((float)0, (float)L.dotProduct(hitNormal)));

                    // Specular takes the intensity times the specular colour of the object times the amount the normal is pointing towards the light to the power of the shininess
                    Vec3<float> tempL = (L - ray.nvector);
                    Vec3<float> specular = ld.intensity * *isect.hitObject->getSColour() * 
                        pow( std::max((float)0, hitNormal.dotProduct( tempL / sqrt(tempL.dotProduct(tempL)) ) ), 128 * isect.hitObject->getShininess() );

                    // We then add all these and times them by the attenuation and the diffuse colour of the object
                    output += (attenuation * spotlightFactor * *lights[i]->getDColour() * (ambient + diffuse + specular));
                }
            }
        }
        // We cap the output from 0 to 1
        output.cap(0.0,1.0);
        return std::make_shared<Vec3<float>>(output.x,output.y,output.z);
    }
    return NULL;
}

// bool Scene::trace(const RAY_STRUCT<float> &ray, IsectInfo &isect, RayType type) {
//     std::vector<std::shared_ptr<Shape>> objList = tree->getObjects();
//     isect.hitObject = nullptr;
//     isect.tNear = std::numeric_limits<float>::infinity();
//     std::shared_ptr<float> tnear = std::make_shared<float>(0);
//     *tnear = std::numeric_limits<float>::infinity();
//     Vec2<float> uv = Vec2<float>(0,0);
//     std::shared_ptr<int> triIndex = std::make_shared<int>(0);
//     for (int i = 0; i < objList.size(); i++) {
//         if (objList[i]->getType() != Shapes::CAMERA) {
//             if (objList[i]->intersect(ray, tnear, triIndex, uv)) {
//                 // if (objList[i]->getType() == Shapes::TRIANGLEMESH && type == ShadowRay) std::cout << "Point: " << ray.point << "\n  dir" << ray.nvector << "\n Intersects " << *triIndex << std::endl;
//                 if (*tnear < isect.tNear) {
//                     // if (type == ShadowRay && ) continue;
//                     isect.hitObject = objList[i];
//                     // if (objList[i]->getType() == Shapes::TRIANGLEMESH) std::cout << "BEFORE " << *tnear << " " << isect.tNear << std::endl;
//                     isect.tNear = *tnear;
//                     // if (objList[i]->getType() == Shapes::TRIANGLEMESH) std::cout << "AFTER " << *tnear << " " << isect.tNear << std::endl;
//                     isect.index = *triIndex;
//                     isect.uv = uv;
//                 }
//             }
//         }
//     }
//     return (isect.hitObject != nullptr);
// }

void Scene::parseX3D(octreeType optimisation) {
    if (tree != nullptr) {
        delete tree;
    }
    std::shared_ptr<float> kk = std::make_shared<float>(0);
    *kk = std::numeric_limits<float>::infinity();
    tree = new Octree(0, nullptr, kk);
    tree->setParent(tree);
    // tree->TYPE_TRAVERSE = tt;
    // tree->TRI_COST = tc;
    // tree->SPHERE_COST = sc;

    using boost::property_tree::ptree;
    ptree pt;
    pt = peetree.get_child("X3D").get_child("Scene");

    //Setting background colours for the scene
    std::string gcolour = pt.get_child("Background").get("<xmlattr>.groundColor","0 0 0");
    this->groundcolour = strToTuple<float>(gcolour);
    std::string scolour = pt.get_child("Background").get("<xmlattr>.skyColor","1 1 1");
    this->skycolour = strToTuple<float>(scolour);

    //Setting the cameras position, rotation, type and fov

    std::string cpos = pt.get_child("Viewpoint").get("<xmlattr>.position","0 0 0");
    std::string crotation = pt.get_child("Viewpoint").get("<xmlattr>.rotation","0 0 0");
    //std::shared_ptr<Vec3<float>> p = std::make_shared<Vec3<float>>();
    this->sceneCamera = std::make_shared<Shape>(strToTuple<float>(cpos), std::make_shared<Vec3<float>>(0,0,0), strToTuple<float>(crotation), Shapes::CAMERA);

    std::string cfov = pt.get_child("Viewpoint").get("<xmlattr>.fieldOfView","0.9");
    // std::cout << cfov << std::endl;
    try {
        (*this->sceneCamera).setCameradata(1920,1080,boost::lexical_cast<float>(cfov));
    } catch (boost::bad_lexical_cast &) {
        std::cerr << "Error bad cast: " << cfov << std::endl;
    }

    //Parse all the Shapes in the file
    BOOST_FOREACH( ptree::value_type const& v, pt) {
        std::shared_ptr<Shape> temp = std::make_shared<Shape>();
        parseNode(v, temp);
        if (temp->getType() != Shapes::UNIDENTFIED) {  
            tree->addShape(temp);
        }
    }
    tree->resize(optimisation);
    tree->optimize(optimisation);
    // std::cout << "Optimizations complete" << std::endl;   
    //this->groundcolour = pt.get_child("Background").second.get("<xmlattr>.groundColor", false);
}

nodeTypes Scene::stringToNode(std::string in) {
    if (in == "Transform") return TransformNode;
    if (in == "SpotLight") return SpotLightNode;
    if (in == "PointLight") return PointLightNode;
    if (in == "DirectionalLight") return DirectionalLightNode;
    if (in == "Shape") return ShapeNode;
    if (in == "Sphere") return SphereNode;
    if (in == "Box") return BoxNode;
    if (in == "IndexedTriangleSet") return TriangleSetNode;
    if (in == "Coordinate") return CoordinateNode;
    if (in == "Color") return ColorNode;
    if (in == "Appearance") return AppearanceNode;
    if (in == "Viewpoint") return ViewpointNode;
    if (in == "Background") return BackgroundNode;
    if (in == "Material") return MaterialNode;
    if (in == "Normal") return NormalNode;
    return IgnoreNode;
}

void Scene::printOctree() {
    std::cout << this->tree->toString();
}

void Scene::parseNode(boost::property_tree::ptree::value_type const &node, std::shared_ptr<Shape> output) {
    // std::cout << "Parsing: " << node.first << "\n";
    nodeTypes n = stringToNode(node.first);
    switch(n) {
    case DirectionalLightNode:
        output->setType(DIRECTIONALLIGHT);
        output->setDirectionallightdata(*strToTuple<float>(node.second.get("<xmlattr>.direction","0 0 -1")));
        this->lights.push_back(output);
       break;
    case SpotLightNode: 
        output->setType(SPOTLIGHT);
        output->movePosition(strToTuple<float>(node.second.get("<xmlattr>.location","0 0 0")));
        try {

        output->setSpotlightdata(
            *strToTuple<float>(node.second.get("<xmlattr>.attenuation","1 0 0")),
            *strToTuple<float>(node.second.get("<xmlattr>.direction","0 0 -1")),
            boost::lexical_cast<float>(node.second.get("<xmlattr>.radius","100")),
            boost::lexical_cast<float>(node.second.get("<xmlattr>.beamWidth","1.57079632")),
            boost::lexical_cast<float>(node.second.get("<xmlattr>.cutOffAngle","0.78539816")));
        this->lights.push_back(output);
        } catch (boost::bad_lexical_cast &) {
            std::cerr << "Bad cast SPOTLIGHT DATA" << std::endl;
        }
        break;
    case PointLightNode:
        output->setType(POINTLIGHT);
        output->movePosition(strToTuple<float>(node.second.get("<xmlattr>.location","0 0 0")));
        try {
            output->setPointlightdata(
                *strToTuple<float>(node.second.get("<xmlattr>.attenuation","1 0 0")),
                boost::lexical_cast<float>(node.second.get("<xmlattr>.radius","100")));
            this->lights.push_back(output);
        } catch (boost::bad_lexical_cast &) {
            std::cerr << "Bad cast POINTLIGHT DATA" << std::endl;
        }
        break;
    case TransformNode:
        output->movePosition(strToTuple<float>(node.second.get("<xmlattr>.translation","0 0 0")));
        // std::cout << "No error here " << *output->getPos() << std::endl;
        output->moveRotation(strToTuple<float>(node.second.get("<xmlattr>.rotation","0 0 0")));
        BOOST_FOREACH(boost::property_tree::ptree::value_type const& v, node.second) {
            parseNode(v, output);
        }
        break;
    case ShapeNode:
        BOOST_FOREACH(boost::property_tree::ptree::value_type const& v, node.second) {
            parseShape(v, output);
        }
        break;
    default:
        break;
    }
    if (n == DirectionalLightNode || n == SpotLightNode || n == PointLightNode) {
        output->setDColour(strToTuple<float>(node.second.get("<xmlattr>.color","1 1 1")));
        output->setSColour(strToTuple<float>(node.second.get("<xmlattr>.color","1 1 1")));
        try {
            output->setLightdata(boost::lexical_cast<float>(node.second.get("<xmlattr>.intensity","1")));
        } catch (boost::bad_lexical_cast &) {
            std::cerr << "Bad cast intensity" << std::endl;
        }

    }
}

void Scene::parseShape(boost::property_tree::ptree::value_type const &node, std::shared_ptr<Shape> output) {
    // std::cout << "Parsing shape: " << node.first << "\n";
    std::shared_ptr<std::vector<int>> IndexArray;
    std::shared_ptr<std::vector<Vec3<float>>> tempCoord;
    std::shared_ptr<Vec3<float>> col;
    std::string npv;
    switch(stringToNode(node.first)) {
    case SphereNode:
        output->setType(Shapes::SPHERE);
        try {

            output->setSpheredata(boost::lexical_cast<float>(node.second.get("<xmlattr>.radius", "0.0")));
        } catch (boost::bad_lexical_cast &) {
            std::cerr << "Bad cast sphere radius " << node.second.get("<xmlattr>.radius", "0.0") << std::endl;
        }
        break;
    case BoxNode:
        output->setType(Shapes::BOX);
        //output->setBoxdata()
    case AppearanceNode:
        break;
    case MaterialNode:
        //std::cout << "Putting in colour\n";
        output->setDColour(strToTuple<float>(node.second.get("<xmlattr>.diffuseColor","1 0 0")));
        output->setShininess(boost::lexical_cast<float>(node.second.get("<xmlattr>.shininess","0.2")));
        output->setSColour(strToTuple<float>(node.second.get("<xmlattr>.specularColor","1 1 1")));
        output->setTransparency(boost::lexical_cast<float>(node.second.get("<xmlattr>.transparency","0.0")));
        break;
    case TriangleSetNode:
        output->setType(Shapes::TRIANGLEMESH);
        IndexArray = std::make_shared<std::vector<int>>(strToVector<int>(node.second.get("<xmlattr>.index","0 0 0")));
        output->setTriangleIndexes(IndexArray->size()/3, IndexArray);
        
        if(boost::lexical_cast<int>(node.second.get("<xmlattr>.creaseAngle","0")) > 0) {
            output->setShadingType(true);
        } else {
            output->setShadingType(false);
        }

        npv = node.second.get("<xmlattr>.normalPerVertex","na");
        if (npv == "na") {
            normalsInFile = false;
        } else if(npv == "true") {
            normalsInFile = true;
            output->setShadingType(true);
        }

        break;
    case CoordinateNode:
        tempCoord = std::make_shared<std::vector<Vec3<float>>>(strToVec3Vector<float>(node.second.get("<xmlattr>.point","0 0 0"))); 
        output->setTriangleCoords(tempCoord);
        // We add any translations to the coords themselves
        col = std::make_shared<Vec3<float>>(0.0);
        if (output->getPos() != nullptr) {   
            for (int i = 0; i < (*tempCoord).size(); i++) {
                (*tempCoord)[i] = (*tempCoord)[i] + *(output->getPos());
                *col += (*tempCoord)[i];
            }
            *col = *col / (*tempCoord).size();
            output->setPosition(col);
        }
        output->setTriwidth();
        output->makeTriangleNormals(output->getShadingType());
        break;
    case NormalNode:
        if (normalsInFile == true) {
            tempCoord = std::make_shared<std::vector<Vec3<float>>>(strToVec3Vector<float>(node.second.get("<xmlattr>.vector","0 0 0")));
            if ((*tempCoord)[0] == 0) {
                output->makeTriangleNormals(output->getShadingType()); // we still want to make normals even if the x3d file is mis-formed
            }
            else {   
                // std::cout << "Getting normals" << std::endl;
                // std::cout << "NOT" << std::endl;
                output->addVertexNormals(tempCoord);
            }
        }
        break;
    case ColorNode:
        break;
    }
    BOOST_FOREACH(boost::property_tree::ptree::value_type const& v, node.second) {
        parseShape(v, output);
    }
}
