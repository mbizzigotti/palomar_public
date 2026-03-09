#pragma once
#include "core.h"
#include "shaders/shared.inl"

#define MAX_FRAMES_IN_FLIGHT  2
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

	VkInstance            instance;
	VkSurfaceKHR          surface;
	VkPhysicalDevice      physical_device;
	VkDevice              device;
	VkQueue               queue;
	VkCommandPool         command_pool;
	VkCommandBuffer       command_buffers[MAX_FRAMES_IN_FLIGHT];
	VkSwapchainKHR        swap_chain;
	VkFormat              format;
	uint32_t              image_count;
	VkExtent2D            image_extent;
	VkImage               images[MAX_SWAP_CHAIN_IMAGES];
	VkImageView           image_views[MAX_SWAP_CHAIN_IMAGES];
	VkSemaphore           image_ready_semaphores[MAX_FRAMES_IN_FLIGHT];    // Signaled when image is ready to be rendered to
	VkSemaphore           present_ready_semaphores[MAX_SWAP_CHAIN_IMAGES]; // Signaled when image is ready to be presented
	VkFence               fences[MAX_FRAMES_IN_FLIGHT];
	VkRenderPass          render_pass;
	VkFramebuffer         frame_buffers[MAX_SWAP_CHAIN_IMAGES];
	VkImage               depth_image;
	VkDeviceMemory        depth_memory;
	VkImageView           depth_view;
    VkDescriptorPool      descriptor_pool;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorSet       descriptor_set;
	VkPipelineLayout      pipeline_layout;
	VkPipeline            pipeline;
    uint64_t              frame_count; // How many frames have we rendered?
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
    VkDeviceMemory  memories[__PARTITION_COUNT__];
    VkBuffer        buffers[__PARTITION_COUNT__];
    void*           mapped_memory[__PARTITION_COUNT__];
    uint32_t        partition_sizes[__PARTITION_COUNT__] = {};
    bool            memory_available = false;

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
