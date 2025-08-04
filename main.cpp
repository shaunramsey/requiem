#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <thread>
#include <chrono>

// #include "imconfig.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
// #include "imgui_internal.h"
// #include "imstb_rectpack.h"
// #include "imstb_textedit.h"
// #include "imstb_truetype.h"
#include "Console.h" //includes
#include "GameSettings.h"
#include "externs.h"

// 1280x720, 1920x1080, 2560x1440, 3840x2160
const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

const int MAX_FRAMES_IN_FLIGHT = 2;
GameSettings gameSettings;
GameSettings modifiableGameSettings; // a copy of game settings for use in the game settings display window
Ramsey::Console _console;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// A struct to manage data related to one image in vulkan
struct MyTextureData
{
    VkDescriptorSet DS; // Descriptor set: this is what you'll pass to Image()
    int Width;
    int Height;
    int Channels;

    // Need to keep track of these to properly cleanup
    VkImageView ImageView;
    VkImage Image;
    VkDeviceMemory ImageMemory;
    VkSampler Sampler;
    VkBuffer UploadBuffer;
    VkDeviceMemory UploadBufferMemory;

    MyTextureData() { memset(this, 0, sizeof(*this)); }
};

static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

static void check_vk_result_with_message(VkResult err, const char *msg)
{
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d. With Message %s\n", err, msg);
    if (err < 0)
        abort();
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct ShaderData
{
    glm::vec2 res;
    glm::vec2 rotation;
    glm::vec3 camera;
    float time;
};

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

const std::vector<Vertex> vertices = {
    {{-1.0f, -1.f}, {1.0f, 0.0f, 0.0f}},
    {{1.0f, -1.f}, {0.0f, 1.0f, 0.0f}},
    {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
    {{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}};

const std::vector<uint16_t> vertexIndices = {
    0, 1, 2, 2, 3, 0};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

class HelloRayMarchSphereApplication
{
public:
    void run()
    {
        initSettings();
        initWindow();
        initVulkan();
        initImGui(); // imguiinit call
        mainLoop();
        cleanup();
    }

    ShaderData UniformData;

private:
    int _buildNumber = 0;
    bool _hide_all_gui = false;
    bool _show_about = false;
    bool _show_stats = true;
    bool _show_console = false;
    bool _show_game_settings = false;

    GLFWwindow *window;
    VkInstanceCreateInfo _createInfo{};
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkAllocationCallbacks *NullAllocator = nullptr;
    VkPipelineCache NullPipelineCache = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamily = (uint32_t)-1;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorPool imguiDescriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void *> uniformBuffersMapped;

    VkRenderPass renderPass;
    VkRenderPass imguiRenderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    uint32_t currentFrame = 0;
    std::vector<VkFence> imagesInFlight;
    uint64_t frameCount = 0;
    toml::table loadingMessagesTable;
    MyTextureData my_texture;

    bool framebufferResized = false;

    int firstTabOpen = 1;

    void initSettings()
    {
        _console.DebugLog("LOAD", "Loading Game Settings From TOML");
        gameSettings.loadDefaults();
        // Load uniform data from file
        std::ifstream file("build_number.txt");
        if (file.is_open())
        {
            file >> _buildNumber;
            file.close();
        }
    }

    void initWindow()
    {
        _console.DebugLog("LOAD", "Initializing Window");
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

        if (gameSettings.graphicsSettings.fullscreenPrimary)
        {
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            if (gameSettings.graphicsSettings.borderlessWindow)
            {

                window = glfwCreateWindow(mode->width, mode->height, "Fluxwerkz's Window", nullptr, nullptr);
            }
            else
            {
                window = glfwCreateWindow(mode->width, mode->height, "Fluxwerkz's Window", monitor, nullptr);
            }
        }
        else
        {
            window = glfwCreateWindow(WIDTH, HEIGHT, "Ramsey's Window", nullptr, nullptr);
        }
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    // int width, int height
    static void framebufferResizeCallback(GLFWwindow *window, int, int)
    {
        auto app = reinterpret_cast<HelloRayMarchSphereApplication *>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
        // UniformData.res = glm::vec2(width,height);
    }

    void initVulkan()
    {
        _console.DebugLog("LOAD", "Initializing Vulkan");
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();

        createImGuiRenderPass();

        createDescriptorSetLayout();
        createGraphicsPipeline(); // start
        createFramebuffers();
        createCommandPool();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers(); // creating unifrom Buffers
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }

    unsigned char *DefaultImage(MyTextureData *tex_data)
    {
        // Load a default image if the texture data is not set
        tex_data->Width = 2;
        tex_data->Height = 2;
        tex_data->Channels = 4; // RGBA

        // Create a 1x1 white pixel
        unsigned char *image_data = new unsigned char[16];
        image_data[0] = 255;  // R
        image_data[1] = 100;  // G
        image_data[2] = 255;  // B
        image_data[3] = 255;  // A
        image_data[4] = 0;    // R
        image_data[5] = 0;    // G
        image_data[6] = 0;    // B
        image_data[7] = 255;  // A
        image_data[8] = 0;    // R
        image_data[9] = 0;    // G
        image_data[10] = 0;   // B
        image_data[11] = 255; // A
        image_data[12] = 255; // R
        image_data[13] = 100; // G
        image_data[14] = 255; // B
        image_data[15] = 255; // A

        return image_data;
    }

    // Helper function to load an image with common settings and return a MyTextureData with a VkDescriptorSet as a sort of Vulkan pointer
    bool LoadTextureFromFile(const char *filename, MyTextureData *tex_data)
    {
        // Specifying 4 channels forces stb to load the image in RGBA which is an easy format for Vulkan
        tex_data->Channels = 4;
        unsigned char *image_data = stbi_load(filename, &tex_data->Width, &tex_data->Height, 0, tex_data->Channels);

        if (image_data == NULL)
        {
            _console.WarningLog("LOAD", "[*] Failed to load texture from file: \"%s\", using default image", filename);
            image_data = DefaultImage(tex_data);
        }

        if (image_data == NULL)
            return false;

        // Calculate allocation size (in number of bytes)
        size_t image_size = tex_data->Width * tex_data->Height * tex_data->Channels;

        VkResult err;

        // Create the Vulkan image.
        {
            VkImageCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.imageType = VK_IMAGE_TYPE_2D;
            info.format = VK_FORMAT_R8G8B8A8_UNORM;
            info.extent.width = tex_data->Width;
            info.extent.height = tex_data->Height;
            info.extent.depth = 1;
            info.mipLevels = 1;
            info.arrayLayers = 1;
            info.samples = VK_SAMPLE_COUNT_1_BIT;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            err = vkCreateImage(device, &info, NullAllocator, &tex_data->Image);
            check_vk_result(err);
            VkMemoryRequirements req;
            vkGetImageMemoryRequirements(device, tex_data->Image, &req);
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            err = vkAllocateMemory(device, &alloc_info, NullAllocator, &tex_data->ImageMemory);
            check_vk_result(err);
            err = vkBindImageMemory(device, tex_data->Image, tex_data->ImageMemory, 0);
            check_vk_result(err);
        }

        // Create the Image View
        {
            VkImageViewCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = tex_data->Image;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = VK_FORMAT_R8G8B8A8_UNORM;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.layerCount = 1;
            err = vkCreateImageView(device, &info, NullAllocator, &tex_data->ImageView);
            check_vk_result(err);
        }

        // Create Sampler
        {
            VkSamplerCreateInfo sampler_info{};
            sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_info.magFilter = VK_FILTER_LINEAR;
            sampler_info.minFilter = VK_FILTER_LINEAR;
            sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
            sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.minLod = -1000;
            sampler_info.maxLod = 1000;
            sampler_info.maxAnisotropy = 1.0f;
            err = vkCreateSampler(device, &sampler_info, NullAllocator, &tex_data->Sampler);
            check_vk_result(err);
        }

        // Create Descriptor Set using ImGUI's implementation
        tex_data->DS = ImGui_ImplVulkan_AddTexture(tex_data->Sampler, tex_data->ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Create Upload Buffer
        {
            VkBufferCreateInfo buffer_info = {};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = image_size;
            buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            err = vkCreateBuffer(device, &buffer_info, NullAllocator, &tex_data->UploadBuffer);
            check_vk_result(err);
            VkMemoryRequirements req;
            vkGetBufferMemoryRequirements(device, tex_data->UploadBuffer, &req);
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            err = vkAllocateMemory(device, &alloc_info, NullAllocator, &tex_data->UploadBufferMemory);
            check_vk_result(err);
            err = vkBindBufferMemory(device, tex_data->UploadBuffer, tex_data->UploadBufferMemory, 0);
            check_vk_result(err);
        }

        // Upload to Buffer:
        {
            void *map = NULL;
            err = vkMapMemory(device, tex_data->UploadBufferMemory, 0, image_size, 0, &map);
            check_vk_result(err);
            memcpy(map, image_data, image_size);
            VkMappedMemoryRange range[1] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = tex_data->UploadBufferMemory;
            range[0].size = image_size;
            err = vkFlushMappedMemoryRanges(device, 1, range);
            check_vk_result(err);
            vkUnmapMemory(device, tex_data->UploadBufferMemory);
        }

        // Release image memory using stb
        stbi_image_free(image_data);

        // Create a command buffer that will perform following steps when hit in the command queue.
        // TODO: this works in the example, but may need input if this is an acceptable way to access the pool/create the command buffer.
        // VkCommandPool command_pool = g_MainWindowData.Frames[g_MainWindowData.FrameIndex].CommandPool;
        VkCommandBuffer command_buffer;
        {
            VkCommandBufferAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandPool = commandPool;
            alloc_info.commandBufferCount = 1;

            err = vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);
            check_vk_result(err);

            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(command_buffer, &begin_info);
            check_vk_result(err);
        }

        // Copy to Image
        {
            VkImageMemoryBarrier copy_barrier[1] = {};
            copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier[0].image = tex_data->Image;
            copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copy_barrier[0].subresourceRange.levelCount = 1;
            copy_barrier[0].subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, copy_barrier);

            VkBufferImageCopy region = {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.layerCount = 1;
            region.imageExtent.width = tex_data->Width;
            region.imageExtent.height = tex_data->Height;
            region.imageExtent.depth = 1;
            vkCmdCopyBufferToImage(command_buffer, tex_data->UploadBuffer, tex_data->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            VkImageMemoryBarrier use_barrier[1] = {};
            use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            use_barrier[0].image = tex_data->Image;
            use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            use_barrier[0].subresourceRange.levelCount = 1;
            use_barrier[0].subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, use_barrier);
        }

        // End command buffer
        {
            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            err = vkEndCommandBuffer(command_buffer);
            check_vk_result(err);
            err = vkQueueSubmit(graphicsQueue, 1, &end_info, VK_NULL_HANDLE);
            check_vk_result(err);
            err = vkDeviceWaitIdle(device);
            check_vk_result(err);
        }

        return true;
    }

    // Helper function to cleanup an image loaded with LoadTextureFromFile
    void RemoveTexture(MyTextureData *tex_data)
    {
        vkFreeMemory(device, tex_data->UploadBufferMemory, nullptr);
        vkDestroyBuffer(device, tex_data->UploadBuffer, nullptr);
        vkDestroySampler(device, tex_data->Sampler, nullptr);
        vkDestroyImageView(device, tex_data->ImageView, nullptr);
        vkDestroyImage(device, tex_data->Image, nullptr);
        vkFreeMemory(device, tex_data->ImageMemory, nullptr);
        // ImGui_ImplVulkan_RemoveTexture(tex_data->DS);
    }

    void drawAbout()
    {
        static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings; // ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Once);
        if (!ImGui::Begin("About", &_show_about, window_flags))
        {
            ImGui::End();
            return;
        }

        ImGui::Text("About this application");
        ImGui::BulletText("Descriptive text follows");
        ImGui::Separator();
        ImGui::Text("Licensing");
        ImGui::BulletText("This software is not licensed.");
        ImGui::TextWrapped("This software is intentionally left without a license and all copyright belongs to its creator.");
        ImGui::Separator();
        ImGui::Text("About the creator");
        ImGui::BulletText("(C) 2025 Shaun David Ramsey, Ph.D.");
        ImGui::Indent();
        ImGui::TextWrapped("Shaun is a senior computational scientist at Rocketwerkz and a full professor of mathematics and computer science at Washington College.");
        ImGui::Unindent();
        ImGui::Separator();
        ImGui::Text("Build/Version Information");
        ImGui::BulletText("Version 0.0.1");
        ImGui::BulletText("Build Number %d", _buildNumber);
        ImGui::BulletText("Built on %s at %s", __DATE__, __TIME__);
        ImGui::Separator();
        ImGui::Text("Built Using:");
        ImGui::BulletText("GLFW %s", glfwGetVersionString());
        uint32_t version;
        vkEnumerateInstanceVersion(&version);

        ImGui::BulletText("Vulkan %d.%d.%d", VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version), VK_API_VERSION_PATCH(version));
        ImGui::BulletText("Dear ImGui %s", ImGui::GetVersion());

        ImGui::End();
    }

    // display fps and other statistics
    void drawStats(float in_dt)
    {
        static float use_dt = 16.0f;
        static float total_dt = 0.0f;
        static int frames = 0;
        total_dt += in_dt;
        frames++;
        if (total_dt > 1000.0f)
        {
            use_dt = total_dt / frames;
            total_dt = 0.0f;
            frames = 0;
        }

        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 win_size = ImVec2(work_pos.x + work_size.x, work_pos.y + work_size.y);

        ImVec2 padding = ImVec2(0.0, 0.0);
        ImVec2 pos = ImVec2(win_size.x - padding.x, work_pos.y + padding.y);
        static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar;
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (!ImGui::Begin("Stats", &_show_stats, window_flags))
        {
            ImGui::PopStyleVar();
            ImGui::End();
            return;
        }

        ImGui::Text("%5.1fms (%3.0f FPS)", use_dt, 1000.0 / use_dt);
        // ImGui::Text("%f, %f -> %f, %f", win_size.x, win_size.y, text_size.x, text_size.y);

        ImGui::PopStyleVar();
        ImGui::End();
    }

    void drawConsole()
    {
        _console.drawImGui(&_show_console);
    }

    void toggleGameSettingsWindow()
    {
        _show_game_settings = !_show_game_settings;
        if (_show_game_settings)
        { // then make a copy of the current game settings for use in game settings display
            firstTabOpen = 1;
            modifiableGameSettings = gameSettings;
        }
    }

    void gameSettingsButtonBar(bool sameline, bool isEqual)
    {

        float buttonWidth = sameline ? 150.0f : 250.0f;
        if (!isEqual)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0f, 0.4f, 0.0f, 1.0f});

        if (ImGui::Button("Save and Apply", ImVec2(buttonWidth, 0.0f)))
        {
            gameSettings = modifiableGameSettings;
            gameSettings.saveChanges(); // save the modified settings to main.toml
            toggleGameSettingsWindow(); // close the window
        }
        if (!isEqual)
            ImGui::PopStyleColor();

        if (sameline)
            ImGui::SameLine();

        if (!isEqual)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.6f, 0.0f, 0.0f, 1.0f});
        if (ImGui::Button("Discard Changes", ImVec2(buttonWidth, 0.0f)))
        {
            toggleGameSettingsWindow();
        }
        if (!isEqual)
            ImGui::PopStyleColor();

        if (sameline)
            ImGui::SameLine();

        if (!isEqual)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0f, 0.25f, 0.5f, 1.0f});

        if (ImGui::Button("Restore Defaults", ImVec2(buttonWidth, 0.0f)))
        {
            modifiableGameSettings = GameSettings();
            // gameSettings.saveChanges(); // save the default settings to main.toml
            // toggleGameSettingsWindow(); // close the window
        }

        if (!isEqual)
            ImGui::PopStyleColor();
    }

    void drawImGuiGameSettingsWindow(GameSettings &gs, const GameSettings &comparisonGS)
    {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::SetNextWindowPos(work_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(work_size, ImGuiCond_Always);
        if (!ImGui::Begin("Game Settings", &_show_game_settings, window_flags))
        {
            ImGui::End();
            return;
        }
        bool isEqual = gameSettings.isEqual(modifiableGameSettings);
        gameSettingsButtonBar(true, isEqual);
        {
            ImGui::Text("Which settings would you like to change?");
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("GameSettingsTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Settings", nullptr, firstTabOpen ? ImGuiTabItemFlags_SetSelected : tab_bar_flags))
                {
                    ImGui::Text("Welcome to the Overall Game Settings!");
                    firstTabOpen = 0;
                    gameSettingsButtonBar(false, isEqual);
                    if (ImGui::Button("Quit - Close Application - Do Not Save Changes"))
                    {
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Graphics"))
                {
                    ImGui::Text("Welcome to the Graphics Settings!");
                    gs.graphicsSettings.drawImGui(comparisonGS.graphicsSettings);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("KeyBinds"))
                {
                    ImGui::Text("Welcome to the KeyBinds Settings!");
                    gs.keyBindSettings.drawImGui(comparisonGS.keyBindSettings);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Console"))
                {
                    ImGui::Text("Welcome to the Console Settings!");
                    gs.consoleSettings.drawImGui(comparisonGS.consoleSettings);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }
    }

    void drawImGui(float dt)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static bool show_demo_window = false;

        static const double splashTime = 1.0f;
        if (glfwGetTime() < splashTime)
        {
            static toml::array *loadingMessages = loadingMessagesTable["loading_messages"].as_array();
            ImGuiViewport *imguiViewport = ImGui::GetMainViewport();
            ImVec2 size = imguiViewport->WorkSize;
            size = ImVec2(size.x + 4.0f, size.y + 4.0f); // add a little padding to the size
            ImGui::SetNextWindowPos(ImVec2(-2.0, -2.0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(size, ImGuiCond_Always);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("Splash Screen", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
            // ImGui::Text("descriptor set = %p", my_texture.DS);
            // ImGui::Text("size = %d x %d", my_texture.Width, my_texture.Height);
            // ImGui::Text("winsize = %f, %f", size.x, size.y);
            // std::string log = "win size = " + std::to_string(size.x) + ", " + std::to_string(size.y);
            // std::cout << log << std::endl;
            //_console.Log("LOAD", log.c_str(), nullptr);
            ImGui::Image((ImTextureID)my_texture.DS, size, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // ImVec2(float(my_texture.Width), float(my_texture.Height)));

            ImGui::PopStyleVar(2);

            ImGui::SetNextWindowPos(ImVec2(size.x * 0.5f, size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.25f));
            ImGui::BeginChild("Loading Messages", ImVec2(300.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_AlwaysUseWindowPadding); //, nullptr); //, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
            ImGui::Text("Loading... Please wait.");
            if (loadingMessages != nullptr)
            {
                double timeEach = splashTime / loadingMessages->size();
                int index = int(glfwGetTime() / timeEach);
                // std::cout << "index = " << index << ", timeEach = " << timeEach << ", glfwGetTime() = " << glfwGetTime() << std::endl;
                if (index <= 0 || index > loadingMessages->size())
                {
                    index = 0; // reset to first message if out of bounds
                }
                toml::array msgArray = *loadingMessages;
                std::string message = msgArray[index].value<std::string>().value_or("Loading Errors...");
                ImGui::Text("%s", message.c_str());
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();

            ImGui::End();

            ImGui::Render();
            return;
        }

        if (_hide_all_gui)
        {
            ImGui::Render();
            return;
        }

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Settings", gameSettings.keyBindSettings.toggleSettingsKeyName.c_str()))
                {
                    toggleGameSettingsWindow();
                }
                if (ImGui::MenuItem("Exit", "ALT+F4")) // , "ALT+F4"
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Binds"))
            {
                if (ImGui::MenuItem("Toggle GUI", gameSettings.keyBindSettings.toggleUiKeyName.c_str()))
                {
                    _hide_all_gui = !_hide_all_gui;
                }
                if (ImGui::MenuItem("Toggle Stats", gameSettings.keyBindSettings.toggleStatsKeyName.c_str()))
                {
                    _show_stats = !_show_stats;
                }
                if (ImGui::MenuItem("Toggle Console", gameSettings.keyBindSettings.toggleConsoleKeyName.c_str()))
                {
                    _show_console = !_show_console;
                }
                // ImGui::Separator();
                if (ImGui::MenuItem("Quit", "ALT+F4"))
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About"))
                {
                    _show_about = !_show_about;
                }
                if (ImGui::MenuItem("Show Stats", gameSettings.keyBindSettings.toggleStatsKeyName.c_str()))
                {
                    _show_stats = !_show_stats;
                }
                if (ImGui::MenuItem("Toggle ImGui Demo Window"))
                {
                    show_demo_window = !show_demo_window;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        // call all the draw stuffs
        if (_show_about)
        {
            drawAbout();
        }

        if (_show_stats)
        {
            drawStats(dt);
        }

        drawConsole();
        if (_show_console)
        {
            ImGui::SetWindowFocus("Console");
        }

        if (_show_game_settings)
        {
            drawImGuiGameSettingsWindow(modifiableGameSettings, gameSettings);
        }

        ImGui::Render();
    }

    void processImGuiEvents()
    {

        //    if (ImGui::GetIO().WantCaptureKeyboard)
        //     return;
        if (ImGui::IsKeyPressed(ImGuiKey_Backslash) || ImGui::IsKeyPressed(gameSettings.keyBindSettings.toggleConsoleKey))
        // if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) // || ImGui::IsKeyPressed(ImGuiKey_Backslash))
        {
            _show_console = !_show_console;
        }

        if (ImGui::IsKeyPressed(gameSettings.keyBindSettings.toggleUiKey))
        {
            _hide_all_gui = !_hide_all_gui;
        }
        if (ImGui::IsKeyPressed(gameSettings.keyBindSettings.toggleStatsKey))
        {
            _show_stats = !_show_stats;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_F1))
        {
            _console.Log("F1", "Test #1", nullptr);
            _console.WarningLog("F1", "Test #2", nullptr);
            _console.ErrorLog("F1", "Test #3", nullptr);
            _console.DebugLog("F1", "Test #4", nullptr);
        }
        if (ImGui::IsKeyPressed(gameSettings.keyBindSettings.toggleSettingsKey))
        {
            toggleGameSettingsWindow();
        }
    }

    void mainLoop()
    {

        // glfwSetWindowOpacity(window,1.0f);
        // start_server(25565);

        UniformData.camera = glm::vec3(0.0, 0.0, -3.0);

        // static auto startTime = std::chrono::high_resolution_clock::now();
        auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();

        // Get the period of the high_resolution_clock
        using period = std::chrono::high_resolution_clock::period;

        while (!glfwWindowShouldClose(window))
        {
            lastTime = currentTime;
            currentTime = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastTime).count();

            drawImGui(dt);
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

            processImGuiEvents();
            glfwPollEvents();

            // get_info();

            frameCount += 1;

            UniformData.rotation.x = glm::mod(UniformData.rotation.x, 3.14159265f * 2.0f);
            UniformData.rotation.y = glm::mod(UniformData.rotation.y, 3.14159265f * 2.0f);

            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void cleanupSwapChain()
    {
        for (auto framebuffer : swapChainFramebuffers)
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews)
        {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void cleanup()
    {
        std::cout << "[*] Performing Cleanup" << std::endl;
        ImGui_ImplVulkan_Shutdown();

        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        cleanupSwapChain();
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroyRenderPass(device, imguiRenderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyDescriptorPool(device, imguiDescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        RemoveTexture(&my_texture);

        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void initImGui() // imguiinit
    {
        //        std::cout << "[*] Initializing Dear ImGui" << std::endl;
        _console.DebugLog("LOAD", "Initializing Dear ImGui", nullptr);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        // ImGuiIO &io = ImGui::GetIO();
        // glfwSetKeyCallback(window, key_callback);
        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = physicalDevice;
        init_info.Device = device;
        init_info.QueueFamily = graphicsQueueFamily;
        init_info.Queue = graphicsQueue;
        init_info.PipelineCache = NullPipelineCache;
        init_info.DescriptorPool = imguiDescriptorPool;
        init_info.RenderPass = imguiRenderPass;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = NullAllocator;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);

        std::string filename = "images/splash.png";
        bool ret = LoadTextureFromFile(filename.c_str(), &my_texture);
        IM_ASSERT(ret);
        _console.Log("LOAD", "Acquired texture: \"%s\", size %d x %d", filename.c_str(), my_texture.Width, my_texture.Height);
        // std::cout << "[*] Loaded texture: " << my_texture.DS << " from: " << filename << " with size: " << my_texture.Width << " x " << my_texture.Height << std::endl;

        try
        {
            loadingMessagesTable = toml::parse_file("text/loading.toml");
            _console.Log("LOAD", "Splash messages loaded from \"text/loading.toml\"");
        }
        catch (const toml::parse_error &err)
        {
            _console.WarningLog("TOML", "[*] Failed to read splash messages from \"%s\": %s", "text/loading.toml", err.what());
        }
    }

    void createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(ShaderData);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    void createDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }

        // THIS IS WHERE WE BEGIN WITH THE IMGUI DESCRIPTOR POOL
        VkDescriptorPoolSize pool_sizes[] =
            {
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2},
            };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (VkDescriptorPoolSize &pool_size : pool_sizes)
            pool_info.maxSets += pool_size.descriptorCount;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        VkResult err = vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiDescriptorPool);
        check_vk_result_with_message(err, "failed to create imgui descriptor pool");

        // VkDescriptorPoolSize imguiPoolSizes[] = {
        // 					{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        // 					{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        // 					{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        // 					{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        // 					{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        // 					{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        // 					{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        // 					{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        // 					{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        // 					{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        // 					{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        //                 };
        // // poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // // poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        // VkDescriptorPoolCreateInfo imguiPoolInfo{};
        // poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        // poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        // poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(imguiPoolSizes);
        // poolInfo.pPoolSizes = imguiPoolSizes;
        // poolInfo.maxSets = 1000 * IM_ARRAYSIZE(imguiPoolSizes); // static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        // if (vkCreateDescriptorPool(device, &imguiPoolInfo, nullptr, &imguiDescriptorPool) != VK_SUCCESS)
        // {
        //     throw std::runtime_error("failed to create imgui descriptor pool!");
        // }
    }

    void createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(ShaderData);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    void createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createFramebuffers();
    }

    void createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello RM Sphere Test"; // TODO: change to production name
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.pEngineName = "FluxWerkz";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        _createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        _createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        _createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        _createInfo.ppEnabledExtensionNames = extensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers)
        {
            _createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            _createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            _createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
        else
        {
            _createInfo.enabledLayerCount = 0;

            _createInfo.pNext = nullptr;
        }

        std::cout << "[*] Creating Vulkan Instance" << std::endl;
        if (vkCreateInstance(&_createInfo, NullAllocator, &instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
        std::cout << "[*] Vulkan Instance Created Successfully" << std::endl;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {

        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger()
    {
        if (!enableValidationLayers)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto &tdevice : devices)
        {
            if (isDeviceSuitable(tdevice))
            {
                physicalDevice = tdevice;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        graphicsQueueFamily = indices.graphicsFamily.value();
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        // std::cout << "[*] Swapchain image count: " << imageCount << std::endl;
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void createImGuiRenderPass()
    {
        VkAttachmentDescription colorAttachment{}; // framebuffer image
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &imguiRenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment{}; // framebuffer image
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createGraphicsPipeline()
    {
        // auto vertShaderCode = readFile("C:\\messaround\\clonerequiem\\x64\\Debug\\vertShader.spv");
        // auto fragShaderCode = readFile("C:\\messaround\\clonerequiem\\x64\\Debug\\fragShader.spv");

        auto vertShaderCode = readFile("build\\vertShader.spv");
        auto fragShaderCode = readFile("build\\fragShader.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        // colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void createFramebuffers()
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            VkImageView attachments[] = {
                swapChainImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    void createVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void createIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(vertexIndices[0]) * vertexIndices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertexIndices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    // Helper function to find Vulkan memory type bits. See ImGui_ImplVulkan_MemoryType() in imgui_impl_vulkan.cpp
    // uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
    // {
    //     VkPhysicalDeviceMemoryProperties mem_properties;
    //     vkGetPhysicalDeviceMemoryProperties(physicalDevice, &mem_properties);

    //     for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    //         if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
    //             return i;

    //     return 0xFFFFFFFF; // Unable to find memoryType
    // }

    void createCommandBuffers()
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(vertexIndices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        VkRenderPassBeginInfo imguiRenderPassInfo{};
        imguiRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        imguiRenderPassInfo.renderPass = imguiRenderPass;
        imguiRenderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        imguiRenderPassInfo.renderArea.offset = {0, 0};
        imguiRenderPassInfo.renderArea.extent = swapChainExtent;
        /// we need a "barrier/sema/fence" to wait for the render pass to finish
        vkCmdBeginRenderPass(commandBuffer, &imguiRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[currentFrame]);
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects()
    {

        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(swapChainImages.size());
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapChainImages.size());

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create render finished semaphores for a swapchain!");
            }

            // printf("rende Semaphore %d -> %p\n", (int)i, renderFinishedSemaphores[i]);
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image available semaphores for a swapchain!");
            }
            // printf("avail Semaphore %d -> %p\n", (int)i, imageAvailableSemaphores[i]);
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization fences for a frame!");
            }
        }
    }

    void updateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        // UniformBufferObject ubo{};
        // ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
        // ubo.proj[1][1] *= -1;

        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        UniformData.res = glm::vec2(width, height);
        UniformData.time = time;

        memcpy(uniformBuffersMapped[currentImage], &UniformData, sizeof(ShaderData));
    }

    void drawFrame()
    {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        // std::cout << "[*] Acquired image index: " << imageIndex << " with current frame: " << currentFrame << std::endl;

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        // {
        //     vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        // }
        // // Mark this image as now being in use by this frame
        // imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        updateUniformBuffer(currentFrame);

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore renderFinishedSemaphore[] = {renderFinishedSemaphores[imageIndex]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = renderFinishedSemaphore;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = renderFinishedSemaphore;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
        {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    VkShaderModule createShaderModule(const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
        {
            // if (availableFormat.format == VK_FORMAT_R16G16B16A16_SFLOAT && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            // {
            //     return availableFormat;
            // }

            if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
            // if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            // {
            //     return availableFormat;
            // }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        if (gameSettings.graphicsSettings.vsync)
        {
            return VK_PRESENT_MODE_FIFO_KHR; // enables vsync
        }
        for (const auto &availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR; // enables vsync
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)};

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice indevice)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(indevice, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(indevice, surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(indevice, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(indevice, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(indevice, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice indevice)
    {
        QueueFamilyIndices indices = findQueueFamilies(indevice);

        bool extensionsSupported = checkDeviceExtensionSupport(indevice);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(indevice);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice indevice)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(indevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(indevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice indevice)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(indevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(indevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(indevice, i, surface, &presentSupport);

            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    std::vector<const char *> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    static std::vector<char> readFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        std::string datanull = pUserData == NULL ? "yes" : "no";
        std::string msg = pCallbackData->pMessage;
        if (msg.find("Loading layer library") != std::string::npos)
        {
            return VK_FALSE; // Ignore layer loading messages
        }
        if (msg.find("Unloading layer library") != std::string::npos)
        {
            return VK_FALSE; // Ignore layer unloading messages
        }
        std::cerr << "  [*] Validation layer: " << pCallbackData->pMessage;
        std::cerr << " Type: " << messageType << " Severity:" << messageSeverity << " userdatanull?:" << datanull << std::endl;
        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            _console.ErrorLog("VULKAN", "Validation Layer error message %s", pCallbackData->pMessage);
        }
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            _console.WarningLog("VULKAN", "Validation Layer warning message %s", pCallbackData->pMessage);
        }
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            _console.DebugLog("VULKAN", "Validation Layer info message %s", pCallbackData->pMessage);
        }
        else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            _console.DebugLog("VULKAN", "Validation Layer info message %s", pCallbackData->pMessage);
        }
        //_console.ErrorLog("VULKAN", "Validation Layer error message %s", pCallbackData->pMessage);
        //_console.ErrorLog("VULKAN", "Type: %d Severity: %d userdatanull? %s", messageType, messageSeverity, datanull.c_str());

        return VK_FALSE;
    }
};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    std::string logMsg = "key " + std::to_string(key) + " scancode " + std::to_string(scancode) + " action " + std::to_string(action) + " mods " + std::to_string(mods);
    _console.Log("key_callback", logMsg.c_str());

    HelloRayMarchSphereApplication *ptr = (HelloRayMarchSphereApplication *)glfwGetWindowUserPointer(window);
    bool pressed = action == GLFW_REPEAT || action == GLFW_PRESS;
    if (key == GLFW_KEY_UP && pressed)
    {
        ptr->UniformData.rotation += glm::vec2(0.0, -0.1);
    }
    if (key == GLFW_KEY_DOWN && pressed)
    {
        ptr->UniformData.rotation += glm::vec2(0.0, 0.1);
    }
    if (key == GLFW_KEY_LEFT && pressed)
    {
        ptr->UniformData.rotation += glm::vec2(-0.1, 0.0);
    }
    if (key == GLFW_KEY_RIGHT && pressed)
    {
        ptr->UniformData.rotation += glm::vec2(0.1, 0.0);
    }

    std::string logMsg2 = "Old Rotation Is: " + std::to_string(ptr->UniformData.rotation.x) + ", " + std::to_string(ptr->UniformData.rotation.y);
    _console.Log("key_callback", logMsg2.c_str());
    ptr->UniformData.rotation.x = glm::mod(ptr->UniformData.rotation.x, 3.14159265f * 2.0f);
    ptr->UniformData.rotation.y = glm::mod(ptr->UniformData.rotation.y, 3.14159265f * 2.0f);
    std::string logMsg3 = "New Rotation Is: " + std::to_string(ptr->UniformData.rotation.x) + ", " + std::to_string(ptr->UniformData.rotation.y);
    _console.Log("key_callback", logMsg3.c_str());
}

int main()
{
    HelloRayMarchSphereApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}