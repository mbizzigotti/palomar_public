#pragma once
#include "core.h"
#include "shaders/shared.inl"

#define MAX_FRAMES_IN_FLIGHT  1 /* Normally this would be >=2, but 1 is chosen to make the code simplier */
#define MAX_SWAP_CHAIN_IMAGES 4 /* For mobile devices, this would need to be higher.. */
struct Graphics {
    struct Frame {
        uint32_t        frame_index;
        uint32_t        image_index;
        VkCommandBuffer command_buffer;
        VkFramebuffer   frame_buffer;
    };

    enum Partition {
        SCENE_DATA,
        VERTEX_BUFFER,
        INDEX_BUFFER,
        __PARTITION_COUNT__,
    };

	// Common Vulkan objects
	VkInstance            instance; // One of these will exist per application.
	VkPhysicalDevice      physical_device; // This represents the actual graphics card in your system.
	VkDevice              device; // Device acts as an API to access features from a physical device.
	VkQueue               queue; // All commands are sent to the GPU via a command queue (this).
	VkCommandPool         command_pool; // Command buffers are allocated via command pools.
	VkCommandBuffer       command_buffers[MAX_FRAMES_IN_FLIGHT]; // Command buffers are what you fill up to send commands to the GPU.
	
	// Swap chain related
	VkSurfaceKHR          surface; // This represents the connection between Vulkan, and the windowing system.
	VkSwapchainKHR        swap_chain; // A swap chain is a collection of images that will be presented to the screen.
	VkFormat              format; // Image format of the swap chain (typically 8-bit BGRA).
	uint32_t              image_count;
	VkExtent2D            image_extent;
	VkImage               images[MAX_SWAP_CHAIN_IMAGES];
	VkImageView           image_views[MAX_SWAP_CHAIN_IMAGES];
	VkSemaphore           render_finished_semaphores[MAX_SWAP_CHAIN_IMAGES]; // Signaled when image has finished rendering.
	VkSemaphore           image_acquired_semaphores[MAX_FRAMES_IN_FLIGHT]; // Signaled when image is ready to be modified.
	VkFence               fences[MAX_FRAMES_IN_FLIGHT];
	
	// Rendering/Graphics related
	VkRenderPass          render_pass; // Render passes contain information about input/output for a "pass".
	VkFramebuffer         frame_buffers[MAX_SWAP_CHAIN_IMAGES]; // Framebuffers are collections of images to render output to.
	VkImage               depth_image;
	VkDeviceMemory        depth_memory;
	VkImageView           depth_view;
    VkDescriptorPool      descriptor_pool;       // > Descriptors are how you set data in shaders...
    VkDescriptorSetLayout descriptor_set_layout; // > Look up "Vulkan Descriptor Sets" to learn more.
    VkDescriptorSet       descriptor_set;        // > This concept is universal in all newer graphics APIs.
	VkPipelineLayout      pipeline_layout;
	VkPipeline            pipeline; // This represents an entire graphics pipeline, lots of information stored here.
    
	uint64_t              frame_count;
    Frame                 current_frame;
    RGFW_window         * window = nullptr;
    shader::Scene         scene_data;

    Result setup(bool enable_validation);
    Result attach(RGFW_window *window);
    Result prepare_frame();
    Result submit_frame();
    Result allocate_required_memory();
    
    uint32_t allocate_buffer(Partition type, u64 num_bytes);

    void write_vertex_buffer(u32 offset, void *data, u64 num_bytes);
    void write_index_buffer(u32 offset, void *data, u64 num_bytes);

	void flush_partition(Partition partition, u32 offset, u64 size);

private:
	// For every memory partition, we store..
    VkDeviceMemory  memories[__PARTITION_COUNT__]; // ..the handle to the memory,
    VkBuffer        buffers[__PARTITION_COUNT__]; // ..a view (buffer) into that memory,
    void*           mapped_memory[__PARTITION_COUNT__]; //..and a pointer to it's memory mapping.
    uint32_t        partition_sizes[__PARTITION_COUNT__] = {}; // This is only used for allocation.
    bool            memory_available = false; // When memory available is true, you can no longer allocate memory via `allocate_buffer`.

    Result create_instance(bool enable_validation);
    Result pick_physical_device();
    Result create_device(bool enable_validation);
    Result create_command_buffers();
    Result create_depth_buffer();
    Result create_frame_buffers();
    Result create_layouts();
    Result create_render_pipeline();
    uint32_t find_memory_type(uint32_t mask, VkFlags required_properties);
};
