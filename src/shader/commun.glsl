//#define MAP_TORUS
//#define MAP_SPHERE
//#define MAP_SPHERESPHERE
//#define MAP_PLAN
#define MAP_ARLO
//#define MAP_FISH
//#define MAP_ELEPHANT

//###################VERTEX ATTRIBUTE#################

struct vertexAttribut{
    vec3 uvw;
    mat3 repere;
    vec3 cpos;
};

#define VA vertexAttribut

VA interpolation(VA a, VA b, float t){
    return vertexAttribut(
        (1-t)*a.uvw + t*b.uvw
        ,(1-t)*a.repere + t*b.repere
        ,(1-t)*a.cpos + t*b.cpos
    );
}

VA divise(VA a, float w){
    return vertexAttribut(
        a.uvw / w
        ,a.repere / w
        ,a.cpos / w
    );
}

VA barycentre(VA a, VA b, VA c, float la, float lb, float lc){
    return VA(
        a.uvw * la + b.uvw * lb + c.uvw * lc
        ,a.repere * la + b.repere * lb + c.repere * lc
        ,a.cpos * la + b.cpos * lb + c.cpos * lc
    );
}

//###################UTILS##################################

vec3 angleToVec(vec2 thetaPhi){
  return vec3(sin(thetaPhi[0])*cos(thetaPhi[1]),
              sin(thetaPhi[0])*sin(thetaPhi[1]),
              cos(thetaPhi[0]));
}

uint randstate;

uint randu() {
    randstate = randstate * 747796405u + 2891336453u;
    uint value = ((randstate >> ((randstate >> 28u) + 4u)) ^ randstate) * 277803737u;
    return (value >> 22u) ^ value;
}

float randf() {
    return randu() / 4294967296.0;
}

vec3 randColor(uint t){
  randstate = t;
  return vec3(randf(),randf(),randf());
}



//#################### VOXEL ###############################



vec2 sphIntersect( in vec3 ro, in vec3 rd, in vec4 sph )
{
	vec3 oc = ro - sph.xyz;
	float b = dot( oc, rd );
	float c = dot( oc, oc ) - sph.w*sph.w;
	float h = b*b - c;
	if( h<0.0 ) return vec2(-1.0);
    h = sqrt( h );
	return vec2(-b - h, -b+h);
}

// https://iquilezles.org/articles/distfunctions
vec2 sdSegment( in vec3 p, vec3 a, vec3 b )
{
	vec3 pa = p - a, ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return vec2( length( pa - ba*h ), h );
}

// https://iquilezles.org/articles/distfunctions
float sdSphere( in vec3 p, in vec3 c, in float r )
{
    return length(p-c) - r;
}

// https://iquilezles.org/articles/distfunctions
float sdEllipsoid( in vec3 p, in vec3 c, in vec3 r )
{
    return (length( (p-c)/r ) - 1.0) * min(min(r.x,r.y),r.z);
}

// http://research.microsoft.com/en-us/um/people/hoppe/ravg.pdf
float det( vec2 a, vec2 b ) { return a.x*b.y-b.x*a.y; }
vec3 getClosest( vec2 b0, vec2 b1, vec2 b2 ) 
{
    float a =     det(b0,b2);
    float b = 2.0*det(b1,b0);
    float d = 2.0*det(b2,b1);
    float f = b*d - a*a;
    vec2  d21 = b2-b1;
    vec2  d10 = b1-b0;
    vec2  d20 = b2-b0;
    vec2  gf = 2.0*(b*d21+d*d10+a*d20); gf = vec2(gf.y,-gf.x);
    vec2  pp = -f*gf/dot(gf,gf);
    vec2  d0p = b0-pp;
    float ap = det(d0p,d20);
    float bp = 2.0*det(d10,d0p);
    float t = clamp( (ap+bp)/(2.0*a+b+d), 0.0 ,1.0 );
    return vec3( mix(mix(b0,b1,t), mix(b1,b2,t),t), t );
}

// https://www.shadertoy.com/view/ldj3Wh
vec2 sdBezier( vec3 a, vec3 b, vec3 c, vec3 p )
{
	vec3 w = normalize( cross( c-b, a-b ) );
	vec3 u = normalize( c-b );
	vec3 v = normalize( cross( w, u ) );

	vec2 a2 = vec2( dot(a-b,u), dot(a-b,v) );
	vec2 b2 = vec2( 0.0 );
	vec2 c2 = vec2( dot(c-b,u), dot(c-b,v) );
	vec3 p3 = vec3( dot(p-b,u), dot(p-b,v), dot(p-b,w) );

	vec3 cp = getClosest( a2-p3.xy, b2-p3.xy, c2-p3.xy );

	return vec2( sqrt(dot(cp.xy,cp.xy)+p3.z*p3.z), cp.z );
}

// https://iquilezles.org/articles/smin
float smin( float a, float b, float k )
{
    float h = max(k-abs(a-b),0.0);
    return min(a, b) - h*h*0.25/k;
}

// https://iquilezles.org/articles/smin
float smax( float a, float b, float k )
{
    float h = max(k-abs(a-b),0.0);
    return max(a, b) + h*h*0.25/k;
}

#define ZERO (min(iFrame,0))

//---------------------------------------------------------------------------

mat3 base( in vec3 ww )
{
    vec3  vv  = vec3(0.0,0.0,1.0);
    vec3  uu  = normalize( cross( vv, ww ) );
    return mat3(uu.x,ww.x,vv.x,
                uu.y,ww.y,vv.y,
                uu.z,ww.z,vv.z);
}

mat3 setCamera( in vec3 ro, in vec3 rt, in float cr )
{
	vec3 cw = normalize(rt-ro);
	vec3 cp = vec3(sin(cr), cos(cr),0.0);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
    return mat3( cu, cv, -cw );
}

//---------------------------------------------------------------------------

vec2 leg( in vec3 p, in vec3 pa, in vec3 pb, in vec3 pc, float m, float h )
{
    float l = sign(pa.z);
    
    vec2 b = sdSegment( p, pa, pb );

    float d3 = b.x - 0.15;
    b = sdSegment( p, pb, pc );
    d3 = smin( d3, b.x - 0.15, 0.1 );

    // knee
    float d4 = sdEllipsoid( p, pb+vec3(-0.02,0.05,0.0), vec3(0.14) );
    //d4 -= 0.01*abs(sin(50.0*p.y));
    d4 -= 0.015*abs(sin(40.0*p.y));
    d3 = smin( d3, d4, 0.05 );

    // paw        
    vec3 ww = normalize( mix( normalize(pc-pb), vec3(0.0,1.0,0.0), h) );
    mat3 pr = base( ww );
    vec3 fc = pr*((p-pc))-vec3(0.2,0.0,0.0)*(-1.0+2.0*h);
    d4 = sdEllipsoid( fc, vec3(0.0), vec3(0.4,0.25,0.4) );

    // nails
    float d6 = sdEllipsoid( fc, vec3(0.32,-0.06,0.0)*(-1.0+2.0*h), 0.95*vec3(0.1,0.2,0.15));
    d6 = min( d6, sdEllipsoid( vec3(fc.xy,abs(fc.z)), vec3(0.21*(-1.0+2.0*h),-0.08*(-1.0+2.0*h),0.26), 0.95*vec3(0.1,0.2,0.15)) );
    // space for nails
    d4 = smax( d4, -d6, 0.03 );

    // shape paw
    float d5 = sdEllipsoid( fc, vec3(0.0,1.85*(-1.0+2.0*h),0.0), vec3(2.0,2.0,2.0) );
    d4 = smax( d4, d5, 0.03 );
    d6 = smax( d6, d5, 0.03 );
    d5 = sdEllipsoid( fc, vec3(0.0,-0.75*(-1.0+2.0*h),0.0), vec3(1.0,1.0,1.0) );
    d4 = smax( d4, d5, 0.03 );
    d6 = smax( d6, d5, 0.03 );

    d3 = smin( d3, d4, 0.1 );
    
    // muslo
    d4 = sdEllipsoid( p, pa+vec3(0.0,0.2,-0.1*l), vec3(0.35)*m );
    d3 = smin( d3, d4, 0.1 );

	return vec2(d3,d6);
}


// make all these zero for the rest position
const float headOffCenter = 0.0;
const vec3  headAngle = vec3(-0.0,-0.0,0);

vec3 headTransform( in vec3 p )
{
    const vec3 ce = vec3(-2.6,1.7,headOffCenter);
    
    p -= ce;
    p.xy = mat2(cos(headAngle.x),sin(headAngle.x),-sin(headAngle.x),cos(headAngle.x))*p.xy;
    p.yz = mat2(cos(headAngle.y),sin(headAngle.y),-sin(headAngle.y),cos(headAngle.y))*p.yz;
    p.xz = mat2(cos(headAngle.z),sin(headAngle.z),-sin(headAngle.z),cos(headAngle.z))*p.xz;
    p += ce;
    p.z -= headOffCenter;
    
    return p;
}

vec2 sd2Segment( vec3 a, vec3 b, vec3 p )
{
	vec3  pa = p - a;
	vec3  ba = b - a;
	float t = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	vec3  v = pa - ba*t;
	return vec2( dot(v,v), t );
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

//-----------------------------------------------------------------------------------

vec2 sdBezier( vec3 a, vec3 b, vec3 c, vec3 p, out vec2 pos )
{
	vec3 w = normalize( cross( c-b, a-b ) );
	vec3 u = normalize( c-b );
	vec3 v = normalize( cross( w, u ) );

	vec2 a2 = vec2( dot(a-b,u), dot(a-b,v) );
	vec2 b2 = vec2( 0.0 );
	vec2 c2 = vec2( dot(c-b,u), dot(c-b,v) );
	vec3 p3 = vec3( dot(p-b,u), dot(p-b,v), dot(p-b,w) );

	vec3 cp = getClosest( a2-p3.xy, b2-p3.xy, c2-p3.xy );

    pos = cp.xy;
    
	return vec2( sqrt(dot(cp.xy,cp.xy)+p3.z*p3.z), cp.z );
}

vec2 mapArlo( vec3 p )
{
// bounding volumes
//res.x = min(res.x,length(p-vec3(-0.05,0.45,0.2))-3.5);
//res.x = min(res.x,sdLine(p,vec3(-1.5,0.65,0.4),vec3(1.3,-0.7,0.2)).x-2.28);


    // body
    vec3 q = p;
    float co = cos(0.2);
    float si = sin(0.2);
    q.xy = mat2(co,si,-si,co)*q.xy;
    float d1 = sdEllipsoid( q, vec3(0.0,0.0,0.0), vec3(1.3,0.75,0.8) );
    float d2 = sdEllipsoid( q, vec3(0.05,0.45,0.0), vec3(0.8,0.6,0.5) );
    float d = smin( d1, d2, 0.4 );
    
    //neck wrinkles
    float r = length(p-vec3(-1.2,0.2,0.0));
    d -= 0.05*abs(sin(35.0*r))*exp(-7.0*abs(r)) * clamp(1.0-(p.y-0.3)*10.0,0.0,1.0);

    // tail
    {
    vec2 b = sdBezier( vec3(1.0,-0.4,0.0), vec3(2.0,-0.96,0.0), vec3(3.0,-0.5,0.0), p );
    float tr = 0.3 - 0.25*b.y;
    float d3 = b.x - tr;
    d = smin( d, d3, 0.2 );
    }
    
    // neck
    {
    vec2 b = sdBezier( vec3(-0.9,0.3,0.0), vec3(-2.2,0.5,0.0), vec3(-2.6,1.7,headOffCenter), p );
    float tr = 0.35 - 0.23*b.y;
    float d3 = b.x - tr;
    d = smin( d, d3, 0.15 );
	}

    float dn;
    // front-left leg
    {
    vec2 d3 = leg( p, vec3(-0.8,-0.1,0.5), vec3(-1.0,-0.8,0.65), vec3(-0.7,-1.5,0.65), 0.5, 1.0 );
    d = smin(d,d3.x,0.2);
    dn = d3.y;
    }
    // back-left leg
    {
    vec2 d3 = leg( p, vec3(0.5,-0.4,0.6), vec3(0.3,-1.05,0.6), vec3(0.8,-1.6,0.6), 0.5, 1.0 );
    d = smin(d,d3.x,0.2);
    dn = min(dn,d3.y);
    }
    // front-right leg
    {
    vec2 d3 = leg( p, vec3(-0.8,-0.1,-0.5), vec3(-1.0,-0.8,-0.65), vec3(-0.7,-1.5,-0.65), 0.5, 1.0 );
    d = smin(d,d3.x,0.2);
    dn = min(dn,d3.y);
    }
    // back-right leg
    {
    vec2 d3 = leg( p, vec3(0.5,-0.4,-0.6), vec3(0.3,-1.05,-0.6), vec3(0.8,-1.6,-0.7), 0.5, 1.0 );
    d = smin(d,d3.x,0.2);
    dn = min(dn,d3.y);
    }
        
    // head
    p = headTransform(p);
    vec3 s = vec3(p.xy,abs(p.z));
    {
    vec2 l = sdSegment( p, vec3(-2.7,2.36,0.0), vec3(-2.6,1.7,0.0) );
    float d3 = l.x - (0.22-0.1*smoothstep(0.1,1.0,l.y));
        
    // mouth
    vec3 mp = p-vec3(-2.7,2.16,0.0);
    l = sdSegment( mp*vec3(1.0,1.0,1.0-0.2*abs(mp.x)/0.65), vec3(0.0), vec3(-3.35,2.12,0.0)-vec3(-2.7,2.16,0.0) );
        
    float d4 = l.x - (0.12 + 0.04*smoothstep(0.0,1.0,l.y));      
    float d5 = sdEllipsoid( s, vec3(-3.4,2.5,0.0), vec3(0.8,0.5,2.0) );
    d4 = smax( d4, d5, 0.03 );
        
    d3 = smin( d3, d4, 0.1 );
        
    // mouth bottom
    {
    vec2 b = sdBezier( vec3(-2.6,1.75,0.0), vec3(-2.7,2.2,0.0), vec3(-3.25,2.12,0.0), p );
    float tr = 0.11 + 0.02*b.y;
    d4 = b.x - tr;
    d3 = smin( d3, d4, 0.001+0.06*(1.0-b.y*b.y) );
    }
        
    // brows    
    vec2 b = sdBezier( vec3(-2.84,2.50,0.04), vec3(-2.81,2.52,0.15), vec3(-2.76,2.4,0.18), s+vec3(0.0,-0.02,0.0) );
    float tr = 0.035 - 0.025*b.y;
    d4 = b.x - tr;
    d3 = smin( d3, d4, 0.025 );

    // eye wholes
    d4 = sdEllipsoid( s, vec3(-2.79,2.36,0.04), vec3(0.12,0.15,0.15) );
    d3 = smax( d3, -d4, 0.025 );    
        
    // nose holes    
    d4 = sdEllipsoid( s, vec3(-3.4,2.17,0.09), vec3(0.1,0.025,0.025) );
    d3 = smax( d3, -d4, 0.04 );    
        
    d = smin( d, d3, 0.02 );
    }
    vec2 res = vec2(d,0.0);
    
    // eyes
    float d4 = sdSphere( s, vec3(-2.755,2.36,0.045), 0.16 );
    if( d4<res.x ) res = vec2(d4,1.0);
    
    float te = 0; //textureLod( iChannel0, 3.0*p.xy, 0.0 ).x;
    float ve = normalize(p).y;
    res.x -= te*0.01*(1.0-smoothstep(0.6,1.5,length(p)))*(1.0-ve*ve);
    
    if( dn<res.x )  res = vec2(dn,3.0);

    return res;
}



vec3 randDir3D(ivec3 seed) {
    const int width = 10000;
    randstate = seed.x + seed.y * width + seed.z * width*width;
    return normalize(vec3(randf(), randf(), randf()) *2-1);
}

float cornerValue(vec3 gridUVW, ivec3 corner) {
    ivec3 cell = ivec3(gridUVW) + corner;
    vec3 offset = gridUVW - cell;
    vec3 gradient = randDir3D(cell);
    return dot(offset, gradient);
}

float smootherstep(float a0, float a1, float w) {
    return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}

float perlin(ivec3 gridSize, vec3 uvw) {
    vec3 gridUVW = uvw * gridSize;
    vec3 cellUVW = fract(gridUVW);
    return
    smootherstep(
        smootherstep(
            smootherstep(
                cornerValue(gridUVW, ivec3(0, 0, 0)),
                cornerValue(gridUVW, ivec3(1, 0, 0)),
                cellUVW.x),
            smootherstep(
                cornerValue(gridUVW, ivec3(0, 1, 0)),
                cornerValue(gridUVW, ivec3(1, 1, 0)),
                cellUVW.x),
            cellUVW.y),
        smootherstep(
            smootherstep(
                cornerValue(gridUVW, ivec3(0, 0, 1)),
                cornerValue(gridUVW, ivec3(1, 0, 1)),
                cellUVW.x),
            smootherstep(
                cornerValue(gridUVW, ivec3(0, 1, 1)),
                cornerValue(gridUVW, ivec3(1, 1, 1)),
                cellUVW.x),
            cellUVW.y),
        cellUVW.z);
}

float fbm(ivec3 gridSize, int octaves, vec3 uvw) {
    float value = 0;
    float coef = 1;
    for (int o = 0; o < octaves; o++) {
        value += perlin(gridSize, uvw) * coef;
        gridSize *= 2;
        coef /= 2;
    }
    return value;
}

//############### FONCTION IMPLICITE ########################

float map(vec3 pos){
  #ifdef MAP_TORUS
      vec3 p = pos-0.4863;
      vec2 t = vec2(0.35, 0.1);
      vec2 q = vec2(length(p.xz)+0.00*sin(p.x*1000)-t.x,p.y);
      return length(q)-t.y;
  #endif

  #ifdef MAP_SPHERE
      return length(pos-0.5) - 0.4123;
  #endif

  #ifdef MAP_SPHERESPHERE
      randstate = 0;
      float d = 100;
      for(int i = 0; i < 10; i++){
        vec3 center = normalize(vec3(randf()-0.5,randf()-0.5,randf()-0.5))*0.2+0.5;
        d = min(d, sphere(pos,center,0.2));
      }
      return d;
  #endif
  
  #ifdef MAP_PLAN
    return abs(pos-0.4863+0.2*cos(time)).y;
  #endif

  #ifdef MAP_ARLO
    return mapArlo(pos*4).x/4;
    float corps = mapArlo(pos*4).x/4;
    if (corps > 0) return  (corps + smootherstep(0.1,0.,min(corps/0.019,1))*(fbm(ivec3(40), 3, (pos+1)/2)+0.2))/4;
    return (corps + 0.1*(fbm(ivec3(40), 3, (pos+1)/2)+0.2))/4;
  #endif

  return 0;
}

vec3 map_normal(vec3 p){
    float dx = 0.0001;
    vec3 n = vec3(0);
    float mp = map(p);
    n.x = (mp - map(p-vec3(dx,0,0)))/dx;
    n.y = (mp - map(p-vec3(0,dx,0)))/dx;
    n.z = (mp - map(p-vec3(0,0,dx)))/dx;
    return normalize(n); 
}