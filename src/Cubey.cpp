#include <iostream>
#include <map>
#include <string>
#include <format>   // Required for std::format
#include <random>

// NEW: GLAD should be included BEFORE GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h> // GLFW header

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_truetype.h" // For font rendering

#define WIN_WIDTH 900
#define WIN_HEIGHT 700

// --- Global variables for font rendering ---
GLuint textVAO, textVBO;
GLuint textShaderProgram;
stbtt_bakedchar charData[96];
GLuint fontTexture;

// --- Callback for GLFW window resize events ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// --- Callback for keyboard input ---
void processInput(GLFWwindow *window, float& rotationX, float& rotationY) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        rotationX -= 2.0f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        rotationX += 2.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rotationY -= 2.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rotationY += 2.0f;
}

// --- Shader Sources ---
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;

    out vec3 ourColor;

    uniform mat4 mvp; // Model-View-Projection Matrix

    void main() {
        gl_Position = mvp * vec4(aPos, 1.0);
        ourColor = aColor;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec3 ourColor;

    void main() {
        FragColor = vec4(ourColor, 1.0);
    }
)";

// 2D Text Shader Sources ---
const char* textVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec4 vertex; // vec2 pos, vec2 tex
    out vec2 TexCoords;

    uniform mat4 projection;

    void main() {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
        TexCoords = vertex.zw;
    }
)";
const char* textFragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoords;
    out vec4 color;

    uniform sampler2D text;
    uniform vec3 textColor;

    void main() {
        // The font texture is single-channel (alpha). We use its value
        // to set the alpha of our output color.
        float alpha = texture(text, TexCoords).r;
        color = vec4(textColor, alpha);
    }
)";

// --- Helper function to compile shaders ---
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // --- 1. Compile Vertex Shader ---
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // --- 2. Compile Fragment Shader ---
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // --- 3. Link Shaders into a Program ---
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // --- 4. Clean Up ---
    // The individual shaders are no longer needed after they've been linked into the program.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// --- Text Rendering Function Implementations ---
void loadFont(const char* fontPath) {
    // Read font file
    FILE* fontFile = fopen(fontPath, "rb");
    fseek(fontFile, 0, SEEK_END);
    long size = ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);
    unsigned char* ttfBuffer = new unsigned char[size];
    fread(ttfBuffer, 1, size, fontFile);
    fclose(fontFile);

    // Bake font bitmap
    const int FONT_ATLAS_WIDTH = 512;
    const int FONT_ATLAS_HEIGHT = 512;
    unsigned char fontBitmap[FONT_ATLAS_WIDTH * FONT_ATLAS_HEIGHT];
    stbtt_BakeFontBitmap(ttfBuffer, 0, 48.0f, fontBitmap, FONT_ATLAS_WIDTH, FONT_ATLAS_HEIGHT, 32, 96, charData); // ASCII 32-127
    delete[] ttfBuffer;

    // Create OpenGL texture for the font atlas
    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_ATLAS_WIDTH, FONT_ATLAS_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, fontBitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Render text at position (x, y) with given scale
void renderText(const std::string& text, float x, float y, float scale) {
    glUseProgram(textShaderProgram);
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), 1.0f, 1.0f, 1.0f); // White text
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    // Iterate through all characters
    for (char c : text) {
        if (c >= 32 && c < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(charData, 512, 512, c - 32, &x, &y, &q, 1);

            float vertices[6][4] = {
                { q.x0, q.y0, q.s0, q.t0 },
                { q.x0, q.y1, q.s0, q.t1 },
                { q.x1, q.y1, q.s1, q.t1 },

                { q.x0, q.y0, q.s0, q.t0 },
                { q.x1, q.y1, q.s1, q.t1 },
                { q.x1, q.y0, q.s1, q.t0 }
            };

            // Render glyph texture over quad
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- Main Function ---
int main() {
    // --- 1. Initialize GLFW ---
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure OpenGL context (Core Profile 3.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS
#endif

    // --- 2. Create a GLFW window ---
    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Cubey (GLFW)", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // --- 3. Initialize GLAD ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Enable depth testing and blending for 3D and text rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); // Enable blending for text transparency.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- 4. Define Cube Geometry ---
    float vertices[] = {
        // positions          // colors
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // Red face
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Green face
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // Blue face (left)
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // Yellow face (right)
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f, // Magenta face (bottom)
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // Cyan face (top)
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0, // Face 1
        4, 5, 6, 6, 7, 4, // Face 2
        8, 9, 10, 10, 11, 8, // Face 3
        12, 13, 14, 14, 15, 12, // Face 4
        16, 17, 18, 18, 19, 16, // Face 5
        20, 21, 22, 22, 23, 20  // Face 6
    };

    // Setup VAO, VBO, EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind and set vertex buffers and attribute pointers
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // --- 5. Compile Shaders and Set Up Matrices ---
    GLuint cubeShaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    // --- 6. Font Loading and Text Rendering Setup ---
    textShaderProgram = createShaderProgram(textVertexShaderSource, textFragmentShaderSource);
    loadFont("arial.ttf"); // Make sure arial.ttf is in your project root

    // Random rotation speeds
    std::mt19937 gen(std::random_device{}()); // Random number generator
    std::uniform_real_distribution<float> rndDistrib(0.1f, 2.0f); // Random speed between .1 and 2 degrees per frame
    float rotationXSpeed = rndDistrib(gen);
    float rotationYSpeed = rndDistrib(gen);

    float rotationX = 0.0f;
    float rotationY = 0.0f;

    // --- Main Render Loop 
    while (!glfwWindowShouldClose(window)) {
        // Input processing
        processInput(window, rotationX, rotationY);
        rotationX += rotationXSpeed;
        rotationY += rotationYSpeed;
        if (rotationX > 360.0f || rotationX < -360.0f ) rotationX = 0.0f;
        if (rotationY > 360.0f || rotationY < -360.0f ) rotationY = 0.0f;

        // Rendering
        glEnable(GL_DEPTH_TEST); // Ensure depth test is on for the 3D part
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(cubeShaderProgram);

        // Update model matrix for rotation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

        // Calculate final MVP matrix and send to shader
        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // --- RENDER 2D TEXT ---
        glDisable(GL_DEPTH_TEST); // Disable depth test for the 2D overlay.

        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Flip the projection's Y-axis to match the font library.
        // The arguments are left, right, bottom, top.
        // We set bottom=height and top=0 to make Y increase downwards.
        glm::mat4 ortho_projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);
        
        glUseProgram(textShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(ortho_projection));
        
        // Since y=0 is now the top of the screen, we use a small positive
        //std::string txt = "Arrow keys control the rotation " + std::to_string(rotationX) + ", " + std::to_string(rotationY);
        std::string txt = std::format("Arrow keys control the rotation ({:.1f}, {:.1f})", rotationX, rotationY);
        renderText(txt, 25.0f, 50.0f, 1.0f);

                
        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- 7. Cleanup ---
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(cubeShaderProgram);
    
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(textShaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate(); // Terminate GLFW

    return 0;
}