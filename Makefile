


TARGET = main.exe
SOURCES = main.cpp
SHADERS = build/fragShader.spv build/vertShader.spv

all: $(TARGET) $(SHADERS)

$(TARGET): $(SOURCES)
	cl /nologo /Fo.\build\ /W4 /MD /std:c++20 /EHsc main.cpp /I"C:\VulkanSDK\glfw-3.4.bin.WIN64\include" /I"C:\VulkanSDK\1.4.309.0\Include" /link /IMPLIB:"C:\portfolio\VulkanTest\x64\Debug\VulkanTest.lib" /LIBPATH:"C:\VulkanSDK\1.4.309.0\Lib" vulkan-1.lib /LIBPATH:"C:\VulkanSDK\glfw-3.4.bin.WIN64\lib-vc2022" glfw3.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NODEFAULTLIB:library /Fe:main.exe 
	

build/fragShader.spv: shaders/fragShader.frag
	glslc shaders/fragShader.frag -o build/fragShader.spv

build/vertShader.spv: shaders/vertShader.vert
	glslc shaders/vertShader.vert -o build/vertShader.spv

clean:
	del "build\\main.obj"
	del "main.exe"
	del "build\\fragShader.spv"
	del "build\\vertShader.spv"