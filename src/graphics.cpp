#include "graphics.h"

namespace shader {
#include "shaders/main.slang.h"
}

// References:
//   - https://vulkan-tutorial.com/
//   - https://docs.vulkan.org/

struct PartitionInfo {
	int partition;
	u32 binding;
	u32 usage;
};

PartitionInfo partition_infos[] = {
	{ .partition =  Graphics::SCENE_DATA,     .binding = 0,          .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT },
	{ .partition =  Graphics::VERTEX_BUFFER,  .binding = 1,          .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT },
	{ .partition =  Graphics::INDEX_BUFFER,   .binding = UINT32_MAX, .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT   },
};

VkBufferUsageFlags partition_buffer_usage(int partition) {
	for (auto &info: partition_infos) {
		if (info.partition == partition)
			return info.usage;
	}
	ERROR("Must define buffer usage for graphics memory partition!");
	exit(1);
}

uint32_t partition_buffer_binding(int partition) {
	for (auto &info: partition_infos) {
		if (info.partition == partition)
			return info.binding;
	}
	ERROR("Must define binding index for buffer partition!");
	exit(1);
}

VkDescriptorType descriptor_type_from_buffer_usage(VkFlags usage) {
	switch (usage) {
		case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}
	ERROR("No descriptor type for this buffer usage!");
	exit(1);
}

Result Graphics::create_instance(bool enable_validation) {
	VkApplicationInfo application_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_4,
	};

	const char* extension_names[] = {
	#if defined(__APPLE__)
		VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
	#endif
		VK_KHR_SURFACE_EXTENSION_NAME,
		RGFW_VK_SURFACE,
	};

	const char* layer_names[] = {
		"VK_LAYER_KHRONOS_validation",
	};
		
	VkInstanceCreateInfo instance_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	#if defined(__APPLE__)
		.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
	#endif
		.pApplicationInfo = &application_info,
		.enabledLayerCount = enable_validation ? 1u : 0u,
		.ppEnabledLayerNames = layer_names,
		.enabledExtensionCount = (uint32_t)std::size(extension_names),
		.ppEnabledExtensionNames =  extension_names,
	};

#if defined(__APPLE__)
	// This is a hack so that Vulkan knows what driver to load (MoltenVK)
	setenv("VK_ICD_FILENAMES", "./lib/macos/MoltenVK_icd.json", 1);
#endif

	VkResult result = vkCreateInstance(&instance_info, 0, &instance);
	if (result == VK_ERROR_LAYER_NOT_PRESENT) {
		return ERROR("Layer not present!");
	} else if (result != VK_SUCCESS) {
		return ERROR("Failed to create Vulkan instance! (%i)", (int)(result));
	}
	return Success;
}

Result Graphics::pick_physical_device() {
	uint32_t count = 1; // Just pick the first device
	if (vkEnumeratePhysicalDevices(instance, &count, &physical_device) < 0)
		return ERROR("Failed to enumerate physical devices!");
	return Success;
}

Result Graphics::create_device(bool enable_validation) {
	const char* device_extensions[] = {
	#if defined(__APPLE__)
		"VK_KHR_portability_subset",
	#endif
		"VK_KHR_swapchain",
		"VK_KHR_shader_draw_parameters",
	};

	const char* layer_names[] = { "VK_LAYER_KHRONOS_validation" };

	float priority = 1.0f;
	VkDeviceQueueCreateInfo queue_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueCount = 1,
		.pQueuePriorities = &priority,
	};

	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue_info,
		.enabledLayerCount = (u32)(enable_validation ? (uint32_t)std::size(layer_names) : 0),
		.ppEnabledLayerNames = layer_names,
		.enabledExtensionCount = (uint32_t)std::size(device_extensions),
		.ppEnabledExtensionNames = device_extensions,
	};
	
	if (vkCreateDevice(physical_device, &device_info, 0, &device) != VK_SUCCESS)
		return ERROR("Failed to create logical device!");

	vkGetDeviceQueue(device, 0, 0, &queue);
	return Success;
}

Result Graphics::create_command_buffers() {
	VkCommandPoolCreateInfo graphics_pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = 0,
	};
	if (vkCreateCommandPool(device, &graphics_pool_info, 0, &command_pool) != VK_SUCCESS)
		return Failed;

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = (uint32_t)std::size(command_buffers),
	};
	if (vkAllocateCommandBuffers(device, &allocate_info, command_buffers) != VK_SUCCESS)
		return Failed;

	VkSemaphoreCreateInfo semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};
	for (uint32_t i = 0; i < MAX_SWAP_CHAIN_IMAGES; ++i)
	{
		assert(vkCreateSemaphore(device, &semaphore_info, 0, &render_finished_semaphores[i]) == VK_SUCCESS);
	}

	VkFenceCreateInfo fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		assert(vkCreateSemaphore(device, &semaphore_info, 0, &image_acquired_semaphores[i]) == VK_SUCCESS);
		assert(vkCreateFence(device, &fence_info, 0, &fences[i]) == VK_SUCCESS);
	}

	return Success;
}

Result Graphics::create_depth_buffer() {
	VkFormat depth_format = VK_FORMAT_D32_SFLOAT;
	
	VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = depth_format,
		.extent = {
			.width = image_extent.width,
			.height = image_extent.height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	if (vkCreateImage(device, &image_info, 0, &depth_image) != VK_SUCCESS)
		return Failed;

	VkMemoryRequirements memory_requirements = {};
	vkGetImageMemoryRequirements(device, depth_image, &memory_requirements);

	VkMemoryAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memory_requirements.size,
		.memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	};
	if (vkAllocateMemory(device, &allocate_info, 0, &depth_memory) != VK_SUCCESS)
		return Failed;

	if (vkBindImageMemory(device, depth_image, depth_memory, 0) != VK_SUCCESS)
		return Failed;

	VkImageViewCreateInfo view_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.flags = 0,
		.image = depth_image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = depth_format,
		.components = {}, // Identity
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	if (vkCreateImageView(device, &view_info, 0, &depth_view) != VK_SUCCESS)
		return Failed;

	return Success;
}

Result Graphics::create_frame_buffers() {
	VkImageView attachments[] = {
		0, depth_view,
	};
	VkFramebufferCreateInfo frame_buffer_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = render_pass,
		.attachmentCount = (uint32_t)std::size(attachments),
		.pAttachments = attachments,
		.width = image_extent.width,
		.height = image_extent.height,
		.layers = 1,
	};
	for (uint32_t i = 0; i < image_count; ++i)
	{
		attachments[0] = image_views[i];
		if (vkCreateFramebuffer(device, &frame_buffer_info, 0, &frame_buffers[i]) != VK_SUCCESS)
			return ERROR("Failed to create frame buffer!");
	}
	return Success;
}

Result Graphics::create_layouts() {
	VkDescriptorPoolSize pool_sizes[] = {
		{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         .descriptorCount = 16,  },
		{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         .descriptorCount = 16,  },
		{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 16,  },
	};
	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 1,
		.poolSizeCount = (uint32_t)std::size(pool_sizes),
		.pPoolSizes = pool_sizes,
	};
	if (vkCreateDescriptorPool(device, &pool_info, 0, &descriptor_pool) != VK_SUCCESS)
		return Failed;

	uint32_t binding_count = 0;
	VkDescriptorSetLayoutBinding bindings[__PARTITION_COUNT__];
	for (int i = 0; i < __PARTITION_COUNT__; ++i) {
		if (partition_infos[i].binding == UINT32_MAX)
			continue;
		bindings[binding_count++] = {
			.binding = partition_infos[i].binding,
			.descriptorType = descriptor_type_from_buffer_usage(partition_infos[i].usage),
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
		};
	}
	VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = binding_count,
		.pBindings = bindings,
	};
	if (vkCreateDescriptorSetLayout(device, &layout_info, 0, &descriptor_set_layout) != VK_SUCCESS)
		return Failed;

	VkDescriptorSetAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptor_set_layout,
	};
	if (vkAllocateDescriptorSets(device, &allocate_info, &descriptor_set) != VK_SUCCESS)
		return Failed;

	VkPushConstantRange push_constant_range = {
		.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
		.offset = 0,
		.size = sizeof(shader::PerDraw),
	};
	VkPipelineLayoutCreateInfo pipeline_layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptor_set_layout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range,
	};
	if (vkCreatePipelineLayout(device, &pipeline_layout_info, 0, &pipeline_layout) != VK_SUCCESS)
		return Failed;

	return Success;
}

Result Graphics::create_render_pipeline() {
	VkShaderModule shader = 0;
	VkShaderModuleCreateInfo shader_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shader::data_sizeInBytes,
		.pCode = (uint32_t*)(shader::data),
	};
	if (vkCreateShaderModule(device, &shader_info, 0, &shader) != VK_SUCCESS)
		return Failed;

	VkPipelineShaderStageCreateInfo shader_stages[] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = shader,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = shader,
			.pName = "main",
		},
	};
	VkPipelineVertexInputStateCreateInfo vertex_input_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};
	VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};
	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};
	VkPipelineRasterizationStateCreateInfo rasterization_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f,
	};
	VkPipelineMultisampleStateCreateInfo multisample_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
	};
	VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};
	VkPipelineColorBlendAttachmentState blend_attachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	VkPipelineColorBlendStateCreateInfo color_blend_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1,
		.pAttachments = &blend_attachment,
	};
	VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = (uint32_t)std::size(dynamic_states),
		.pDynamicStates = dynamic_states,
	};
	VkGraphicsPipelineCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount          = (uint32_t)std::size(shader_stages),
		.pStages             = shader_stages,
		.pVertexInputState   = &vertex_input_state,
		.pInputAssemblyState = &input_assembly_state,
		.pViewportState      = &viewport_state,
		.pRasterizationState = &rasterization_state,
		.pMultisampleState   = &multisample_state,
		.pDepthStencilState  = &depth_stencil_state,
		.pColorBlendState    = &color_blend_state,
		.pDynamicState       = &dynamic_state,
		.layout              = pipeline_layout,
		.renderPass          = render_pass,
		.subpass             = 0,
	};
	if (vkCreateGraphicsPipelines(device, 0, 1, &info, 0, &pipeline) != VK_SUCCESS)
		return Failed;

	vkDestroyShaderModule(device, shader, 0);
	return Success;
}

uint32_t Graphics::find_memory_type(uint32_t mask, VkFlags required_properties) {
	VkPhysicalDeviceMemoryProperties properties = {};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);
	uint32_t memory_type = VK_MAX_MEMORY_TYPES;
	for (uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
		if (!(mask & (1 << i)))
			continue; // Skip memory type if we don't support it in our mask
		if ((properties.memoryTypes[i].propertyFlags & required_properties) != required_properties)
			continue; // Also skip memory type if it doesn't have the properties we want
		memory_type = i;
	}
	return memory_type;
}

Result Graphics::setup(bool enable_validation) {
	if (create_instance(enable_validation)) return Failed;
	if (pick_physical_device()) return Failed;
	if (create_device(enable_validation)) return Failed;
	if (create_command_buffers()) return Failed;
	if (create_layouts()) return Failed;
	allocate_buffer(SCENE_DATA, sizeof(shader::Scene));
	return Success;
}

Result Graphics::attach(RGFW_window *_window) {
	window = _window; // We use this now!

	if (RGFW_window_createSurface_Vulkan(window, instance, &surface) != VK_SUCCESS)
		return ERROR("Failed to create Vulkan surface!");

	VkSurfaceCapabilitiesKHR capabilities;
	assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities) == VK_SUCCESS);
	image_extent = capabilities.currentExtent;
	assert(image_extent.width != 0 || image_extent.height != 0);

	VkSurfaceFormatKHR surface_format;
	uint32_t count = 1;
	// NOTE: We only care if this function doesn't fail (return code >= 0)
	assert(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, &surface_format) >= 0);
	format = surface_format.format;
	
	VkSwapchainCreateInfoKHR swap_chain_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = capabilities.minImageCount + 1,
		.imageFormat = surface_format.format,
		.imageColorSpace = surface_format.colorSpace,
		.imageExtent = capabilities.currentExtent,
		.imageArrayLayers = 1, /* For non-stereoscopic-3D applications (non-VR), this value is 1 */
		.imageUsage = capabilities.supportedUsageFlags,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.clipped = VK_TRUE, /* "... allows more efficient presentation methods to be used on some platforms." */
	};

	if (vkCreateSwapchainKHR(device, &swap_chain_info, 0, &swap_chain) != VK_SUCCESS)
		return ERROR("Failed to create swap chain!");

	assert(vkGetSwapchainImagesKHR(device, swap_chain, &image_count, 0) == VK_SUCCESS);
	assert(image_count > 0);
	assert(image_count <= MAX_SWAP_CHAIN_IMAGES);

	if (vkGetSwapchainImagesKHR(device, swap_chain, &image_count, images) != VK_SUCCESS)
		return ERROR("Failed to get swap chain images!");

	for (uint32_t i = 0; i < image_count; ++i)
	{
		VkImageViewCreateInfo view_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.flags = 0,
			.image = images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.components = {}, // Identity
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
		if (vkCreateImageView(device, &view_info, 0, image_views + i) != VK_SUCCESS)
			return ERROR("Failed to create view on swap chain image!");
	}

	VkAttachmentDescription attachments[] = {
		// Color Attachment
		{
			.format = format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		},
		// Depth Attachment
		{
			.format = VK_FORMAT_D32_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		}
	};
	VkAttachmentReference color_attachment_ref = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentReference depth_attachment_ref = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref,
		.pDepthStencilAttachment = &depth_attachment_ref,
	};
	VkSubpassDependency subpass_dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};
	VkRenderPassCreateInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = (uint32_t)std::size(attachments),
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &subpass_dependency,
	};
	if (vkCreateRenderPass(device, &render_pass_info, 0, &render_pass) != VK_SUCCESS)
		return ERROR("Failed to create render pass!");

	if (create_depth_buffer()) return Failed;
	if (create_frame_buffers()) return Failed;
	if (create_render_pipeline()) return Failed;
	return Success;
}

Result Graphics::prepare_frame() {
	current_frame.frame_index = (uint32_t)(frame_count % MAX_FRAMES_IN_FLIGHT);
	VkFence fence = fences[current_frame.frame_index];
	VkSemaphore image_acquired_semaphore = image_acquired_semaphores[current_frame.frame_index];

	if (vkWaitForFences(device, 1, &fence, true, UINT64_MAX) != VK_SUCCESS)
		return ERROR("vkWaitForFences failed!");

	assert(vkResetFences(device, 1, &fence) == VK_SUCCESS);

	VkResult result = VK_ERROR_UNKNOWN;
	while (result != VK_SUCCESS) {
		result = vkAcquireNextImageKHR(device, swap_chain,
			UINT64_MAX, image_acquired_semaphore, 0, &current_frame.image_index);

		if (result == VK_SUBOPTIMAL_KHR) {
			break; // eh, thats fine.
		}
		else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		//	resize(); // ok, we actually need to do something here..
			continue;
		}
		else if (result != VK_SUCCESS) {
			return ERROR("Failed to get swap chain image! (%i)\n", result);
		}
	}

	current_frame.command_buffer = command_buffers[current_frame.frame_index];
	current_frame.frame_buffer = frame_buffers[current_frame.image_index];

	VkCommandBufferBeginInfo cmd_begin = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	if (vkBeginCommandBuffer(current_frame.command_buffer, &cmd_begin) != VK_SUCCESS)
		return Failed;

	VkClearValue clear_values[] = {
		{.color = {{0.0f, 0.0f, 0.0f, 0.0f}}},
		{.depthStencil = {1.0f, 0}},
	};
	VkRenderPassBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_pass,
		.framebuffer = current_frame.frame_buffer,
		.renderArea = { {0, 0}, image_extent },
		.clearValueCount = (uint32_t)std::size(clear_values),
		.pClearValues = clear_values,
	};
	vkCmdBeginRenderPass(current_frame.command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	VkViewport viewport = {
		.width = (float)(image_extent.width),
		.height = (float)(image_extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(current_frame.command_buffer, 0, 1, &viewport);
	VkRect2D scissor = {
		.extent = image_extent,
	};
	vkCmdSetScissor(current_frame.command_buffer, 0, 1, &scissor);
	vkCmdBindPipeline(current_frame.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindIndexBuffer(current_frame.command_buffer, buffers[INDEX_BUFFER], 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(current_frame.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, 0);
	return Success;
}

Result Graphics::submit_frame() {
	memcpy(mapped_memory[SCENE_DATA], &scene_data, sizeof(scene_data));
	flush_partition(SCENE_DATA, 0, VK_WHOLE_SIZE);

	vkCmdEndRenderPass(current_frame.command_buffer);

	if (vkEndCommandBuffer(current_frame.command_buffer) != VK_SUCCESS)
		return Failed;

	VkFence fence = fences[current_frame.frame_index];
	VkPipelineStageFlags wait_mask = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &image_acquired_semaphores[current_frame.frame_index],
		.pWaitDstStageMask = &wait_mask,
		.commandBufferCount = 1,
		.pCommandBuffers = &current_frame.command_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &render_finished_semaphores[current_frame.image_index],
	};
	if (vkQueueSubmit(queue, 1, &submit_info, fence) != VK_SUCCESS)
		return Failed;

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &render_finished_semaphores[current_frame.image_index],
		.swapchainCount = 1,
		.pSwapchains = &swap_chain,
		.pImageIndices = &current_frame.image_index,
	};
	if (vkQueuePresentKHR(queue, &present_info) != VK_SUCCESS)
		return Failed;

	frame_count += 1;
	return Success;
}

uint32_t Graphics::allocate_buffer(Partition partition, u64 num_bytes) {
	if (memory_available) {
		ERROR("GPU memory already available, too late to allocate buffers!");
		exit(1);
	}
	uint32_t offset = partition_sizes[partition];
	partition_sizes[partition] += (u32)(num_bytes);
	return offset;
}

Result Graphics::allocate_required_memory() {
	uint32_t total_bytes_allocated = 0;
	for (int partition = 0; partition < __PARTITION_COUNT__; ++partition) {
		auto size = partition_sizes[partition];
		auto usage = partition_buffer_usage(partition);

		VkBufferCreateInfo buffer_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size  = size,
			.usage = usage,
		};
		if (vkCreateBuffer(device, &buffer_info, 0, &buffers[partition]) != VK_SUCCESS)
			return Failed;

		VkBuffer buffer = buffers[partition];
		VkMemoryRequirements memory_requirements = {};
		vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);

		//                     (Memory lives on GPU)                 (Able to be memory-mapped)
		VkFlags memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		VkMemoryAllocateInfo allocate_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memory_requirements.size,
			.memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits, memory_flags),
		};
		if (vkAllocateMemory(device, &allocate_info, 0, &memories[partition]) != VK_SUCCESS)
			return ERROR("Failed to allocate GPU memory!");

		if (vkBindBufferMemory(device, buffer, memories[partition], 0) != VK_SUCCESS)
			return Failed;
		
		if (vkMapMemory(device, memories[partition], 0, VK_WHOLE_SIZE, 0, &mapped_memory[partition]) != VK_SUCCESS)
			return Failed;

		uint32_t binding = partition_buffer_binding(partition);
		if (binding == UINT32_MAX)
			continue;

		VkDescriptorBufferInfo descriptor_buffer_info = {
			.buffer = buffer,
			.offset = 0,
			.range = size,
		};
		VkWriteDescriptorSet write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptor_set,
			.dstBinding = binding,
			.descriptorCount = 1,
			.descriptorType = descriptor_type_from_buffer_usage(usage),
			.pBufferInfo = &descriptor_buffer_info,
		};
		vkUpdateDescriptorSets(device, 1, &write, 0, 0);
		total_bytes_allocated += size;
	}
	printf("Info: Allocated %.3f MB of GPU memory.\n", (float)(total_bytes_allocated) / (1024.0f * 1024.0f));
	memory_available = true;
	return Success;
}

void Graphics::write_vertex_buffer(u32 offset, void *data, u64 num_bytes) {
	assert(memory_available && "GPU memory is not available yet!");
	memcpy((u8*)(mapped_memory[VERTEX_BUFFER]) + offset, data, num_bytes);
}

void Graphics::write_index_buffer(u32 offset, void *data, u64 num_bytes) {
	assert(memory_available && "GPU memory is not available yet!");
	memcpy((u8*)(mapped_memory[INDEX_BUFFER]) + offset, data, num_bytes);
}

void Graphics::flush_partition(Partition partition, u32 offset, u64 size) {
	VkMappedMemoryRange memory_range = {
		.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		.memory = memories[partition],
		.offset = offset,
		.size = size,
	};
	assert(vkFlushMappedMemoryRanges(device, 1, &memory_range) == VK_SUCCESS);
}
