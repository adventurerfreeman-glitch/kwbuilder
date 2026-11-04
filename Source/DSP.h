#pragma once
#include <algorithm>
#include <cmath>
namespace kw {
constexpr double pi=3.14159265358979323846;
inline double clamp(double x,double a,double b){return std::max(a,std::min(x,b));}
struct LP {double a=0,z=0;void tune(double f,double fs){a=std::exp(-2*pi*clamp(f,1,fs*.45)/fs);}double tick(double x){z+=(1-a)*(x-z);return z;}void reset(){z=0;}};
struct HP {LP lp;void tune(double f,double fs){lp.tune(f,fs);}double tick(double x){return x-lp.tick(x);}void reset(){lp.reset();}};
// Cookbook low shelf plus compensating gain: low=-tilt, high=+tilt.
struct Tilt {
 double b0=1,b1=0,b2=0,a1=0,a2=0,x1=0,x2=0,y1=0,y2=0,makeup=1;
 void reset(){x1=x2=y1=y2=0;}
 void tune(double tilt,double fs){
  tilt=clamp(tilt,-3,3);double A=std::pow(10,-2*tilt/40),w=2*pi*800/fs,c=std::cos(w),alpha=std::sin(w)*std::sqrt(2.)/2,k=2*std::sqrt(A)*alpha;
  double a0=(A+1)+(A-1)*c+k;
  b0=A*((A+1)-(A-1)*c+k)/a0;b1=2*A*((A-1)-(A+1)*c)/a0;b2=A*((A+1)-(A-1)*c-k)/a0;
  a1=-2*((A-1)+(A+1)*c)/a0;a2=((A+1)+(A-1)*c-k)/a0;makeup=std::pow(10,tilt/20);
 }
 double tick(double x){double y=b0*x+b1*x1+b2*x2-a1*y1-a2*y2;x2=x1;x1=x;y2=y1;y1=y;return makeup*y;}
};
struct Settings {double gain=.5,level=-6,texture=.5,tilt=0;int voice=1,path=1,inputCap=1,bypass=0,drain=1,coupling=1,shunt=1,source=1,load=0,lowcut=2;};
class Core {
 double fs=192000,output=.5,path=1,g=.5,d1=5,d2=10,b1=.1,b2=.1,bypassMix=1;bool cutOn=true;
 HP pre,input,c2,c4,c5,dc1,dc2;LP bypassBand,hf;Tilt tiltFilter;
 static double stage(double x,double d,double b){double t=std::tanh(b);return (std::tanh(d*x+b)-t)/(1-t*t);}
public:
 double drive1=0,drive2=0;bool guard=false;
 void reset(){pre.reset();input.reset();c2.reset();c4.reset();c5.reset();dc1.reset();dc2.reset();bypassBand.reset();hf.reset();tiltFilter.reset();drive1=drive2=0;guard=false;}
 void prepare(double rate){fs=rate;reset();configure(Settings{});}
 void configure(const Settings& s){
  constexpr double ci[]={47e-9,100e-9,220e-9},cm[]={10e-9,22e-9,47e-9},rd[]={6800,10000,15000},r2[]={680,1000,1500},cb[]={0,100e-9,1e-6,22e-6},cuts[]={0,40,65,90,120,180};
  auto ix=[](int n,int size){return std::max(0,std::min(n,size-1));};int v=ix(s.voice,4),b=ix(s.bypass,5);double cap=b==0?cb[v]:cb[b-1],rs=v==0?2700:820;
  g=clamp(s.gain,0,1);double t=clamp(s.texture,0,1);output=std::pow(10,clamp(s.level,-24,12)/20);path=ix(s.path,2);
  double f=cuts[ix(s.lowcut,6)];cutOn=f>0;pre.tune(f>0?f:40,fs);
  input.tune(1/(2*pi*(s.load==0?1e6:2e6)*ci[ix(s.inputCap,3)]),fs);c2.tune(1/(2*pi*150000*22e-9),fs);c4.tune(1/(2*pi*320000*cm[ix(s.coupling,3)]),fs);c5.tune(1/(2*pi*500000*22e-9),fs);
  double potR=900+1800*4*g*(1-g);hf.tune(1/(2*pi*potR*cm[ix(s.shunt,3)]),fs);bypassMix=cap>0?1:0;bypassBand.tune(cap>0?1/(2*pi*rs*cap):100,fs);dc1.tune(5,fs);dc2.tune(5,fs);
  double tex=.7+1.2*t;d1=(1.4+6*g)*tex*(rd[ix(s.drain,3)]/10000)*(v==0?.65:1);d2=(2+14*g)*tex*(1000/r2[ix(s.source,3)]);b1=.025+.23*t;b2=.015+.18*t;tiltFilter.tune(s.tilt,fs);
 }
 double tick(double x){
  if(!std::isfinite(x)){reset();return 0;}x=clamp(x,-8,8);double cut=pre.tick(x);if(cutOn)x=cut;
  double in=input.tick(x),high=in-bypassBand.tick(in),stageIn=in+1.7*bypassMix*high;drive1=std::abs(stageIn*d1);
  double first=.38*dc1.tick(stage(stageIn,d1,b1));double w=hf.tick(c2.tick(first))*(.35+1.65*g);double q2in=c4.tick(w);drive2=std::abs(q2in*d2);
  double second=.36*c5.tick(dc2.tick(stage(q2in,d2,b2)));double selected=(1-path)*w+path*second;
  double y=tiltFilter.tick(selected)*output;guard=std::abs(y)>1;
  if(!std::isfinite(y)){reset();return 0;}return clamp(y,-1,1);
 }
};
}
