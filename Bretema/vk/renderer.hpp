#pragma once

#include "base.hpp"
#include "str.hpp"
#include "types.hpp"

// ^^^ Include the <vk/dx/gl/mt/wg>-Renderer files before the BaseRenderer

#include "../bm/base.hpp"
#include "../bm/utils.hpp"
#include "../bm/renderer.hpp"

#include <vma/vk_mem_alloc.h>

namespace bm::vk
{

class Renderer : public bm::BaseRenderer
{
    static constexpr u64      sOneSec       = 1000000000;
    static constexpr u64      sFlightFrames = 3;
    static constexpr VkFormat sDepthFormat  = VK_FORMAT_D32_SFLOAT;  // @todo: Check VK_FORMAT_D32_SFLOAT_S8_UINT  ??

public:
    Renderer(sPtr<bm::Window> window);
    virtual void update() override { bm::BaseRenderer::update(); }
    virtual void draw(Camera const &cam) override;
    virtual void cleanup() override;

private:
    void initVulkan();
    void initSwapchain(VkSwapchainKHR prev = VK_NULL_HANDLE);
    void initCommands();
    void initDefaultRenderPass();
    void initFramebuffers();
    void initSyncStructures();

    void initDescriptors();

    void initMaterials();
    void initMeshes();
    void initTestScene();

    void recreateSwapchain();

    void executeImmediately(VkCommandPool pool, VkQueue queue, const std::function<void(VkCommandBuffer cb)> &fn);

    AllocatedBuffer createBuffer(
      u64                   byteSize,
      VkBufferUsageFlags    usage,
      VkMemoryPropertyFlags reqFlags,
      VkMemoryPropertyFlags prefFlags     = 0,
      bool                  addToDelQueue = true);
    AllocatedBuffer createBufferStaging(void const *data, u64 bytes, VkBufferUsageFlags usage);

    MeshGroup createMesh(bm::MeshGroup const &meshes);
    Material *createMaterial(VkPipeline pipeline, VkPipelineLayout layout, std::string const &name);

    void drawScene(std::string const &name, Camera const &cam);

    //-------

    inline Material  *material(std::string const &name) { return mMatMap.count(name) > 0 ? &mMatMap[name] : nullptr; }
    inline MeshGroup *mesh(std::string const &name) { return mMeshMap.count(name) > 0 ? &mMeshMap[name] : nullptr; }
    inline Mesh      *mesh0(std::string const &name) { return mMeshMap.count(name) > 0 ? &mMeshMap[name][0] : nullptr; }
    inline FrameData &frame() { return mFrames[mFrameNumber % sFlightFrames]; }

    //-------

    inline VkExtent2D extent2D() { return VkExtent2D((u32)w(), (u32)h()); }
    inline VkExtent3D extent3D() { return VkExtent3D((u32)w(), (u32)h(), 1); }
    inline u32        extentW() { return (u32)w(); }
    inline u32        extentH() { return (u32)h(); }
    inline u32        extentD() { return 1; }

    //-------

    template<typename T>
    size_t paddedSizeUBO()
    {
        size_t const min    = mProperties.limits.minUniformBufferOffsetAlignment;
        size_t const padded = (min > 0) ? (sizeof(T) + min - 1) / min * min : sizeof(T);
        BM_INFOF("Gathering PaddedSizeUBO of {} -> {} : {} : {}", BM_STR_TYPE<T>(), min, sizeof(T), padded);
        return padded;
    }

    //-------

    // IDSM : INSTANCE, DEVICE, SURFACE
    VkInstance                 mInstance       = VK_NULL_HANDLE;  // Vulkan library handle
    VkDebugUtilsMessengerEXT   mDebugMessenger = VK_NULL_HANDLE;  // Vulkan debug output handle
    VkPhysicalDevice           mChosenGPU      = VK_NULL_HANDLE;  // GPU chosen as the default device
    VkDevice                   mDevice         = VK_NULL_HANDLE;  // Vulkan device for commands
    VkSurfaceKHR               mSurface        = VK_NULL_HANDLE;  // Vulkan window surface
    VkPhysicalDeviceProperties mProperties     = {};

    // QUEUEs
    vk::Queue mGraphics = {};
    vk::Queue mPresent  = {};
    vk::Queue mCompute  = {};
    vk::Queue mTransfer = {};

    // MEMORY
    bm::ds::DeletionQueue mDqSwapchain = {};
    bm::ds::DeletionQueue mDqMain      = {};
    VmaAllocator          mAllocator   = VK_NULL_HANDLE;  // Memory Allocator - AMD lib

    // SWAPCHAIN
    VkSwapchainKHR           mSwapchain            = VK_NULL_HANDLE;  // Vulkan swapchain
    // VkFormat                 mSwapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;  // Image format expected by window
    VkFormat                 mSwapchainImageFormat = VK_FORMAT_B8G8R8A8_UINT;  // Image format expected by window
    std::vector<VkImage>     mSwapchainImages      = {};                       // List of images from the swapchain
    std::vector<VkImageView> mSwapchainImageViews  = {};                       // List of image-views from the swapchain

    // DEPTH
    VkImageView    mDepthImageView    = VK_NULL_HANDLE;
    AllocatedImage mDepthImage        = {};
    VkRenderPass   mDefaultRenderPass = VK_NULL_HANDLE;

    // FBOs
    std::vector<VkFramebuffer> mFramebuffers = {};

    // FRAMEs
    FrameData mFrames[sFlightFrames];

    // MATERIALs
    std::vector<VkPipelineLayout>             mPipelineLayouts = {};  // Bucket of pipeline-layouts
    std::vector<VkPipeline>                   mPipelines       = {};  // Bucket of pipelines
    std::unordered_map<std::string, Material> mMatMap          = {};

    // GEOMETRY
    std::unordered_map<std::string, MeshGroup>                 mMeshMap = {};
    std::unordered_map<std::string, std::vector<RenderObject>> mScenes  = {};

    // DESCRIPTORS
    VkDescriptorSetLayout mDescSetLayout;
    VkDescriptorPool      mDescPool;

    // DATA
    SceneData       mSceneData;
    AllocatedBuffer mSceneDataBuff;
    size_t          mSceneDataPaddedSize = 0;
};

}  // namespace bm::vk
