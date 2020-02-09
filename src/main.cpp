#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "camera.hpp"
#include "shader.hpp"
#include "model.hpp"

void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void setupPointLights(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors);
void renderModel(Model model, Shader shader, std::vector<glm::vec3> objectPositions, glm::mat4 projection, glm::mat4 view);
void renderPointLights(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors, glm::mat4 projection, glm::mat4 view);
void renderCube();
void renderQuad();

// settings
const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;
const unsigned int NR_POINT_LIGHTS = 14;

// camera
Camera camera(glm::vec3(-5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -35.0f, -40.0f);
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// flags
bool deferredShadingFlag = true;
bool deferredShadingFlagPressed = false;

bool rotateModelFlag = true;
bool rotateModelFlagPressed = false;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BaseOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader baseShader("../src/shaders/base_shader.vs", "../src/shaders/base_shader.fs");
    Shader geometryPassShader("../src/shaders/geometry_pass.vs", "../src/shaders/geometry_pass.fs");
    Shader lightingPassShader("../src/shaders/lighting_pass.vs", "../src/shaders/lighting_pass.fs");
    Shader lightBoxShader("../src/shaders/light_box.vs", "../src/shaders/light_box.fs");

    // load models
    // -----------
    // Model charModel("../assets/models/nanosuit/nanosuit.obj");
    Model shipModel("../assets/models/SF_Light-Fighter_X6/SF_Light_Fighter-X6.obj");
    std::vector<glm::vec3> objectPositions;
    objectPositions.push_back(glm::vec3(-5.0,  0.0, -8.0));
    objectPositions.push_back(glm::vec3( 0.0,  0.0, -8.0));
    objectPositions.push_back(glm::vec3( 5.0,  0.0, -8.0));
    objectPositions.push_back(glm::vec3(-5.0,  0.0,  0.0));
    objectPositions.push_back(glm::vec3( 0.0,  0.0,  0.0));
    objectPositions.push_back(glm::vec3( 5.0,  0.0,  0.0));
    objectPositions.push_back(glm::vec3(-5.0,  0.0,  8.0));
    objectPositions.push_back(glm::vec3( 0.0,  0.0,  8.0));
    objectPositions.push_back(glm::vec3( 5.0,  0.0,  8.0));

    // configure g-buffer framebuffer
    // ------------------------------
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedoSpec, gEmission;
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // emission buffer
    glGenTextures(1, &gEmission);
    glBindTexture(GL_TEXTURE_2D, gEmission);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gEmission, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // lighting info
    // -------------
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    srand(99);
    for (unsigned int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        // calculate slightly random offsets
        float xPos = ((rand() % 100) / 100.0) * 16.0 - 8.0;
        float yPos = ((rand() % 100) / 100.0) * 4.0 - 2.0;
        float zPos = ((rand() % 100) / 100.0) * 10.0 - 5.0;
        lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
        // also calculate random color
        float rColor = ((rand() % 100) / 200.0) + 0.1;
        float gColor = ((rand() % 100) / 200.0) + 0.1;
        float bColor = ((rand() % 100) / 200.0) + 0.1;
        lightColors.push_back(glm::vec3(rColor, gColor, bColor));
    }

    // shader configuration
    // --------------------
    lightingPassShader.Use();
    lightingPassShader.SetInteger("gPosition", 0);
    lightingPassShader.SetInteger("gNormal", 1);
    lightingPassShader.SetInteger("gAlbedoSpec", 2);
    lightingPassShader.SetInteger("gEmission", 3);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        if (!deferredShadingFlag)
        {
            // ------------------- FORWARD SHADING START --------------- //
            baseShader.Use();
            // set shader uniforms
            baseShader.SetVector3f("viewPos", camera.Position);
            baseShader.SetMatrix4("projection", projection);
            baseShader.SetMatrix4("view", view);
            // point light properties
            setupPointLights(baseShader, lightPositions, lightColors);
            // render ship models
            renderModel(shipModel, baseShader, objectPositions, projection, view);
            // render point lights
            renderPointLights(lightBoxShader, lightPositions, lightColors, projection, view);
            // ------------------- FORWARD SHADING END --------------- //
        }

        if (deferredShadingFlag)
        {
            // ------------------- DEFERRED SHADING START --------------- //
            // 1. geometry pass: render scene's geometry/color data into gbuffer
            // -----------------------------------------------------------------
            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                renderModel(shipModel, geometryPassShader, objectPositions, projection, view);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
            // -----------------------------------------------------------------------------------------------------------------------
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            lightingPassShader.Use();
            lightingPassShader.SetVector3f("viewPos", camera.Position);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, gEmission);
            // point light properties
            setupPointLights(lightingPassShader, lightPositions, lightColors);
            // render the quad
            renderQuad();

            // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
            // ----------------------------------------------------------------------------------
            glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
            // blit to default framebuffer
            glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                              0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                              GL_DEPTH_BUFFER_BIT,
                              GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 3. render lights on top of scene
            // --------------------------------
            glViewport(0, 0, framebufferWidth, framebufferHeight);
            renderPointLights(lightBoxShader, lightPositions, lightColors, projection, view);
            // ------------------- DEFERRED SHADING END --------------- //
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// renderModel() renders the given model with the given shader
// -------------------------------------------------
void renderModel(Model model, Shader shader, std::vector<glm::vec3> objectPositions, glm::mat4 projection, glm::mat4 view)
{
    shader.Use();
    shader.SetMatrix4("projection", projection);
    shader.SetMatrix4("view", view);

    for (unsigned int i = 0; i < objectPositions.size(); i++)
    {
        glm::mat4 modelMat = glm::mat4(1.0);
        modelMat = glm::translate(modelMat, objectPositions[i]);
        modelMat = glm::scale(modelMat, glm::vec3(0.05f));
        if (rotateModelFlag)
            modelMat = glm::rotate(modelMat, (float)glfwGetTime() * -0.4f, glm::normalize(glm::vec3(-0.5, -0.6, 0.8)));
        shader.SetMatrix4("model", modelMat);
        model.Draw(shader);
    }
}

void setupPointLights(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors)
{
    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        shader.SetVector3f("pointLights[" + std::to_string(i) + "].Position", lightPositions[i]);
        shader.SetVector3f("pointLights[" + std::to_string(i) + "].Color", lightColors[i]);
        // update attenuation parameters and calculate radius
        const float constant  = 1.0;
        const float linear    = 0.5;
        const float quadratic = 1.2;
        shader.SetFloat("pointLights[" + std::to_string(i) + "].Constant", constant);
        shader.SetFloat("pointLights[" + std::to_string(i) + "].Linear", linear);
        shader.SetFloat("pointLights[" + std::to_string(i) + "].Quadratic", quadratic);
    }
}

void renderPointLights(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors, glm::mat4 projection, glm::mat4 view)
{
    shader.Use();
    shader.SetMatrix4("projection", projection);
    shader.SetMatrix4("view", view);
    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        glm::mat4 modelMat = glm::mat4(1.0);
        modelMat = glm::translate(modelMat, lightPositions[i]);
        modelMat = glm::scale(modelMat, glm::vec3(0.05f));
        shader.SetMatrix4("model", modelMat);
        shader.SetVector3f("lightColor", lightColors[i]);
        renderCube();
    }
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(CAMERA_FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(CAMERA_BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(CAMERA_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(CAMERA_RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(CAMERA_DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(CAMERA_UP, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !deferredShadingFlagPressed)
    {
        deferredShadingFlag = !deferredShadingFlag;
        deferredShadingFlagPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
        deferredShadingFlagPressed = false;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !rotateModelFlagPressed)
    {
        rotateModelFlag = !rotateModelFlag;
        rotateModelFlagPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
        rotateModelFlagPressed = false;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow * /* window */, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow * /* window */, double /* xoffset */, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow * window, int /* width */, int /* height */)
{
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
}