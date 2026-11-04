#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace kw {
constexpr double pi = 3.14159265358979323846;
inline double bound(double x, double lo, double hi) { return std::max(lo, std::min(x, hi)); }
struct LP {
 double z=0, a=0;
 void tune(double hz,double fs) { a=std::exp(-2*pi*bound(hz,1,fs*.45)/fs); }
 double tick(double x) { z=(1-a)*x+a*z; return z; }
 void reset() { z=0; }
};
struct HP {
 LP lp;
 void tune(double hz,double fs) { lp.tune(hz,fs); }
 double tick(double x) { return x-lp.tick(x); }
 void reset() { lp.reset(); }
};
struct Settings {
 double gain=.45, level=-6, voice=1, path=1, inputCap=1, bypass=0,
 drain=1, coupling=1, shunt=1, source=1, load=0, texture=1;
};
// Behavioral approximation. This is NOT a SPICE/WDF solver or fitted capture.
// RC poles use simplified effective loads; loading interactions are not solved.
// Drain/source and bypass choices influence bounded gain/shape proxies.
class Core {
 double fs=192000;
 HP input, c2, c4, c5, dc1, dc2;
 LP hf, sourceBand;
 double g=0.45, output=.5, path=1, bypassMix=1, q1Drive=1, q2Drive=1;
 double bend1=.12,bend2=.1, loadGain=1;
 static double shaped(double x,double drive,double bias) {
   // Unit small-signal slope before deliberate drive; subtract DC offset.
   return (std::tanh(drive*x+bias)-std::tanh(bias)) / (1-std::tanh(bias)*std::tanh(bias));
 }
public:
 void prepare(double sampleRate) { fs=sampleRate; reset(); configure(Settings{}); }
 void reset() { input.reset();c2.reset();c4.reset();c5.reset();dc1.reset();dc2.reset();hf.reset();sourceBand.reset(); }
 void configure(const Settings& s) {
   static constexpr double capsIn[]={47e-9,100e-9,220e-9};
   static constexpr double capsInter[]={10e-9,22e-9,47e-9};
   static constexpr double rs2[]={680,1000,1500};
   static constexpr double rd[]={6800,10000,15000};
   static constexpr double bypassC[]={0,100e-9,1e-6,22e-6};
   auto ix=[](double x,int n){return int(bound(std::round(x),0,n-1));};
   int v=ix(s.voice,4),b=ix(s.bypass,5);
   double rsource=v==0?2700:820;
   double cb=b==0?(v==0?0:bypassC[v]):bypassC[b-1];
   bypassMix=cb>0?1:0;
   double inputR=s.load<.5?1e6:2e6;
   loadGain=s.load<.5?1.0:.995; // UI exploration proxy, not a pickup model.
   input.tune(1/(2*pi*inputR*capsIn[ix(s.inputCap,3)]),fs);
   c2.tune(1/(2*pi*150000*22e-9),fs);
   c4.tune(1/(2*pi*320000*capsInter[ix(s.coupling,3)]),fs);
   c5.tune(1/(2*pi*500000*22e-9),fs);
   // Pot-dependent effective impedance proxy, not complete pot/RC interaction.
   double potR=1500+60000*4*s.gain*(1-s.gain);
   hf.tune(1/(2*pi*potR*capsInter[ix(s.shunt,3)]),fs);
   sourceBand.tune(cb>0?1/(2*pi*rsource*cb):200,fs);
   dc1.tune(8,fs);dc2.tune(8,fs);
   g=bound(s.gain,0,1);
   output=std::pow(10,bound(s.level,-24,12)/20);
   path=bound(s.path,0,1);
   double drainRatio=rd[ix(s.drain,3)]/10000;
   double tex=bound(s.texture,.5,1.5);
   q1Drive=tex*drainRatio*(v==0?1.25:1.8);
   q2Drive=tex*2.2*1000/rs2[ix(s.source,3)];
   bend1=.1+.07*(drainRatio-1);bend2=.12+.04*(1000/rs2[ix(s.source,3)]-1);
 }
 double tick(double x) {
   if(!std::isfinite(x)) { reset(); return 0; }
   x=bound(x,-8,8);
   double in=input.tick(x)*loadGain;
   double upper=in-sourceBand.tick(in);
   double driveSignal=in+upper*(1.6*bypassMix);
   double first=dc1.tick(shaped(driveSignal,q1Drive,bend1));
   double w=hf.tick(c2.tick(first)*g);
   double second=c5.tick(dc2.tick(shaped(c4.tick(w),q2Drive,bend2)));
   double result=((1-path)*w+path*second)*output;
   if(!std::isfinite(result)) { reset(); return 0; }
   // Fixed protection ceiling; not part of the pedal model. No auto makeup.
   return bound(result,-1.0,1.0);
 }
};
}
