#include <bitmapsave.h>

using namespace help;

BitmapSave::BitmapSave(int swidth, int sheight) {
    image = new boost::numeric::ublas::matrix<std::shared_ptr<Vec3<int>> >(swidth, sheight);
    width = swidth;
    height = sheight;
    Buffer = nullptr;
}

void BitmapSave::setVal(int x, int y, int r, int g, int b) {
    if (x < 0 || y < 0 || x > width -1|| y > height -1) {
        std::cout << "Invalid input: " << x << ", "<< y;
    } else {
        if (r > 256) {
            std::cout << "(" << r;
        }
        if (g > 256) {
            std::cout << "," << g;
        }
        if (b > 256) {
            std::cout << "," << b << ")\n";
        }
        (*image)(x,y)= std::make_shared<Vec3<int>>(r % 256,g % 256,b % 256);
    }
}

void BitmapSave::setVal(int x, int y, std::shared_ptr<Vec3<float>> colour) {
    if (x < 0 || y < 0 || x > width -1|| y > height -1) {
        std::cout << "Invalid input: " << x << ", "<< y;
    } else {
        if ((*colour) > 1) {
            std::cout << "(" << (*colour).x << "," << (*colour).y << "," << (*colour).z << ")" << std::endl;
        }
        (*image)(x,y) = std::make_shared<Vec3<int>>((*colour).x * 255, (*colour).y * 255, (*colour).z * 255);
    }
}

void BitmapSave::convertRGBtoBMPBuffer() {
    int padding = 0;
    int scanlinebytes = width * 3;
    if (image == nullptr || width == 0 || height == 0) {
        //
    } else {
    while ((scanlinebytes + padding) % 4 != 0) {
        padding++;
    }
    currentsize = height * (scanlinebytes + padding);
    if (Buffer != nullptr) {
        delete(Buffer);
    }
    Buffer = new BYTE[currentsize];
    memset(Buffer, 0, currentsize);
    int red = 0;
    int green = 0;
    int blue = 0;
    std::shared_ptr<Vec3<int>> temp = std::make_shared<Vec3<int>>();
    long position;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            temp = (*image)(i,j);
            if (temp != nullptr) {
                red = temp->x;
                green = temp->y;
                blue = temp->z;
                position = (height - 1 - j) * (scanlinebytes + padding) + (i * 3);
                Buffer[position] = blue;
                Buffer[position+1] = green;
                Buffer[position+2] = red;
            }
        }
    }
    }
}

void BitmapSave::printimage() {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            std::shared_ptr<Vec3<int>> temp = (*image)(i,j);
            //Needs finishing
        }
    }
}

void BitmapSave::saveBMP(std::string filename) {
        BITMAPFILEHEADER bmfh;
        BITMAPINFOHEADER info;
        std::memset( &bmfh, 0, sizeof(BITMAPFILEHEADER));
        std::memset( &info, 0, sizeof(BITMAPINFOHEADER));

        bmfh.bfType = 0x4d42;
        bmfh.bfReserved1 = 0;
        bmfh.bfReserved1 = 0;
        bmfh.bfSize = 14 + sizeof(BITMAPINFOHEADER) + currentsize;
        //std::cout << sizeof(BITMAPFILEHEADER) << "\n";
        bmfh.bfOffBits = 0x36;

        info.biSize = 40;
        info.biWidth = width;
        info.biHeight = height;
        info.biPlanes = 1;
        info.biBitCount = 24;
        info.biCompression = 0;
        info.biSizeImage = 0;
        info.biXPelsPerMeter = 0x0ec4;
        info.biYPelsPerMeter = 0x0ec4;
        info.biClrUsed = 0;
        info.biClrImportant = 0;

        std::ofstream outputfile;
        outputfile.open(filename+".bmp", std::ios_base::out | std::ios_base::binary);
        outputfile.imbue(std::locale::classic());

        //outputfile.write((char *)&bmfh, sizeof(BITMAPFILEHEADER));
        outputfile.write((char *)&bmfh.bfType, 2);
        outputfile.write((char *)&bmfh.bfSize, 4);
        outputfile.write((char *)&bmfh.bfReserved1, 2);
        outputfile.write((char *)&bmfh.bfReserved2, 2);
        outputfile.write((char *)&bmfh.bfOffBits, 4);
        //std::cout << Buffer << "\n";

        //write info
        outputfile.write((char *)&info.biSize, 4);
        outputfile.write((char *)&info.biWidth, 4);
        outputfile.write((char *)&info.biHeight, 4);
        outputfile.write((char *)&info.biPlanes, 2);
        outputfile.write((char *)&info.biBitCount, 2);
        outputfile.write((char *)&info.biCompression, 4);
        outputfile.write((char *)&info.biSizeImage, 4);
        outputfile.write((char *)&info.biXPelsPerMeter, 4);
        outputfile.write((char *)&info.biYPelsPerMeter, 4);
        outputfile.write((char *)&info.biClrUsed, 4);
        outputfile.write((char *)&info.biClrImportant, 4);

        //Write bitmap data
        outputfile.write((char *)Buffer, currentsize);
        outputfile.close();
}

BitmapSave::~BitmapSave() {
    delete image;
    delete [] Buffer;
}
