//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core
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

	//GET UV COORDS
	vec2 Uvs = vec2(gl_FragCoord.x / 1280, gl_FragCoord.y / 720);

	//FETCH STUFF FROM MAPS
	vec4 worldPos = texture(GBufferPos, Uvs);
	vec4 normalIn = texture(GBufferNormals, Uvs);
	vec3 diffuse_color = texture(GBufferDiffuse, Uvs).xyz;
	vec3 specular_k = vec3(1, 1, 1);
	vec4 viewDir = vec4(texture(GBufferSpecGloss, Uvs).xyz, 0);
	float specExponent = texture(GBufferSpecGloss, Uvs).w;

	//Attenuation stuff
	vec3 objToLight = lightWorldPos.xyz - worldPos.xyz;
	float distToLight = length(objToLight) + 0.00001; //So its never zero
	float attenuation = (radius / (distToLight*distToLight)) - (1.0 / (radius));
	attenuation = clamp(attenuation, 0.0f, 1.0f);
	if (distToLight > radius)
		discard;

	vec4 m = normalize(normalIn);
	vec4 L = vec4(objToLight.xyz / distToLight, 0);

	//DIFFUSE
	diffuse = max(dot(m, L), 0) * diffuse_color * lightColor.xyz * attenuation;

	vec4 v = normalize(viewDir);
	vec4 h = normalize(L + v);
	specular += pow(max(dot(h,m),0), specExponent) * specular_k * lightColor * attenuation;
	
	//FINAL COLOR
    finalColor = diffuse + specular; 
	frag_color = vec4(finalColor.xyz ,1);
}

