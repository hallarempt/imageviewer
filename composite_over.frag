/*
 * shader for handling the "normal" or "over" composite op
 */
uniform sampler2D inputTextureTop;
uniform sampler2D inputTextureBot;

uniform highp float alphaTop;

varying highp vec2 textureCoordinate;

void main()
{
    lowp vec4 pixelTop = texture2D(inputTextureTop, textureCoordinate);
    lowp vec4 pixelBot = texture2D(inputTextureBot, textureCoordinate);

    // gl_FragColor = vec4(textureCoordinate.x, textureCoordinate.y, 0, 1);
    // Top = src
    if (pixelTop.a == 1.0) {
        gl_FragColor = pixelTop.rgba;
    }
    gl_FragColor.rgb = mix(pixelTop.rgb, pixelBot.rgb, pixelBot.a * alphaTop);



}
