#define LAVACAKE_WINDOW_MANAGER_GLFW
#include <LavaCake/Framework/Framework.h> 
#include <LavaCake/Helpers/ABBox.h> 

#define MC_IMPLEM_ENABLE
#include "MC.h"

using namespace LavaCake;
using namespace LavaCake::Geometry;
using namespace LavaCake::Framework;
#include <iostream>
#include <fstream>
#include <chrono>

std::string root = PROJECT_ROOT;

int main() {
    Device* device = Device::getDevice();
	VkPhysicalDeviceFeatures feature;
	device->initDevices(1, 1, nullptr);
	
	CommandBuffer commandBuffer;
	std::shared_ptr<Semaphore> semaphore = std::make_shared<Semaphore>();

	GraphicQueue graphicQueue = device->getGraphicQueue(0);
	PresentationQueue presentQueue = device->getPresentQueue();

    int N = 64;
	std::vector<float> dist(N*N*N);

	Buffer mystorageBuffer(graphicQueue, commandBuffer, dist, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT|VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_FORMAT_UNDEFINED,VK_ACCESS_SHADER_WRITE_BIT);

	DescriptorSet set;
	set.addBuffer(mystorageBuffer, VK_SHADER_STAGE_COMPUTE_BIT, 0);  
	set.compile();

	ComputePipeline I2VPipeline;
	ComputeShaderModule I2Vshader(root+"/shaders/9_miscellaneous/implicite2voxel.comp.spv");
	I2VPipeline.setComputeModule(I2Vshader);
	I2VPipeline.setDescriptorLayout(set.getLayout());
  	I2VPipeline.compile();
	
	commandBuffer.wait();
	commandBuffer.resetFence();

	commandBuffer.beginRecord();

	I2VPipeline.bindPipeline(commandBuffer);
	I2VPipeline.bindDescriptorSet(commandBuffer,set);
	I2VPipeline.compute(commandBuffer, N,N,N);

	commandBuffer.endRecord();
	commandBuffer.submit(graphicQueue, {}, {});
	commandBuffer.wait();
	commandBuffer.resetFence();

	std::cout << "SHADER FINISHED" << std::endl;

    float* buffer = (float*)mystorageBuffer.map();

	MC::mcMesh mesh;
	MC::marching_cube(buffer, N, N, N, mesh);

	std::cout << "MC FINISHED" << std::endl;

	// Export the result as an .obj file
	std::ofstream out;
	out.open(root+"objects/test.obj");
	if (out.is_open() == false)
		return 0;
	out << "g " << "Obj" << std::endl;
	for (size_t i = 0; i < mesh.vertices.size(); i++)
		out << "v " << mesh.vertices.at(i).x / (N-1) * 2 - 1 << " " << mesh.vertices.at(i).y / (N-1) * 2 - 1 << " " << mesh.vertices.at(i).z / (N-1) *2 -1 << '\n';
	for (size_t i = 0; i < mesh.vertices.size(); i++)
		out << "vn " << mesh.normals.at(i).x << " " << mesh.normals.at(i).y << " " << mesh.normals.at(i).z << '\n';
	for (size_t i = 0; i < mesh.indices.size(); i += 3)
	{
		out << "f " << mesh.indices.at(i) + 1 << "//" << mesh.indices.at(i) + 1
			<< " " << mesh.indices.at(i + 1) + 1 << "//" << mesh.indices.at(i + 1) + 1
			<< " " << mesh.indices.at(i + 2) + 1 << "//" << mesh.indices.at(i + 2) + 1
			<< '\n';
	}
	out.close();

	device->waitForAllCommands();
}