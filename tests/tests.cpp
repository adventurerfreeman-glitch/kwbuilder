#include "../Source/DSP.h"
#include <iostream>
#include <vector>
#include <limits>
struct Render {std::vector<double> y;double guard=0,drive1=0,drive2=0;};
Render render(kw::Settings s,double amplitude=.08,double frequency=110,double rate=192000){kw::Core c;c.prepare(rate);c.configure(s);int n=(int)(rate*.5);Render r;r.y.resize((size_t)n);for(int i=0;i<2*n;++i){double y=c.tick(amplitude*std::sin(2*kw::pi*frequency*i/rate));if(!std::isfinite(y)||std::abs(y)>1)throw 1;if(i>=n){r.y[(size_t)(i-n)]=y;r.guard+=c.guard?1:0;r.drive1=std::max(r.drive1,c.drive1);r.drive2=std::max(r.drive2,c.drive2);}}r.guard/=n;return r;}
double harmonic(const std::vector<double>& a,int h){double w=2*kw::pi*110*h/192000,cr=std::cos(w),ci=std::sin(w),zr=1,zi=0,re=0,im=0;for(double y:a){re+=y*zr;im+=y*zi;double t=zr*cr-zi*ci;zi=zi*cr+zr*ci;zr=t;}return 2*std::hypot(re,im)/a.size();}
double thd(const std::vector<double>& a){double v=0;for(int h=2;h<=12;++h){double x=harmonic(a,h);v+=x*x;}return std::sqrt(v)/std::max(1e-12,harmonic(a,1));}
double diff(const std::vector<double>& a,const std::vector<double>& b){double s=0;for(size_t i=0;i<a.size();++i)s+=(a[i]-b[i])*(a[i]-b[i]);return std::sqrt(s/a.size());}
double tiltResponse(double db,double hz){kw::Tilt t;t.tune(db,192000);double in=0,out=0;for(int i=0;i<192000;++i){double x=.1*std::sin(2*kw::pi*hz*i/192000),y=t.tick(x);if(i>=96000){in+=x*x;out+=y*y;}}return 10*std::log10(out/in);}
int main(){try{
 std::cout<<"input_peak_dbfs,voice,gain_0_to_10,texture_0_to_10,thd_percent,fundamental_peak_dbfs,stage1_drive_index,stage2_drive_index,guard_percent\n";
 kw::Settings ref;
 for(double db:{-36.,-24.,-12.})for(int v=1;v<=3;++v){double first=0,last=0;for(double g:{0.,.25,.5,.75,1.}){auto s=ref;s.voice=v;s.gain=g;auto r=render(s,std::pow(10,db/20));double th=thd(r.y);if(g==0)first=th;if(g==1)last=th;std::cout<<db<<","<<v+1<<","<<g*10<<",5,"<<th*100<<","<<20*std::log10(std::max(1e-12,harmonic(r.y,1)))<<","<<r.drive1<<","<<r.drive2<<","<<r.guard*100<<"\n";if(r.guard!=0)return 2;}if(last<=first)return 3;}
 auto a=render(ref);auto s=ref;s.voice=2;auto b=render(s);s.voice=3;auto c=render(s);if(diff(a.y,b.y)<1e-6||diff(b.y,c.y)<1e-6)return 4;
 // Post tilt must NOT change the arguments of either nonlinear stage.
 s=ref;s.tilt=3;auto high=render(s);if(std::abs(a.drive1-high.drive1)>1e-12||std::abs(a.drive2-high.drive2)>1e-12||diff(a.y,high.y)<1e-6)return 5;
 s=ref;s.lowcut=0;auto open=render(s,.08,55);s.lowcut=5;auto tight=render(s,.08,55);if(tight.drive1>=open.drive1)return 6;
 if(std::abs(tiltResponse(0,800))>1e-8||std::abs(tiltResponse(3,800))>.01||tiltResponse(3,40)>-2.9||tiltResponse(3,8000)<2.9)return 7;
 for(int i=0;i<14;++i){s=ref;switch(i){case 0:s.gain=.8;break;case 1:s.level=-12;break;case 2:s.texture=.9;break;case 3:s.voice=3;break;case 4:s.path=0;break;case 5:s.inputCap=0;break;case 6:s.bypass=1;break;case 7:s.drain=2;break;case 8:s.coupling=0;break;case 9:s.shunt=0;break;case 10:s.source=2;break;case 11:s.load=1;break;case 12:s.lowcut=5;break;case 13:s.tilt=3;break;}if(diff(a.y,render(s).y)<1e-7)return 8;}
 for(double sr:{44100.,48000.,96000.})for(int v=0;v<4;++v)for(int p=0;p<2;++p)for(int variant=0;variant<3;++variant){kw::Core x;x.prepare(sr*4);s=ref;s.voice=v;s.path=p;s.gain=variant*.5;s.texture=variant*.5;s.level=variant==2?12:-6;s.tilt=variant==0?-3:variant==1?0:3;s.lowcut=variant==0?0:variant==1?2:5;x.configure(s);for(int n=0;n<12000;++n){double y=x.tick(.2*std::sin(2*kw::pi*173*n/(sr*4)));if(!std::isfinite(y)||std::abs(y)>1)return 9;}}
 kw::Core x;x.prepare(192000);if(x.tick(std::numeric_limits<double>::quiet_NaN())!=0)return 10;for(int i=0;i<1000;++i)if(x.tick(0)!=0)return 11;
 }catch(...){return 12;}return 0;}
