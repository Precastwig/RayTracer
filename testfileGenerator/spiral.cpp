#include <fstream>
#include <iostream>
#include <math.h>
#include <boost/lexical_cast.hpp>
// #include <structs.h>

using namespace std;

#define PI 3.14159265

// using namespace help;

int main(int argc, char *argv[]){
	std::string ret = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE X3D PUBLIC \"ISO//Web3D//DTD X3D 3.2//EN\" \"http://www.web3d.org/specifications/x3d-3.2.dtd\"><X3D profile='Immersive' version='3.2' xmlns:xsd='http://www.w3.org/2001/XMLSchema-instance' xsd:noNamespaceSchemaLocation='http://www.web3d.org/specifications/x3d-3.2.xsd'>	<head> </head>\n<Scene>\n<Viewpoint position='0 0 2500' fieldOfView='1.3'/>\n<Background groundColor='0.43 0.54 0.72' skyColor='0.65 0.74 0.88'/>\n<DirectionalLight direction='1 0 -1' intensity='0.3'/>\n<DirectionalLight direction='0 1 -1' intensity='0.3'/>\n";

	std::string colour = "1 0.5 0";
	int size = 10;
	int dist = 1000;

	float r[5] = {0.66,0.13,0.66, 0.17,0.42};
	float g[5] = {0.22,0.40,0.42, 0.53,0.25};
	float b[5] = {0.22,0.40,0.22, 0.17,0.52};
	int k = 0;
	// std::vector<Vec3<float>> made = std::vector<Vec3<float>>();
	for (dist = 1500; dist >= 500; dist -= 250) {

	for (float i = 0; i <= 2 * PI; i = i + 0.1) {
		for (float j = 1; j < 20; j++ ) {
			float x = dist * cos(i) * sin(i * j);
			float y = dist * sin(i) * sin(i * j);
			float z = dist * cos(i * j);
				// made.push_back(Vec3<float>(x,y,z));
				colour = boost::lexical_cast<string>(r[k]) +
						" " + boost::lexical_cast<string>(g[k]) +
						" " + boost::lexical_cast<string>(b[k]);
				ret += "<Transform translation='" +
					boost::lexical_cast<string>(x) + " " +
					boost::lexical_cast<string>(y) + " " +
					boost::lexical_cast<string>(z) + "'>\n" +
					"<Shape>\n<Sphere radius='" + boost::lexical_cast<string>(size) + "'/>\n<Appearance>\n<Material diffuseColor='" +
					colour + "'/>\n</Appearance>\n</Shape>\n</Transform>\n";
		}
	}
	k++;
	}


	ret += 	"\n</Scene>\n</X3D>";

	ofstream file;
	file.open("testing2.x3d");
	file << ret;
	file.close();
	return 0;
}