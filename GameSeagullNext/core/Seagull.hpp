#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <cmath>
namespace sg {
struct V { double x=0,y=0; V operator+(V b)const{return{x+b.x,y+b.y};} V operator-(V b)const{return{x-b.x,y-b.y};} V operator*(double k)const{return{x*k,y*k};} };
inline double dot(V a,V b){return a.x*b.x+a.y*b.y;}
inline double length(V a){return std::sqrt(dot(a,a));}
inline V unit(V a){double n=length(a);return n>1e-9?a*(1/n):V{};}
enum Kind {unknown=0,cue=1,solid=2,stripe=3,eight=4};
struct Ball { V p; double r=0; Kind kind=unknown; double confidence=0; };
struct Table {double left=0,top=0,right=0,bottom=0; std::vector<V> pockets; double pocketRadius=0; bool inside(V p,double margin=0)const;};
struct State {Table table; std::vector<Ball> balls; V aim; double aimConfidence=0,confidence=0; bool valid=false; std::string reason;};
struct Segment {V a,b; int role=0;}; // 0 cue, 1 object, 2 residual cue, 3 recommendation
struct Prediction {std::vector<Segment> lines; bool scratch=false; int firstBall=-1;};
struct Shot {int ball=-1,pocket=-1,secondBall=-1; std::string type; double score=0,relativeEffort=0; bool scratch=false; std::vector<Segment> lines;};
struct Options {Kind group=unknown; bool eightAllowed=false,banks=true,combinations=true;};
struct Motion {Ball ball; V velocity; bool pocketed=false;};
struct Dynamics {double deceleration=15,ballRestitution=1,railRestitution=.8;};
// Units: detected-table pixels and seconds. Defaults are uncalibrated, not GamePigeon measurements.
void advance(std::vector<Motion>& balls,const Table& table,double seconds,const Dynamics& parameters={});
double rayCircle(V p,V d,V c,double r);
double segmentDistance(V p,V a,V b);
bool clearPath(V a,V b,double radius,const std::vector<Ball>& balls,int ignoreA=-1,int ignoreB=-1);
Prediction predict(const State& s,V direction,int maxRails=3);
std::vector<Shot> optimize(const State& s,const Options& options={});
State detectBGRA(const uint8_t* pixels,int width,int height,int stride);
std::string json(const State& s,const Options& options={});
}
