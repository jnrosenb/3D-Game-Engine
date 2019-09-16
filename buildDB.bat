@echo off

mkdir "C:\Users\Jose\Desktop\OpenGl_Framework\Binary\"
pushd "C:\Users\Jose\Desktop\OpenGl_Framework\Binary\"

cl /EHsc /Zi /I. /DVERBOSE ..\Source\Program.cpp ..\Source\ForwardRenderer.cpp ..\Source\DeferredRenderer.cpp ..\Source\Shader.cpp ..\Source\Camera.cpp ..\Source\BaseMaterial.cpp ..\Source\GameObject.cpp ..\Source\Texture.cpp ..\Source\RenderTarget.cpp ..\Source\GameObjectManager.cpp ..\Source\TransformComponent.cpp ..\Source\RendererComponent.cpp ..\Source\FrameRateController.cpp ..\Source\Affine.cpp ..\Source\InputManager.cpp ..\Source\ResourceManager.cpp ..\Source\Model.cpp ..\Source\LoadedMesh.cpp ..\Source\Sphere.cpp ..\Source\Quad.cpp OpenGL32.lib "..\External\Libs\SDL2\SDL2_image.lib" "..\External\Libs\SDL2\SDL2.lib" ..\External\Libs\glew\glew32.lib "..\External\Libs\SDL2\SDL2main.lib" ..\External\Includes\Glad_\glad.c "..\External\Includes\ImGui\imgui.cpp" "..\External\Includes\ImGui\imgui_draw.cpp" "..\External\Includes\ImGui\imgui_widgets.cpp" "..\External\Includes\ImGui\imgui_demo.cpp" "..\External\Includes\ImGui\imgui_impl_win32.cpp" "..\External\Includes\ImGui\imgui_impl_sdl.cpp" "..\External\Includes\ImGui\imgui_impl_opengl3.cpp" "..\External\Libs\Assimp\assimp-vc140-mt.lib" /link /subsystem:console

popd

