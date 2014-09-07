#ifdef OPENGL_ES
precision mediump float;
#endif

//////////////////////////
// Uniforms
uniform sampler2D u_refractionTexture;
uniform sampler2D u_reflectionTexture;

//////////////////////////
// Varyings
varying vec4 v_vertexRefractionPosition;
varying vec4 v_vertexReflectionPosition;

//////////////////////////

vec2 fromClipSpace(vec4 position)
{
	return position.xy / position.w / 2.0 + 0.5;
}

void main()
{
	//refraction sample
	vec2 textureCoord = fromClipSpace(v_vertexRefractionPosition);	
	vec4 refractionColour = texture2D(u_refractionTexture, textureCoord);
	
	//reflection sample
	textureCoord = fromClipSpace(v_vertexReflectionPosition);
	vec4 reflectionColour = texture2D(u_reflectionTexture, textureCoord);
	
	gl_FragColor = reflectionColour;
}