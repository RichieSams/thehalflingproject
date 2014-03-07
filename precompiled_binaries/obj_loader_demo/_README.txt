This demo is the current (March 06, 2014) build of ObjLoaderDemo
As this project is still in progress, if you want to most up-to-date version of the demo, download the full source and compile.


Overview:
After finishing the deferred shading demo, I wanted a more complex scene to test deferred against forward. OBJ model files naturally were my first thought.
So this demo is my current progress on loading an entire scene represented in an OBJ file and rendering it.

As of now, it is hardcoded to only load the supplied Sponza Atrium Scene. The code is capable of loading other scenes, but I haven't implemented the interface to allow that yet.

The actual rendering is very similar to the DeferredShadingDemo. Most of the modifications were actually bug fixes.

The GBuffers are laid out as follows:
Albedo-MaterialIndex    DXGI_FORMAT_R10G10B11_UNORM
Normal                  DXGI_FORMAT_R16G16_FLOAT
MaterialIndex           DXGI_FORMAT_R16_FLOAT
Depth                   DXGI_FORMAT_R32_FLOAT

Albedo           Stores the RGB diffuse color read from texture mapping
Normal           The fragment surface unit normal stored in spherical coordinates. (We don't store r since we know it's 1 for a unit normal)
MaterialIndex    An offset index to a global material array in the shader
Depth            The hardware depth buffer. It stores (1 - z/w). By swapping the depth planes, we spread the depth precision out more evenly. http://mynameismjp.wordpress.com/2010/03/22/attack-of-the-depth-buffer/)


Controls:
Hold left mouse and drag to rotate
Scroll middle mouse to zoom

Click the little arrow in the bottom left-corner to change settings


Things I still need to add/fix:
- A 'fly-through' camera
- Forward and Deferred are creating slightly different colors on certain faces
    - Perhaps the normal buffer?
- Allow users to specify their own OBJ file
