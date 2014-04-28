This demo is the current (April 28, 2014) build of ObjLoaderDemo
As this demo is still in progress, if you want to most up-to-date version of the demo, download the full source and compile.


Overview:
After finishing the deferred shading demo, I wanted a more complex scene to test deferred against forward. OBJ model files naturally were my first thought.
So this demo is my current progress on loading an entire scene represented in an OBJ file and rendering it.

As of now, it is hardcoded to only load the supplied Sponza Atrium Scene. The code is capable of loading other scenes, but I haven't implemented the interface to allow that yet.

The actual rendering is very similar to the DeferredShadingDemo. Most of the modifications were actually bug fixes.

The GBuffers are laid out as follows:
Albedo           DXGI_FORMAT_R10G10B11_UNORM
Normal           DXGI_FORMAT_R16G16_FLOAT
MaterialIndex    DXGI_FORMAT_R16_FLOAT
Depth            DXGI_FORMAT_R32_FLOAT

Albedo           Stores the RGB diffuse color read from texture mapping
Normal           The fragment surface unit normal stored in spherical coordinates. (We don't store r since we know it's 1 for a unit normal)
MaterialIndex    An offset index to a global material array in the shader
Depth            The hardware depth buffer. It stores (1 - z/w). By swapping the depth planes, we spread the depth precision out more evenly. http://mynameismjp.wordpress.com/2010/03/22/attack-of-the-depth-buffer/)


Controls:
Alt + left mouse and drag to rotate
Alt + middle mouse and drag to pan
Scroll middle mouse to zoom


Click the little arrow in the bottom left-corner to change settings


Things I still need to add/fix:
- Forward and Deferred are creating slightly different colors on certain faces
    - Perhaps the normal buffer?
- Allow users to specify their own OBJ file


Changelog:

April 28, 2014 SHA bb42984aba144c2ce393ea3d92ad1ef11c483530
- All textures are now required to be DDS files (but added a tool to auto-convert them during HMF creation).
  WIC was allowing us to load some other image formats, but the library required an ImmediateContext in order to generate MipMaps. 
    - As of right now, this tool isn't included because sponza.hmf is provided and is hardcoded to be the only scene to run
- Fixed the bug causing lighting to be incorrectly calculated on surfaces with normal (0, 0, 1)
    - It had to do with the spherical encoding. See SHA bb5bb68c5558e7dfaf292a91c16ae17a22b37edb
- Added normal mapping to forward and deferred shading
- Added an option in the Settings bar to show a GBuffer full screen

April 3, 2014 - SHA 181b7e0387c0da2beb3002ce77d9840ec72d48c5
- Increased the resolution to 1280 x 720
- Changed the camera controls to "Maya-like" controls
    - Tumble
    - Pan
    - Scroll
- Created a HalflingModelFile file format
    - All model data is now stored in this instead of in obj
    - The format is binary, which makes reading extremely fast
- Removed loading screen
    - Since scene load is so fast
    - I may add this back for people with slower systems so the program is responsive
