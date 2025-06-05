#define LAVACAKE_WINDOW_MANAGER_GLFW
#include <LavaCake/Framework/Framework.h> 
#include <LavaCake/Helpers/ABBox.h> 

#include "utils/trackball.h"
#include "utils/Tetmesh.h"
#include "utils/betterGui.h"

using namespace LavaCake;
using namespace LavaCake::Geometry;
using namespace LavaCake::Framework;
#include <chrono>

std::string root = PROJECT_ROOT;

int main() {
	vec2u res = vec2u({1920,1080});

	// On initialise la fenetre et tout les bidules
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "Real-time rendering of animated meshless representations", nullptr, nullptr);
	glfwSetScrollCallback(window, scroll_callback);
	ErrorCheck::printError(true, 5);
	
	GLFWSurfaceInitialisator surfaceInitialisator(window);

	Device* device = Device::getDevice();
	device->enableMeshShader();
	VkPhysicalDeviceFeatures feature;
	device->initDevices(1, 1, surfaceInitialisator);

	SwapChain* swapChain = SwapChain::getSwapChain();
	swapChain->init();

	CommandBuffer  commandBuffer;
	std::shared_ptr<Semaphore> semaphore = std::make_shared<Semaphore>();

	GraphicQueue graphicQueue = device->getGraphicQueue(0);
	PresentationQueue presentQueue = device->getPresentQueue();

	VkExtent2D size = swapChain->size();
	Trackball trackball(vec3f({-1.2f,1.9,1.0f}),vec3f({0.0f,0.0f,.2f}),vec3f({0.0f,0.0f,-1.0f}));
	trackball.m_moveSpeed = 0.02f;
	CustomGui myGui;

	float freq = 10.0f;
	trackball.m_rotationSpeed = 3.14159265f*2.0 / freq;
	
	auto view = trackball.getview();


	// L'uniform buffer pour gere la camera dans nos shaders
	UniformBuffer viewproj;

	auto model = Identity();
	model = PrepareRotationMatrix(90,vec3f({1,0,0})) * PrepareRotationMatrix(180,vec3f({0,0,1}));
	viewproj.addVariable("model", transpose(model));
	viewproj.addVariable("view", transpose(view));

	float near = 0.1f;
	auto perspective = PreparePerspectiveProjectionMatrix(float(size.width)/float(size.height),40.0f,near,100.0f);
  	perspective = transpose(perspective);
  	viewproj.addVariable("perpective", perspective );
	viewproj.addVariable("near", near );
	viewproj.end();

	
	// On définit ici tous les buffer utile à notre programme
	int maxTetNumber = 1000000;

	std::vector<int> nbTet(3); 			            // Un micro storage buffer qui permet de récupérer coté CPU des données écrtie depuis le GPU
	std::vector<vec4u> tempindex(maxTetNumber*8);
	std::vector<vec4f> vertt(4*maxTetNumber);		// Vertex buffer des tetraedres englobant générés
	std::vector<vec4u> indd(maxTetNumber);		    // Index buffer pouer les tetraedres englobant générés
	std::vector<float> attributs((3+9)*4*maxTetNumber);		// Un buffer qui stock par vertex le vecteur déformation

	Buffer vertexBuffer(graphicQueue, commandBuffer, vertt, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_FORMAT_UNDEFINED);
	Buffer deformBuffer(graphicQueue, commandBuffer, attributs, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_FORMAT_UNDEFINED);
	Buffer indicesBuffer(graphicQueue, commandBuffer, indd, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_FORMAT_UNDEFINED);
	Buffer mystorageBuffer(graphicQueue, commandBuffer, nbTet, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT|VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT|VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_FORMAT_UNDEFINED,VK_ACCESS_SHADER_WRITE_BIT);
	Buffer tempindicesBuffer(graphicQueue, commandBuffer, tempindex, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_FORMAT_UNDEFINED);

	DescriptorSet set;
	set.addUniformBuffer(viewproj, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);  
	set.addBuffer(mystorageBuffer, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT, 1);  
	set.addBuffer(vertexBuffer, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT, 2);									   
	set.addBuffer(indicesBuffer, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT , 3);								   
	set.addBuffer(deformBuffer, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT , 4);								  
	set.addUniformBuffer(*myGui.guiBuffer, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 5);
	set.addBuffer(tempindicesBuffer, VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT, 6);
	set.compile();


	//On compute les pipelines
	ComputePipeline coarsePipeline;
	ComputeShaderModule coarseShader(root+"/shaders/0_coarse/uniformQuadSurface.comp.spv");
	coarsePipeline.setComputeModule(coarseShader);
	coarsePipeline.setDescriptorLayout(set.getLayout());
  	coarsePipeline.compile();

	ComputePipeline spacePipeline;
	ComputeShaderModule spaceShader(root+"/shaders/1_spaceDeformation/globalImplicite.comp.spv");
	spacePipeline.setComputeModule(spaceShader);
	spacePipeline.setDescriptorLayout(set.getLayout());
  	spacePipeline.compile();

	ComputePipeline clipTetPipeline;
	ComputeShaderModule clipTetShader(root+"/shaders/2_intervalShading/clipTet.comp.spv");
	clipTetPipeline.setComputeModule(clipTetShader);
	clipTetPipeline.setDescriptorLayout(set.getLayout());
  	clipTetPipeline.compile();

	

	GraphicPipeline graphicPipeline(
		vec3f({ 0,0,0 }), 
		vec3f({ float(res[0]),float(res[1]),1.0f }), 
		vec2f({ 0,0 }), 
		vec2f({ float(res[0]),float(res[1]) })
	);

	MeshShaderModule meshShader(root+"/shaders/2_intervalShading/interval_noclip.mesh.spv");

//	FragmentShaderModule fragmentShaderEmpty(root+"/shaders/optrta/interval_EMPTY.frag.spv");
	FragmentShaderModule fragmentShader(root+"/shaders/3_fragment/surface.frag.spv");

	graphicPipeline.setMeshModule(meshShader);
	graphicPipeline.setFragmentModule(fragmentShader);
	graphicPipeline.setDescriptorLayout(set.getLayout());
	//graphicPipeline.setPolygonMode(VK_POLYGON_MODE_LINE);
	//graphicPipeline.setLineWidth(4.0f);
	graphicPipeline.setCullMode(VK_CULL_MODE_NONE);
	graphicPipeline.setVerticesInfo({},{},VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);


	RenderPass renderPass;

	SubPassAttachments SA;
	SA.setDepthFormat(VK_FORMAT_D16_UNORM);
	SA.addColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT);
	SA.m_attachments[0].m_blendState = {
		VK_FALSE,                  					 // VkBool32                 blendEnable
		VK_BLEND_FACTOR_ONE,                      		// VkBlendFactor            srcColorBlendFactor
		VK_BLEND_FACTOR_ONE,            			    // VkBlendFactor            dstColorBlendFactor
		VK_BLEND_OP_ADD,                                // VkBlendOp                colorBlendOp
		VK_BLEND_FACTOR_ONE,           					// VkBlendFactor            srcAlphaBlendFactor
		VK_BLEND_FACTOR_ONE,                            // VkBlendFactor            dstAlphaBlendFactor
		VK_BLEND_OP_MIN,                                // VkBlendOp                alphaBlendOp
		VK_COLOR_COMPONENT_R_BIT |                      // VkColorComponentFlags    colorWriteMask
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT
	};

	uint32_t passNumber = renderPass.addSubPass(SA);
	graphicPipeline.setSubPassNumber(passNumber);

	uint64_t start = 0; 
	renderPass.compile();

	graphicPipeline.compile(renderPass.getHandle(),SA);

	FrameBuffer frameBuffer =  FrameBuffer(res[0], res[1]);
	renderPass.prepareOutputFrameBuffer(graphicQueue, commandBuffer, frameBuffer);

	VkImageMemoryBarrier frameMemoryBarrier;
	frameMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	frameMemoryBarrier.pNext = nullptr;
	frameMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	frameMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	frameMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	frameMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	frameMemoryBarrier.srcQueueFamilyIndex = graphicQueue.getIndex();
	frameMemoryBarrier.dstQueueFamilyIndex = graphicQueue.getIndex();
	frameMemoryBarrier.image = frameBuffer.getImage(0)->getHandle();
	frameMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };


    std::shared_ptr<Mesh_t> quad =std::make_shared<IndexedMesh<Geometry::TRIANGLE>>(Geometry::P3UV);
	
	quad->appendVertex({ -1.0,-1.0,0.0,0.0,0.0 });
	quad->appendVertex({ -1.0, 1.0,0.0,0.0,1.0 });
	quad->appendVertex({  1.0, 1.0,0.0,1.0,1.0 });
	quad->appendVertex({  1.0,-1.0,0.0,1.0,0.0 });

	quad->appendIndex(0);
	quad->appendIndex(1);
	quad->appendIndex(2);

	quad->appendIndex(2);
	quad->appendIndex(3);
	quad->appendIndex(0);              
	
	std::shared_ptr<VertexBuffer> quad_vertex_buffer = std::make_shared<VertexBuffer>(graphicQueue, commandBuffer, std::vector<std::shared_ptr<Mesh_t>>({ quad }));


	vec4f color;
	vec4f lightColor;
	color = vec4f({255/255.0,51/255.0,51/255.0f,11.0f});
	vec4f reflection = vec4f({1,1,1,0.25});

	

    DescriptorSet postProcessDescriptor;
	postProcessDescriptor.addFrameBuffer(frameBuffer,VK_SHADER_STAGE_FRAGMENT_BIT,0);
    postProcessDescriptor.addUniformBuffer(viewproj, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);  

	postProcessDescriptor.compile();

    GraphicPipeline postProcessPipeline(
		vec3f({ 0,0,0 }), 
		vec3f({ float(size.width),float(size.height),1.0f }), 
		vec2f({ 0,0 }), 
		vec2f({ float(size.width),float(size.height) })
	);

    VertexShaderModule possProcessVert(root+"/shaders/4_postProcess/default.vert.spv");
	FragmentShaderModule possProcessFrag(root+"/shaders/4_postProcess/0_sdf.frag.spv");
	postProcessPipeline.setVertexModule(possProcessVert);
	postProcessPipeline.setFragmentModule(possProcessFrag);
	postProcessPipeline.setVerticesInfo(quad_vertex_buffer->getBindingDescriptions(), quad_vertex_buffer->getAttributeDescriptions(), quad_vertex_buffer->primitiveTopology());
	postProcessPipeline.setDescriptorLayout(postProcessDescriptor.getLayout());

	SubPassAttachments SA2;
  	SA2.addSwapChainImageAttachment(swapChain->imageFormat());
    SA2.m_attachments[0].m_blendState.blendEnable = VK_TRUE;
 
    RenderPass postProcessPass;

	postProcessPass.addSubPass(SA2);

	postProcessPass.compile();


	postProcessPipeline.compile(postProcessPass.getHandle(),SA2);

	ImGuiWrapper* gui = new ImGuiWrapper(graphicQueue, commandBuffer, vec2i({ (int)size.width ,(int)size.height }), vec2i({ (int)size.width ,(int)size.height }));
    
	prepareInputs(window);

    gui->getPipeline()->setSubPassNumber(0);
    gui->getPipeline()->compile(postProcessPass.getHandle(),SA2);

	FrameBuffer postProcessBuffer(swapChain->size().width, swapChain->size().height);
	postProcessPass.prepareOutputFrameBuffer(graphicQueue, commandBuffer, postProcessBuffer);

	
	commandBuffer.wait();
	commandBuffer.resetFence();

	trackball.rotate(vec2f({freq/2.0f,0.0f}));

	float dt = 0.0;
	float elapsed = freq*0.0f;

	bool rotate = false;
	bool pressed = false;
	bool test = true;

	float avgCost = 0.0;
	
	int* tetnumberptr = (int*)mystorageBuffer.map();

	while (!glfwWindowShouldClose(window)) {
		
		tetnumberptr[0] = nbTet[0];
		tetnumberptr[1] = nbTet[1];
		tetnumberptr[2] = nbTet[2];
		
		start = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		commandBuffer.wait();
		glfwPollEvents();

		{
			ImGuiIO& io = ImGui::GetIO();
			IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

			// Setup display size (every frame to accommodate for window resizing)
			int width, height;
			int display_w, display_h;
			glfwGetWindowSize(window, &width, &height);
			glfwGetFramebufferSize(window, &display_w, &display_h);
			io.DisplaySize = ImVec2((float)width, (float)height);
			if (width > 0 && height > 0)
				io.DisplayFramebufferScale = ImVec2((float)display_w / width, (float)display_h / height);
		}

		myGui.draw();
        gui->prepareGui(graphicQueue, commandBuffer);

		const SwapChainImage& image = swapChain->acquireImage();
		std::vector<waitSemaphoreInfo> waitSemaphoreInfos = {};
		waitSemaphoreInfos.push_back({
			image.getSemaphore(),                           // VkSemaphore            Semaphore
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT	  // VkPipelineStageFlags   WaitingStage
		});

		
		ImGuiIO& io = ImGui::GetIO();

		if(!io.WantCaptureMouse){
			auto translation = get_movement(window);
			trackball.move(translation);

			
			int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		
			auto dmov = get_mouse_mouvement(window);
			if (state == GLFW_PRESS){

				dmov[0] = dmov[0]/float(size.width) *2.0 * 3.14159265;
				dmov[1] = dmov[1]/float(size.width) *2.0 * 3.14159265;
				trackball.rotate(vec2f({float(dmov[0]),float(dmov[1])}));
			}

			
			if(scrolled){
				scrolled = false;
				trackball.zoom(scroll);
			}
		
		}
		
		//time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (! myGui.timeStop) myGui.time += 0.01;

		view = trackball.getview();
    	viewproj.setVariable("view",transpose(view));

		myGui.updateUniform();

		commandBuffer.beginRecord();

		viewproj.update(commandBuffer);
		myGui.guiBuffer->update(commandBuffer);

        postProcessPass.setSwapChainImage(postProcessBuffer, image);
		postProcessPass.begin(commandBuffer, postProcessBuffer, vec2u({ 0,0 }), vec2u({size.width, size.height}), 
		{{ 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0 }});

		postProcessPipeline.bindPipeline(commandBuffer);
		
		postProcessPipeline.bindDescriptorSet(commandBuffer,postProcessDescriptor);
		bindVertexBuffer(commandBuffer, *quad_vertex_buffer->getVertexBuffer());
		bindIndexBuffer(commandBuffer, *quad_vertex_buffer->getIndexBuffer());

		drawIndexed(commandBuffer, quad_vertex_buffer->getIndicesNumber());
		

        gui->drawGui(commandBuffer);
		postProcessPass.end(commandBuffer);


		commandBuffer.endRecord();
		
		commandBuffer.submit(graphicQueue, waitSemaphoreInfos, { semaphore });

		swapChain->presentImage(presentQueue, image, { semaphore });

		commandBuffer.wait();
		commandBuffer.resetFence();
		uint64_t stop = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		
		myGui.cost = float(stop - start)/1000.0f; 
		
		dt = myGui.cost/1000.0f;
		
	}
	device->waitForAllCommands();
}