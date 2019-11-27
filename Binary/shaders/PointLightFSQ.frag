//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
	int ScreenWidth;
	int ScreenHeight;
};

uniform sampler2D GBufferPos;
uniform sampler2D GBufferNormals;
uniform sampler2D GBufferDiffuse;
uniform sampler2D GBufferSpecGloss;

uniform vec4 lightWorldPos;
uniform vec3 lightColor;
uniform float radius;

//Input
in vec4 fragmentWorldPos;

//output
out vec4 frag_color;



//MAIN
void main(void) 
{
	vec3 finalColor = vec3(0);
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	float intensity = 1.0;

	//GET UV COORDS
	vec2 Uvs = vec2(gl_FragCoord.x / ScreenWidth, gl_FragCoord.y / ScreenHeight);

	//FETCH STUFF FROM MAPS
	vec4 worldPos = texture(GBufferPos, Uvs);
	vec4 normalIn = texture(GBufferNormals, Uvs);
	vec3 diffuse_color = texture(GBufferDiffuse, Uvs).xyz;
	vec3 specular_k = texture(GBufferSpecGloss, Uvs).xyz;
	float specExponent = texture(GBufferSpecGloss, Uvs).w;
	vec4 viewDir = eye - worldPos;

	//Attenuation stuff
	float edgeConstant = 1.0 / radius;
	vec3 objToLight = lightWorldPos.xyz - worldPos.xyz;
	float distToLight = length(objToLight) + 0.001; //So its never zero
	float attenuation = radius / (distToLight*distToLight) - edgeConstant;
	//attenuation = clamp(attenuation, 0.0f, 1.0f);
	intensity *= attenuation;
	if (distToLight > radius)
		discard;

	vec4 m = normalize(normalIn);
	vec4 L = vec4(objToLight.xyz / distToLight, 0);

	//DIFFUSE
	diffuse = max(dot(m, L), 0) * diffuse_color * lightColor.xyz * intensity;

	vec4 v = normalize(viewDir);
	vec4 h = normalize(L + v);
	specular += pow(max(dot(h,m),0), specExponent) * specular_k * lightColor * intensity;
	

	//FINAL COLOR
    finalColor = (diffuse + specular); 
	frag_color = vec4(finalColor.xyz ,1);
}

