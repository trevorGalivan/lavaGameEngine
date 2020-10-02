# lavaGameEngine
A simple "2.5D" game engine with a small fighting game built on top.

**Requries** the external libraries:
- GLFW https://www.glfw.org/
- Asimp https://www.assimp.org/
- SoLoud https://sol.gfxile.net/soloud/
- GLM https://glm.g-truc.net/0.9.9/index.html

This project contains the following libraries and/or files:
- GLAD https://glad.dav1d.de/
- STB_IMAGE https://github.com/nothings/stb
- SMAA http://www.iryoku.com/smaa/

Contains Assets from CC0Textures.com, licensed under the Creative Commons CC0 License.

## Controls:
###### Player 1 (keyboard):
Move: WASD
Jump: Spacebar
Shoot: F
Throw: Q

###### Player 2 (Gamepad):
Move: DPad
Jump: A
Shoot: X
Throw: B

## Graphics:
The gameplay and physics are 2D, but the graphics remain fully 3D, witha  custom deffered rendering pipeline.
All objects in the game have three textures: a base color texture, an "MREA" map (metalness, roughness, emmisiveness, ambient occlusion), and a normal map.
The game supports full transparency for any part of the object, but at this point does not support partial transparency.

There are two post-processing effects: Bloom, and SMAA. The bloom is fairly conservative, and only effects values that would otherwise be clipped by the conversion to a low dynamic range.

## Screenshots:

![Desert](https://github.com/trevorGalivan/lavaGameEngine/blob/master/screenshots/desert.PNG)
![Space](https://github.com/trevorGalivan/lavaGameEngine/blob/master/screenshots/space.PNG)
![Volcano](https://github.com/trevorGalivan/lavaGameEngine/blob/master/screenshots/volcano.PNG)
