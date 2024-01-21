# Dough

A Game engine written in C\+\+ using Vulkan.
Please note: I started this project so I could learn Vulkan, therefore, the code is not recommended for production use and is subject to change.
A small number of features from C\+\+17 are used so that version will be required to compile the project as is, however, only small parts rely on these features so changing to C\+\+14 could be done easily.

## External Libraries Used:
- [Vulkan](https://www.lunarg.com/vulkan-sdk/)
- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- [STB](https://github.com/nothings/stb)
- [ImGui](https://github.com/ocornut/imgui)
- [Tiny OBJ Loader](https://github.com/tinyobjloader)
- [MSDF Font Rendering](https://github.com/Chlumsky/msdfgen) - GLSL code used for shader and texture atlas generation.

## Preview Images
Here are a few images showing some of the features currently available:
![Text preview](Dough/previewImages/text_preview.png "Soft Mask and MSDF text rendering.")
![3D obj preview](Dough/previewImages/obj_preview.png "3D model (OBJ) rendering.")
![Batch renderig preview](Dough/previewImages/batch_preview.png "Batch renderer with Orthographic camera.")
