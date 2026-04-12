# Eclipse
A 2D platforming game made in C99 for the FIICODE 2026 GameDev Competition

> **This project is unfortunately unfinished. The last commit from the competition being on 12 April 2026**

# Install 
The only supported platform is currently Windows 64bit. 

The [Releases](https://github.com/EnnBruh/Eclipse/releases) Tab should hold the latest compiled binary of the project. Unzip it and open `main.exe` (creative name).

# Building from source
This project was made in C using Makefile. We recommend installing the [Git](https://git-scm.com/install/windows) environment for Windows along with [MingW64](https://www.mingw-w64.org/) and placing them in your PATH.

After installing you should be able to run
```
git clone https://github.com/EnnBruh/Eclipse.git
cd Eclipse
cd Dev
make
```
This will compile to the `Debug` version of the project
```
make build_release
```
Will compile in Release mode.

# Dependencies
The dependencies are
1. GLFW 3 (https://www.glfw.org/)
2. OpenGL 3.3 loaded using GLAD (https://glad.dav1d.de/)
3. STB_image (https://github.com/nothings/stb)
   
These are present in the `Dev/Dep` directory and are statically linked with the executable so there should be no reason to install them separately.

# Assets and resources
This project couldn't have been made without the work of
1. CraftPix - Background Modern City Assets - https://free-game-assets.itch.io/free-city-backgrounds-pixel-art
2. Josee - Character Template - https://n3cloud.itch.io/2d-pixel-art-character-template-platformer-metroidvania