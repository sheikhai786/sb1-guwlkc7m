#include "Seagull.hpp"
#include <algorithm>
namespace sg {
void advance(std::vector<Motion>&bs,const Table&t,double seconds,const Dynamics&par){
 if(!std::isfinite(seconds)||seconds<=0||seconds>10)return;
 double left=seconds;int count=0;
 while(left>1e-9&&count++<20000){
  double speed=0,r=1e9;for(auto b:bs)if(!b.pocketed){speed=std::max(speed,length(b.velocity));r=std::min(r,b.ball.r);}
  if(r<=0||!std::isfinite(speed))return;
  double dt=std::min({left,1./240.,r*.2/std::max(1.,speed)});left-=dt;
  for(auto&b:bs)if(!b.pocketed){
   b.ball.p=b.ball.p+b.velocity*dt;
   for(V p:t.pockets)if(length(b.ball.p-p)<=t.pocketRadius){b.pocketed=true;b.velocity={};break;}
   if(b.pocketed)continue;
   double e=std::clamp(par.railRestitution,0.,1.);auto&p=b.ball.p;double rad=b.ball.r;
   if(p.x<t.left+rad){p.x=t.left+rad;b.velocity.x=std::abs(b.velocity.x)*e;}
   if(p.x>t.right-rad){p.x=t.right-rad;b.velocity.x=-std::abs(b.velocity.x)*e;}
   if(p.y<t.top+rad){p.y=t.top+rad;b.velocity.y=std::abs(b.velocity.y)*e;}
   if(p.y>t.bottom-rad){p.y=t.bottom-rad;b.velocity.y=-std::abs(b.velocity.y)*e;}
  }
  for(size_t i=0;i<bs.size();++i)for(size_t j=i+1;j<bs.size();++j){auto&a=bs[i];auto&b=bs[j];if(a.pocketed||b.pocketed)continue;
   V delta=b.ball.p-a.ball.p;double d=length(delta),sum=a.ball.r+b.ball.r;if(d>sum)continue;
   V n=d>1e-9?delta*(1/d):V{1,0};double approach=dot(a.velocity-b.velocity,n);
   if(approach>0){double impulse=(1+std::clamp(par.ballRestitution,0.,1.))*approach/2;a.velocity=a.velocity-n*impulse;b.velocity=b.velocity+n*impulse;}
   double correction=(sum-d)*.5+.00001;a.ball.p=a.ball.p-n*correction;b.ball.p=b.ball.p+n*correction;
  }
  for(auto&b:bs)if(!b.pocketed){double v=length(b.velocity);b.velocity=unit(b.velocity)*std::max(0.,v-std::max(0.,par.deceleration)*dt);}
 }
}
}
