#ifdef OPENGL_ES
precision mediump float;
#endif

//////////////////////////
// Uniforms
uniform sampler2D u_refractionTexture;

//////////////////////////
// Varyings
varying vec4 v_vertexRefractionPosition;

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
	
	gl_FragColor = refractionColour;
}