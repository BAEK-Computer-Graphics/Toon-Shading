Real-time cel and contour shading using GLSL vertex and fragment shaders
## How to run
Run ToonShading.exe in the base directory.
## Controls
* Arrow keys/Home/End: Adjust camera
* C: Toggle backface culling
* Number keys 1,2,3: Toggle lights 1,2,3
* D: Toggle diffuse light
* S: Toggle specular light
* A: Toggle ambient light
## Screenshots
<p align="center">
  <img width="802" height="646" src="https://github.com/a-alten/Toon-Shading/blob/master/screenshot/2.png">
  <img width="802" height="646" src="https://github.com/a-alten/Toon-Shading/blob/master/screenshot/1.png">
</p>
## Notes
A character is loaded from an OBJ file and rendered using a toon shader (cel shading + contour shader). Cel shader is a modified version of the Phong fragment shader. Contours use multi pass rendering. Normal and depth textures are rendered to frame buffers and then an edge detection algorithm draws the contours. OBJ file loader reads in vertex coords, normal coords, texture coords, and faces from OBJ file and draws every triangle with unique vertices to deal with vertices with multiple normal/texture coordinates. Probably not optimal.