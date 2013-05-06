/*
 * shader for handling scaling
 */
uniform sampler2D texture0;

varying vec2 textureCoordinate;

/*const vec4 c_red = vec4(1.0, 0.0, 0.0, 0.0);*/

void main() {
    gl_FragColor = texture2D(texture0, textureCoordinate);
}
