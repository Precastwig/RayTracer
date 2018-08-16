# Ray tracer
Was built to test my custom ray tracing optimisation used in my masters thesis. Now being updated to actually be functional for other things! The masters thesis pdf can be viewed in this repository titled "mastersthesis".

## Building the project
The project builds with the external library flags -lboost_system, -lboost_chrono, -lnlopt and -lm, from the libraries NLopt and Boost. These libraries must be installed before the project will compile.

These libraries dowload and install instructions can be found at:
NLopt: https://nlopt.readthedocs.io/en/latest/#download-and-installation
Boost: https://www.boost.org/doc/libs/1_66_0/more/getting_started/unix-variants.html

Additionally the extra boost library boost_chrono must be built, the instructions to do so are also included in the above link. Finally call ```make``` or ```make fast``` to compile the project.

## Running the project

The project is a command line based application, with several parameters you can alter (if not filled they will revert to working defaults). For a list of available parameters the application can be run with the flag ```-h``` or can be seen below:

```--file=path/to/file``` : sets the .x3d file to be read
```--rep=15 : sets the``` number of repetitions to 15
```--res=1920,1080``` : sets the resolution of the output image
```--type=uniform``` : sets the octree type to be used to uniform
```--type=nonuniform``` : sets the octree type to be used to non-uniform
```-showoctree``` : before running the tests displays the generated octree as a text output (extremely verbose)
```-compare``` : runs both types of octree on the given file and compares the two output arrays of colours to confirm they generate the same image
```-h``` : displays this command list

The program will the generate the .bmp output image in the same directory as the source image.
