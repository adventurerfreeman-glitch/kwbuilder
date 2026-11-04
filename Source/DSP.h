#pragma once
#include <algorithm>
#include <cmath>
namespace kw {
constexpr double pi=3.14159265358979323846;
inline double clamp(double x,double a,double b){return std::max(a,std::min(x,b));}
struct LP {double a=0,z=0;void tune(double f,double fs){a=std::exp(-2*pi*clamp(f,1,fs*.45)/fs);}double tick(double x){z+=(1-a)*(x-z);return z;}void reset(){z=0;}};
struct HP {LP lp;void tune(double f,double fs){lp.tune(f,fs);}double tick(double x){return x-lp.tick(x);}void reset(){lp.reset();}};
struct Settings {double gain=.5,level=-6,texture=.5;int voice=1,path=1,inputCap=1,bypass=0,drain=1,coupling=1,shunt=1,source=1,load=0;};
// Behavioral core, not a circuit-accurate JFET solver or reference capture.
class Core {
 double fs=192000,output=.5,path=1,g=.5,d1=5,d2=10,b1=.1,b2=.1,bypassMix=1;
 HP input,c2,c4,c5,dc1,dc2;LP bypassBand,hf;
 static double stage(double x,double d,double b){double t=std::tanh(b);return (std::tanh(d*x+b)-t)/(1-t*t);}
public:
 void reset(){input.reset();c2.reset();c4.reset();c5.reset();dc1.reset();dc2.reset();bypassBand.reset();hf.reset();}
 void prepare(double rate){fs=rate;reset();configure(Settings{});}
 void configure(const Settings& s){
  constexpr double ci[]={47e-9,100e-9,220e-9},cm[]={10e-9,22e-9,47e-9},rd[]={6800,10000,15000},r2[]={680,1000,1500},cb[]={0,100e-9,1e-6,22e-6};
  auto ix=[](int n,int size){return std::max(0,std::min(n,size-1));};int v=ix(s.voice,4),b=ix(s.bypass,5);double cap=b==0?cb[v]:cb[b-1],rs=v==0?2700:820;
  g=clamp(s.gain,0,1);double t=clamp(s.texture,0,1);output=std::pow(10,clamp(s.level,-24,12)/20);path=ix(s.path,2);
  input.tune(1/(2*pi*(s.load==0?1e6:2e6)*ci[ix(s.inputCap,3)]),fs);c2.tune(1/(2*pi*150000*22e-9),fs);c4.tune(1/(2*pi*320000*cm[ix(s.coupling,3)]),fs);c5.tune(1/(2*pi*500000*22e-9),fs);
  double potR=900+1800*4*g*(1-g);hf.tune(1/(2*pi*potR*cm[ix(s.shunt,3)]),fs);bypassMix=cap>0?1:0;bypassBand.tune(cap>0?1/(2*pi*rs*cap):100,fs);dc1.tune(5,fs);dc2.tune(5,fs);
  double tex=.7+1.2*t;d1=(1.4+6*g)*tex*(rd[ix(s.drain,3)]/10000)*(v==0?.65:1);d2=(2+14*g)*tex*(1000/r2[ix(s.source,3)]);b1=.025+.23*t;b2=.015+.18*t;
 }
 double tick(double x){
  if(!std::isfinite(x)){reset();return 0;}x=clamp(x,-8,8);double in=input.tick(x),high=in-bypassBand.tick(in);
  double first=.38*dc1.tick(stage(in+1.7*bypassMix*high,d1,b1));double w=hf.tick(c2.tick(first))*(.35+1.65*g);
  double second=.36*c5.tick(dc2.tick(stage(c4.tick(w),d2,b2)));double y=((1-path)*w+path*second)*output;
  if(!std::isfinite(y)){reset();return 0;}return clamp(y,-1,1);
 }
};
}
