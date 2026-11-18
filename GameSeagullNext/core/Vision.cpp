#include "Seagull.hpp"
#include <algorithm>
#include <queue>
#include <numeric>
namespace sg {
struct RGB{int r,g,b;};
struct Image{const uint8_t*p;int w,h,stride;RGB at(int x,int y)const{x=std::clamp(x,0,w-1);y=std::clamp(y,0,h-1);auto q=p+y*stride+x*4;return{q[2],q[1],q[0]};}};
static bool felt(RGB c){return c.g>45&&c.g>c.r*1.24&&c.g>c.b*.85&&std::max(c.g,c.b)>c.r+25;}
struct Component{std::vector<int> ids;int l=10000,t=10000,r=0,b=0;};
static std::vector<Component> components(std::vector<uint8_t> mask,int w,int h){
 std::vector<Component> out;std::vector<int> q;
 for(int i=0;i<w*h;++i)if(mask[i]){Component c;q.clear();q.push_back(i);mask[i]=0;
  for(size_t k=0;k<q.size();++k){int a=q[k],x=a%w,y=a/w;c.ids.push_back(a);c.l=std::min(c.l,x);c.r=std::max(c.r,x);c.t=std::min(c.t,y);c.b=std::max(c.b,y);
   constexpr int dx[4]={-1,1,0,0},dy[4]={0,0,-1,1};
   for(int j=0;j<4;++j){int nx=x+dx[j],ny=y+dy[j];if(nx>=0&&nx<w&&ny>=0&&ny<h&&mask[ny*w+nx]){mask[ny*w+nx]=0;q.push_back(ny*w+nx);}}
  }out.push_back(std::move(c));
 }return out;
}
State detectBGRA(const uint8_t*p,int w,int h,int stride){
 State s;s.reason="No supported table";
 if(!p||w<64||h<64||w>1280||h>1280||stride<w*4)return s;
 Image im{p,w,h,stride};std::vector<uint8_t> mask(w*h);
 for(int y=0;y<h;++y)for(int x=0;x<w;++x)mask[y*w+x]=felt(im.at(x,y));
 auto cs=components(mask,w,h);if(cs.empty())return s;
 auto table=std::max_element(cs.begin(),cs.end(),[](auto&a,auto&b){return a.ids.size()<b.ids.size();});
 double tw=table->r-table->l+1,th=table->b-table->t+1,aspect=std::max(tw,th)/std::min(tw,th);
 if(table->ids.size()<w*h*.12||aspect<1.5||aspect>2.35||table->ids.size()/(tw*th)<.70)return s;
 s.table={double(table->l),double(table->t),double(table->r),double(table->b),{},0};
 RGB base{};for(int id:table->ids){auto c=im.at(id%w,id/w);base.r+=c.r;base.g+=c.g;base.b+=c.b;}
 int n=(int)table->ids.size();base.r/=n;base.g/=n;base.b/=n;
 std::fill(mask.begin(),mask.end(),0);
 for(int y=table->t+2;y<table->b-1;++y)for(int x=table->l+2;x<table->r-1;++x){auto c=im.at(x,y);int dist=std::abs(c.r-base.r)+std::abs(c.g-base.g)+std::abs(c.b-base.b);mask[y*w+x]=dist>85;}
 // Opening separates thin guidelines from ball silhouettes before connected components.
 std::vector<uint8_t> eroded(w*h),opened(w*h);
 for(int y=1;y<h-1;++y)for(int x=1;x<w-1;++x){bool all=true;for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx)all=all&&mask[(y+dy)*w+x+dx];eroded[y*w+x]=all;}
 for(int y=1;y<h-1;++y)for(int x=1;x<w-1;++x){bool any=false;for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx)any=any||eroded[(y+dy)*w+x+dx];opened[y*w+x]=any;}
 auto blobs=components(opened,w,h);double nominal=std::min(tw,th)/36.;
 for(auto&b:blobs){double bw=b.r-b.l+1,bh=b.b-b.t+1,ratio=bw/bh,area=b.ids.size(),fill=area/(bw*bh);
  if(b.l<=table->l+3||b.t<=table->t+3||b.r>=table->r-3||b.b>=table->b-3)continue;
  if(ratio<.68||ratio>1.46||bw<nominal*1.25||bw>nominal*3.1||bh<nominal*1.25||bh>nominal*3.1||fill<.53||fill>.96)continue;
  V center{(b.l+b.r)*.5,(b.t+b.b)*.5};double r=(bw+bh)*.25;
  if(!s.table.inside(center,r*.9))continue;
  int whites=0,darks=0,all=0;for(int y=b.t;y<=b.b;++y)for(int x=b.l;x<=b.r;++x)if(length(V{double(x),double(y)}-center)<r*.8){auto c=im.at(x,y);int hi=std::max({c.r,c.g,c.b}),lo=std::min({c.r,c.g,c.b});whites+=(lo>175&&hi-lo<48);darks+=(hi<65);++all;}
  double white=double(whites)/std::max(1,all),dark=double(darks)/std::max(1,all);
  Kind kind=white>.88?cue:dark>.72?eight:white>.24?stripe:solid;
  s.balls.push_back({center,r,kind,.65});
 }
 // Circular contrast search also resolves balls connected to the cue stick, guidelines or each other.
 // Components alone cannot split those shapes. The search is bounded at the 640 px capture size.
 struct Candidate{V p;double r,score;};std::vector<Candidate> candidates;
 constexpr int rings=24;V directions[rings];for(int k=0;k<rings;++k){double a=k*6.283185307179586/rings;directions[k]={std::cos(a),std::sin(a)};}
 auto difference=[&](RGB c){return std::abs(c.r-base.r)+std::abs(c.g-base.g)+std::abs(c.b-base.b);};
 for(int y=table->t+int(nominal);y<table->b-nominal;y+=2)for(int x=table->l+int(nominal);x<table->r-nominal;x+=2){
  if(difference(im.at(x,y))<45)continue;
  for(double factor:{.85,1.0,1.15}){double rad=nominal*factor;int edge=0,inside=0,outside=0,bins[8]={},k=0;double strength=0;
   for(V d:directions){auto a=im.at(int(x+d.x*rad*.72),int(y+d.y*rad*.72)),b=im.at(int(x+d.x*rad*1.30),int(y+d.y*rad*1.30));
    int di=difference(a),doo=difference(b);inside+=di>45;outside+=doo<80;edge+=(di-doo)>35;bins[k++/3]+=(di-doo)>35;strength+=std::max(0,std::min(200,di-doo));
   }
   int sectors=0;for(int bin:bins)sectors+=bin>=2;
   if(inside>=17&&outside>=16&&edge>=14&&sectors>=6)candidates.push_back({{double(x),double(y)},rad,edge*10.+strength/rings});
  }
 }
 std::sort(candidates.begin(),candidates.end(),[](auto a,auto b){return a.score>b.score;});
 for(auto c:candidates){
  bool close=false;for(auto b:s.balls)if(length(b.p-c.p)<(b.r+c.r)*.82)close=true;
  if(close||!s.table.inside(c.p,c.r*.9))continue;
  // Do not mistake an isolated fragment of a pocket for a ball.
  bool nearEdge=c.p.x-s.table.left<c.r*1.5||s.table.right-c.p.x<c.r*1.5||c.p.y-s.table.top<c.r*1.5||s.table.bottom-c.p.y<c.r*1.5;
  if(nearEdge)continue;
  s.balls.push_back({c.p,c.r,unknown,.55});if(s.balls.size()>16)break;
 }
 if(s.balls.empty()||s.balls.size()>16){s.reason="Ball segmentation uncertain";return s;}
 std::vector<double> radii;for(auto b:s.balls)radii.push_back(b.r);std::sort(radii.begin(),radii.end());double r=radii[radii.size()/2];
 for(auto&b:s.balls)b.r=r;
 // Classify from disk AND peripheral white area, reducing confusion with the white number spot.
 for(auto&b:s.balls){int white=0,dark=0,total=0,outerWhite=0,outer=0;
  for(int y=int(b.p.y-b.r);y<=b.p.y+b.r;++y)for(int x=int(b.p.x-b.r);x<=b.p.x+b.r;++x){double dist=length(V{double(x),double(y)}-b.p);if(dist>b.r*.85)continue;auto c=im.at(x,y);int hi=std::max({c.r,c.g,c.b}),lo=std::min({c.r,c.g,c.b});bool isWhite=lo>175&&hi-lo<48;white+=isWhite;dark+=hi<65;++total;if(dist>b.r*.55){outerWhite+=isWhite;++outer;}}
  double wf=double(white)/std::max(1,total),df=double(dark)/std::max(1,total),ow=double(outerWhite)/std::max(1,outer);
  b.kind=wf>.82?cue:df>.68?eight:ow>.28?stripe:solid;
 }
 if(s.balls.empty()||s.balls.size()>16){s.reason="Ball segmentation uncertain";return s;}
 
 auto&t=s.table;std::vector<V> anchors{{t.left,t.top},{t.right,t.top},{t.left,t.bottom},{t.right,t.bottom}};
 if(th>tw){anchors.push_back({t.left,(t.top+t.bottom)/2});anchors.push_back({t.right,(t.top+t.bottom)/2});}
 else{anchors.push_back({(t.left+t.right)/2,t.top});anchors.push_back({(t.left+t.right)/2,t.bottom});}
 // Verify dark pocket mouths near geometrical anchors; do not silently assume six detected pockets.
 for(V a:anchors){double sx=0,sy=0;int count=0;for(int y=int(a.y-r*1.6);y<=a.y+r*1.6;++y)for(int x=int(a.x-r*1.6);x<=a.x+r*1.6;++x){auto c=im.at(x,y);if(std::max({c.r,c.g,c.b})<48){sx+=x;sy+=y;++count;}}
  if(count>r*r*.35)t.pockets.push_back({sx/count,sy/count});
 }
 t.pocketRadius=r*1.65;
 int cueCount=0,ci=-1;for(int i=0;i<(int)s.balls.size();++i)if(s.balls[i].kind==cue){++cueCount;ci=i;}
 if(cueCount!=1){s.reason="Cue ball ambiguous";return s;}
 if(t.pockets.size()!=6){s.reason="Six pockets not verified";return s;}
 s.valid=true;s.confidence=.65;s.reason="Approximate geometry; uncalibrated";
 // Look for the existing thin white guide starting beyond the cue ball. Never substitute a recommendation for observed aim.
 V cp=s.balls[ci].p;double best=0,runner=0;int bestAngle=0;
 std::vector<double> scores(360);
 for(int a=0;a<360;++a){V d{std::cos(a*3.141592653589793/180),std::sin(a*3.141592653589793/180)};double score=0,seen=0;
  for(double dist=r*1.8;dist<r*8;dist+=1){V q=cp+d*dist;if(!t.inside(q,r))break;
   bool onBall=false;for(int j=0;j<(int)s.balls.size();++j)if(j!=ci&&length(s.balls[j].p-q)<r*1.3)onBall=true;if(onBall)break;
   auto c=im.at((int)q.x,(int)q.y);score+=(std::min({c.r,c.g,c.b})>180&&std::max({c.r,c.g,c.b})-std::min({c.r,c.g,c.b})<45);++seen;
  }scores[a]=seen>r*2?score/seen:0;if(scores[a]>best){best=scores[a];bestAngle=a;}
 }
 for(int a=0;a<360;++a){int diff=std::abs(a-bestAngle);diff=std::min(diff,360-diff);if(diff>12)runner=std::max(runner,scores[a]);}
 if(best>.65&&best-runner>.15){double a=bestAngle*3.141592653589793/180;s.aim={std::cos(a),std::sin(a)};s.aimConfidence=best;}
 return s;
}
}
