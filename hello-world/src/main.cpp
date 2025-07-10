#include <iostream>
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>

#include "tiny_gltf.h"
#include "basic_types.hpp"

struct WindowGLContext{
    GLuint indecesCount;
    GLuint vertexArrayObject;
    GLuint program;
    GLuint indexBuffer;
    std::unordered_map<std::string, float> materialUniformFloats;
    std::unordered_map<std::string, Vector4> materialUniformVector4;
};

struct WindowContext{
    WindowGLContext gl;
};


static void materialSetProperty(WindowGLContext& glContext, std::string uniformName, float value);
static void materialSetProperty(WindowGLContext& glContext, std::string uniformName, Vector4 value);

static void materialUpdateProperties(WindowGLContext& glContext);

static float getCurrentTime(){
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();
    return duration / 1000.0f;
}

void loadMaterial(WindowContext& windowContext, tinygltf::Model model, std::filesystem::path gltfDirectory, unsigned int materialId);
void loadMesh(WindowContext& windowContext, tinygltf::Model model, unsigned int meshId);

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
    
    std::string gltfFilename = "../examples/gltf/05_suzanne_uniforms/export/suzanne.gltf";

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    if(!loader.LoadASCIIFromFile(&model, &err, &warn, gltfFilename)){
        std::cerr << "Failed to load gltf file" << gltfFilename << "/n";
        std::cerr << "Error: " << err << std::endl;
        std::cerr << "Warning: " << warn << std::endl;
        return 1;
    }
    
    std::filesystem::path gltfPath = gltfFilename;
    std::filesystem::path gltfDirectory = gltfPath.parent_path();

    int meshId = 0, materialId = 0;
    loadMesh(windowContext, model, meshId);
    loadMaterial(windowContext, model, gltfDirectory, materialId);

    while (!glfwWindowShouldClose(window)){
        glClearColor(0.5F, 0.0F, 0.7F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
         
        glBindVertexArray(windowContext.gl.vertexArrayObject);
        glUseProgram(windowContext.gl.program);
 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, windowContext.gl.indexBuffer);

        materialSetProperty(windowContext.gl, "iTime", getCurrentTime());
        materialUpdateProperties(windowContext.gl);

        glDrawElements(GL_TRIANGLES, windowContext.gl.indecesCount, GL_UNSIGNED_SHORT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
} 

void loadMesh(WindowContext& windowContext, tinygltf::Model model, unsigned int meshId){
    GLuint vertexBuffer = 0;
    GLuint normalBuffer = 0;
    GLuint texCoordBuffer = 0;
    GLuint indexBuffer = 0;

    uint32_t gltfAccessorPositionIndex = model.meshes[meshId].primitives[0].attributes["POSITION"];
    uint32_t gltfAccessorNormalIndex = model.meshes[meshId].primitives[0].attributes["NORMAL"];
    uint32_t gltfAccessorTexCoordIndex = model.meshes[meshId].primitives[0].attributes["TEXCOORD_0"];
    uint32_t gltfAccessorIndecesINdex = model.meshes[meshId].primitives[0].indices;

    uint32_t gltfBufferViewPositionIndex = model.accessors[gltfAccessorPositionIndex].bufferView;
    uint32_t gltfBufferViewNormalIndex = model.accessors[gltfAccessorNormalIndex].bufferView;
    uint32_t gltfBufferViewTexCoordIndex = model.accessors[gltfAccessorTexCoordIndex].bufferView;
    uint32_t gltfBufferViewIndecesIndex = model.accessors[gltfAccessorIndecesINdex].bufferView;

    uint32_t gltfBufferIndexPosition = model.bufferViews[gltfBufferViewPositionIndex].buffer;
    uint32_t gltfBufferIndexNormal = model.bufferViews[gltfBufferViewNormalIndex].buffer;
    uint32_t gltfBufferIndexTexCoord = model.bufferViews[gltfBufferViewTexCoordIndex].buffer;
    uint32_t gltfBufferIndexIndeces = model.bufferViews[gltfBufferViewIndecesIndex].buffer;
    
    unsigned char* gltfBufferDataPosition = model.buffers[gltfBufferIndexPosition].data.data();
    unsigned char* gltfBufferDataNormal = model.buffers[gltfBufferIndexNormal].data.data();
    unsigned char* gltfBufferDataTexCoord = model.buffers[gltfBufferIndexTexCoord].data.data();
    unsigned char* gltfBufferDataIndeces = model.buffers[gltfBufferIndexIndeces].data.data();

    uint32_t gltfPositionByteOffset = model.bufferViews[gltfBufferViewPositionIndex].byteOffset;
    uint32_t gltfNormalByteOffset = model.bufferViews[gltfBufferViewNormalIndex].byteOffset;
    uint32_t gltfTexCoordByteOffset = model.bufferViews[gltfBufferViewTexCoordIndex].byteOffset;
    uint32_t gltfIndecesByteOffset = model.bufferViews[gltfBufferViewIndecesIndex].byteOffset;

    uint32_t gltfPositionByteLength = model.bufferViews[gltfBufferViewPositionIndex].byteLength;
    uint32_t gltfNormalByteLength = model.bufferViews[gltfBufferViewNormalIndex].byteLength;
    uint32_t gltfTexCoordByteLength = model.bufferViews[gltfBufferViewTexCoordIndex].byteLength;
    uint32_t gltfIndecesByteLength = model.bufferViews[gltfBufferViewIndecesIndex].byteLength;

    windowContext.gl.indecesCount = gltfPositionByteLength / sizeof(GLushort);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, gltfPositionByteLength, gltfBufferDataPosition + gltfPositionByteOffset, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gltfIndecesByteLength, gltfBufferDataIndeces + gltfIndecesByteOffset, GL_STATIC_DRAW);
    windowContext.gl.indexBuffer = indexBuffer;

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
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0,0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void materialSetProperty(WindowGLContext& glContext, std::string uniformName, float value){
    if (glContext.materialUniformFloats.find(uniformName) != glContext.materialUniformFloats.end())
    {
        glContext.materialUniformFloats[uniformName] = value;
    }
}

static void materialSetProperty(WindowGLContext& glContext, std::string uniformName, Vector4 value){
    if (glContext.materialUniformVector4.find(uniformName) != glContext.materialUniformVector4.end())
    {
        glContext.materialUniformVector4[uniformName] = value;
    }
}

static void materialUpdateProperties(WindowGLContext& glContext){
    for (auto& uniform : glContext.materialUniformFloats){
        GLint location = glGetUniformLocation(glContext.program, uniform.first.c_str());
        if (location != -1){
            glUniform1f(location, uniform.second);
        }
        std::cout << "Uniform: " << uniform.first << " = " << uniform.second << std::endl;
    }
}

void loadMaterial(WindowContext& windowContext, tinygltf::Model model, std::filesystem::path gltfDirectory, unsigned int materialId)
{
    const char* defaultVertexShaderSource = R"(
        attribute vec2 position;
        void main(){
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";

    const char* defaultFragmentShaderSource = R"(
        void main(){
            gl_Position = vec4(0.0, 1.0, 0.0, 1.0);
        }
    )";
    
    std::filesystem::path vertexShaderPath; 
    std::filesystem::path fragmentShaderPath;
    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    
    if(materialId < model.materials.size()){
        auto gltfMaterialExstras = model.materials[materialId].extras;
        if(gltfMaterialExstras.Has("shader")){
            auto gltfMaterialShader = gltfMaterialExstras.Get("shader");

            if(gltfMaterialShader.Has("vertex")){
                std::string gltfMaterialShaderVertex = gltfMaterialShader.Get("vertex").Get<std::string>();
                vertexShaderPath = gltfDirectory / gltfMaterialShaderVertex;
            }
            if(gltfMaterialShader.Has("fragment")){
                std::string gltfMaterialShaderFragment = gltfMaterialShader.Get("fragment").Get<std::string>();
                fragmentShaderPath = gltfDirectory / gltfMaterialShaderFragment;
            }
            if(gltfMaterialShader.Has("uniforms")){
                auto gltfUniforms = gltfMaterialShader.Get("uniforms");
                for (int uniformIdx = 0; uniformIdx < gltfUniforms.ArrayLen(); uniformIdx++){
                    auto uniform = gltfUniforms.Get(uniformIdx);
                    std::string uniformName;
                    if (uniform.Has("name")){                    
                        uniformName = uniform.Get("name").Get<std::string>();
                    }
                    if (uniform.Has("type")){
                        std::string type = uniform.Get("type").Get<std::string>();
                        auto uniformValue = uniform.Get("value");
                        if(type == "Float"){
                            double uniformValueFloat = uniformValue.Get(0).Get<double>();
                            windowContext.gl.materialUniformFloats[uniformName] = uniformValueFloat;
                            std::cout << "Uniform: " << uniformName << " = " << uniformValueFloat << std::endl; 
                        }
                        if(type == "Vector4"){
                            double uniformValueFloat = uniformValue.Get(0).Get<double>();
                            std::cout << "Uniform: " << uniformName << " = " << uniformValueFloat << std::endl; 
                        }
                    }
                }
            }
        }
    }else{
        vertexShaderSource = defaultVertexShaderSource;
        fragmentShaderSource = defaultFragmentShaderSource;
    }

    std::ifstream vertexShaderFile(vertexShaderPath);
    if(vertexShaderFile.is_open()){
        std::stringstream buffer;
        buffer << vertexShaderFile.rdbuf();
        vertexShaderSource = buffer.str();
    }

    std::ifstream fragmentShaderFile(fragmentShaderPath);
    if(fragmentShaderFile.is_open()){
        std::stringstream buffer;
        buffer << fragmentShaderFile.rdbuf();
        fragmentShaderSource = buffer.str();
    }

    const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, nullptr);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr);
    
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