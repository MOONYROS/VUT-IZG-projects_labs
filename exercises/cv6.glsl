#ifdef VERTEX_SHADER
precision highp float;

uniform mat4 view = mat4(1);
uniform mat4 proj = mat4(1);

uniform float iTime;

flat out int vM;

int drawQuad(int offset,mat4 vp,mat4 mm,int m){
  vec2 verts[] = vec2[](
    vec2(0,0),
    vec2(1,0),
    vec2(0,1),
    vec2(0,1),
    vec2(1,0),
    vec2(1,1)
  );

  int vID = gl_VertexID - offset;

  if(vID < 0 || vID>=verts.length())return verts.length();

  gl_Position = vp * mm * vec4(verts[vID]*2-1,0.f,1.f);
  vM = m;
  return verts.length();
}

mat4 T(vec3 v){
  mat4 r = mat4(1);
  r[3] = vec4(v,1);
  return r;
}

mat4 Rz(float a){
  mat4 r = mat4(1);
  float ca = cos(a);
  float sa = sin(a);
  r[0][0] = ca;
  r[0][1] = sa;
  r[1][0] = -sa;
  r[1][1] = ca;

  return r;
}

mat4 S(float x){
  mat4 r = mat4(1);
  r[0][0] = x;
  r[1][1] = x;
  r[2][2] = x;
  return r;
}


vec3 getLine(vec2 a,vec2 b){
  vec2 s = b-a;
  vec2 n = vec2(-s.y,s.x);
  return vec3(n,-dot(n,a));
}

float dist(vec3 line,vec2 a){
  return dot(line,vec3(a,1));
}

void main(){
  gl_Position = vec4(0.f,0.f,0.f,1.f);

  mat4 vp = proj * view;

  vec3 center = vec3(20,20,0);
  mat4 modelMatrix = T(center)*Rz(iTime)*T(-center);

  vec2 tri[3] = vec2[](
    modelMatrix*vec4( 3, 3,0,1),
    modelMatrix*vec4(35, 6,0,1),
    modelMatrix*vec4(22,37,0,1)
  );

  vec3 lines[3];
  for(int i=0;i<3;++i)
    lines[i] = getLine(tri[i],tri[(i+1)%3]);

  int offset = 0;
  for(int j=0;j<40;++j)
    for(int i=0;i<40;++i){
      vec2 p = vec2(i,j);
      int inside = int (dist(lines[0],p) > 0 && dist(lines[1],p) > 0 && dist(lines[2],p) > 0);
      offset += drawQuad(offset,vp,T(vec3(i-20,j-20,0))*S(.4),inside);
    }

}
#endif



#ifdef FRAGMENT_SHADER
precision highp float;

out vec4 fColor;

flat in int vM;

void main(){
  if(vM==0)fColor = vec4(vec3(1),1);
  if(vM==1)fColor = vec4(vec3(0),1);
}

#endif