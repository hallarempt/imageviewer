/*
 * Vertex shader for handling composite ops
 */
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

attribute vec4 vertex;
attribute vec2 inputTextureCoordinate;

varying vec2 textureCoordinate;

void main()
{
    gl_Position = (projectionMatrix * viewMatrix * modelMatrix * vertex);
    textureCoordinate = inputTextureCoordinate;
}


