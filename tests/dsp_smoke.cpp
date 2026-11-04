#include "../Source/BuilderDSP.h"
#include <iostream>
#include <iomanip>
int main(){
 std::cout<<"sample_rate,voice,path,variant,peak,rms,finite\n";
 for(double sr:{44100.,48000.,96000.})for(int voice=0;voice<4;++voice)for(int path=0;path<2;++path)for(int variant=0;variant<3;++variant){
  kw::Core c;c.prepare(sr*4);kw::Settings s;s.voice=voice;s.path=path;s.bypass=0;s.gain=variant==0?.01:variant==1?.45:1;s.level=variant==2?12:-6;
  s.inputCap=variant;s.drain=variant;s.coupling=variant;s.shunt=variant;s.source=variant;c.configure(s);
  double peak=0,sum=0;bool finite=true;int N=24000;
  for(int i=0;i<N;++i){double x=.2*std::sin(2*kw::pi*110*i/(sr*4))+.07*std::sin(2*kw::pi*1700*i/(sr*4));double y=c.tick(x);finite=finite&&std::isfinite(y);peak=std::max(peak,std::abs(y));sum+=y*y;}
  if(!finite||peak>1.00001)return 1;
  std::cout<<sr<<","<<voice+1<<","<<path<<","<<variant<<","<<peak<<","<<std::sqrt(sum/N)<<","<<finite<<"\n";
 }
 kw::Core c;c.prepare(192000);if(c.tick(NAN)!=0)return 2;for(int i=0;i<1000;++i)if(c.tick(0)!=0)return 3;
 return 0;
}
