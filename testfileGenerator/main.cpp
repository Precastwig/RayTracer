#include <fstream>
#include <iostream>
#include <math.h>
#include <boost/lexical_cast.hpp>

using namespace std;

#define PI 3.14159265

int main(int argc, char *argv[]){
	std::string ret = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE X3D PUBLIC \"ISO//Web3D//DTD X3D 3.2//EN\" \"http://www.web3d.org/specifications/x3d-3.2.dtd\"><X3D profile='Immersive' version='3.2' xmlns:xsd='http://www.w3.org/2001/XMLSchema-instance' xsd:noNamespaceSchemaLocation='http://www.web3d.org/specifications/x3d-3.2.xsd'>	<head> </head>\n<Scene>\n<Viewpoint position='0 0 3300' fieldOfView='1.2'/>\n<Background groundColor='0.43 0.54 0.72' skyColor='0.91 0.81 0.91'/>\n<DirectionalLight direction='1 0 -1' intensity='1'/>\n";

	std::string colour = "1 0.5 0";

	for (int bx = -1000; bx <=1000; bx += 2000) {
		for (int by = -1000; by <=1000; by += 2000) {
			for (int bz = -1000; bz <=1000; bz += 2000) {
				for (int xm = -250; xm <= 250; xm +=500) {
					for (int ym = -250; ym <= 250; ym += 500) {
						for (int zm = -250; zm <= 250; zm += 500) {
							int x = bx + xm;
							int y = by + ym;
							int z = bz + zm;
							int size = 15;
							for (int i = 0; i < 50; i++) {
								if (rand()% 2 == 0) {
									x += (rand() % 50);
								} else {
									x -= rand() % 50;
								}
								if (rand()% 2 == 0) {
									y += rand() % 50;
								} else {
									y -= rand() % 50;
								}
								if (rand()% 2 == 0) {
									z += rand() % 50;
								} else {
									z -= rand() % 50;
								}
								colour = boost::lexical_cast<string>(fabs((float)  (x % 100) / 100.0)) +
				" " + boost::lexical_cast<string>(fabs((float) (y % 100) / 100.0)) +
				" " + boost::lexical_cast<string>(fabs((float) (z % 100) / 100.0));
								ret += "<Transform translation='" +
			boost::lexical_cast<string>(x) + " " +
			boost::lexical_cast<string>(y) + " " +
			boost::lexical_cast<string>(z) + "'>\n<Shape>\n";
								ret += "<IndexedTriangleSet index='0 1 4 2 4 1 3 1 0 1 3 2 0 5 4 2 4 5 3 5 0 5 3 2' solid='true' creaseAngle='0'>\n<Coordinate point='"+  boost::lexical_cast<string>(size)+ " 0 0 0 0 "+  boost::lexical_cast<string>(size) + " "+  boost::lexical_cast<string>(-size) + " 0 0 0 " + boost::lexical_cast<string>(size) + " 0 0 "+  boost::lexical_cast<string>(-size) + " 0 0 0 " + boost::lexical_cast<string>(-size) +"'/>\n<Normal vector='1 0 0 0 0 1 -1 0 0 0 1 0 0 -1 0 0 0 -1'/>\n<Appearance>\n<Material diffuseColor='" + colour + "'/>\n</Appearance>\n</IndexedTriangleSet>";
								ret += "</Shape>\n</Transform>\n";
							}
						}
					}
				}
			}
		}
	}
	ret += 	"\n</Scene>\n</X3D>";
	ofstream file;
	file.open("clumped.x3d");
	file << ret;
	file.close();
	return 0;

	for (float i = 0; i < 1000; i++) {
		int x = (rand() % 1000) - 500;
		int y = (rand() % 1000) - 500;
		int z = (rand() % 1000) - 500;
		int size = max(1.0, (rand() % 15) + 1 - (z / 250.0));
		colour = boost::lexical_cast<string>(fabs((float)  (x % 100) / 100.0)) +
				" " + boost::lexical_cast<string>(fabs((float) (y % 100) / 100.0)) +
				" " + boost::lexical_cast<string>(fabs((float) (z % 100) / 100.0));
		ret += "<Transform translation='" +
			boost::lexical_cast<string>(x) + " " +
			boost::lexical_cast<string>(y) + " " +
			boost::lexical_cast<string>(z) + "'>\n<Shape>\n";
		if (false) {
			ret += "<Sphere radius='" + boost::lexical_cast<string>(size) + "'/>\n<Appearance>\n<Material diffuseColor='" + colour + "'/>\n</Appearance>\n";
		} else {
			ret += "<IndexedTriangleSet index='0 1 4 2 4 1 3 1 0 1 3 2 0 5 4 2 4 5 3 5 0 5 3 2' solid='true' creaseAngle='0'>\n<Coordinate point='"+  boost::lexical_cast<string>(size)+ " 0 0 0 0 "+  boost::lexical_cast<string>(size) + " "+  boost::lexical_cast<string>(-size) + " 0 0 0 " + boost::lexical_cast<string>(size) + " 0 0 "+  boost::lexical_cast<string>(-size) + " 0 0 0 " + boost::lexical_cast<string>(-size) +"'/>\n<Normal vector='1 0 0 0 0 1 -1 0 0 0 1 0 0 -1 0 0 0 -1'/>\n<Appearance>\n<Material diffuseColor='" + colour + "'/>\n</Appearance>\n</IndexedTriangleSet>";
		}
		ret += "</Shape>\n</Transform>\n";
	}


}