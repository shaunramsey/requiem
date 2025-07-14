

IMGUI_PATH = C:\VulkanSDK\imgui
IMGUI_SOURCES = "$(IMGUI_PATH)\backends\imgui_impl_vulkan.cpp" "$(IMGUI_PATH)\backends\imgui_impl_glfw.cpp" "$(IMGUI_PATH)\imgui.cpp" "$(IMGUI_PATH)\imgui_draw.cpp" "$(IMGUI_PATH)\imgui_tables.cpp" "$(IMGUI_PATH)\imgui_widgets.cpp" "$(IMGUI_PATH)\imgui_demo.cpp"
IMGUI_OBJS = "build\imgui.obj" "build\imgui_demo.obj" "build\imgui_draw.obj" "build\imgui_tables.obj" "build\imgui_widgets.obj" "build\imgui_impl_vulkan.obj" "build\imgui_impl_glfw.obj"
# IMGUI_OBJS = $(patsubst %.cpp,build/%.obj,$(IMGUI_SOURCES))

TARGET = main.exe
SOURCES = main.cpp 
SHADERS = build/fragShader.spv build/vertShader.spv

INCLUDES = /I"C:\VulkanSDK\glfw-3.4.bin.WIN64\include" /I"C:\VulkanSDK\1.4.309.0\Include" /I"C:\VulkanSDK\imgui" /I"C:\VulkanSDK\imgui\backends"
LIBS = /IMPLIB:"C:\portfolio\VulkanTest\x64\Debug\VulkanTest.lib" /LIBPATH:"C:\VulkanSDK\1.4.309.0\Lib" vulkan-1.lib /LIBPATH:"C:\VulkanSDK\glfw-3.4.bin.WIN64\lib-vc2022" glfw3.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
FLAGS = /nologo /Fo.\build\ /W4 /MD /std:c++20 /EHsc /Zi
all: $(TARGET) $(SHADERS)

debug:
	echo $(IMGUI_OBJS)

$(TARGET): $(SOURCES) $(IMGUI_OBJS)
	cl $(FLAGS) $(SOURCES) $(INCLUDES)  /link $(LIBS) /NODEFAULTLIB:library $(IMGUI_OBJS) /Fe:main.exe 

OBJDIR = build


build\imgui_impl_vulkan.obj: $(IMGUI_PATH)/backends/imgui_impl_vulkan.cpp
	cl /c $(FLAGS) $(INCLUDES)  $(IMGUI_PATH)/backends/imgui_impl_vulkan.cpp /Fo:$@

build\imgui_impl_glfw.obj: $(IMGUI_PATH)/backends/imgui_impl_glfw.cpp
	cl /c $(FLAGS) $(INCLUDES)  $(IMGUI_PATH)/backends/imgui_impl_glfw.cpp /Fo:$@

build\imgui.obj: $(IMGUI_PATH)/imgui.cpp
	cl /c $(FLAGS) $(INCLUDES)  $(IMGUI_PATH)/imgui.cpp /Fo:$@

build\imgui_demo.obj: $(IMGUI_PATH)/imgui_demo.cpp
	cl /c $(FLAGS) $(INCLUDES)  $(IMGUI_PATH)/imgui_demo.cpp /Fo:$@

build\imgui_draw.obj: $(IMGUI_PATH)/imgui_draw.cpp
	cl /c $(FLAGS) $(INCLUDES)  $(IMGUI_PATH)/imgui_draw.cpp /Fo:$@

build\imgui_tables.obj: $(IMGUI_PATH)/imgui_tables.cpp
	cl /c $(FLAGS) $(INCLUDES)  $(IMGUI_PATH)/imgui_tables.cpp /Fo:$@

build\imgui_widgets.obj: $(IMGUI_PATH)/imgui_widgets.cpp
	cl /c $(FLAGS) $(INCLUDES)  $(IMGUI_PATH)/imgui_widgets.cpp /Fo:$@

build/fragShader.spv: shaders/fragShader.frag
	glslc shaders/fragShader.frag -o build/fragShader.spv

build/vertShader.spv: shaders/vertShader.vert
	glslc shaders/vertShader.vert -o build/vertShader.spv

clean:
	del "build\\*.obj"
	del "main.exe"
	del "build\\*.spv"