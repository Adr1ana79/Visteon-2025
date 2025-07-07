#include <iostream>
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>

#include "tiny_gltf.h"

struct WindowGLContext{
    GLuint vertexArrayObject;
    GLuint program;
};

struct WindowContext{
    WindowGLContext gl;
};

void LoadShader(WindowContext windowContext);

void LoadMesh(WindowContext windowContext, tinygltf::Model model, unsigned int meshId);

int main(void){
    WindowContext windowContext;

    GLFWwindow* window;

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API); 

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window){
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window); 
    
    std::string gltfFilename = "../examples/gltf/01_triangle/export/triangle.gltf";

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    if(!loader.LoadASCIIFromFile(&model, &err, &warn, gltfFilename)){
        std::cerr << "Failed to load gltf file" << gltfFilename << "/n";
        std::cerr << "Error: " << err << std::endl;
        std::cerr << "Warning: " << warn << std::endl;
        return 1;
    }

    GLfloat postitionData[] = {
        -0.5f, -0.5f, 0.0f, // positions[0]​
      0.5f, -0.5f, 0.0f, // positions[1]​
        0.0f, 0.5f, 0.0f, // positions[2]​
    };

    GLfloat normalData[] = {
        0.0f, 0.0f, 1.0f, // normals[0]​
        0.0f, 0.0f, 1.0f, // normals[1]​
        0.0f, 0.0f, 1.0f, // normals[2]​
    };

    GLfloat textData[] = {
        0.25f, 0.25f, // texture coordinates[0]​
        0.75f, 0.25f, // texture coordinates[1]​
        0.5f, 0.75f // texture coordinates[2]​
    };

    unsigned int bufferID = 0;
    unsigned int buffer2ID = 0;
    unsigned int buffer3ID = 0;
    
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);    
    glBufferData(GL_ARRAY_BUFFER, sizeof(postitionData), postitionData, GL_STATIC_DRAW);

    glGenBuffers(1, &buffer2ID);
    glBindBuffer(GL_ARRAY_BUFFER, buffer2ID); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(normalData), normalData, GL_STATIC_DRAW);
    
    glGenBuffers(1, &buffer3ID); 
    glBindBuffer(GL_ARRAY_BUFFER, buffer3ID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textData), textData, GL_STATIC_DRAW);

    const int POSITION_INDEX = 0;
    const int NORMAL_INDEX = 1;
    const int TEXT_INDEX = 2;  
    
    GLuint vertexArrayObject1 = 0;
    glGenVertexArrays(1, &vertexArrayObject1);
    glBindVertexArray(vertexArrayObject1);


    glBindBuffer(GL_ARRAY_BUFFER, bufferID); 
    glEnableVertexAttribArray(POSITION_INDEX);
    glVertexAttribPointer(POSITION_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0); 
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer2ID);
    glEnableVertexAttribArray(NORMAL_INDEX);
    glVertexAttribPointer(NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer3ID);
    glEnableVertexAttribArray(TEXT_INDEX);  
    glVertexAttribPointer(TEXT_INDEX, 2, GL_FLOAT, GL_FALSE,  0, 0);


    GLuint vertexArrayObject2 = 0;
    glGenVertexArrays(1, &vertexArrayObject2);
    glBindVertexArray(vertexArrayObject2);

    
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);    
    glEnableVertexAttribArray(TEXT_INDEX);
    glVertexAttribPointer(TEXT_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0); 
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer2ID);    
    glEnableVertexAttribArray(NORMAL_INDEX);
    glVertexAttribPointer(NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, buffer3ID);    
    glEnableVertexAttribArray(POSITION_INDEX);  
    glVertexAttribPointer(POSITION_INDEX, 2, GL_FLOAT, GL_FALSE,  0, 0);

    LoadShader(windowContext);

    while (!glfwWindowShouldClose(window)){
        glClearColor(0.5F, 0.0F, 0.7F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vertexArrayObject1);
        glUseProgram(windowContext.gl.program);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(vertexArrayObject2);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);

        glfwPollEvents();
     }

    glfwTerminate();
    return 0;
} 

void LoadMesh(WindowContext windowContext, tinygltf::Model model, unsigned int meshId){
    GLuint vertexBuffer = 0;
    GLuint normalBuffer = 0;
    GLuint texCoordBuffer = 0;

    uint32_t gltfAccessorPositionIndex = model.meshes[meshId].primitives[0].attributes["POSITION"];
    uint32_t gltfAccessorNormalIndex = model.meshes[meshId].primitives[0].attributes["NORMAL"];
    uint32_t gltfAccessorTexCoordIndex = model.meshes[meshId].primitives[0].attributes["TEXCOORD_0"];

    uint32_t gltfBufferViewPositionIndex = model.accessors[gltfAccessorPositionIndex].bufferView;
    uint32_t gltfBufferViewNormalIndex = model.accessors[gltfAccessorNormalIndex].bufferView;
    uint32_t gltfBufferViewTexCoordIndex = model.accessors[gltfAccessorTexCoordIndex].bufferView;

    uint32_t gltfBufferIndexPosition = model.bufferViews[gltfBufferViewPositionIndex].buffer;
    uint32_t gltfBufferIndexNormal = model.bufferViews[gltfBufferViewNormalIndex].buffer;
    uint32_t gltfBufferIndexTexCoord = model.bufferViews[gltfBufferViewTexCoordIndex].buffer;

    unsigned char* gltfBufferDataPosition = model.buffers[gltfBufferIndexPosition].data.data();
    unsigned char* gltfBufferDataNormal = model.buffers[gltfBufferIndexNormal].data.data();
    unsigned char* gltfBufferDataTexCoord = model.buffers[gltfBufferIndexTexCoord].data.data();

    uint32_t gltfPositionByteOffset = model.bufferViews[gltfBufferViewPositionIndex].byteOffset;
    uint32_t gltfNormalByteOffset = model.bufferViews[gltfBufferViewNormalIndex].byteOffset;
    uint32_t gltfTexCoordByteOffset = model.bufferViews[gltfBufferViewTexCoordIndex].byteOffset;

    uint32_t gltfPositionByteLength = model.bufferViews[gltfBufferViewPositionIndex].byteLength;
    uint32_t gltfNormalByteLength = model.bufferViews[gltfBufferViewNormalIndex].byteLength;
    uint32_t gltfTexCoordByteLength = model.bufferViews[gltfBufferViewTexCoordIndex].byteLength;
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, gltfPositionByteLength, gltfBufferDataPosition + gltfPositionByteOffset, GL_STATIC_DRAW);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, gltfNormalByteLength, gltfBufferDataNormal + gltfNormalByteOffset, GL_STATIC_DRAW);

    glGenBuffers(1, &texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, gltfTexCoordByteLength, gltfBufferDataTexCoord + gltfTexCoordByteOffset, GL_STATIC_DRAW);

    glGenVertexArrays(1, &windowContext.gl.vertexArrayObject);
    glBindVertexArray(windowContext.gl.vertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0,0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void LoadShader(WindowContext windowContext){
    const char* vertexShaderSource = R"(
        #version 300 es

        layout(location = 0) in vec3 position;
        void main(){
            gl_Position = vec4(position, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 300 es

        precision mediump float;
        out vec4 fragColor;
        void main(){
            fragColor = vec4(0.0, 1.0, 0.7, 1.0);
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);

    glCompileShader(vertexShader);
    GLint status;
    char buffer[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE){
        GLint length;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &length);
        glGetShaderInfoLog(vertexShader, length, &length, buffer);
        std::cout << "Vertex shader Compilation Failed" << buffer << std::endl;
    }

    glCompileShader(fragmentShader);
    if(status == GL_FALSE){
        GLint length;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &length);
        glGetShaderInfoLog(fragmentShader, length, &length, buffer);
        std::cout << "Fragment shader Compilation Failed" << buffer << std::endl;
    }

    windowContext.gl.program = glCreateProgram();
    glAttachShader(windowContext.gl.program, vertexShader);
    glAttachShader(windowContext.gl.program, fragmentShader);
    
    glLinkProgram(windowContext.gl.program);
    glGetProgramiv(windowContext.gl.program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE){
        GLint length;
        glGetProgramiv(windowContext.gl.program, GL_INFO_LOG_LENGTH, &length);
        glGetProgramInfoLog(windowContext.gl.program, length, &length, buffer);
        std::cout << "Program Linking Failed" << buffer << std::endl;
    }

    glUseProgram(0);
}