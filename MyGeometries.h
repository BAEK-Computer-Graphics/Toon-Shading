#pragma once

// 
// MyGeometries.h   ---  Header file for MyGeometries.cpp.
// 
//   Sets up and renders 
//     - the ground plane, and
//     - the surface of rotation
//   for the Math 155A project #4.
//
//

//
// Function Prototypes
//

#include <vector>

using namespace std;


void MySetupSurfaces();                // Called once, before rendering begins.
void SetupForTextures();               // Loads textures, sets Phong material

void MyRenderGeometries();            // Called to render the two surfaces
void MyRenderPass2();            // Called to render the two surfaces
bool loadOBJ(const char* path, vector<vector<float>> & out_vertices, vector<unsigned int> & out_elements);



