#include "Seagull.hpp"
#include <sstream>
#include <iomanip>
namespace sg {
static void point(std::ostream&o,V p){o<<'['<<p.x<<','<<p.y<<']';}
static void lines(std::ostream&o,const std::vector<Segment>&ls){o<<'[';bool first=true;for(auto l:ls){if(!first)o<<',';first=false;o<<"{\"a\":";point(o,l.a);o<<",\"b\":";point(o,l.b);o<<",\"role\":"<<l.role<<'}';}o<<']';}
std::string json(const State&s,const Options&opt){std::ostringstream o;o<<std::fixed<<std::setprecision(3);
 o<<"{\"valid\":"<<(s.valid?"true":"false")<<",\"reason\":\""<<s.reason<<"\",\"confidence\":"<<s.confidence;
 o<<",\"table\":["<<s.table.left<<','<<s.table.top<<','<<s.table.right<<','<<s.table.bottom<<"],\"pockets\":[";
 for(size_t i=0;i<s.table.pockets.size();++i){if(i)o<<',';point(o,s.table.pockets[i]);}o<<"],\"balls\":[";
 for(size_t i=0;i<s.balls.size();++i){if(i)o<<',';auto b=s.balls[i];o<<"{\"p\":";point(o,b.p);o<<",\"r\":"<<b.r<<",\"kind\":"<<int(b.kind)<<'}';}o<<"],\"aimConfidence\":"<<s.aimConfidence<<",\"prediction\":";
 auto pr=s.aimConfidence>.65?predict(s,s.aim):Prediction{};lines(o,pr.lines);o<<",\"scratch\":"<<(pr.scratch?"true":"false")<<",\"shots\":[";
 auto shots=optimize(s,opt);for(size_t i=0;i<shots.size();++i){if(i)o<<',';auto sh=shots[i];o<<"{\"type\":\""<<sh.type<<"\",\"ball\":"<<sh.ball<<",\"pocket\":"<<sh.pocket<<",\"score\":"<<sh.score<<",\"effort\":"<<sh.relativeEffort<<",\"scratch\":"<<(sh.scratch?"true":"false")<<",\"lines\":";lines(o,sh.lines);o<<'}';}o<<"]}";return o.str();}
}
