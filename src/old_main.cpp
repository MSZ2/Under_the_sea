#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#define NUM_FISH 7

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(char const * path);

bool diveMaskActive = false;
int k = 1; // trebace za pomeraj
bool napred = true;
bool fishJump = false;
glm::vec3 pomeraj= glm::vec3 (k*-0.22198,0,k*0.975150);

//glm::vec4 angledDirection;


//random fish
float rx[NUM_FISH];
float ry[NUM_FISH];
float rz[NUM_FISH];
float fishAngle = 0;//pokret ribe
int  fishAngleDirection = 1;
// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; // 0.0f
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;

    glm::vec3 uglyFishPosition = glm::vec3(70.f, 60, -250);
    float uglyFishScale = 2.0f;

    glm::vec3 uglyRockPosition = glm::vec3(0, -630, 0);
    float uglyRockScale = 100.0f;


    glm::vec3 sandPosition = glm::vec3(305.0f,5,315);
    float sandScale = 500.0f;

    glm::vec3 submarinePosition = glm::vec3(300.0f, 1470.f, 600);
    float submarineScale = 10.0f;

    glm::vec3 rocksPosition = glm::vec3(0, 25, 25);
    float rocksScale = 100.0f;

    glm::vec3 blueFishPosition = glm::vec3(360, 300, 260);
    float blueFishScale = 100.0f;







    PointLight pointLight;
    DirLight dirLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    std::srand(56);
    for(int i= 0; i<NUM_FISH; i++){
        rx[i] = -100 + std::rand()%200;
        ry[i]= -15 + std::rand()%30;
        rz[i]= 200 + std::rand()%400;
    }

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs","resources/shaders/skybox.fs");
    Shader cameraShader("resources/shaders/cameraView.vs", "resources/shaders/cameraView.fs");

    // load models
    // -----------
    Model uglyFishModel("resources/objects/Giant_Monster_Fish-OBJ/Giant Monster Fish-OBJ/Giant Monster Fish.obj");
    uglyFishModel.SetShaderTextureNamePrefix("material.");

    stbi_set_flip_vertically_on_load(false);
    Model uglyRockModel("resources/objects/spinsters-rock/model.obj");
    uglyRockModel.SetShaderTextureNamePrefix("material.");

    Model sandModel("resources/objects/sand/uploads_files_3835558_desert.obj");
    sandModel.SetShaderTextureNamePrefix("material.");
    //stbi_set_flip_vertically_on_load(false);
    Model submarineModel("resources/objects/submarine/submarine.obj");
    submarineModel.SetShaderTextureNamePrefix("material.");

    Model rocksModel("resources/objects/RockSet/RockSet.obj");
    rocksModel.SetShaderTextureNamePrefix("material.");

    Model blueFishModel("resources/objects/Blue_fish/Coral_Beauty_Angelfish_v1_L3.123ca9f8106d-73bb-4428-9416-4a734455026d/13009_Coral_Beauty_Angelfish_v1_l3.obj");
    blueFishModel.SetShaderTextureNamePrefix("material.");

    //fish Model
   std::vector<Model> fishModels;
    for (int i= 1; i<10; i++){
        string path = "resources/objects/tropical_fish/TropicalFish";// +num + obj
        Model tmp(path + "0" + to_string(i) + ".obj");

        fishModels.push_back(tmp);
    }
    for (int i= 10; i<16; i++){
        string path = "resources/objects/tropical_fish/TropicalFish";// +num + obj
        Model tmp(path + to_string(i)+".obj");
        fishModels.push_back(tmp);
    }
    for(int i=0; i<15; i++){
        fishModels[i].SetShaderTextureNamePrefix("material.");
    }




    // Diving mask
    float transparentVertices[] = {
            0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  0.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  1.0f
    };

    unsigned int cameraVAO, cameraVBO;
    glGenVertexArrays(1, &cameraVAO);
    glGenBuffers(1, &cameraVBO);
    glBindVertexArray(cameraVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cameraVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    stbi_set_flip_vertically_on_load(true);
    unsigned int transparentCameraTexture = loadTexture(FileSystem::getPath("resources/textures/overlay.png").c_str());
    cameraShader.use();
    cameraShader.setInt("texture1",0);

    
    stbi_set_flip_vertically_on_load(false);
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };



    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);




    vector<std::string> faces {
            FileSystem::getPath("resources/textures/skybox/nx.jpg"),
            FileSystem::getPath("resources/textures/skybox/px.jpg"),
            FileSystem::getPath("resources/textures/skybox/py.jpg"),
            FileSystem::getPath("resources/textures/skybox/ny.jpg"),
            FileSystem::getPath("resources/textures/skybox/nz.jpg"),
            FileSystem::getPath("resources/textures/skybox/pz.jpg")
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(241.0f, 597.0f, -17.0f);
    pointLight.ambient = glm::vec3(20, 12, 0);
    pointLight.diffuse = glm::vec3(0.9, 0.9, 0.9);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 5.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    DirLight& dirLight = programState->dirLight;

    dirLight.direction = glm::vec3(0.0f, 30.0f, -35.0f);
    dirLight.ambient = glm::vec3(0.5f);
    dirLight.diffuse = glm::vec3( 0.2f);
    dirLight.specular = glm::vec3(0.2f);


    glm::vec3 rocksPositions[] = {
            glm::vec3( 120.0f,  10,  93.0f),
            glm::vec3( -85.0f, 10, -82.0f),
            glm::vec3(-12.0f,  10, 55.0f),
            glm::vec3( -80.0f,  10, 85.0f),
            glm::vec3( 75.0f,  10, 20.0f),
            glm::vec3( 25.0f,  10, -20.0f)
    };

    glm::vec3 fishPosition[] = {
            glm::vec3 (45,28,94),
            glm::vec3 (94,38,94),
            glm::vec3 (23,95,246),
            glm::vec3 (-45,54,123),
            glm::vec3 (-45,54,-123),
            glm::vec3 (-124,98,250),
            glm::vec3 (289,54,1-29),
            glm::vec3 (-289,54,123),
            glm::vec3 (-89,154,123),
            glm::vec3 (356,84,22),
            glm::vec3 (-350,74,123),
            glm::vec3 (-212,87,321),
            glm::vec3 (-98,254,423),
            glm::vec3 (-45,54,817),
            glm::vec3 (45,54,123)
    };

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        //pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        ourShader.setVec3("dirLight.direction", dirLight.direction);
        ourShader.setVec3("dirLight.ambient", dirLight.ambient);
        ourShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        ourShader.setVec3("dirLight.specular", dirLight.specular);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 150000.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model

        // ugly fish
        glm::mat4 uglyFishM = glm::mat4(1.0f);



        if(fishJump) {

            if (k <360 and napred == true){
                k+=15;
            }else if(napred == false and k > 0){
                k-=3;
            }
            else if (napred == false and k <= 5) {
                fishJump = false;
            }
            else{
                napred = !napred;
            }
            uglyFishM = glm::translate(uglyFishM,
                                       programState->uglyFishPosition + glm::vec3(k)* pomeraj); // translate it down so it's at the center of the scene

        } else
            uglyFishM = glm::translate(uglyFishM,
                                       programState->uglyFishPosition);



        uglyFishM = glm::scale(uglyFishM, glm::vec3(programState->uglyFishScale));// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", uglyFishM);
        uglyFishModel.Draw(ourShader);

        //ugly Rock
        glm::mat4 uglyRockM = glm::mat4(1.0f);
            uglyRockM = glm::rotate(uglyRockM, glm::radians(-50.0f), glm::vec3(0.0f,1.0f,0.0f));
        uglyRockM = glm::translate(uglyRockM,
                                   programState->uglyRockPosition); // translate it down so it's at the center of the scene
        uglyRockM = glm::scale(uglyRockM, glm::vec3(programState->uglyRockScale));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", uglyRockM);
        uglyRockModel.Draw(ourShader);

        //sand
        glm::mat4 sandM = glm::mat4(1.0f);
        sandM = glm::translate(sandM,
                                   programState->sandPosition); // translate it down so it's at the center of the scene
        sandM = glm::scale(sandM, glm::vec3(programState->sandScale));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", sandM);
        sandModel.Draw(ourShader);

        //submarine
        glm::mat4 submarineM = glm::mat4(1.0f);
        submarineM = glm::translate(submarineM,
                               programState->submarinePosition); // translate it down so it's at the center of the scene
        submarineM = glm::scale(submarineM, glm::vec3(programState->submarineScale));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", submarineM);
        submarineModel.Draw(ourShader);
        //rocks
        float uglovi[6] = {-12, -29, 45, 56, 87, 45};
        glm::mat4 rocksM = glm::mat4(1.0f);
        for(unsigned int i=0; i<6; i++) {
            rocksM = glm::mat4(1.0f);
            rocksM = glm::rotate(rocksM, uglovi[i], glm::vec3(0.0f,1.0f,0.0f));
            rocksM = glm::translate(rocksM,
                                    glm::vec3(20, 1, 20)*rocksPositions[i]); // translate it down so it's at the center of the scene
            rocksM = glm::scale(rocksM, glm::vec3(
                    programState->rocksScale));    // it's a bit too big for our scene, so scale it down
            ourShader.setMat4("model", rocksM);
            rocksModel.Draw(ourShader);
        }

        // blue fish
       /* glm::mat4 blueFishM = glm::mat4(1.0f);
        blueFishM = glm::translate(blueFishM,
                               programState->blueFishPosition); // translate it down so it's at the center of the scene
        blueFishM = glm::scale(blueFishM, glm::vec3(programState->blueFishScale));    // it's a bit too big for our scene, so scale it down
        blueFishM = glm::rotate(blueFishM, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        blueFishM = glm::rotate(blueFishM, glm::radians(-60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ourShader.setMat4("model", blueFishM);
        blueFishModel.Draw(ourShader);
    */


        //Tropical fish
        float uglovi2[15] = {0, 180, 45, 56, 87, 45, 39, 45, -45, -64, -48, 83, 23, 47, -74};


        for(unsigned int i=0; i<15; i++) {


            glm::mat4 fishM = glm::mat4(1.0f);
            fishM = glm::scale(fishM, glm::vec3(0.5));

            glm::mat4 selfrotationMatrix = glm::rotate( glm::mat4 (1.f), uglovi2[i], glm::vec3(0.0f,1.0f,0.0f));

            //Fish group movement

            /*glm::vec3 rotationCenter = glm::vec3(10)*fishPosition[i] + glm::vec3(15) * glm::vec3(.978510,0, 0.206201);
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -rotationCenter);
            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), (float) (currentFrame*0.3), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), rotationCenter);*/

            //fishM =   translateBack * rotationMatrix * translateToOrigin;

            glm::vec4 angledDirection = glm::normalize(selfrotationMatrix*glm::vec4(0,0,1,0));
            //fishM = glm::scale(fishM, glm::vec3(.5));



            fishM = glm::translate(fishM, glm::vec3(10)*fishPosition[i] + glm::vec3(150*currentFrame)*glm::vec3( angledDirection[0],0, angledDirection[2] ));

            fishM *= selfrotationMatrix;







            // translate it down so it's at the center of the scene
               // it's a bit too big for our scene, so scale it down
            ourShader.setMat4("model", fishM);
            fishModels[i].Draw(ourShader);
            fishM = glm::scale(fishM, glm::vec3(
                    0.3));



            if (fishAngle<0.8 and fishAngleDirection ==1){
                fishAngle+=0.004;
            }
            if (fishAngle>=0.8 and fishAngleDirection== 1){
                fishAngleDirection = -1;
                fishAngle = 0.776;

            }
           if (fishAngleDirection == -1 and fishAngle> -1){
                fishAngle-=0.004;
            }
            if (fishAngle<=-0.8 and fishAngleDirection== -1){
                fishAngleDirection = 1;
                fishAngle = - 0.776;
            }


            glm::mat4 tmpFishM = fishM;


            for (int j=1; j<NUM_FISH;j++){

                fishM = tmpFishM;



                fishM = glm::translate(fishM, glm::vec3 (8,1, -10)*glm::vec3(rx[j],ry[j],rz[j]));
                fishM = glm::rotate( fishM, fishAngle, glm::vec3(0.0f,1.0f,0.0f));




                ourShader.setMat4("model", fishM);
                fishModels[i].Draw(ourShader);

            }



        }




        // Dive mask
        glm::mat4 model = glm::mat4(1.0f);

        if(diveMaskActive) {
            cameraShader.use();
            projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                          (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 5000.0f);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.0f,0.0f,-0.5));
            model = glm::scale(model, glm::vec3(2.0f));

            cameraShader.setMat4("model", model);

            glBindVertexArray(cameraVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, transparentCameraTexture);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisable(GL_CULL_FACE);
        }
        //

        // skybox
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &skyboxVBO);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, 0.7f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, 0.7f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, 0.7f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, 0.7f);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(UP, 0.7f);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(DOWN, 0.7f);
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        //programState->CameraMouseMovementUpdateEnabled = !programState->CameraMouseMovementUpdateEnabled ;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        //std::cout << angledDirection[0]<< " " << angledDirection[1]<< " "<< angledDirection[2] << std::endl;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        //ImGui::DragFloat3("Backpack position", (float*)&programState->);
        ImGui::DragFloat("Backpack scale", &programState->uglyFishScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            //programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if(key == GLFW_KEY_M && action == GLFW_PRESS) {
        diveMaskActive = !diveMaskActive;
    }

    if(key == GLFW_KEY_F && action == GLFW_PRESS) {
        fishJump = true;
        k = 1;
        napred = true;
    }
}
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RED;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}