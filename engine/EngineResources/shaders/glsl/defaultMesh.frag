
out vec4 FragColor;


in vec3 WorldFragPos;
in vec4 WorldFragPosLightSpace;
in vec3 WorldCameraPos;
in vec3 TangentFragPos;
in vec3 TangentLightPos[4];
in vec3 TangentCameraPos;
in vec2 TexCoords;
in vec3 localNormal;

in mat3 TBN;

uniform sampler2D shadowMap;
uniform samplerCube shadowMapPoint;
uniform samplerCube skyboxTexture;

uniform bool useAlbedoTexture;
uniform sampler2D albedoTexture;

uniform bool useNormalTexture;
uniform sampler2D normalTexture;

uniform float farPlane;

uniform int LightsCount;
uniform vec3 LightPositions[4];
uniform vec3 LightColors[4];

uniform bool UseDirectionalLight;
uniform bool UseEnvironmentMapping;
uniform vec3 DirectionalLightColour;
uniform vec3 DirectionalLightDir;

const float pi = 3.141592653589;
const vec3 albedo = vec3(0.83, 0.68, 0.22);
const float metallic = 0.0;
const float roughness = 0.5;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (1.0 + NdotH2 * (a2 - 1.0));
    denom = pi * denom * denom;

    return nom / denom;
}

// RTR4 9.53
// https://google.github.io/filament/Filament.html#listing_anisotropicbrdf
float DistributionGGX_Aniso(vec3 N, vec3 T, vec3 B, vec3 H, float roughnessX, float roughnessY)
{
    float NdotH = dot(N, H);
    float ab = roughnessX;
    float at = roughnessY;

    float TdotH = dot(T, H);
    float BdotH = dot(B, H);
    float a2 = at * ab;
    vec3 v = vec3(ab * TdotH, at * BdotH, a2 * NdotH);
    float v2 = dot(v, v);
    float w2 = a2 / v2;
    return a2 * w2 * w2 * (1.0 / pi);
}

float DistributionBeckmann(vec3 N, vec3 H, float roughness)
{

    float NdotH = max(dot(N, H), 0.00001);// add some margin to avoid divide-by-zero
    float cos2Alpha = NdotH * NdotH;
    float tan2Alpha = (cos2Alpha - 1.0) / cos2Alpha;
    float roughness2 = roughness * roughness;
    float denom = pi * roughness2 * cos2Alpha * cos2Alpha;
    return exp(tan2Alpha / roughness2) / denom;

    /*
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.00001);// add some margin to avoid divide-by-zero
    float NdotH2 = NdotH*NdotH;
    float NdotH4 = NdotH2*NdotH2;

    float nom = NdotH;
    float denom = pi * a2 * NdotH4;
    float nom_exp = NdotH2 - 1.0;
    float denom_exp = a2 * NdotH2;

    return (nom/denom) * exp(nom_exp/denom_exp);*/
}

float GeometryOriginalCookTorrance(vec3 N, vec3 V, vec3 L)
{
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    float max1 = max((2.0*NdotH*NdotV)/(VdotH), (2.0*NdotH*NdotL)/(VdotH));
    return max(1.0, max1);

}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlickApprox(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 lambertian_brdf(vec3 in_direction, vec3 out_direction, vec3 normal)
{
    if (useAlbedoTexture)
    {
        return texture(albedoTexture, TexCoords).rgb/pi;
    }
    else
    {
        return albedo/pi;
    }
}

// https://www.shadertoy.com/view/ldBGz3
// https://garykeen27.wixsite.com/portfolio/oren-nayar-shading
float oren_nayar_brdf(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal) {
    float r2 = roughness*roughness;
    float a = 1.0 - 0.5*(r2/(r2+0.57));
    float b = 0.45*(r2/(r2+0.09));

    float nl = dot(surfaceNormal, lightDirection);
    float nv = dot(surfaceNormal, viewDirection);

    float ga = dot(viewDirection-surfaceNormal*nv, surfaceNormal-surfaceNormal*nl);

    return max(0.0, nl) * (a + b*max(0.0, ga) * sqrt((1.0-nv*nv)*(1.0-nl*nl)) / max(nl, nv));
}

vec3 blinn_phong_brdf(vec3 in_direction, vec3 out_direction, vec3 normal){

    vec3 halfwayVector = normalize(out_direction + in_direction);

    float kL = roughness;
    float kG = 1.0 - kL;
    float s = 16.0;
    vec3 pL;
    if (useAlbedoTexture)
    {
        pL = texture(albedoTexture, TexCoords).rgb;
    }
    else
    {
        pL = albedo;
    }
    vec3 pG = vec3(1.0);

    vec3 retVec = kL * (pL / pi) + kG *(pG*((8.0+s)/(8.0*pi))* pow(max(dot(normal, halfwayVector), 0.0), s));
    return retVec;

}

vec3 cook_torrance_brdf(vec3 in_direction, vec3 out_direction, vec3 normal){

    // The "metallic workflow"
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 halfwayVector = normalize(out_direction + in_direction);

    //float NDF = DistributionBeckmann(normal, halfwayVector, roughness);
    float NDF = DistributionGGX(normal, halfwayVector, roughness);

    //float G   = GeometryOriginalCookTorrance(normal, out_direction, in_direction);
    float G   = GeometrySmith(normal, out_direction, in_direction, roughness);

    vec3 F    = fresnelSchlickApprox(clamp(dot(halfwayVector, out_direction), 0.0, 1.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, out_direction), 0.0) * max(dot(normal, in_direction), 0.0) + 0.0001;// add some margin to avoid divide-by-zero
    vec3 specular = numerator / denominator;

    // kS is equal to the fresnel
    vec3 kS = F;

    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;

    return kD * albedo / pi + specular;

}

float SampleShadowMap(sampler2D shadowMap, vec2 coords, float compare)
{
    return step(compare, texture(shadowMap, coords.xy).r);
}

float SampleShadowMapLinear(sampler2D shadowMap, vec2 coords, float compare, vec2 texelSize)
{
    vec2 pixelPos = coords/texelSize + vec2(0.5);
    vec2 fracPart = fract(pixelPos);
    vec2 startTexel = (pixelPos - fracPart) * texelSize;

    float blTexel = SampleShadowMap(shadowMap, startTexel, compare);
    float brTexel = SampleShadowMap(shadowMap, startTexel + vec2(texelSize.x, 0.0), compare);
    float tlTexel = SampleShadowMap(shadowMap, startTexel + vec2(0.0, texelSize.y), compare);
    float trTexel = SampleShadowMap(shadowMap, startTexel + texelSize, compare);

    float mixA = mix(blTexel, tlTexel, fracPart.y);
    float mixB = mix(brTexel, trTexel, fracPart.y);

    return mix(mixA, mixB, fracPart.x);
}


float ShadowCalculationPoint(vec3 fragPos, vec3 lightPos)
{
    vec3 worldPosToLight = fragPos - lightPos;

    // Sample from the cube shadow texture
    float closestDepth = texture(shadowMapPoint, worldPosToLight).r;

    // Re-map from 0 to 1 back to 0 to far plane linearly
    closestDepth *= farPlane;

    // The current depth from the light source
    float currentDepth = length(worldPosToLight);

    float bias = 2.5;
    float shadow = currentDepth - bias > closestDepth ? 0.0 : 1.0;

    return shadow;
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 N, vec3 L)
{
    // Perspective division
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Re-map to 0 to 1 range
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
    {
        // When sampling outside of the lights frustum
        return 1.0;
    }

    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // The current depth from the light source from the lights perspective
    float currentDepth = projCoords.z;

    // Slightly add bias for mitigating shadow acne further
    float bias = max(0.001 * (1.0 - dot(N, -L)), 0.00015);

    ivec2 textureSize2d = textureSize(shadowMap, 0);
    float sizeTexture = float(textureSize2d.x);
    float texelSize = 1.0 / sizeTexture;

    float shadow = 0.0;
    // Blur the shadows by sampling close by coordinates in the shadow map
    const int halfkernelWidth = 2;
    for (int x = -halfkernelWidth; x <= halfkernelWidth; ++x)
    {
        for (int y = -halfkernelWidth; y <= halfkernelWidth; ++y)
        {
            shadow += SampleShadowMapLinear(shadowMap, projCoords.xy + vec2(x, y) * texelSize, currentDepth - bias, vec2(texelSize));
        }
    }
    shadow /= ((float(halfkernelWidth)*2.0+1.0)*(float(halfkernelWidth)*2.0+1.0));

    return shadow;
}

float CalculateAttenuation(float distance, float constant, float linear, float quadratic)
{
    // For finding reasonable values, see:
    // https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation

    return distance / (constant + linear * distance + quadratic * (distance*distance));
}

void main()
{

    vec3 N;
    if (useNormalTexture)
    {
        vec3 normal = texture(normalTexture, TexCoords).rgb;
        N = normalize(normal * 2.0 - 1.0);
    } else
    {
        N = vec3(0.0, 0.0, 1.0);
    }
    vec3 V = normalize(TangentCameraPos - TangentFragPos);

    vec3 LightOutTotal = vec3(0.0);
    for (int l = 0; l < LightsCount; ++l)
    {
        vec3 L = normalize(TangentLightPos[l] - TangentFragPos);

        // Incident angle
        float NdotL = max(dot(N, L), 0.0);

        // Incoming radiance from the light source
        float distance = length(TangentLightPos[l] - TangentFragPos);
        float attenuation = CalculateAttenuation(distance, 1.0, 0.35, 0.44);
        vec3 radiance = LightColors[l] * attenuation * ShadowCalculationPoint(WorldFragPos, LightPositions[l]);

        //LightOutTotal += radiance * blinn_phong_brdf(L, V, N) * NdotL;
         LightOutTotal += radiance * lambertian_brdf(L, V, N) * NdotL;
        // LightOutTotal += radiance * cook_torrance_brdf(L, V, N) * NdotL;
        // LightOutTotal += radiance * albedo * oren_nayar_brdf(L, V, N) * NdotL;

    }

    vec3 worldView = normalize(WorldCameraPos - WorldFragPos);
    vec3 worldSpaceNormal = transpose(TBN) * N;

    if (UseDirectionalLight)
    {
        vec3 L = normalize(DirectionalLightDir);

        // Incident angle
        float NdotL = max(dot(worldSpaceNormal, L), 0.0);

        // Incoming radiance, depends on shadows from other objects
        vec3 radiance = DirectionalLightColour;// * ShadowCalculation(WorldFragPosLightSpace, N, L);

        LightOutTotal += radiance * lambertian_brdf(L, worldView, worldSpaceNormal) * NdotL;

    }

    if(UseEnvironmentMapping)
    {
        vec3 R = reflect(-worldView,normalize(worldSpaceNormal));
        vec3 environmentColor = texture(skyboxTexture, R).xyz;
        float reflectanceCoeff = roughness;
        LightOutTotal = reflectanceCoeff * environmentColor + (1.0-reflectanceCoeff)*LightOutTotal;
    }

    // HDR tonemapping
    //LightOutTotal = LightOutTotal / (LightOutTotal + vec3(1.0));

    // Gamma correction
    //LightOutTotal = pow(LightOutTotal, vec3(1.0/2.2));

    FragColor = vec4(LightOutTotal, 1.0);
}



