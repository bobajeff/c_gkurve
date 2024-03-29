# c_gkurve
This is a c port of the [gkurve example](https://github.com/hexops/mach-examples/tree/main/examples/gkurve) in mach-examples. 
## What is gkurve
A simpler way to do 2D/Vector graphics on the GPU. Right now it uses a combination different types of triangle primatives. Where some triangles can be rendered as filled in curves via the fragment shader.

Here's a [video presentation about it](https://www.youtube.com/watch?v=QTybQ-5MlrE) and the [slides of that presentation](https://docs.google.com/presentation/d/1IY-05VHPuUfS3e22D-BorDh8HMz14i6cLcddAGAhUZo) and also the [discord where it's discussed](https://discord.com/channels/996677443681267802/1150498877338828832/1153338351672373359).
## Building
### Install dependencies
* freetype2 library (required)
    * for ubuntu based systems the command is `sudo apt-get install libfreetype-dev`
* wgpu-native v0.17.0.2 (required) -
    * Download the [wgpu-native v0.17.0.2](https://github.com/gfx-rs/wgpu-native/releases/tag/v0.17.0.2) build
    * extract in source directory 
    * rename folder to `wgpu`

### build the project
In the source directory run:
```sh
cmake -B build
cmake --build build
```
## Running

### Get the assets first
* Download the [FiraSans-Regular.ttf and gotta-go-fast.png files](https://github.com/bobajeff/c_gkurve_assets)
* Put them in the source directory.
### Run gkurve
From the source directory run:
```sh
./build/gkurve
```
