#include "Seagull.hpp"
#include <algorithm>
#include <limits>
namespace sg {
static constexpr double inf=1e30;
bool Table::inside(V p,double m)const{return p.x>=left+m&&p.x<=right-m&&p.y>=top+m&&p.y<=bottom-m;}
double rayCircle(V p,V d,V c,double r){
 d=unit(d); if(length(d)<.5)return inf;
 V q=p-c; double b=dot(q,d), disc=b*b-dot(q,q)+r*r;
 if(disc<0)return inf;
 double t=-b-std::sqrt(disc);return t>=-1e-7?std::max(0.,t):inf;
}
double segmentDistance(V p,V a,V b){V d=b-a;double z=dot(d,d);return length(p-(a+d*(z>1e-12?std::clamp(dot(p-a,d)/z,0.,1.):0.)));}
bool clearPath(V a,V b,double r,const std::vector<Ball>& balls,int ia,int ib){
 for(int i=0;i<(int)balls.size();++i)if(i!=ia&&i!=ib&&segmentDistance(balls[i].p,a,b)<r+balls[i].r-.01)return false;
 return true;
}
static int cueIndex(const State&s){for(int i=0;i<(int)s.balls.size();++i)if(s.balls[i].kind==cue)return i;return -1;}
static double railHit(const Table&t,V p,V d,double r,bool& x,bool& y){
 double tx=inf,ty=inf;
 if(d.x>1e-8)tx=(t.right-r-p.x)/d.x;else if(d.x< -1e-8)tx=(t.left+r-p.x)/d.x;
 if(d.y>1e-8)ty=(t.bottom-r-p.y)/d.y;else if(d.y< -1e-8)ty=(t.top+r-p.y)/d.y;
 double q=std::min(tx,ty);x=std::abs(tx-q)<1e-6;y=std::abs(ty-q)<1e-6;return q;
}
static void trace(const State&s,V p,V direction,double r,int ignore,int rails,int role,Prediction&out,bool collision){
 V d=unit(direction); if(length(d)<.5)return;
 for(int n=0;n<=rails;++n){
  bool rx=false,ry=false;double distance=railHit(s.table,p,d,r,rx,ry);int hit=-1;bool pocket=false;
  if(distance<0||distance>inf/2)return;
  for(auto target:s.table.pockets){double v=rayCircle(p,d,target,s.table.pocketRadius);if(v<distance){distance=v;pocket=true;}}
  for(int i=0;i<(int)s.balls.size();++i)if(i!=ignore){
   double v=rayCircle(p,d,s.balls[i].p,r+s.balls[i].r);
   if(v<distance){distance=v;hit=i;pocket=false;}
  }
  V end=p+d*distance;out.lines.push_back({p,end,role});
  if(pocket){if(role==0||role==2)out.scratch=true;return;}
  if(hit>=0){
   if(!collision)return;
   V normal=unit(s.balls[hit].p-end);double transfer=dot(d,normal);
   if(transfer<1e-6){p=end+d*.02;continue;}
   out.firstBall=hit;
   // Equal masses, no spin, instantaneous elastic impact. Subsequent impacts stop the trace.
   State after=s;after.balls[ignore].p=end;
   trace(after,s.balls[hit].p+normal*.02,normal,s.balls[hit].r,hit,rails,1,out,false);
   V residual=d-normal*transfer;
   if(length(residual)>.02){after.balls[hit].p=after.balls[hit].p+normal*(r*.1);trace(after,end+unit(residual)*.02,residual,r,ignore,rails,2,out,false);}
   return;
  }
  if(rx)d.x=-d.x;
  if(ry)d.y=-d.y;
  p=end+d*.02;
 }
}
Prediction predict(const State&s,V d,int maxRails){Prediction out;int c=cueIndex(s);if(!s.valid||c<0)return out;trace(s,s.balls[c].p,d,s.balls[c].r,c,std::clamp(maxRails,0,6),0,out,true);return out;}
static bool legal(const Ball&b,const Options&o){if(b.kind==cue)return false;if(b.kind==eight)return o.eightAllowed;return o.group==unknown||b.kind==o.group;}
std::vector<Shot> optimize(const State&s,const Options&o){
 std::vector<Shot> shots;int ci=cueIndex(s);if(!s.valid||ci<0)return shots;
 auto cb=s.balls[ci];double diagonal=std::hypot(s.table.right-s.table.left,s.table.bottom-s.table.top);
 auto add=[&](int bi,int pi,V first,std::vector<Segment> path,std::string type,int second=-1){
  auto b=s.balls[bi];V out=unit(first-b.p),ghost=b.p-out*(cb.r+b.r),in=unit(ghost-cb.p);
  double cut=dot(in,out);if(cut<.22||!s.table.inside(ghost,cb.r)||!clearPath(cb.p,ghost,cb.r,s.balls,ci,bi))return;
  double distance=length(ghost-cb.p);for(auto l:path)distance+=length(l.b-l.a);
  auto pr=predict(s,in,2);if(pr.firstBall!=bi)return;
  Shot sh;sh.ball=bi;sh.pocket=pi;sh.secondBall=second;sh.type=type;sh.scratch=pr.scratch;
  sh.relativeEffort=distance/(diagonal*std::max(.2,cut)); // NOT a game power percentage
  sh.score=std::clamp(100*cut-25*distance/diagonal-(type=="bank"?18:type=="combination"?28:0)-(sh.scratch?45:0),0.,100.);
  sh.lines.push_back({cb.p,ghost,3});for(auto l:path){l.role=3;sh.lines.push_back(l);}shots.push_back(sh);
 };
 for(int bi=0;bi<(int)s.balls.size();++bi){if(bi==ci||!legal(s.balls[bi],o))continue;auto b=s.balls[bi];
  for(int pi=0;pi<(int)s.table.pockets.size();++pi){V p=s.table.pockets[pi];
   if(clearPath(b.p,p,b.r,s.balls,bi,ci))add(bi,pi,p,{{b.p,p,3}},"direct");
   if(o.banks)for(int side=0;side<4;++side){
    bool vertical=side<2;double wall=side==0?s.table.left+b.r:side==1?s.table.right-b.r:side==2?s.table.top+b.r:s.table.bottom-b.r;
    V mirror=p;if(vertical)mirror.x=2*wall-p.x;else mirror.y=2*wall-p.y;
    V delta=mirror-b.p;double denom=vertical?delta.x:delta.y;if(std::abs(denom)<1e-6)continue;
    double t=(wall-(vertical?b.p.x:b.p.y))/denom;if(t<=0||t>=1)continue;V bounce=b.p+delta*t;
    if(!s.table.inside(bounce,b.r-.001))continue;
    bool mouth=false;for(V pk:s.table.pockets)if(length(pk-bounce)<s.table.pocketRadius+b.r)mouth=true;
    if(mouth||!clearPath(b.p,bounce,b.r,s.balls,bi,ci)||!clearPath(bounce,p,b.r,s.balls,bi,ci))continue;
    add(bi,pi,bounce,{{b.p,bounce,3},{bounce,p,3}},"bank");
   }
   if(o.combinations)for(int bj=0;bj<(int)s.balls.size();++bj){
    if(bj==ci||bj==bi||!legal(s.balls[bj],o))continue;
    auto last=s.balls[bj];
    V out=unit(p-last.p),contact=last.p-out*(last.r+b.r),incoming=unit(contact-b.p);
    if(dot(incoming,out)<.5||!s.table.inside(contact,b.r))continue;
    if(!clearPath(b.p,contact,b.r,s.balls,bi,bj)||!clearPath(last.p,p,last.r,s.balls,bj,bi))continue;
    add(bi,pi,contact,{{b.p,contact,3},{last.p,p,3}},"combination",bj);
   }
  }
 }
 std::sort(shots.begin(),shots.end(),[](const Shot&a,const Shot&b){return a.score>b.score;});if(shots.size()>8)shots.resize(8);return shots;
}
}
