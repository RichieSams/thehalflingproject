This demo is the current (January 28, 2014) build of DeferredShadingDemo
As this project is still in progress, if you want to most up-to-date version of the demo, download the full source and compile.


Overview:
This demo is my first attempt at deferred shading. Initially, I was going to go straight to Tiled-Light Culling Deferred, but after a while
I chose to first cut my teeth on naive, no light culling, deferred shading.


The GBuffers are laid out as follows:
Albedo-MaterialIndex    DXGI_FORMAT_R8G8B8A8_UNORM
Normal                  DXGI_FORMAT_R16G16_FLOAT
Depth                   DXGI_FORMAT_R32_FLOAT

Albedo           Stores the RGB diffuse color read from texture mapping
MaterialIndex    An offset index to a global material array in the shader
Normal           The fragment surface unit normal stored in spherical coordinates. (We don't store r since we know it's 1 for a unit normal)
Depth            The hardware depth buffer. It stores (1 - z/w). By swapping the depth planes, we spread the depth precision out more evenly. http://mynameismjp.wordpress.com/2010/03/22/attack-of-the-depth-buffer/)


Pass Descriptions:

GBuffer pass
- Render all our geometry to the GBuffers, doing a texture lookup for the albedo term

NoCullFinalGather pass:
- Render a fullscreen triangle, guaranteeing a pixel shader thread for each pixel on-screen
- Sample the GBuffers and use some math to back-calculate the sample normal and position
- Lookup the material using the MaterialIndex
- Cycle through each light and apply a lighting equation, accumulating a final color for the pixel


Controls:
Hold left mouse and drag to rotate
Scroll middle mouse to zoom

Click the little arrow in the bottom left-corner to change settings