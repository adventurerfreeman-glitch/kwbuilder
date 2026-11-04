#include "../Source/DSP.h"
#include <iostream>
#include <vector>
#include <limits>
std::vector<double> render(kw::Settings s){kw::Core c;c.prepare(192000);c.configure(s);std::vector<double> out(24000);for(int n=0;n<48000;++n){double x=.08*std::sin(2*kw::pi*110*n/192000)+.025*std::sin(2*kw::pi*1700*n/192000);double y=c.tick(x);if(!std::isfinite(y)||std::abs(y)>1)throw 1;if(n>=24000)out[(size_t)(n-24000)]=y;}return out;}
double diff(const std::vector<double>& a,const std::vector<double>& b){double ss=0;for(size_t i=0;i<a.size();++i)ss+=(a[i]-b[i])*(a[i]-b[i]);return std::sqrt(ss/a.size());}
int main(){try{
 kw::Settings ref;auto a=render(ref);std::cout<<"control,changed_output_rms\n";
 for(int i=0;i<12;++i){auto s=ref;switch(i){case 0:s.gain=.8;break;case 1:s.level=-12;break;case 2:s.texture=.9;break;case 3:s.voice=3;break;case 4:s.path=0;break;case 5:s.inputCap=0;break;case 6:s.bypass=1;break;case 7:s.drain=2;break;case 8:s.coupling=0;break;case 9:s.shunt=0;break;case 10:s.source=2;break;case 11:s.load=1;break;}auto b=render(s);double d=diff(a,b);if(d<1e-7)return 2;std::cout<<i<<","<<d<<"\n";}
 auto s=ref;s.voice=2;auto b=render(s);s.voice=3;auto c=render(s);if(diff(a,b)<1e-6||diff(b,c)<1e-6||diff(a,c)<1e-6)return 3;
 for(double sr:{44100.,48000.,96000.})for(int v=0;v<4;++v)for(int p=0;p<2;++p)for(int k=0;k<3;++k){kw::Core x;x.prepare(sr*4);auto t=ref;t.voice=v;t.path=p;t.gain=k*.5;t.texture=k*.5;t.level=k==2?12:-6;t.inputCap=k;t.drain=k;t.shunt=k;t.source=k;t.coupling=k;x.configure(t);for(int n=0;n<12000;++n){double y=x.tick(.2*std::sin(2*kw::pi*173*n/(sr*4)));if(!std::isfinite(y)||std::abs(y)>1)return 4;}}
 kw::Core x;x.prepare(192000);if(x.tick(std::numeric_limits<double>::quiet_NaN())!=0)return 5;for(int n=0;n<1000;++n)if(x.tick(0)!=0)return 6;
 }catch(...){return 9;}return 0;}
