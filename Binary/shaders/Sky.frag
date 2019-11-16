//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

vec3 gammaCorrection(float exposure, vec3 SRGBcolor);

//Uniforms
uniform sampler2D skyMap;

in VS_OUT
{
	vec4 nml;
	vec2 uv;
};

out vec4 fragColor;

void main()
{
	float exposure = 3;
	vec3 srgbColor = texture2D(skyMap, uv).xyz;
	
	//fragColor = vec4(srgbColor, 1);
	fragColor = vec4(gammaCorrection(exposure, srgbColor), 1);
}


//Gamma correction
vec3 gammaCorrection(float exposure, vec3 SRGBcolor)
{
	float gamma = 1.0 / 2.2;
	vec3 cout = (exposure * SRGBcolor) / (exposure * SRGBcolor + vec3(1, 1, 1));
	cout.r = pow(cout.r, gamma);
	cout.g = pow(cout.g, gamma);
	cout.b = pow(cout.b, gamma);
	return cout;
}