This demo is the current (June 28, 2014) build of ObjLoaderDemo
As this demo is still in progress, if you want to most up-to-date version of the demo, download the full source and compile.


Overview:
After finishing the deferred shading demo, I wanted a more complex scene to test deferred against forward. OBJ model files naturally were my first thought.
So this demo is my current progress on loading an entire scene represented in an OBJ file and rendering it.

The GBuffers are laid out as follows:
Diffuse Albedo     DXGI_FORMAT_R11G11B10_FLOAT
Specular Albedo    DXGI_FORMAT_R8G8B8A8_UNORM
Normal             DXGI_FORMAT_R16G16_FLOAT
Depth              DXGI_FORMAT_D32_FLOAT

Diffuse Albedo     Stores the RGB diffuse color
Specular Albedo    Stores the specular color in the RBG components and the specular power in the alpha component
Normal             The fragment surface unit normal stored in spherical coordinates. (We don't store r since we know it's 1 for a unit normal)
Depth              The hardware depth buffer. It stores (1 - z/w). By swapping the depth planes, we spread the depth precision out more evenly. http://mynameismjp.wordpress.com/2010/03/22/attack-of-the-depth-buffer/)


Controls:
Alt + left mouse and drag to rotate
Alt + middle mouse and drag to pan
Scroll middle mouse to zoom


Click the little arrow in the bottom left-corner to change settings





Changelog:

June 28, 2014 - SHA da36d4a101e68ba93773830a6083e901e2c31821
- Implemented Tiled Culled Deferred Shading
- In all shading methods (Forward, No-cull deferred, tiled cull deferred), we now render to an HDR render target, not directly to the backbuffer
    - DXGI_FORMAT_R16G16B16A16_FLOAT
	- We then do a simple full screen triangle post-process pixel shader to map the HDR to the backbuffer
	    - In its current state, it's just extra overhead to have this post process step
		- However, Tiled Culled requires a post-process step since a compute shader can't write directly to the backbuffer
		- This sets up the necessary groundwork for future post processing such as tone-mapping and bloom
- General clean-up of the code base
    - Moved from #include guards to #pragma once
	- Lots of variable name style fixes
	- etc

June 21, 2014 - SHA 0dcabe7245637c2533098ee02d701d34398ac7e8
- Re-add loading screen
- Added a scene.json
    - Defines what is rendered
        - Models
		    - Can be instanced
        - Lights
- Added a cube model to demonstrate instancing
- Deferred shading no longer stores an index to a material
    - Instead it just bakes all the material data into the G-Buffers
	- Added a specular G-Buffer to handle this
- Modify the sponza materials to allow specular
- Moved all demo files to a sub-folder and put a shortcut in the main folder
    - Makes it a a lot easier for a user to find the exe file

April 28, 2014 - SHA bb42984aba144c2ce393ea3d92ad1ef11c483530
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
