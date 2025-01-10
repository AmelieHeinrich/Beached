# Beached : a DirectX 12 renderer

Work in progress

## Current features

### Renderer
- Cascaded Shadow Maps
- Point Shadows
- Spot Shadows
- Z-Prepass

### Engine
- CPU side frustum culling
- Asset caching for shaders and textures

## WIP

- Bokeh DOF

## Roadmap

### Planned
- Deferred
- Motion Blur
- Bokeh DOF
- SSAO
- Bloom
- TAA
- Auto Exposure
- Color Grading
- Draw Indirect
- Tiled Forward
- Clustered Forward
- Compute Frustum/Occlusion culling
- Reference path-tracer
- PBR
- Reflective Shadow Maps
- Raytraced Soft Shadows
- Raytraced Reflections
- Raytraced Ambient Occlusion
- Raytraced Global Illumination

### Maybe...

- GPU particles (rain & snow effect)
- Sum of sines ocean
- Multithreading
- DirectStorage support
- Frame upscaling with FSR/XeSS/DLSS
- Pixel Sorter using GPU Work Graphs
- Physics
- Audio

## Screenshots

### Shadows

| CSM         | Point       | Spot |
| ----------- | ----------- | ---- |
| ![CSM](.github/CSM.png) | ![PointShadows](.github/PS.png) | ![SpotShadows](.github/SS.png) |
