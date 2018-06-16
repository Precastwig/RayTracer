#define MAX_RAY_COUNT = 32;

float3 normalize(float3 in) {
    float l2 = (in.x*in.x) + (in.y*in.y) + (in.z*in.z);
    if (l2 > 0) {
        float invL = 1 / SQRT(l2);
        return (in.x * invL,
                in.y * invL,
                in.z * invL);
    }
}

float3 crossProduct(float3 u, float3 v) {
    return (float3)(u.y * v.z - u.z * v.y, 
                    u.z * v.x - u.x * v.z,
                    u.x * v.y - u.y * v.x)
}

float dotProduct(float3 u, float3 v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

float distance(float3 u, float3 v) const {
    return (u.x * v.x) * (u.x * v.x) + (u.y * v.y) * (u.y * v.y) + (u.z * v.z) * (u.z * v.z);
}

float3 abs(float3 u) {
    return (float3)(fabs(u.x),fabs(u.y),fabs(u.z));
}

typedef enum primType {
    TRIANGLE = 0,
    SPHERE = 1
} prim_type;

typedef enum rayType {
    ORIGIN = 0,
    REFLECTED = 1
} ray_type;

typedef struct {
    // Triangle stuff    
    float3 p1,p2,p3;
    float3 n1,n2,n3;
    // Sphere stuff
    float radius, radius2;
    float3 midpoint;
    // All stuff
    prim_type type;
    float3 shininess;
    float3 transparency;
    float3 dColour;
    float3 sColour;
} Primitive;

typedef struct {
    float3 pos;
    float3 vec;
    ray_type type;
} Ray;

__kernel void opencl_kernel (
    const int xRes,
    const int yRes,
    float3 cameraPos,
    float fov,
    global float3 *global_prims,
    const int n_prims,
    local float3 *local_prims,
    global float3 *Colour) {

    // Copy primitives to local memory
    event_t events[1];
    events[0] = async_work_group_copy(local_prims, global_prims, (size_t) ((sizeof(Primitive) * n_prims) / sizeof(float3)), 0);
    wait_group_events(1, events);

    local Primitive *primitives = (local Primitive *) local_prims;
    // Work out x and y pixels of thread
    const int gid = get_global_id(0);
    const int x = gid % xRes;
    const int y = gid % yRes; // MAYBE WRONG, MAYBE gid / xRes WE WILL SEE

    // Check out of bounds
    if (x >= xRes || y >= yRes)
        return;

    Ray queue[MAX_RAY_COUNT];
    int rays_in_queue = 0;
    int front_ray_ptr = 0;
    int back_ray_ptr = 0;

    float3 col = (float3)(0,0,0);

    // Find the rays parametric equation
    float aspectRatio = (float)xRes / (float) yRes;
    float scale = tan(fov / 2.0);

    float pixelX = (2 * ((x + 0.5) / (float)xRes) - 1) * aspectRatio * scale;
    float pixelY = (1 - 2 * (y + 0.5) / (float)cam.yRes) * scale;
    Ray r;
    r.pos = cameraPos;
    r.vec = (float3)(pixelX,pixelY,-1);
    r.vec = normalize(r.vec);
    r.type = ORIGIN;

}