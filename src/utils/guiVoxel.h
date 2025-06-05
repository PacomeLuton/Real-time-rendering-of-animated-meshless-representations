#pragma once
#include <LavaCake/Framework/Framework.h> 

using namespace LavaCake;
using namespace LavaCake::Geometry;
using namespace LavaCake::Framework;

class CustomGui{
    public:
        float cost;
        float time;
        float smoothcost;
        bool timeStop;

        UniformBuffer *guiBuffer;

        int currentTetNumber = 0;
        int voxelSize;

    public:
        CustomGui() : time(0), smoothcost(0), timeStop(true), cost(0.0f), voxelSize(64){

            guiBuffer = new UniformBuffer();
            guiBuffer->addVariable("time",time);
            guiBuffer->addVariable("voxelSize",voxelSize);
            guiBuffer->end();
        }

        void updateUniform(){
            guiBuffer->setVariable("time", time);
            guiBuffer->setVariable("voxelSize",voxelSize);
        }

        void draw(){
            drawUserControl();
        }

        void drawUserControl(){
            ImGui::NewFrame();
            ImGui::Begin("Controls");
            smoothcost = 0.95*smoothcost+0.05*cost;
		    std::string text = "cost: "+std::to_string(smoothcost) + "ms";
		    ImGui::Text(text.data());
            text = "NbTet: "+std::to_string(currentTetNumber);
            ImGui::Text(text.data());

            ImGui::Checkbox("Stop Time", &timeStop);

            ImGui::InputInt("Voxel grid size", &voxelSize);

		    ImGui::End();
        }

};


vec2d mousepos = vec2d({0,0});

vec2d get_mouse_mouvement(GLFWwindow* window){
  
  vec2d new_mousepos;
  glfwGetCursorPos(window, &new_mousepos[0], &new_mousepos[1]);
  auto delta = new_mousepos - mousepos;
  mousepos = vec2d(new_mousepos.data);
  return delta;
}

double scroll = 0;
bool scrolled = false;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if(yoffset!=0){
    scroll = -yoffset * 10.0f;
    scrolled = true;
  }

}


vec3f get_movement(GLFWwindow* window){
    vec3f translation;
    int state = glfwGetKey(window, GLFW_KEY_UP);
    if (state == GLFW_PRESS)
    {
        translation = translation + vec3f({0.0f,0.0f,-1.0});
    }
    state = glfwGetKey(window, GLFW_KEY_DOWN);
    if (state == GLFW_PRESS)
    {
        translation = translation + vec3f({0.0f,0.0f,1.0});
    }

    state = glfwGetKey(window, GLFW_KEY_LEFT);
    if (state == GLFW_PRESS)
    {
        translation = translation + vec3f({-1.0f,0.0f,0.0});
    }
    state = glfwGetKey(window, GLFW_KEY_RIGHT);
    if (state == GLFW_PRESS)
    {
       translation = translation +  vec3f({1.0f,0.0f,0.0});
    }

    state = glfwGetKey(window, GLFW_KEY_SPACE);
    if (state == GLFW_PRESS)
    {
       translation = translation +  vec3f({0.0f,-1.0f,0.0});
    }

    state = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
    if (state == GLFW_PRESS)
    {
       translation = translation +  vec3f({0.0f,1.0f,0.0});
    }

    return translation;
}