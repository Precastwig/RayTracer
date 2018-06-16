#ifndef BITMAPSAVE_H
#define BITMAPSAVE_H
#include <structs.h>

using namespace help;

class BitmapSave
{
    public:
        BitmapSave(int swidth, int sheight);
        void saveBMP(std::string filename);
        void convertRGBtoBMPBuffer();
        void setVal(int x, int y, int r, int g, int b);
        void setVal(int x, int y, std::shared_ptr<Vec3<float>> colour);
        void printimage();
        bool operator == (const BitmapSave &other) const {
            if (height != other.height) {
                std::cout << height << " " << other.height << std::endl;
                return false;
            }
            if (width != other.width) {
                std::cout << width << " " << other.width << std::endl;
                return false;
            }
            bool ret = true;
            Vec3<int> temp = Vec3<int>();
            Vec3<int> othertemp = Vec3<int>();
            for (int j = 0; j < height; j++) {
                for (int i = 0; i < width; i++) {
                    temp = *(*image)(i,j);
                    othertemp = *(*other.image)(i,j);
                    if (temp.x != othertemp.x) {
                        std::cout << i << " " << j << ": colour r: " << temp.x << " " << othertemp.x << std::endl;   
                        ret = false;
                    }
                    if (temp.y != othertemp.y) {
                        std::cout << i << " " << j << ": colour g: " << temp.y << " " << othertemp.y << std::endl;   
                        ret = false;
                    }
                    if (temp.z != othertemp.z) {
                        std::cout << i << " " << j << ": colour b: " << temp.z << " " << othertemp.z << std::endl;
                        ret = false;
                    }
                }
            }
            return ret;
        }
        virtual ~BitmapSave();
        boost::numeric::ublas::matrix<std::shared_ptr<Vec3<int>> > *image;
        int width;
        int height;
        long currentsize;
        BYTE* Buffer;
};

#endif //
