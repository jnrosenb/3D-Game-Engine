//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

#define DELTA	0.00001
#define R		1.0
#define PI		3.14159265

layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
	int ScreenWidth;
	int ScreenHeight;
};

//Function for sampling points used for AO
vec2 GetSpiralSampleUVCoord(int i, float phi, vec2 uv, float depth, float radius, int n);

//Texture we are writting to
layout (location = 0) out float Out_AOBuffer;

//Uniform samplers for AO calculation
uniform sampler2D GBufferPos;
uniform sampler2D GBufferNormals;

//Just for checking if this works
uniform mat4 view;


//Parameters to modify
float scale = 3.0;
float contrast = 2.0;


//MAIN
void main(void) 
{
	//(since Im using a FSQ, I can just ignore this I think)
	ivec2 coords = ivec2(gl_FragCoord.x, gl_FragCoord.y);
	vec2 uvs = vec2(gl_FragCoord.x / ScreenWidth, gl_FragCoord.y / ScreenHeight);
	float phi = (30*coords.x ^ coords.y) + 10*coords.x*coords.y;

	//WorldPos and Normal for the Center point
	vec4 P = texture(GBufferPos, uvs);
	vec4 N = texture(GBufferNormals, uvs);
	
	//Falloff 1 val point
	float c = 0.1 * R;

	//Number of samples
	int n = 10;

	//Integrate the n sample points
	float sum = 0.0f;
	for (int i = 0; i < n; ++i)
	{
		//Main pixel depth
		float d = (view * P).z;
		float d_world = P.z;
		
		//Get a ray wi by sampling a spiral
		vec2 newUV = GetSpiralSampleUVCoord(i, phi, uvs, d, R, n);
		
		//Get position and depth info from the direction we are pointing at
		vec4 Pi = texture2D(GBufferPos, newUV);
		float di_world = Pi.z;
		vec4 wi = Pi - P;

		if( (R - length(wi)) > 0.0 )
			sum += max(0.0, dot(N, wi) - DELTA*di_world) / max(c*c, dot(wi, wi));
			//sum += max(0.0, dot(N, wi) - DELTA*di_world) / max(c*c, dot(wi, wi));
	}

	//Get and write the final ambient occlusion value
	float S = sum * (2*PI*c) / n;
	float A = max(0.0, pow(1.0 - scale*S, contrast));
	Out_AOBuffer = A;
}


vec2 GetSpiralSampleUVCoord(int i, float phi, vec2 uv, float d, float radius, int n)
{	
	float alpha = (i + 0.5) / n;
	float h = alpha * radius / d;
	float theta = 2*PI*alpha*(7*n/9) + phi;

	//HERE I USE FSQ UV's. CHECK LATER IF BETTER TO USE THE glfrag/size FORMULA!
	vec2 newUV = uv + h * vec2(cos(theta), sin(theta));
	return newUV;
}