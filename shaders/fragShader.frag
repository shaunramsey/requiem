// #define ShaderToy
#version 450

#ifndef ShaderToy
layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform ShaderData{
    vec2 iResolution;
    vec2 rotation;
    vec3 camera;
    float time;
} data;
#else //ShaderToy Defined
struct ShaderData{
    vec2 iResolution;
    vec2 rotation;
    vec3 camera;
    float time;
} data;
#endif //ShaderToyDefined

#define Skin(object) Material(vec3(0.0), vec3(0.9, 0.75, 0.58), vec3(0.5), 5.0, object)

const float shadow_dist = 0.01; // 0 - 1
const float shadow_sharpness = 5.0; // 5-50
#define HIT_EPS 0.001
const int max_steps = 256;
const float max_dist = 100.0;
const vec2 EPS = vec2(0.01, 0.0);

// Returns a pseudo-random float in [0, 1)
float hash(vec2 p) {
    // Dot product scramble and fract for randomness
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

struct Material{
    vec3 ambientColor;
    float ambient_strength;
    vec3 diffuse;
    float diffuse_strength;
    vec3 specular;
    float specular_strength;
    float shininess;
    float dist;
};

Material m1 = Material(vec3(1.0, 0.5, 0.0), 0.1, vec3(1.0, 0.5, 0.0), 0.5, vec3(1.0), 0.5, 100, 10);

// utils
float sdSphere(vec3 p, float s){
    return length(p) - s;
}

float sdEllipsoid(vec3 p, vec3 r){
    float k1 = length(p/r);
    float k2 = length(p/(r*r));
    return k1*(k1-1.0)/k2;
}

float sdCapsule( vec3 p, vec3 a, vec3 b, float r ){
  vec3 pa = p - a, ba = b - a;
  float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return length( pa - ba*h ) - r;
}

float sdCappedCylinder( vec3 p, float h, float r )
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - vec2(r,h);
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float sdBox(vec3 p, vec3 b){
    vec3 q = abs(p) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float opUnion(float d1, float d2){
    return min(d1,d2);
}

float opSubtraction(float d1, float d2){
    return max(-d1,d2);
}

float opIntersection(float d1, float d2){
    return max(d1,d2);
}

float opSmoothUnion(float d1, float d2, float k){
    float h = clamp(0.5 + 0.5 * (d2-d1)/k, 0.0,1.0);
    return mix(d2, d1, h) - k*h*(1.0-h);
}

float opSmoothSub(float d1, float d2, float k){
    float h = clamp(0.5 - 0.5 * (d2+d1)/k, 0.0,1.0);
    return mix(d2, -d1, h) - k*h*(1.0-h);
}

float opSmoothIntersection(float d1, float d2, float k){
    float h = clamp(0.5 - 0.5 * (d2-d1)/k, 0.0,1.0);
    return mix(d2, d1, h) - k*h*(1.0-h);
}

/// if k 
float smin(float a, float b, float k){
    float h = max(k-abs(a-b), 0.0) / k;
    return min(a,b) - h*h*h*k*1.0/6.0;
}

float smax(float a, float b, float k){
	return 0.5 * ((a + b) + sqrt(((a-b)*(a-b)) + k));
}

mat2 rot2D(float angle){ // omit axis of rotation
    float s = sin(angle);
    float c = cos(angle);
    return mat2(vec2(c,-s),vec2(s,c));
}

//Lighting
struct LightSource {
    vec3 pos;
    vec3 color;
    float energy;
};
                                    //pos, color, energy
LightSource Light1 = LightSource( vec3(-20.0, 5.0, -5.0), vec3(0.0, 1.0, 0.0), 0.5);

LightSource Light2 = LightSource(vec3(20.0, 5.0, -5.0), vec3(1.0, 0.0, 0.0), 0.5);

float sdPlane(vec3 p, vec3 n, float h) {
    return dot(p,n) + h;
}

vec2 map(vec3 p){
    vec3 origin = vec3(0.,0.,0.);
	float map = sdSphere(p, 1.0);
	return vec2(map, 1.0);
}

vec3 get_normal(vec3 p){
	vec3 normal = vec3(
		map(p + EPS.xyy).x - map(p - EPS.xyy).x,
		map(p + EPS.yxy).x - map(p - EPS.yxy).x,
		map(p + EPS.yyx).x - map(p - EPS.yyx).x
	);
	return normalize(normal);
}

float get_shadow(vec3 p, vec3 normal, float sharpness, LightSource light){
    float shadow = 1.0;
    p += normal * shadow_dist; // bias "shadow bias"
    float shadow_distance = 0.0;
    vec3 light_dir = normalize(light.pos - p);
    for (int i = 0; i< max_steps; i++){
        // p = ro + rd * t
        vec3 shadow_point = p + light_dir * shadow_distance;
        vec2 hit = map(shadow_point);
        if(abs(hit.x) < HIT_EPS){
            return 0.0;
        }
        if(shadow_distance > max_dist){
            break;
        }
        shadow_distance += hit.x;
        shadow = min(shadow, sharpness * hit.x / shadow_distance);
    }

    return shadow;
}


//material index is currently unusued
vec3 get_light(float mat_index, vec3 p, vec3 rd, vec3 normal, LightSource light){
    Material mat = m1; //should grab from mat_index later
    vec3 light_dir = normalize(light.pos - p);
    vec3 ambient = mat.ambientColor * mat.ambient_strength;
    vec3 diffuse = max(0.0, dot(normal, light_dir)) * mat.diffuse * light.color * light.energy * mat.diffuse_strength;
    vec3 reflection = reflect(light_dir, normal);
    float specular_angle = max(0.0, dot(reflection, rd));
    float specular_power = clamp(pow(specular_angle, mat.shininess), 0.0, 1.0);
    vec3 specular = specular_power * mat.specular * light.energy * mat.specular_strength;
    return ambient + (diffuse + specular) * get_shadow(p, normal, shadow_sharpness, light);
}

vec4 ray_march(vec3 ro, vec3 rd){
    float t = 0.0;
    for(int i = 0; i < max_steps; i++){
        vec3 p = ro + rd * t;
        vec2 hit = map(p);
        t += hit.x;
        if(abs(hit.x) < HIT_EPS){
            vec3 normal = get_normal(p);
            //vec3 get_light(float mat_index, vec3 p, vec3 rd, vec3 normal, LightSource light){
            vec3 color = get_light(hit.y, p, rd, normal, Light1);
            color += get_light(hit.y, p, rd, normal, Light2);
            return vec4(color, 1.0);
        }
        if(t > max_dist) break;
    }
	return vec4(0.0, 0.0, 0.0, 1.0);
}



void main() {

    vec2 fragCoord = gl_FragCoord.xy;
    vec2 res = data.iResolution;
    
    vec2 st = gl_FragCoord.xy / data.iResolution.xy * 2.0 - 1.0;
    st.x *= data.iResolution.x / data.iResolution.y;
    st.y = -st.y;

    vec3 ro = data.camera; //0,0,-3
    vec3 rd = normalize(vec3(st, 1));
    
    outColor = ray_march(ro, rd);
    // vec3 col = 0.5 + 0.5*cos(data.time + uv.xyx + vec3(0,2,4));
    // outColor = vec4(col,1.0);
}
