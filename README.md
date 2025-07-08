# Nova Engine

Game engine using DIRECTX11 & C++
![NovaEngine_Showcase](data/assets/nova.gif)

## ⚙️ Setup

git submodule update --init --recursive

```
cd deps/external/assimp
cmake .
```
Open vs solution and compile for debug or release


```
cd deps/external/DirectXTK
cmake .
```
Open vs solution and compile for debug or release

## 🚀 Starting

Simply open Nova Engine VS solution and compile



# Features

📊 Graphics

- Forward rendering
- Deferred rendering
- Physically Based Rendering
- Emissive Materials
- Cubemap reflections
- Render Solid & Wireframe
- Depth Prepass
- Directional light
- Point light
- Spot Light

⚙️ Engine

- JobSystem templated with variadic arguments
- Asyncronous loading (Meshes and Textures)
- Entity Component System
- Data driven
- Time Measurement on Imgui
- GUI for entities
- Input System
- Material creation
- Parenting