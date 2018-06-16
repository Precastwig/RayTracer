#include <octree.h>
#include <structs.h>

using namespace help;

Octree::Octree(unsigned long id, Vec3<float> mid, Vec3<float> widthp, Vec3<float> widthn, bool setsubdivided, Octree* r, std::shared_ptr<float> svw, std::shared_ptr<std::unordered_map<unsigned long, Octree*>> all) {
    this->midpoint = mid;
    this->pWidth = widthp;
    this->nWidth = widthn;
    Vec3<float> width = pWidth - nWidth;
    this->surfaceArea = fabs(2 * ((width.x * width.z) + (width.y * width.x) + (width.z * width.y) ));
    this->name = id;
    this->subdivided = setsubdivided;
    this->intersectCost = 0;
    this->traverseLCost = 0;
    this->traverseICost = 0;
    this->leaves = std::vector<Octree*>();
    this->objects = std::vector<std::shared_ptr<Shape>>();
    this->lights = std::vector<std::shared_ptr<Shape>>();
    // this->objandlight = std::vector<std::shared_ptr<Shape>>();
    if (setsubdivided) {
        this->leaves.reserve(9);
    }
    this->root = r;
    this->smallestVoxelWidth = svw;
    this->depth = boost::lexical_cast<std::string>(id).size();
    if (this->root->depth < this->depth)
        this->root->depth = this->depth;
    this->allOctrees = all;
    //this->objects.reserve(2);
}

Octree::Octree(unsigned long id, Octree* r, std::shared_ptr<float> svw) {
    // This function is only called once ever. This initialises the root octree node. It's important that we don't alter many things until the node
    // is filled with objects. Once filled with objects we can resize the node and set it's parameters (And begin to subdivide).
    this->midpoint = Vec3<float>(0);
    this->pWidth = Vec3<float>(0);
    this->nWidth = Vec3<float>(0);
    this->surfaceArea = 0;
    this->name = id;
    this->subdivided = false;
    this->intersectCost = 0;
    this->traverseLCost = 0;
    this->traverseICost = 0;
    this->leaves = std::vector<Octree*>();
    this->objects = std::vector<std::shared_ptr<Shape>>();
    this->lights = std::vector<std::shared_ptr<Shape>>();
    // this->objandlight = std::vector<std::shared_ptr<Shape>>();
    this->root = r;
    this->smallestVoxelWidth = svw;
    this->depth = 0;
    this->allOctrees = std::make_shared<std::unordered_map<unsigned long, Octree*>>();
    // opt optimiser = nlopt::opt(GN_ISRES, 3);
        // opt optimiser = nlopt::opt(GN_ESCH, 3);
        // root->optimiser = nlopt::opt(G_MLSL_LDS, 3);
        // opt optimiser = nlopt::opt(AUGLAG, 3);
        // opt lopt = nlopt::opt(LN_NELDERMEAD,3);
    this->lopt = nlopt::opt(nlopt::LN_SBPLX,3);
    this->optimiser = nlopt::opt(nlopt::G_MLSL_LDS, 3);
    // this->lopt = nlopt::opt(nlopt::LN_COBYLA,3);
        // root->lopt = nlopt::opt(LN_COBYLA,3);
        // lopt.set_initial_step()
    // lopt.set_maxtime(0.0001);
        // lopt.set_stopval(2.0);
    this->lopt.set_maxeval(15);
    this->optimiser.set_local_optimizer(lopt);
    // this->optimiser.set_population(6);
    // this->optimiser.set_maxtime(0.005);
    // this->optimiser.set_stopval(1.1);
}

/*
Octree::Octree(int id, Point *mid, float xwidth, float ywidth, float zwidth) {
    this->midpoint = mid;
    this->widthx = xwidth;
    this->widthy = ywidth;
    this->widthz = zwidth;
    this->name = id;
    leaves = nullptr;
}*/

std::string Octree::toString(std::string tabs) {
    std::string temp = tabs + "\033[38;5;196mOctree: " + boost::lexical_cast<std::string>(this->name) + " \033[0m;\n";
    temp += tabs + "Midpoint: " + midpoint.toString() + "\n";
    temp += tabs + "maxPoint: " + (midpoint + pWidth).toString() + "  minPoint: " + (midpoint + nWidth).toString() + "\n";
    temp += tabs + "Surface Area: " + boost::lexical_cast<std::string>(surfaceArea) + " Objects: " + boost::lexical_cast<std::string>(objects.size()) + "\n";
    if (name == 0) {
        temp += tabs + "Lights:\n";
        for (int i = 0; i < this->lights.size(); i++)
            temp += tabs + this->lights[i]->toString(tabs) + "\n";
        temp += tabs + "SmallestVoxelWidth: " +  boost::lexical_cast<std::string>(*smallestVoxelWidth) + "\n";
    }
    // temp += boost::lexical_cast<std::string>(*smallestVoxelWidth) + "\n";
    tabs += "\t";
    if (this->subdivided) {
        temp += tabs + "Leaves:\n";
        for (int i = 0; i < this->leaves.size(); i++) {
            temp += this->leaves[i]->toString(tabs);
        }
    } else {
        temp += tabs + "Objects:\n";
        for (int i = 0; i < this->objects.size(); i++) {
            temp += tabs + this->objects[i]->toString(tabs) + "\n";
        }
        // for (int i = 0; i < this->lights.size(); i++) {
        //     temp += tabs + this->lights[i]->toString(tabs) + "\n";
        // }    
    }
    return temp;
}

void Octree::setParent(Octree* root) {
    this->root = root;
}

void Octree::setMidpoint(Vec3<float> nmid) {
    // Assuming nmid is not out of bounds
    Vec3<float> pMax = this->midpoint + pWidth;
    Vec3<float> pMin = this->midpoint + nWidth;
    this->midpoint = nmid;
    pWidth = pMax - this->midpoint;
    nWidth = pMin - this->midpoint;
}

void Octree::addChild(Octree* leaf, int position) {
    if (subdivided || position > 8 || position < 0) {
        leaves[position] = leaf;
    }
}

void Octree::addShape(std::shared_ptr<Shape> shape) {
    if (!subdivided && shape != nullptr) {
        if (shape->getType() == Shapes::LIGHT) {
            lights.push_back(shape);
        } else {
            objects.push_back(shape);
        }
        // objandlight.push_back(shape);
    }
}

void Octree::svwUpdate() {
    if (*smallestVoxelWidth - this->pWidth.x > kEpsilon) *smallestVoxelWidth = this->pWidth.x;
    if (this->pWidth.y < *smallestVoxelWidth) *smallestVoxelWidth = this->pWidth.y;
    if (this->pWidth.z < *smallestVoxelWidth) *smallestVoxelWidth = this->pWidth.z;
    if (-this->nWidth.x < *smallestVoxelWidth) *smallestVoxelWidth = -this->nWidth.x;
    if (-this->nWidth.y < *smallestVoxelWidth) *smallestVoxelWidth = -this->nWidth.y;
    if (-this->nWidth.z < *smallestVoxelWidth) *smallestVoxelWidth = -this->nWidth.z;
    if (*smallestVoxelWidth < kEpsilon) std::cout << this->name << " svw: " << *smallestVoxelWidth << " pwidth " << pWidth << " nwidth " << nWidth << " epsilon " << kEpsilon << std::endl;
    // this->surfaceArea = fabs((pWidth.x - nWidth.x) * 2 + (pWidth.y - nWidth.y) * 2 + (pWidth.z - nWidth.z) * 2);
}

void Octree::resize(octreeType type) {
    //We find the max positive point and max negative point
    // std::cout << "RESIZING" << std::endl;

    Vec3<float> tempP = Vec3<float>(0.0);
    Vec3<float> tempM = Vec3<float>(0.0);

    Vec3<float> minPos = Vec3<float>(std::numeric_limits<float>::infinity());
    Vec3<float> maxPos = Vec3<float>(-std::numeric_limits<float>::infinity());
    for (int i = 0; i < objects.size(); i++) {
        if (objects[i]->getType() != Shapes::CAMERA) {
            // Find the lowest and highest points of the current object
            tempP = *objects[i]->getPos() + objects[i]->getWidth();
            tempM = *objects[i]->getPos() - objects[i]->getWidth();
            if (tempP.x > maxPos.x) maxPos.x = tempP.x;
            if (tempP.x < minPos.x) minPos.x = tempP.x;
            if (tempM.x > maxPos.x) maxPos.x = tempM.x;
            if (tempM.x < minPos.x) minPos.x = tempM.x;

            if (tempP.y > maxPos.y) maxPos.y = tempP.y;
            if (tempP.y < minPos.y) minPos.y = tempP.y;
            if (tempM.y > maxPos.y) maxPos.y = tempM.y;
            if (tempM.y < minPos.y) minPos.y = tempM.y;

            if (tempP.z > maxPos.z) maxPos.z = tempP.z;
            if (tempP.z < minPos.z) minPos.z = tempP.z;
            if (tempM.z > maxPos.z) maxPos.z = tempM.z;
            if (tempM.z < minPos.z) minPos.z = tempM.z;
        }
    }
    if (type == NONUNIFORM) {
        this->midpoint = (maxPos + minPos) / 2;
        this->pWidth = (maxPos - midpoint).abs();
        this->nWidth = (minPos - midpoint);
        std::shared_ptr<Vec3<float>> mp = computeNewCentre();
        if (mp != nullptr) {
            setMidpoint(*mp);
        }
        else 
            this->midpoint = (maxPos + minPos) / 2;
    } else {    
        this->midpoint = (maxPos + minPos) / 2;
    }
    this->pWidth = (maxPos - midpoint).abs();
    this->nWidth = (minPos - midpoint);
    svwUpdate();
}

void Octree::split(octreeType type) {
    bool cont = true;
    if (type == NONUNIFORM) {
        std::shared_ptr<Vec3<float>> nc = computeNewCentre();
        if (nc == nullptr)
            cont = false;
        else 
            setMidpoint(*nc);
    }
    if (cont) {   
        this->subdivided = true;
        Vec3<float> mid = Vec3<float>(0);
        
        Vec3<float> pMax = midpoint + pWidth; 
        Vec3<float> pMin = midpoint + nWidth;
        // Octree* subd = (Octree *)malloc(8*sizeof(Octree));
        for (int i = 1; i <= 8; i++) {
            Vec3<float> tMax = Vec3<float>(0);
            Vec3<float> tMin = Vec3<float>(0);
            if (i < 5) {
                mid.y = ( (pMax.y + midpoint.y) / 2);
                tMin.y = midpoint.y;
                tMax.y = pMax.y;
            } else {
                mid.y = ( (midpoint.y + pMin.y) / 2);
                tMax.y = midpoint.y;
                tMin.y = pMin.y;
            }
            if (i % 2 == 0) {
                mid.x = ( (pMax.x + midpoint.x) / 2);
                tMin.x = midpoint.x;
                tMax.x = pMax.x;
            } else {
                mid.x = ( (midpoint.x + pMin.x) / 2);
                tMax.x = midpoint.x;
                tMin.x = pMin.x;
            }
            if (i == 3 || i == 4 || i == 7 || i == 8) {
                mid.z = ( (pMax.z + midpoint.z) / 2);
                tMin.z = midpoint.z;
                tMax.z = pMax.z;
            } else {
                mid.z = ( (midpoint.z + pMin.z) / 2);
                tMax.z = midpoint.z;
                tMin.z = pMin.z;
            }
            std::string k = "";
            try {
                k = boost::lexical_cast<std::string>(this->name) + boost::lexical_cast<std::string>(i);
                Vec3<float> newpWidth = tMax - mid;
                Vec3<float> newnWidth = tMin - mid;
                unsigned long newname = boost::lexical_cast<unsigned long>(k);
                Octree* newleaf = new Octree(newname, mid, newpWidth, newnWidth, false, this->root, this->smallestVoxelWidth, this->allOctrees);
                leaves.push_back(newleaf);
            } catch (boost::bad_lexical_cast &) {
                std::cerr << "Bad lexical cast on creation of new octree for name in split()\nTo string: " << this->name << " and: " << i << " to int: " << k << std::endl;
            }
        }
        for (int i = 0; i < objects.size(); i++) {
            Vec3<float> pos = *objects[i]->getPos();
            Vec3<float> w = objects[i]->getWidth();
            if (pos.x + w.x > midpoint.x || pos.x - w.x > midpoint.x) {
                if (pos.y + w.y > midpoint.y || pos.y - w.y > midpoint.y) {
                    if (pos.z + w.z > midpoint.z || pos.z - w.z > midpoint.z) {
                        leaves[3]->addShape(objects[i]);
                    }
                    if (pos.z + w.z < midpoint.z || pos.z - w.z < midpoint.z) {
                        leaves[1]->addShape(objects[i]);
                    }
                }
                if (pos.y + w.y < midpoint.y || pos.y - w.y < midpoint.y) {
                    if (pos.z + w.z > midpoint.z || pos.z - w.z > midpoint.z) {
                        leaves[7]->addShape(objects[i]);
                    }
                    if (pos.z + w.z < midpoint.z || pos.z - w.z < midpoint.z) {
                        leaves[5]->addShape(objects[i]);
                    }
                }
            }
            if (pos.x + w.x < midpoint.x || pos.x - w.x < midpoint.x) {
                if (pos.y + w.y > midpoint.y || pos.y - w.y > midpoint.y) {
                    if (pos.z + w.z > midpoint.z || pos.z - w.z > midpoint.z) {
                        leaves[2]->addShape(objects[i]);
                    }
                    if (pos.z + w.z < midpoint.z || pos.z - w.z < midpoint.z) {
                        leaves[0]->addShape(objects[i]);
                    }
                }
                if (pos.y + w.y < midpoint.y || pos.y - w.y < midpoint.y) {
                    if (pos.z + w.z > midpoint.z || pos.z - w.z > midpoint.z) {
                        leaves[6]->addShape(objects[i]);
                    }
                    if (pos.z + w.z < midpoint.z || pos.z - w.z < midpoint.z) {
                        leaves[4]->addShape(objects[i]);
                    }
                }
            }
        }
        root->voxels = root->countVoxels();
    }
}

void Octree::unsplit() {
    /*for (int i = 0; i < leaves.size(); i++) {
        leaves[i]->unsplit();
    }*/
    subdivided = false;
    for (int i = 0; i < leaves.size(); i++)
    	delete leaves[i];
    leaves.clear();
}

void Octree::optimize(octreeType type) {
    if (!subdivided && objects.size() > 0) {
        if (type == REGULAR) {
            // std::cout << "\nBefore " << std::endl;
            float before = this->root->SAH(type);
            // float before = this->SAH(type);
            // std::cout << before << std::endl;
            this->split(REGULAR);
            // std::cout << "After: " << std::endl;
            float after = this->root->SAH(type);
            // float after = this->SAH(type);
            // std::cout << after << std::endl;
            // std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n');
            if (before <= after) {
                this->unsplit();
                root->voxels = root->countVoxels();
            } else {
                for (int i = 0; i < leaves.size(); i++) {
                    leaves[i]->svwUpdate();
                    leaves[i]->optimize(type);
                    (*allOctrees)[leaves[i]->name] = leaves[i];
                }
            }
        } else {
            // std::cout << "\nBefore " << std::endl;
            float before = this->root->SAH(type);
            // float before = this->SAH(type);
            // std::cout << before << std::endl;
            this->split(NONUNIFORM);

            // std::cout << "After: " << std::endl;
            float after = this->root->SAH(type);
            // float after = this->SAH(type);
            // std::cout << after << std::endl;
            // std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n');
            if (before <= after) {
                // std::cout << "unsplit" << std::endl;
                this->unsplit();
                root->voxels = root->countVoxels();
            } else {
                for (int i = 0; i < leaves.size(); i++) {
                    leaves[i]->svwUpdate();
                    leaves[i]->optimize(type);
                    (*allOctrees)[leaves[i]->name] = leaves[i];
                }
            }
        }
    }
}

Octree* Octree::findleaf(const Vec3<float> &p, RayType type) {
    bool in = p.between(midpoint + nWidth, midpoint + pWidth,kEpsilon);
    if (type == PrimaryRay && !in) {
        return nullptr;
    }

    Octree* temp = this;
    int id = this->name;
    while (temp->subdivided) {
        // std::cout << id << std::endl;
        if (p.x > temp->midpoint.x) {
            // std::cout << "x>m" << std::endl;
            if (p.y > temp->midpoint.y) {
                // std::cout << "y>m" << std::endl;
                if (p.z > temp->midpoint.z) {
                    id = (id * 10) + 4;
                } else {
                    id = (id * 10) + 2;
                }
            } else {
                // std::cout << "y<m" << std::endl;
                if (p.z > temp->midpoint.z) {
                    id = (id * 10) + 8;
                } else {
                    id = (id * 10) + 6;
                }
            }
        } else {
            // std::cout << "x<m" << std::endl;
            if (p.y > temp->midpoint.y) {
                // std::cout << "y>m" << std::endl;
                if (p.z > temp->midpoint.z) {
                    // std::cout << "\t" << id << std::endl;
                    id = (id * 10) + 3;
                    // std::cout << "\t" << id << std::endl;
                } else {
                    // std::cout << "z<m" << std::endl;
                    id = (id * 10) + 1;
                }
            } else {
                // std::cout << "y<m" << std::endl;
                if (p.z > temp->midpoint.z) {
                    id = (id * 10) + 7;
                } else {
                    id = (id * 10) + 5;
                }
            }
        }
        temp = findOctree(id);
        if (temp == nullptr) {
            // std::cout << "returning null" << std::endl;
            return nullptr;
        }
        // std::cout <<"next name: " << temp->name << std::endl;
    }
    // std::cout << "returning " << temp->name << std::endl;
    return temp;
}

Octree* Octree::findOctree(unsigned long name) {
	std::unordered_map<unsigned long,Octree*>::const_iterator got = this->root->allOctrees->find(name);

	if (got == this->root->allOctrees->end() )
		return nullptr;
	else 
		return got->second;
    // std::vector<Octree*> all = *this->root->allOctrees;
    // int i = 0;
    // while (i < all.size()) {
    //     if (all[i]->name == name) {
    //         return all[i];
    //     }
    //     i++;
    // }
    // return nullptr;
}

int Octree::countVoxels() {
    if (subdivided) {
        int ret = 0;
        for (int i = 0; i < leaves.size(); i++) 
            ret += leaves[i]->countVoxels();
        return ret;
    } else {
        return 1;
    }
}

std::shared_ptr<float> Octree::intersectsWith(const RAY_STRUCT<float> &ray) {
    const float kEpsilon = 0.00001;
    bool ret = true;

    Vec3<float> t1 = Vec3<float>(0);
    Vec3<float> t2 = Vec3<float>(0);

    Vec3<float> xnormal = Vec3<float>(0,1,0);
    float x = xnormal.dotProduct(ray.nvector);
    if (fabs(x) > kEpsilon) {
        t1.x = (midpoint + Vec3<float>(0, pWidth.y, 0) - ray.point).dotProduct(xnormal) / x;
        t2.x = (midpoint + Vec3<float>(0, nWidth.y, 0) - ray.point).dotProduct(xnormal) / x;
        if (!subdivided) {
            //If a leaf node, then we set t1 as the biggest element
            if (t2.x - t1.x > kEpsilon) std::swap(t1.x,t2.x);
            if (t1.x < kEpsilon) ret = false;
        } else {
            //If it's not a leaf node then we set t1 as the smallest element above 0
            if (t1.x - t2.x > kEpsilon) std::swap(t1.x,t2.x);
            if (t2.x < kEpsilon) ret = false;
        } 
    }

    Vec3<float> ynormal = Vec3<float>(0,0,1);
    float y = ynormal.dotProduct(ray.nvector);
    if (fabs(y) > kEpsilon) {
        t1.y = (midpoint + Vec3<float>(0, 0, pWidth.z) - ray.point).dotProduct(ynormal) / y;
        t2.y = (midpoint + Vec3<float>(0, 0, nWidth.z) - ray.point).dotProduct(ynormal) / y;
        if (!subdivided) {   
            if (t2.y - t1.y > kEpsilon ) std::swap(t1.y,t2.y);
            if (t1.y < kEpsilon) ret = false;
        } else {
            if (t1.y - t2.y > kEpsilon) std::swap(t1.y,t2.y);
            if (t2.y < kEpsilon) {
                ret = false;
            }
        }
    }

    Vec3<float> znormal = Vec3<float>(1, 0, 0);
    float z = znormal.dotProduct(ray.nvector);
    if (fabs(z) > kEpsilon) {
        t1.z = (midpoint + Vec3<float>(pWidth.x, 0, 0) - ray.point).dotProduct(znormal) / z;
        t2.z = (midpoint + Vec3<float>(nWidth.x, 0, 0) - ray.point).dotProduct(znormal) / z;
        if (!subdivided) {
            if (t2.z - t1.z > kEpsilon) std::swap(t1.z, t2.z);
            if (t1.z < kEpsilon) ret = false;
        } else {
            if (t1.z - t2.z > kEpsilon) std::swap(t1.z,t2.z);
            if (t2.z < kEpsilon) {
                ret = false;
            }
        }
    }
    // if (ray.x == 1172 && ray.y == 426 )
    //     std::cout << this->name << " intersectsWith " << t1 << " " << t2 << " ret: " << ret << std::endl;
    if (ret == false)
        return nullptr;
    float t1p, t2p;
    // We want to take all the non 0 elements of t1 and t2, so we set the 0 elements to values that cant be chosen by the max and min functions

    if (!subdivided) {
        if (t1.x == 0 && t2.x == 0) {
            t1.x = t1.max() + 1;
            t2.x = t2.min() - 1;
        }
        if (t1.y == 0 && t2.y == 0) {
            t1.y = t1.max() + 1;
            t2.y = t2.min() - 1;
        }
        if (t1.z == 0 && t2.z == 0) {
            t1.z = t1.max() + 1;
            t2.z = t2.min() - 1;
        }
        t1p = t1.min();
        t2p = t2.max();
        if (t1p > t2p) return std::make_shared<float>(t1p);
        else return nullptr;
    } else {
        if (t1.x == 0 && t2.x == 0) {
            t1.x = t1.min() - 1;
            t2.x = t2.max() + 1;
        }
        if (t1.y == 0 && t2.y == 0) {
            t1.y = t1.min() - 1;
            t2.y = t2.max() + 1;
        }
        if (t1.z == 0 && t2.z == 0) {
            t1.z = t1.min() - 1;
            t2.z = t2.max() + 1;
        }
        t1p = t1.max();
        t2p = t2.min();
        if (t2p > t1p) return std::make_shared<float>(t1p);
        else return nullptr;
    }
}

Octree* Octree::next(const RAY_STRUCT<float> &ray, RayType type) {
    Vec3<float> nvp;
    // if (!ray.point.between(midpoint + pWidth, midpoint + nWidth)) {
    std::shared_ptr<float> dist = this->intersectsWith(ray); 
    if (dist == nullptr) {            
        // if (type == ShadowRay && ray.x == 1172 && ray.y == 426) 
        //     std::cout << "dist == nullptr" << std::endl;
        return nullptr;
    } else {            
        nvp = ray.point + (*dist * ray.nvector) + (*smallestVoxelWidth / 200 * ray.nvector);
        // if (ray.x == 447 && ray.y == 2) {
        //     std::cout << "Ray: p " << ray.point << " vector " << ray.nvector << std::endl;
        //     std::cout << "dist: " << *smallestVoxelWidth << " " << (*smallestVoxelWidth / 5 * ray.nvector) << " nvp: " << nvp << std::endl;
        //     std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n');
        // } 
        if (nvp.between(root->midpoint + root->nWidth, root->midpoint + root->pWidth,kEpsilon) ) {
            Octree* next = this->root->findleaf(nvp,type);
            if (next->name == this->name ) {
                // std::cerr << "Returning null" << std::endl;
                return nullptr;
            }
            // std::cout << "findleaf: " << this->root->findleaf(nvp,type)->name << std::endl;
            return next;
        }
        else
            return nullptr;
    }
            // std::cout << *smallestVoxelWidth << std::endl;
            /*if (ray.x == 559 && ray.y == 245) {
                if (type == PrimaryRay)
                    std::cout << "Primary ray: " << std::endl;
                else 
                    std::cout << "Shadow ray: " << std::endl;
                std::cout << this->name << ":\n\tDistance: " << *dist << "\n\t" << ray.point + (*dist * ray.nvector) << " " << (*smallestVoxelWidth * ray.nvector) << " = point " << nvp << " leaf " << this->root->findleaf(nvp)->name << std::endl;
                std::cout << "smallestVoxelWidth: " << *smallestVoxelWidth << std::endl;
            }*/
      /*  
    } else {
        nvp = ray.point ;                        
        if (ray.x == 559 && ray.y == 245) {
            if (type == PrimaryRay)
                    std::cout << "Primary ray: " << std::endl;
                else 
                    std::cout << "Shadow ray: " << std::endl;
            std::cout << this->name << ": " << ray.point<< " " << (*smallestVoxelWidth * ray.nvector) << " = point " << nvp << " leaf " << this->root->findleaf(nvp)->name << std::endl;
            std::cout << "smallestVoxelWidth: " << *smallestVoxelWidth << std::endl;
        }
    }*/
}

bool Octree::trace(const RAY_STRUCT<float> &ray, IsectInfo &isect, RayType type) {
    // if (type == ShadowRay && ray.x == 950 && ray.y == 530) 
    // std::cout << "\nName: " << this->name << " " << ray.x << " " << ray.y << std::endl;
    if (!subdivided) {
        if (name == 0)
            if (this->intersectsWith(ray) == nullptr)
                return false;
        isect.hitObject = nullptr;
        isect.tNear = std::numeric_limits<float>::infinity();
        float closestD = std::numeric_limits<float>::infinity();
        Vec2<float> uv = Vec2<float>(0,0);
        std::shared_ptr<int> triIndex = std::make_shared<int>(0);
        Vec3<float> min = midpoint + nWidth;
        Vec3<float> max = midpoint + pWidth;

        for (int i = 0; i < objects.size(); i++) {
            if (objects[i]->intersect(ray, closestD, triIndex, uv)) {
                if (closestD < isect.tNear) {
                	Vec3<float> hitpoint = ray.point + (ray.nvector * closestD);
                	if (hitpoint.between(min,max,0.00001)) {
                        // if (type == ShadowRay && ) continue;
                        isect.hitObject = objects[i];
                        isect.tNear = closestD;
                        isect.index = *triIndex;
                        isect.uv = uv;
                        isect.octreeName = this->name;
                        // if (type == PrimaryRay && ray.x == 1661 && ray.y == 34) {
                        // 	std::cout << "Between!" << std::endl;
                        // }
                	}
                    // if (type == PrimaryRay && ray.x == 1661 && ray.y == 34) {
                    //     std::cout << "Hit in: " << this->toString() << "\nHitpoint: " << hitpoint << "\nHit Object: " << objects[i]->toString() << std::endl;
                    // }
                }
            }
        }
        if (isect.hitObject != nullptr) {
            // if (type == ShadowRay && ray.x == 950 && ray.y == 530)
            //     std::cout << "Return true" << std::endl;
            return true;
        } else {
	    	// if (type == PrimaryRay && ray.x == 949 && ray.y == 487) {
      //           std::cout << "Dont hit anything in " << this->name << std::endl;
      //       }
        }
    } 

    Octree* temp = this->next(ray,type);

    if (temp == nullptr) {
        return false;
    } else {
        // std::cout << "Next " << temp->name << std::endl;
        return temp->trace(ray, isect, type);
    }
}

float Octree::SAH(octreeType type) {
    SAHInfo sah;
    calculateCosts(type);
    this->root->collectSAHInfo(sah);
    // std::cout << "Costs: " << traverseCost << " " << intersectCost << std::endl;
    // std::cout << "SAH info: " << sah.SAInterior << " " << sah.SALeaf << " " << sah.SALeafObj << std::endl;
    // std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n');

    // std::cout << this->name << " intersect " << intersectCost << " surfaceArea " << surfaceArea << " traverse " << traverseCost << std::endl;
    // std::cout << "widths " << pWidth << "  " << nWidth << std::endl;
    if (!subdivided) {        
        // std::cout << "SAH: " << intersectCost << " * " << objects.size() << " / " << surfaceArea << std::endl;
        return traverseLCost + (intersectCost * static_cast<float>(objects.size()));
    } else {

        return (traverseICost * sah.SAInterior + traverseLCost * sah.SALeaf + intersectCost * sah.SALeafObj) / surfaceArea;
        // std::cout << "SAH: " << traverseCost << " * " << surfaceArea << " + " << intersectCost << " * " << temp << " / " << surfaceArea << std::endl;
    }

}

void Octree::collectSAHInfo(SAHInfo &inf) {
    if (!subdivided) {
        // std::cout << "surfacearea: " << this->surfaceArea << std::endl;
        inf.SALeaf += this->surfaceArea;
        inf.SALeafObj += this->surfaceArea * objects.size();
    } else {
        inf.SAInterior += this->surfaceArea;
        for (int i = 0; i < leaves.size(); i++) 
            leaves[i]->collectSAHInfo(inf);
    }
}

std::vector<Vec3<float>> Octree::pBetween(Vec3<float> p1, Vec3<float> p2, bool reduced) {
    std::vector<Vec3<float>> ret = std::vector<Vec3<float>>();
    if (p1 == p2)
        return ret;

    // Make p1 the smallest value of the octree space and p2 the largest
    if (p1.x > p2.x)
        std::swap(p1.x,p2.x);
    if (p1.y > p2.y)
        std::swap(p1.y,p2.y);
    if (p1.z > p2.z)
        std::swap(p1.z,p2.z);

    Vec3<float> max = midpoint + pWidth;
    Vec3<float> min = midpoint + nWidth;

    p1.cap(min, max);
    p2.cap(min, max); 

    // const float kEpsilon = 0.000001;
    // std::cout << "SMALLEST " << p1 << " LARGEST " << p2 << std::endl;
    // Create our return vector
    for (int i = 0; i < objects.size(); i++) {
        Vec3<float> plus = *objects[i]->getPos() + objects[i]->getWidth();
        Vec3<float> minus = *objects[i]->getPos() - objects[i]->getWidth();
        // std::cout << "plus " << plus << " minus " << minus << std::endl;
        // We check whether any of the corner bounds of the object are within the two given points, if so we add that point to the return vector.
        if (reduced) {   
            if (plus.x - p1.x > kEpsilon && p2.x - plus.x > kEpsilon) {
                if (plus.y - p1.y > kEpsilon && p2.y - plus.y > kEpsilon) {
                    if (plus.z - p1.z > kEpsilon && p2.z - plus.z > kEpsilon) 
                        ret.push_back(plus);
                    if (minus.z - p1.z > kEpsilon && p2.z - minus.z > kEpsilon)
                        ret.push_back(Vec3<float>(plus.x, plus.y, minus.z));
                } 
                if (minus.y - p1.y > kEpsilon && p2.y - minus.y > kEpsilon) {
                    if (plus.z - p1.z > kEpsilon && p2.z - plus.z > kEpsilon)
                        ret.push_back(Vec3<float>(plus.x, minus.y, plus.z));
                    if (minus.z - p1.z > kEpsilon && p2.z - minus.z > kEpsilon)
                        ret.push_back(Vec3<float>(plus.x, minus.y, minus.z));
                }
            } 
            if (minus.x - p1.x > kEpsilon && p2.x - minus.x > kEpsilon) {
                if (plus.y - p1.y > kEpsilon && p2.y - plus.y > kEpsilon) {
                    if (plus.z - p1.z > kEpsilon && p2.z - plus.z > kEpsilon)
                        ret.push_back(Vec3<float>(minus.x, plus.y, plus.z));
                    if (minus.z - p1.z > kEpsilon && p2.z - minus.z > kEpsilon)
                        ret.push_back(Vec3<float>(minus.x, plus.y, minus.z));
                } 
                if (minus.y - p1.y > kEpsilon && p2.y - minus.y > kEpsilon) {
                    if (plus.z - p1.z > kEpsilon && p2.z - plus.z > kEpsilon)
                        ret.push_back(Vec3<float>(minus.x, minus.y, plus.z));
                    if (minus.z - p1.z > kEpsilon && p2.z - minus.z > kEpsilon)
                        ret.push_back(minus);
                }
            }
        } else {
            std::vector<bool> added = std::vector<bool>(8, false);
            if (plus.x - p1.x > kEpsilon && p2.x - plus.x > kEpsilon) {
                ret.push_back(plus);
                added[0] = true;
                ret.push_back(Vec3<float>(plus.x,plus.y,minus.z));
                added[1] = true;
                ret.push_back(Vec3<float>(plus.x,minus.y,plus.z));
                added[2] = true; 
                ret.push_back(Vec3<float>(plus.x,minus.y,minus.z));
                added[3] = true;
            }
            if (plus.y - p1.y > kEpsilon && p2.y - plus.y > kEpsilon) {
                if (!added[0]) {
                    ret.push_back(plus);
                    added[0] = true;
                }
                if (!added[1]) {
                    ret.push_back(Vec3<float>(plus.x,plus.y,minus.z));
                    added[1] = true;
                }
                ret.push_back(Vec3<float>(minus.x,plus.y,plus.z));
                added[4] = true;
                ret.push_back(Vec3<float>(minus.x,plus.y,minus.z));
                added[5] = true;
            }
            if (plus.z - p1.z > kEpsilon && p2.z - plus.z > kEpsilon) {
                if (!added[0]) {
                    ret.push_back(plus);
                }
                if (!added[4]) {
                    ret.push_back(Vec3<float>(minus.x,plus.y,plus.z));
                    added[4] = true;
                }
                if (!added[2]) {
                    ret.push_back(Vec3<float>(plus.x,minus.y,plus.z));
                    added[2] = true;
                }
                ret.push_back(Vec3<float>(minus.x,minus.y,plus.z));
                added[6] = true;
            }
            if (minus.x - p1.x > kEpsilon && p2.x - minus.x > kEpsilon) {
                ret.push_back(minus);
                added[7] = true;
                if (!added[4]) {
                    ret.push_back(Vec3<float>(minus.x,plus.y,plus.z));
                }
                if (!added[5]) {
                    ret.push_back(Vec3<float>(minus.x,plus.y,minus.z));
                    added[5] = true;
                }
                if (!added[6]) {
                    ret.push_back(Vec3<float>(minus.x,minus.y,plus.z));
                    added[6] = true;
                }
            }
            if (minus.y - p1.y > kEpsilon && p2.y - minus.y > kEpsilon) {
                if (!added[7]) {
                    ret.push_back(minus);
                    added[7] = true;
                }
                if (!added[6]) {
                    ret.push_back(Vec3<float>(minus.x,minus.y,plus.z));
                }
                if (!added[2]) {
                    ret.push_back(Vec3<float>(plus.x,minus.y,plus.z));
                }
                if (!added[3]) {
                    ret.push_back(Vec3<float>(plus.x,minus.y,minus.z));
                    added[3] = true;
                }
            }
            if (minus.z - p1.z > kEpsilon && p2.y - minus.y > kEpsilon) {
                if (!added[3]) {
                    ret.push_back(Vec3<float>(plus.x,minus.y,minus.z));
                }
                if (!added[7]) {
                    ret.push_back(minus);
                }
                if (!added[5]) {
                    ret.push_back(Vec3<float>(minus.x,plus.y,minus.z));
                }
                if (!added[1]) {
                    ret.push_back(Vec3<float>(plus.x,plus.y,minus.z));
                }   
            }

        }
    }

    // std::cout << "\tsmallest  " << p1 << "\n\tlargest  " << p2 << std::endl;
    // printf("\tsmallest %Lf,%Lf,%Lf\n\tlargest %Lf,%Lf,%Lf\n",p1.x,p1.y,p1.z,p2.x,p2.y,p2.z);

    // for (int i = 0; i < ret.size(); i++) {
    //     printf("%Lf,%Lf,%Lf\n",ret[i].x,ret[i].y,ret[i].z);
    //     // std::cout << ret[i] << std::endl;
    // }
    return ret;
}


float Octree::midpointScore(Vec3<float> mp) {
    // std::cout << "nWidth: " << nWidth.toString() << " pWidth: " << pWidth.toString() << std::endl;
    Vec3<float> leftEdge = (midpoint + nWidth);
    Vec3<float> rightEdge = (midpoint + pWidth);
    Vec3<float> width = pWidth - nWidth;

    Vec3<float> t = (mp - leftEdge);
    t = t / (rightEdge - leftEdge);

    /*Vec3<float> LSA = (mp - leftEdge).abs();
    LSA.x = LSA.x * width.z * 2 + width.y * LSA.x * 2 + width.z * width.y * 2;
    LSA.y = width.x * width.z * 2 + LSA.y * width.x * 2 + width.z * LSA.y * 2;
    LSA.z = width.x * LSA.z * 2 + width.y * width.x * 2 + LSA.z * width.y * 2;
    Vec3<float> RSA = (mp - rightEdge).abs();
    RSA.x = RSA.x * width.z * 2 + width.y * RSA.x * 2 + width.z * width.y * 2;
    RSA.y = width.x * width.z * 2 + RSA.y * width.x * 2 + width.z * RSA.y * 2;
    RSA.z = width.x * RSA.z * 2 + width.y * width.x * 2 + RSA.z * width.y * 2;

    if (LSA.x > surfaceArea || RSA.x > surfaceArea)
        std::cout << "LSA: " << LSA << " RSA: " << RSA << std::endl;*/

    Vec3<float> left = Vec3<float>(0.0);
    Vec3<float> right = Vec3<float>(0.0);
    for (int i = 0; i < objects.size(); i++) {
        Vec3<float> objPos =* objects[i]->getPos();
        Vec3<float> objWidth = objects[i]->getWidth();
        
        if (objPos.x < mp.x) {
            if (objPos.x < leftEdge.x)
                left.x += (objWidth.x - fabs(objPos.x - leftEdge.x)) / objWidth.x;
            else
                left.x++;
        } else {
            if (objPos.x > rightEdge.x)
                right.x += (objWidth.x - fabs(objPos.x - rightEdge.x)) / objWidth.x;
            else
                right.x++;
        }

        if (objPos.y < mp.y) {
            if (objPos.y < leftEdge.y)
                left.y += (objWidth.y - fabs(objPos.y - leftEdge.y)) / objWidth.y;
            else
                left.y++;
        } else {
            if (objPos.y > rightEdge.y)
                right.y += (objWidth.y - fabs(objPos.y - rightEdge.y)) / objWidth.y;
            else
                right.y++;
        }

        if (objPos.z < mp.z) {
            if (objPos.z < leftEdge.z)
                left.z += (objWidth.z - fabs(objPos.z - leftEdge.z)) / objWidth.z;
            else
                left.z++;
        } else {
            if (objPos.z > rightEdge.z)
                right.z += (objWidth.z - fabs(objPos.z - rightEdge.z)) / objWidth.z;
            else
                right.z++;
        }        
    }
    // if (LSA < 0 || RSA < 0) 
    //     std::cout << "OH NO WE GOT PROBLEMS " << LSA << " " << RSA << std::endl;
    // std::cout << "LSA " << LSA << " RSA " << RSA << std::endl; 
    // return ((LSA.x * left.x) + (RSA.x * right.x)) * 
    //         ((LSA.y * left.y) + (RSA.y * right.y)) *
    //         ((LSA.z * left.z) + (RSA.z * right.z));
    if (t - 1.f > kEpsilon)
        std::cout << "OH NO WE GOT PROBLEMS " << t << std::endl;
    // std::cout << "t: " << t << "  left: " << left << "  right: " << right << std::endl;
    return ( ( (t.x * left.x) + ( (1 - t.x) * right.x ) ) *
            ( (t.y * left.y) + ( (1 - t.y) * right.y ) ) *
            ( (t.z * left.z) + ( (1 - t.z) * right.z ) ) );
}

double Octree::mps(unsigned n, const double *x, double *grad, void *my_func_data) {
    mps_data *data = reinterpret_cast<mps_data*>(my_func_data);

    Vec3<double> leftEdge = (data->midpoint + data->nWidth);
    Vec3<double> rightEdge = (data->midpoint + data->pWidth);
    Vec3<double> width = data->pWidth - data->nWidth;
    Vec3<double> mp = Vec3<double>(x[0], x[1], x[2]);
	Vec3<double> t = (mp - leftEdge);
    // t = t / width;

    Vec3<double> left = Vec3<double>(0.0);
    Vec3<double> right = Vec3<double>(0.0);
    for (int i = 0; i < data->objects->size(); i++) {
        Vec3<double> max = *(*data->objects)[i]->getPos() + (*data->objects)[i]->getWidth();
        Vec3<double> min = *(*data->objects)[i]->getPos() - (*data->objects)[i]->getWidth();
        if (min.x < mp.x) {
            left.x++;
        }
        if (max.x > mp.x) {
        	right.x++;
        }
        if (min.y < mp.y) {
        	left.y++;
        }
        if (max.y > mp.y) {
        	right.y++;
        }
        if (min.z < mp.z) {
        	left.z++;
        }
        if (max.z > mp.z) {
        	right.z++;
        }
    }
    // if (t.max() - 1.0 > 0.0001 || t > 1.0) 
    // 	std::cout << "t is too small/large " << t << " " << mp << "\n lE: " << leftEdge << "\n rE: " << rightEdge << "  mp - leftedge: " << (mp - leftEdge) << "\n rE - lE: " << (rightEdge - leftEdge) << "\nWidth: " << width << std::endl;

    return ( ( (t.x * left.x) + ( (width.x - t.x) * right.x ) ) +
            ( (t.y * left.y) + ( (width.y - t.y) * right.y ) ) +
            ( (t.z * left.z) + ( (width.z - t.z) * right.z ) ) );
}

std::shared_ptr<Vec3<float>> Octree::computeNewCentre() {

    Vec3<float> avgPoint = Vec3<float>(0.0);
    for (int i = 0; i < objects.size(); i++) {
        avgPoint += *objects[i]->getPos();
    }
    avgPoint = avgPoint / objects.size();
    Vec3<float> wMax = midpoint + pWidth;
    Vec3<float> wMin = midpoint + nWidth;

    // Vec3<double> pMin = wMin - (nWidth / 10);
    // Vec3<double> pMax = wMax - (pWidth / 10);
    Vec3<double> pMin = avgPoint;
    Vec3<double> pMax = midpoint;

    pMin.cap(wMin, wMax);
    pMax.cap(wMin, wMax);
    if (pMin.x > pMax.x) std::swap(pMin.x,pMax.x);
    if (pMin.y > pMax.y) std::swap(pMin.y,pMax.y);
    if (pMin.z > pMax.z) std::swap(pMin.z,pMax.z);
    std::shared_ptr<Vec3<float>> ret = nullptr;

    if ( (wMax - pMax).min() < 0.1 || (pMin - wMin).min() < 0.1) {
	    return nullptr;
    }

    if (pWidth.min() > 1.0 && nWidth.max() < -1.0)
    	ret = std::make_shared<Vec3<float>>(pMax);
	
	double minf = 0.0;
	// std::cout << "pMax " << pMax << " pMin " <<pMin << std::endl;
	/*if ( (pMax - pMin).min() <= 0.05 ) {
		std::cout << pWidth.min() << " " << nWidth.max() << std::endl;
		if (pWidth.min() > 1.0) {	
    		pMax = pMax + 1; 
		}

		if (nWidth.max() < -1.0) {
			// std::cout << "asdasd" << std::endl;
    		pMin = pMin + 1;
    	}
	}*/

    if ( (pMax - pMin).min() > 0.05 ) {	    	

	    using namespace nlopt;
		std::vector<double> lb = {pMin.x, pMin.y, pMin.z};
		std::vector<double> ub = {pMax.x, pMax.y, pMax.z};
		root->optimiser.set_lower_bounds(lb);
		root->optimiser.set_upper_bounds(ub);

		mps_data *data = (mps_data*)calloc(1,sizeof(mps_data));
		data->midpoint = midpoint;
		data->nWidth = nWidth;
		data->pWidth = pWidth;
		data->objects = &objects;

        root->optimiser.set_maxeval(std::min(objects.size() + 8,(unsigned long int) 100));
		root->optimiser.set_min_objective(mps, data);
		// optimiser.set_maxtime(0.005);
        nlopt::result res;
        try {
    		res = root->optimiser.optimize(lb, minf);
    		if (res < 0) {
    			std::cout << res << " " << lb[0] << " " << lb[1] << " " << lb[2] << std::endl;
    	        ret = nullptr;
    	        // std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n');
    		} else if (minf > 0) {
    			ret = std::make_shared<Vec3<float>>(lb[0],lb[1],lb[2]);
    		}
        } catch (roundoff_limited) {
            ret = std::make_shared<Vec3<float>>(lb[0],lb[1],lb[2]);
            if (!ret->between(pMin,pMax,kEpsilon))
                ret = nullptr;
        }
		free(data);
    } else {
    	
    }

    return ret;

/*
    Vec3<float> objectMid = Vec3<float>(0,0,0);
    for (int i = 0; i < objects.size(); i++) {
        objectMid += *objects[i]->getPos();
    }
    if (objects.size() != 0)
        objectMid = objectMid / objects.size();

    std::vector<Vec3<float>> testPoints = pBetween(objectMid,this->midpoint);*/
 //    std::vector<Vec3<float>> testPoints = pBetween(pMin, pMax, false);
 //    // std::cout << testPoints.size() << " pmax " << pMax << " pMin " << pMin << std::endl;
 //    if (testPoints.size() == 0 && (pMax - pMin).min() > -0.5 && (pMax - pMin).min() < 0.5) {
 //        // if we cant find any in the reduced range, AND the reduced range is very small we expand our range.
 //        testPoints = pBetween(midpoint + nWidth, midpoint + pWidth, true);
 //    }
 //    // testPoints.push_back(this->midpoint);
 //    float min = std::numeric_limits<float>::infinity();
 //    Vec3<float> temp;
 //    for (int i = 0; i < testPoints.size(); i++) {
 //        if (testPoints[i].between(wMin, wMax, kEpsilon)) {
 //            float s = midpointScore(testPoints[i]);
 //            Vec3<float> big = (wMax - testPoints[i]);
 //            Vec3<float> small = (testPoints[i] - wMin);
 //            if (s < min && small.min() > 0.05 && 
 //                big.min() > 0.05 ) {
 //                min = s;
 //                temp = testPoints[i];
 //            }
 //        }
 //    }
 //    if (ret != nullptr ) {
 //    	if (min != std::numeric_limits<float>::infinity())
	// 		std::cout  /*<< "min: " << wMin << " max: " << wMax*/ << " mine: " << min << " my pos: " << temp << "\ntheirs: " << minf << " their pos: " << *ret <<  std::endl << std::endl;
 //    	else
	// 		std::cout /*<< "min: " << wMin << " max: " << wMax*/ << " mine: " << min << " my pos: null" << "\ntheirs: " << minf << " their pos: " << *ret <<  std::endl << std::endl;
 //    }
	// else
	//     std::cout /*<< "min: " << wMin << " max: " << wMax*/ << " mine: " << min << " my pos: " << temp << "\ntheirs: " << minf << " their pos: null" <<  std::endl << std::endl;
 //    return ret;
}

void Octree::calculateCosts(octreeType type) {
    Vec3<float> width = pWidth - nWidth;
    surfaceArea = fabs( 2 * ((width.x * width.z) + (width.y * width.x) + (width.z * width.y) ));

    traverseICost = this->root->voxels * this->root->voxels;
    traverseLCost = this->root->objects.size() / this->root->voxels; 
    float nlights = this->lights.size();
    float nobjs = 1;
    // if (!subdivided) {
    for (int i = 0; i < objects.size(); i++) {
        if (objects[i]->getType() == Shapes::TRIANGLEMESH) {
            nobjs += (objects[i]->getNTris() * 1.5);
        } else {
            nobjs += 1; //Values taken from bayesian tests
        }
    }
    intersectCost = nlights * nobjs * nobjs;
    // intersectCost = pow(nobjs,nlights);
    // }
    // std::cout << "id = " << this->name << " depth " << this->depth << std::endl;
}

Octree::~Octree() {
    // std::cout << this->toString();
    // for (int i = 0; i < leaves.size(); i++) {
    //     delete leaves[i];
    // }
    if (name == 0) {
        for (auto it = allOctrees->begin(); it != allOctrees->end(); ++it)
        	if (it->second != nullptr) {
        		delete it->second;
        	}
    }
    // for (int i = 0; i < leaves.size(); i++) {
    //     if (leaves[i] != nullptr)
    //         delete leaves[i];
    // }
    leaves.clear();
    // objandlight.clear();
    objects.clear();
    lights.clear();
}
