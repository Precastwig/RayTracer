#include <fstream>
#include <iostream>
#include <math.h>
#include <boost/lexical_cast.hpp>

using namespace std;

#define PI 3.14159265

int main(int argc, char *argv[]){
	std::string ret = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE X3D PUBLIC \"ISO//Web3D//DTD X3D 3.2//EN\" \"http://www.web3d.org/specifications/x3d-3.2.dtd\"><X3D profile='Immersive' version='3.2' xmlns:xsd='http://www.w3.org/2001/XMLSchema-instance' xsd:noNamespaceSchemaLocation='http://www.web3d.org/specifications/x3d-3.2.xsd'>	<head> </head>\n<Scene>\n<Viewpoint position='0 0 200'/>\n<Background groundColor='0.43 0.54 0.72' skyColor='0.65 0.74 0.88'/>\n<PointLight location='0 0 0' radius='100'/>\n<DirectionalLight direction='-0.3 -0.3 -1'/>";

	std::string colour = "1 0.5 0";
	for (int i = -80; i <= 80; i += 20) {
		for (int j = -80; j <= 80; j += 20) {
			for (int k = -80; k <= 80; k += 20) {
				if (!(i == 0 && j == 0 && k == 0)) {

		// int size = max(1.0, (rand() % 10) + 1 - (z / 25.0));
		colour = boost::lexical_cast<string>(fabs((float)(i + 80)/160.0)) +
				" " + boost::lexical_cast<string>(fabs((float)(j + 80)/160.0)) +
				" " + boost::lexical_cast<string>(fabs((float)(k + 80)/160.0));
		ret += "<Transform translation='" +
			boost::lexical_cast<string>(i) + " " +
			boost::lexical_cast<string>(j) + " " +
			boost::lexical_cast<string>(k) + "'>\n" +
			"<Shape>\n<IndexedTriangleSet index='0 1 4 2 4 1 3 1 0 1 3 2 0 5 4 2 4 5 3 5 0 5 3 2' solid='true' creaseAngle='0'>\n<Coordinate point='2 0 0 0 0 2 -2 0 0 0 2 0 0 -2 0 0 0 -2'/>\n<Normal vector='1 0 0 0 0 1 -1 0 0 0 1 0 0 -1 0 0 0 -1'/>\n<Appearance>\n<Material diffuseColor='" + colour + "'/>\n</Appearance>\n</IndexedTriangleSet>\n</Shape>\n</Transform>\n";
				}
			}	
		}
	}

	ret += 	"\n</Scene>\n</X3D>";

	ofstream file;
	file.open("trianglemesh.x3d");
	file << ret;
	file.close();
	return 0;
}