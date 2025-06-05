#pragma once
#include <LavaCake/Framework/Framework.h> 

using namespace LavaCake;
using namespace LavaCake::Geometry;
using namespace LavaCake::Framework;

const char* fragModes[] = {"empty", "transparency", "surface"};
typedef enum{
    FRAG_EMPTY,
    FRAG_TRANSPARENCY,
    FRAG_SURFACE
}FragMode;

const char* meshModes[] = {"empty", "simple", "jacobian", "jacobian Noclip", "memoryTest"};
typedef enum{
    MESH_EMPTY,
    MESH_SIMPLE,
    MESH_JACOBIAN,
    MESH_JACOBIAN_NOCLIP,
    MESH_MEMORYTEST
}MeshMode;


std::pair<FragMode,MeshMode> benchStateList[] = {{FRAG_EMPTY,MESH_EMPTY}, {FRAG_EMPTY,MESH_SIMPLE}, {FRAG_EMPTY,MESH_MEMORYTEST}, {FRAG_EMPTY,MESH_JACOBIAN}, {FRAG_TRANSPARENCY,MESH_SIMPLE}, {FRAG_TRANSPARENCY,MESH_MEMORYTEST}, {FRAG_TRANSPARENCY,MESH_JACOBIAN}};
int benchNList[] = {1,5,10,15,20,25,30,35,40,45,50,55,60,65,70};
int benchSampleSize = 100;

class CustomGui{
    public:
        float cost;
        float time;
        float smoothcost;
        bool timeStop;
        int nbTet[3] = {50,50,50};
        float thetaphi[2] = {0,0};
        int currentTetNumber = 0;

        int fragMode;
        int meshMode;
        
        bool emptyFrag;
        bool fragChanged;

        bool emptyMesh;
        bool meshChanged;

        bool noclip;

        float epaisseur = 0.2;

        UniformBuffer *guiBuffer;

        bool benchmark;
        int benchmarkCounter; // nb de frame au sein d'un test
        int benchmarkState; // numero du test
        int benchmarkN; // numero du test par rapport au nombre de N
        float benchmarkTime; // temps courant du test

    public:
        CustomGui() : time(0), smoothcost(0), timeStop(true), cost(0.0f), fragMode(FRAG_TRANSPARENCY), meshMode(MESH_JACOBIAN), emptyMesh(false), emptyFrag(false), benchmark(false), noclip(false){

            guiBuffer = new UniformBuffer();
            guiBuffer->addVariable("time",time);
            guiBuffer->addVariable("thetaPhi",thetaphi);
            guiBuffer->addVariable("epaisseur",epaisseur);
            guiBuffer->end();
        }

        void updateUniform(){
            guiBuffer->setVariable("time", time);
            guiBuffer->setVariable("thetaPhi", thetaphi);
            guiBuffer->setVariable("epaisseur",epaisseur);
        }

        void draw(){
            drawUserControl();
        }

        void drawUserControl(){
            ImGui::NewFrame();
            ImGui::Begin("Controls");
            smoothcost = 0.8*smoothcost+0.2*cost;
		    std::string text = "cost: "+std::to_string(smoothcost) + "ms";
		    //ImGui::Text(text.data());
            text = "NbTet: "+std::to_string(currentTetNumber);
            ImGui::InputInt3("Initial grid size", nbTet);
            //ImGui::SliderFloat("Epaisseur", &epaisseur,-0.5f,0.5f);
		    ImGui::Text(text.data());
            ImGui::Checkbox("Stop Time", &timeStop);
            //ImGui::Checkbox("No cliping shader", &noclip);
            //fragChanged = ImGui::Checkbox("EmptyFrag Shader", &emptyFrag);
            //fragChanged = fragChanged || ImGui::Combo("Mode Fragment Shader", &fragMode, fragModes, IM_ARRAYSIZE(fragModes));
            //meshChanged = ImGui::Checkbox("EmptyMesh Shader", &emptyMesh);
            //meshChanged = meshChanged || ImGui::Combo("Mode Mesh Shader", &meshMode, meshModes, IM_ARRAYSIZE(meshModes));

            //ImGui::SliderFloat2("Light Direction", thetaphi,-M_PI,M_PI);

            //ImGui::Checkbox("Benchmark", &benchmark);
		    ImGui::End();

            if (benchmark){
                benchmarkCounter = -1;
                benchmarkState = 0;
                benchmarkN = 0;
                benchmarkTime = 0;
                time = 0;
                timeStop = false;

                fragMode = benchStateList[benchmarkState].first;
                meshMode = benchStateList[benchmarkState].second;
                fragChanged = true; meshChanged = true;
            }
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