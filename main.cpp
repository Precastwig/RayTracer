#include <octree.h>
#include <scene.h>
#include <bitmapsave.h>
#include <structs.h>
#include <chrono>
#include <boost/chrono.hpp>
#include <algorithm>

bool SHOWOCTREES = false;
bool COMPARE = false;
bool FIN = false;
int resolutionx = 1920;
int resolutiony = 1080;
int numRepeats = 5;
octreeType type = REGULAR;
std::string filename = "testfiles/trianglemesh";

void parseFlags(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        std::vector<std::string> parts;
        std::vector<std::string> tempp;
        std::string str(argv[i]);
        std::transform(str.begin(),str.end(),str.begin(), ::tolower);        
        boost::algorithm::split(parts, str, boost::algorithm::is_any_of("="));
        if (parts.size() <= 2) {
            if (parts[0] == "--res") {
                boost::algorithm::split(tempp, parts[1], boost::algorithm::is_any_of(","));
                if (tempp.size() == 2) {
                    resolutionx = boost::lexical_cast<int>(tempp[0]);
                } else {
                    std::cout << "Malformed flag " << str << " usage is -res=<resolutionx>,<resolutiony>" << std::endl;
                }
            } else if (parts[0] == "--rep") {
                numRepeats = boost::lexical_cast<int>(parts[1]);
            } else if (parts[0] == "--file") {
                filename = parts[1];
            } else if (parts[0] == "-showoctree") {
                SHOWOCTREES = true;
            } else if (parts[0] == "--type") {
                if (parts[1] == "nonuniform")
                    type = NONUNIFORM;
                else
                    type = REGULAR;
            } else if (parts[0] == "-compare") {
                COMPARE = true;
            } else if (parts[0] == "-h") {
                std::cout << "Avalaible commands are:" << std::endl;
                std::cout << "  --res=<x>,<y> ; sets the resolution of the output image. Where x and y are integer values" << std::endl;
                std::cout << "  --rep=<t> ; sets the number of repititions it will time. Where t is an integer" << std::endl;
                std::cout << "  --file=path/to/file ; sets the .x3d file to run on. Omitting the final .x3d of the file name" << std::endl;
                std::cout << "  -showoctree ; prints the octree before timing" << std::endl;
                std::cout << "  --type=NONUNIFORM ; sets the type of octree to use as a nonuniform octree, default is uniform" << std::endl;
                std::cout << "  -compare ; will perform no tests, but generate the output images for each octree type and compare them pixel by pixel" << std::endl;
                FIN = true;
            } else {
                std::cout << "Malformed flag " << str << " " << parts[0] << std::endl;
            }
        } else {
            std::cout << "Malformed flag " << str << std::endl;
        }
    }
}

int main(int argc, char *argv[]){
    // // //@todo COMMENT EVERYTHING
    // Set default filename and resolution
    //int NUMTHREADS = 16;
    // Get argument from command line
    parseFlags(argc,argv);
    if (FIN)
        return 0;
    // if (argv[1] == "0") {
    //     type = REGULAR;
    // } else {
    //     type = NONUNIFORM;
    // }
    // int tt = boost::lexical_cast<int>(argv[2]);
    // int tc = boost::lexical_cast<int>(argv[3]);
    // int sc = 500;//boost::lexical_cast<int>(argv[3]);

/*
    Shape testTri = Shape(Shapes::TRIANGLEMESH);
    std::shared_ptr<long double> t = std::make_shared<long double>(0);
    long double u,v;
    testTri.intersectTriangle(RAY_STRUCT<long double>(Vec3<long double>(0,0,20),Vec3<long double>(0,0,-1)),
                            Vec3<long double>(0,2,2),
                            Vec3<long double>(2,-2,2),
                            Vec3<long double>(-2,-2,2),
                            t,u,v);
    std::cout << *t << std::endl;
    return 0;*/

    // Create the saver object that generates the final image from an array of colours
    BitmapSave *saver = new BitmapSave(resolutionx, resolutiony);
    Scene *scene;
    if (!COMPARE) {
        std::string output = "";
        if (type == REGULAR)
            output = "UNIFORM";
        else
            output = "NONUNIFORM";
        std::cout << "Running " << numRepeats << " " << output << " tests on " << filename << std::endl;
        // tuple<chrono::process_real_cpu_clock, chrono::process_user_cpu_clock, chrono::process_system_cpu_clock>
        using namespace boost::chrono; 
        milliseconds total{0};

        scene = new Scene(filename+".x3d", type);
        auto s = process_user_cpu_clock::now();
        scene->parseX3D(type);
        auto e = process_user_cpu_clock::now();
        if (SHOWOCTREES) {
            scene->printOctree();
            std::cout << "Press ENTER to start.." << std::endl;
            std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cout << "Generation time: " << duration_cast<milliseconds>(e - s) << std::endl;
        std::cout << "Octree depth: " << scene->tree->depth << std::endl;
        std::cout << "Voxels created: " << scene->tree->countVoxels() << std::endl;

        auto start = process_user_cpu_clock::now();
        for (int k = 0; k < numRepeats; k++) {
            // auto start = std::chrono::steady_clock::now();
            scene->parseX3D(type);
            for (int i = 0; i < resolutionx; i++) {
                for (int j = 0; j < resolutiony; j++) {
                    // std::cout << i << j << std::endl;
                    saver->setVal(i,j,scene->computeRay(i,j));
                }
            }
        }
        auto end = process_user_cpu_clock::now();


        total = duration_cast<milliseconds>(end - start) / numRepeats;
        std::cout << "Total time:\n\033[38;5;46m"<<   total << "\033[0m" << std::endl;

        saver->convertRGBtoBMPBuffer();
        if (type == REGULAR)
            output = "_regular";
        else 
            output = "_nonuniform";
        saver->saveBMP(filename + output);
    } else {
        std::cout << "Comparing on file " << filename << std::endl;
        BitmapSave *saver2 = new BitmapSave(resolutionx, resolutiony);
        scene = new Scene(filename+".x3d", REGULAR);
        scene->parseX3D(REGULAR);
        for (int i = 0; i < resolutionx; i++) {
            for (int j = 0; j < resolutiony; j++) {
                // std::cout << i << j << std::endl;
                saver->setVal(i,j,scene->computeRay(i,j));
            }
        }
        delete(scene);
        scene = new Scene(filename+".x3d", NONUNIFORM);
        scene->parseX3D(NONUNIFORM);        
        for (int i = 0; i < resolutionx; i++) {
            for (int j = 0; j < resolutiony; j++) {
                // std::cout << i << j << std::endl;
                saver2->setVal(i,j,scene->computeRay(i,j));
            }
        }

        if (*saver == *saver2) {
            std::cout << "They produce identical images" << std::endl;
        } else {
            std::cout << "They are NOT the same images" << std::endl;
        }
        saver->convertRGBtoBMPBuffer();
        saver->saveBMP(filename+"_regular");
        saver2->convertRGBtoBMPBuffer();
        saver2->saveBMP(filename+"_nonuniform");
        delete(saver2);
    }
    
    delete(scene);

    delete(saver);
    return 0;
/*
    if (argc >= 4) {
        resolutiony = argv[3];
    }
    if (argc >= 5) {
        NUMTHREADS = argv
    }*/

    /*//Get the current platform
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if(all_platforms.size()==0) {
        std::cout << "No platform found\n";
        exit(1);
    }
    cl::Platform default_platform = all_platforms[0];
    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << std::endl;

    //Get the default device of the current platform
    std::vector<cl::Device> devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    if(devices.size()==0) {
        std::cout << "No devices found\n";
        exit(1);
    }
    cl::Device default_device = devices[0];
    std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << std::endl;
    
    //Create a context to link the device and platform
    cl::Context context({default_device});
*/

    /*
    //Create the program
    cl::Program::Sources sources;

    //Kernel code
	std::string kernel_code =
    "	void kernel simple_add(global const int* A, global const int* B, global int* C){"
    "C[get_global_id(0)]=A[get_global_id(0)]+B[get_global_id(0)];"
    "}";

	sources.push_back({kernel_code.c_str(),kernel_code.length()});

	cl::Program program(context, sources);
        if(program.build({default_device}, nullptr, 0,0) != CL_SUCCESS) {
        std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
		exit(1);
	}

	//Create buffers on device
    cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, sizeof(int)*10);
	cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, sizeof(int)*10);
	cl::Buffer buffer_C(context, CL_MEM_READ_WRITE, sizeof(int)*10);
	//Initialise arrays on host
	int A[] = {5,5,5,5,5,5,5,5,5,5};
	int B[] = {1,2,3,4,5,6,7,8,9,10};

	//Move host into GPU buffers
	cl::CommandQueue que(context,default_device);
	que.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*10,A);
	que.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*10,B);

	//Call function
	cl::Kernel simple_add(cl::Kernel(program,"simple_add"));
    simple_add.setArg(0, buffer_A);
    simple_add.setArg(1, buffer_B);
    simple_add.setArg(2, buffer_C);
    que.enqueueNDRangeKernel(simple_add,cl::NullRange,cl::NDRange(10),cl::NullRange);

	//Now we return the C GPU buffer to the host
    int C[10];
    que.enqueueReadBuffer(buffer_C,CL_TRUE,0,sizeof(int)*10,C);

    std::cout << "result: ";
    for(int i = 0; i < 10; i++) {
		std::cout << C[i] << " ";
    }
    std::cout << "\n";
*/
}
