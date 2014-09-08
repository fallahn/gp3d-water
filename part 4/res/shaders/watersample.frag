#ifdef OPENGL_ES
precision mediump float;
#endif

//////////////////////////
// Uniforms
uniform sampler2D u_refractionTexture;
uniform sampler2D u_reflectionTexture;

uniform sampler2D u_normalMap;

uniform float u_time;

//////////////////////////
// Varyings
varying vec4 v_vertexRefractionPosition;
varying vec4 v_vertexReflectionPosition;

varying vec2 v_texCoord;

varying vec3 v_eyePosition;

//////////////////////////

const float distortAmount = 0.05;
const float specularAmount = 2.5;
const float textureRepeat = 2.0;

const vec4 tangent = vec4(1.0, 0.0, 0.0, 0.0);
const vec4 viewNormal = vec4(0.0, 1.0, 0.0, 0.0);
const vec4 bitangent = vec4(0.0, 0.0, 1.0, 0.0);

vec2 fromClipSpace(vec4 position)
{
	return position.xy / position.w / 2.0 + 0.5;
}

void main()
{	
	//get normal
	vec4 normal = texture2D(u_normalMap, v_texCoord * textureRepeat + u_time);
	normal = normalize(normal * 2.0 - 1.0);
		
	//distortion offset
	vec4 dudv = normal * distortAmount;
	
	//refraction sample
	vec2 textureCoord = fromClipSpace(v_vertexRefractionPosition) + dudv.rg;
	textureCoord = clamp(textureCoord, 0.001, 0.999);
	vec4 refractionColour = texture2D(u_refractionTexture, textureCoord);
		
	//reflection sample
	textureCoord = fromClipSpace(v_vertexReflectionPosition) + dudv.rg;
	textureCoord = clamp(textureCoord, 0.001, 0.999);	
	vec4 reflectionColour = texture2D(u_reflectionTexture, textureCoord);
	
	//put view in tangent space
	vec4 viewDir = normalize(vec4(v_eyePosition, 1.0));
	vec4 viewTanSpace = normalize(vec4(dot(viewDir, tangent), dot(viewDir, bitangent), dot(viewDir, viewNormal), 1.0));	
	vec4 viewReflection = normalize(reflect(-1.0 * viewTanSpace, normal));
	float fresnel = dot(normal, viewReflection);	
	
	vec3 specular = vec3(clamp(pow(dot(viewTanSpace, normal), 255.0), 0.0, 1.0)) * specularAmount;
	
	gl_FragColor = mix(reflectionColour, refractionColour, fresnel) + vec4(specular, 1.0);
}