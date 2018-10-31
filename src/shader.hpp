#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
    public:
        GLuint ID;

        Shader(const char* vertexPath, const char* fragmentPath)
        {
            // 1. retrieve the vertex/fragment source code from filePath
            std::string vertexCode;
            std::string fragmentCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;
            // ensure ifstream objects can throw exceptions:
            vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            try 
            {
                // open files
                vShaderFile.open(vertexPath);
                fShaderFile.open(fragmentPath);
                std::stringstream vShaderStream, fShaderStream;
                // read file's buffer contents into streams
                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();
                // close file handlers
                vShaderFile.close();
                fShaderFile.close();
                // convert stream into string
                vertexCode   = vShaderStream.str();
                fragmentCode = fShaderStream.str();
            }
            catch (std::ifstream::failure e)
            {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
            }
            const GLchar* vShaderCode = vertexCode.c_str();
            const GLchar* fShaderCode = fragmentCode.c_str();
            // 2. compile shaders
            this->Compile(vShaderCode, fShaderCode);
        }

        void Compile(const GLchar *vertexSource, const GLchar *fragmentSource)
        {
            GLuint sVertex, sFragment;
            // Vertex Shader
            sVertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(sVertex, 1, &vertexSource, NULL);
            glCompileShader(sVertex);
            checkCompileErrors(sVertex, "VERTEX");
            // Fragment Shader
            sFragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(sFragment, 1, &fragmentSource, NULL);
            glCompileShader(sFragment);
            checkCompileErrors(sFragment, "FRAGMENT");
            // Shader Program
            this->ID = glCreateProgram();
            glAttachShader(this->ID, sVertex);
            glAttachShader(this->ID, sFragment);
            // Link Program
            glLinkProgram(this->ID);
            checkCompileErrors(this->ID, "PROGRAM");
            // Delete the shaders as they're linked into our program now and no longer necessery
            glDeleteShader(sVertex);
            glDeleteShader(sFragment);
        }

        Shader &Use()
        {
            glUseProgram(this->ID);
            return *this;
        }

        void SetFloat(const GLchar *name, GLfloat value, GLboolean useShader)
        {
            if (useShader)
                this->Use();
            glUniform1f(glGetUniformLocation(this->ID, name), value);
        }
        void SetInteger(const GLchar *name, GLint value, GLboolean useShader)
        {
            if (useShader)
                this->Use();
            glUniform1i(glGetUniformLocation(this->ID, name), value);
        }
        void SetVector2f(const GLchar *name, GLfloat x, GLfloat y, GLboolean useShader)
        {
            if (useShader)
                this->Use();
            glUniform2f(glGetUniformLocation(this->ID, name), x, y);
        }
        void SetVector2f(const GLchar *name, const glm::vec2 &value, GLboolean useShader)
        {
            if (useShader)
                this->Use();
            glUniform2f(glGetUniformLocation(this->ID, name), value.x, value.y);
        }
        void SetVector3f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z, GLboolean useShader)
        {
            if (useShader)
                this->Use();
            glUniform3f(glGetUniformLocation(this->ID, name), x, y, z);
        }
        void SetVector3f(const GLchar *name, const glm::vec3 &value, GLboolean useShader)
        {
            if (useShader)
                this->Use();
            glUniform3f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z);
        }
        void SetVector4f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLboolean useShader)
        {
            if (useShader)
                this->Use();
            glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);
        }
        void SetVector4f(const GLchar *name, const glm::vec4 &value, GLboolean useShader)
        {
            if (useShader)
                this->Use();
            glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w);
        }
        void SetMatrix4(const GLchar *name, const glm::mat4 &matrix, GLboolean useShader)
        {
            if (useShader)
                this->Use();
            glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, GL_FALSE, glm::value_ptr(matrix));
        }

    private:
        void checkCompileErrors(GLuint object, std::string type)
        {
            GLint success;
            GLchar infoLog[1024];
            if (type != "PROGRAM")
            {
                glGetShaderiv(object, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(object, 1024, NULL, infoLog);
                    std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                            << infoLog << "\n -- --------------------------------------------------- -- "
                            << std::endl;
                }
            }
            else
            {
                glGetProgramiv(object, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(object, 1024, NULL, infoLog);
                    std::cout << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
                            << infoLog << "\n -- --------------------------------------------------- -- "
                            << std::endl;
                }
            }
        }
};

#endif