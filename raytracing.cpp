// You can use this example as a starting point for Task 2.

#include "minirt/minirt.h"

#include <thread>
#include <vector>
#include <cmath>
#include <iostream>
#include <queue>
#include <chrono>

using namespace minirt;
using namespace std;
using namespace std::chrono;
using hrc = high_resolution_clock;

void initScene(Scene &scene) {
    Color red {1, 0.2, 0.2};
    Color blue {0.2, 0.2, 1};
    Color green {0.2, 1, 0.2};
    Color white {0.8, 0.8, 0.8};
    Color yellow {1, 1, 0.2};

    Material metallicRed {red, white, 50};
    Material mirrorBlack {Color {0.0}, Color {0.9}, 1000};
    Material matteWhite {Color {0.7}, Color {0.3}, 1};
    Material metallicYellow {yellow, white, 250};
    Material greenishGreen {green, 0.5, 0.5};

    Material transparentGreen {green, 0.8, 0.2};
    transparentGreen.makeTransparent(1.0, 1.03);
    Material transparentBlue {blue, 0.4, 0.6};
    transparentBlue.makeTransparent(0.9, 0.7);

    scene.addSphere(Sphere {{0, -2, 7}, 1, transparentBlue});
    scene.addSphere(Sphere {{-3, 2, 11}, 2, metallicRed});
    scene.addSphere(Sphere {{0, 2, 8}, 1, mirrorBlack});
    scene.addSphere(Sphere {{1.5, -0.5, 7}, 1, transparentGreen});
    scene.addSphere(Sphere {{-2, -1, 6}, 0.7, metallicYellow});
    scene.addSphere(Sphere {{2.2, 0.5, 9}, 1.2, matteWhite});
    scene.addSphere(Sphere {{4, -1, 10}, 0.7, metallicRed});

    scene.addLight(PointLight {{-15, 0, -15}, white});
    scene.addLight(PointLight {{1, 1, 0}, blue});
    scene.addLight(PointLight {{0, -10, 6}, red});

    scene.setBackground({0.05, 0.05, 0.08});
    scene.setAmbient({0.1, 0.1, 0.1});
    scene.setRecursionLimit(20);

    scene.setCamera(Camera {{0, 0, -20}, {0, 0, 0}});
}

// This is a thread function for C++ threads.
// TODO: modify this function for a thread to be able to compute some specified part of the image.
// For example, now ranges of pixels by X and Y always start from 0.
// Think what additional arguments may be required to compute a range not starting from 0.
void threadFunc(Scene &scene, ViewPlane &viewPlane, Image &image, int start, int end, int sizeY, int numOfSamples) {
    for(int x = start ; x < end; x++)
    for(int y = 0; y < sizeY; y++) {
        const auto color = viewPlane.computePixel(scene, x, y, numOfSamples);
        image.set(x, y, color);
    }     
}
// as an additional arguments we have added 'start' and 'end' parameters

int main(int argc, char **argv) {
    // Number of threads to use is the first parameter now.
    // The other parameters are the same as in the sequential app.
    int numberOfThreads = (argc > 1 ? std::stoi(argv[1]) : 1);
    int viewPlaneResolutionX = (argc > 2 ? std::stoi(argv[2]) : 600);
    int viewPlaneResolutionY = (argc > 3 ? std::stoi(argv[3]) : 600);
    int numOfSamples = (argc > 4 ? std::stoi(argv[4]) : 1);    
    std::string sceneFile = (argc > 5 ? argv[5] : "");


    Scene scene;
    if (sceneFile.empty()) {
        initScene(scene);
    } else {
        scene.loadFromFile(sceneFile);
    }

    const double backgroundSizeX = 4;
    const double backgroundSizeY = 4;
    const double backgroundDistance = 15;

    const double viewPlaneDistance = 5;
    const double viewPlaneSizeX = backgroundSizeX * viewPlaneDistance / backgroundDistance;
    const double viewPlaneSizeY = backgroundSizeY * viewPlaneDistance / backgroundDistance;

    ViewPlane viewPlane {viewPlaneResolutionX, viewPlaneResolutionY,
                         viewPlaneSizeX, viewPlaneSizeY, viewPlaneDistance};

    Image image(viewPlaneResolutionX, viewPlaneResolutionY); // computed image
    
    vector<thread> threads;
    
    auto ts = hrc::now();

    // define block as division of viewPlaneResolutionX to numberOfThreads
    const float block = viewPlaneResolutionX / numberOfThreads;

    // TODO: make each thread to compute different part of the image.
    // To do this first decide how the image should be partitioned,
    // then compute a parition and pass this information to each thread.
    for (int i = 0; i < numberOfThreads; i++) {
        thread thr(threadFunc, ref(scene), ref(viewPlane), ref(image), 
            block * i,  block * (i + 1), viewPlaneResolutionY, numOfSamples);
        threads.push_back(move(thr));           
    }

    // instead of 'viewPlaneResolution' adding block size multiple by i
    // and as the nest parameter taking the number after i multiplied by it
    // that is how we pass data into all threads
    
    for (auto &thread: threads) {
        thread.join();
    }
        
    auto te = hrc::now();
    
    double time = duration<double>(te - ts).count();
    
    cout << "Time = " << time << endl;

    image.saveJPEG("raytracing_" + to_string(numberOfThreads) + ".jpg");

    return 0;
}