#include "../core/Seagull.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <chrono>
using namespace sg;
int checks=0;
void check(bool ok,const char*name){++checks;if(!ok){std::cerr<<"FAIL: "<<name<<'\n';std::exit(1);}std::cout<<"PASS: "<<name<<'\n';}
State scene(){State s;s.valid=true;s.confidence=1;s.table={0,0,200,400,{{0,0},{200,0},{0,200},{200,200},{0,400},{200,400}},10};s.balls={{{100,300},5,cue,1},{{100,180},5,solid,1}};return s;}
struct Fixture{
 int w=320,h=600,stride=1296;std::vector<uint8_t> p;
 Fixture():p(stride*h,0){rect(0,0,w,h,90,65,55);rect(30,40,290,560,28,125,112);for(V c:std::vector<V>{{30,40},{290,40},{30,300},{290,300},{30,560},{290,560}})circle(c,13,8,8,8);}
 void pixel(int x,int y,int r,int g,int b){if(x<0||y<0||x>=w||y>=h)return;auto i=y*stride+x*4;p[i]=b;p[i+1]=g;p[i+2]=r;p[i+3]=255;}
 void rect(int x0,int y0,int x1,int y1,int r,int g,int b){for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x)pixel(x,y,r,g,b);}
 void circle(V c,double rad,int r,int g,int b){for(int y=int(c.y-rad);y<=c.y+rad;++y)for(int x=int(c.x-rad);x<=c.x+rad;++x)if(length(V{double(x),double(y)}-c)<=rad)pixel(x,y,r,g,b);}
 State detect(){return detectBGRA(p.data(),w,h,stride);}
};
int main(){
 check(std::abs(rayCircle({0,0},{1,0},{10,0},2)-8)<1e-8,"ray first contact");
 check(std::abs(rayCircle({0,2},{1,0},{10,0},2)-10)<1e-8,"tangent ray");
 check(rayCircle({0,0},{-1,0},{10,0},2)>1e20,"behind ray rejected");
 check(rayCircle({0,0},{0,0},{10,0},2)>1e20,"zero ray rejected");
 check(std::abs(segmentDistance({5,3},{0,0},{10,0})-3)<1e-9,"segment perpendicular distance");
 check(std::abs(segmentDistance({12,0},{0,0},{10,0})-2)<1e-9,"segment endpoint clamp");
 auto s=scene();auto p=predict(s,{0,-1});check(p.firstBall==1,"nearest ball collision");
 check(std::abs(p.lines.front().b.y-190)<1e-6,"finite radius impact location");
 check(p.lines.size()>1&&p.lines[1].role==1,"object trajectory emitted");
 s.balls[1].p={110,180};p=predict(s,{0,-1});bool tangentObject=false;for(auto l:p.lines)tangentObject|=l.role==1;check(!tangentObject,"tangent contact does not impart object velocity");
 s=scene();s.table.pockets={{100,0}};s.balls={{{100,300},5,cue,1},{{100,220},5,solid,1},{{100,140},5,solid,1}};
 auto combo=optimize(s);bool combination=false;for(auto sh:combo)combination|=sh.type=="combination";check(combination,"two-object combination generated through ghost contacts");
 s=scene();
 s.balls={{{100,300},5,cue,1}};p=predict(s,{1,0},2);check(p.lines.size()==3,"multiple rail reflections bounded");
 check(std::abs(p.lines[0].b.x-195)<1e-7,"rail inset by ball radius");
 p=predict(s,{-100,-100});check(p.scratch,"side pocket scratch detection");
 s=scene();check(!clearPath({100,300},{100,0},5,s.balls,0),"blocked path rejected");
 check(clearPath({10,300},{10,20},5,s.balls),"clear path accepted");
 s.balls[1].p={50,150};auto shots=optimize(s);check(!shots.empty(),"rank direct and bank candidates");
 bool bank=false;for(auto sh:shots)bank|=sh.type=="bank";check(bank,"bank solution generated");
 bool ordered=true;for(size_t i=1;i<shots.size();++i)ordered&=shots[i-1].score>=shots[i].score;check(ordered,"shot ranking descending");
 Options o;o.group=stripe;check(optimize(s,o).empty(),"group filter excludes solids");
 s.balls[1].kind=eight;check(optimize(s).empty(),"eight ball withheld by default");
 o.group=unknown;o.eightAllowed=true;check(!optimize(s,o).empty(),"eight ball explicit eligibility");
 s.valid=false;check(predict(s,{1,0}).lines.empty()&&optimize(s).empty(),"invalid detection suppresses recommendations");
 Table dynTable{0,0,1000,1000,{},10};Dynamics dyn;dyn.deceleration=0;dyn.ballRestitution=1;dyn.railRestitution=1;
 std::vector<Motion> bodies{{{{100,500},5,cue,1},{100,0},false},{{{150,500},5,solid,1},{0,0},false}};
 advance(bodies,dynTable,.5,dyn);check(length(bodies[0].velocity)<1e-7&&std::abs(bodies[1].velocity.x-100)<1e-7,"equal mass collision transfers velocity");
 check(std::abs(dot(bodies[0].velocity,bodies[0].velocity)+dot(bodies[1].velocity,bodies[1].velocity)-10000)<1e-6,"elastic kinetic energy conserved");
 bodies={{{{100,500},5,cue,1},{2000,0},false},{{{150,500},5,solid,1},{0,0},false}};
 advance(bodies,dynTable,.03,dyn);check(bodies[1].velocity.x>1900,"adaptive time step prevents fast-ball tunneling");
 bodies={{{{980,500},5,cue,1},{100,0},false}};dyn.railRestitution=.75;advance(bodies,dynTable,.3,dyn);check(std::abs(bodies[0].velocity.x+75)<1e-6,"rail restitution changes speed");
 bodies={{{{100,500},5,cue,1},{100,0},false}};dyn.deceleration=20;advance(bodies,dynTable,1,dyn);check(std::abs(length(bodies[0].velocity)-80)<1e-6,"rolling deceleration applied");
 bodies={{{{100,500},5,cue,1},{100,0},false}};dynTable.pockets={{150,500}};advance(bodies,dynTable,1,dyn);check(bodies[0].pocketed,"dynamic pocket capture");
 Fixture f;f.circle({155,420},7,245,245,245);f.circle({100,170},7,220,35,40);f.circle({210,250},7,230,185,30);f.rect(153,360,156,411,245,245,245);
 auto v=f.detect();std::cerr<<json(v)<<'\n';check(v.valid,"synthetic complete table accepted");
 check(v.balls.size()==3,"three balls detected with connected guideline removed");
 check(v.table.pockets.size()==6,"six dark pockets verified");
 check(v.aimConfidence>.65&&v.aim.y<-.98,"observed cue direction from white guideline");
 int cueCount=0;for(auto b:v.balls)cueCount+=b.kind==cue;check(cueCount==1,"unique white cue classified");
 std::ofstream("artifacts/synthetic-state.json")<<json(v);
 std::ofstream ppm("artifacts/synthetic-frame.ppm",std::ios::binary);ppm<<"P6\n"<<f.w<<' '<<f.h<<"\n255\n";for(int y=0;y<f.h;++y)for(int x=0;x<f.w;++x){auto i=y*f.stride+x*4;char rgb[3]={(char)f.p[i+2],(char)f.p[i+1],(char)f.p[i]};ppm.write(rgb,3);}ppm.close();
 f.circle({210,250},7,245,245,245);check(!f.detect().valid,"ambiguous cue fails closed");
 Fixture blank;check(!blank.detect().valid,"empty table fails closed");
 std::vector<uint8_t> black(320*600*4);check(!detectBGRA(black.data(),320,600,1280).valid,"black frame rejected");
 check(!detectBGRA(nullptr,320,600,1280).valid,"null input rejected");
 check(!detectBGRA(black.data(),320,600,100).valid,"invalid row stride rejected");
 Fixture missing;missing.circle({155,420},7,245,245,245);missing.rect(15,280,45,320,90,65,55);check(!missing.detect().valid,"missing pocket fails closed");
 Fixture moving;moving.circle({155,420},7,245,245,245);moving.circle({100,170},7,220,35,40);
 auto start=std::chrono::steady_clock::now();for(int i=0;i<30;++i){auto frame=moving.detect();if(!frame.valid)return 2;}
 auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/30;
 std::cout<<"RESULT: "<<checks<<" checks passed. Synthetic 320x600 analysis mean "<<ms<<" ms (host only).\n";
}
