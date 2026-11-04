#include <JuceHeader.h>
#include "DSP.h"
#include <array>
namespace {
constexpr int N=12;
const char* ids[N]={"gain","level","texture","voice","path","inputCap","bypass","drain","coupling","shunt","source","load"};
const char* names[N]={"GAIN","LEVEL","TEXTURE","SOURCE VOICE","OUTPUT PATH","INPUT CAP C1","Q1 BYPASS CAP","Q1 DRAIN R4","COUPLING C4","HF SHUNT C3","Q2 SOURCE R8","INPUT LOAD"};
constexpr int rotaryIDs[10]={0,1,2,3,5,6,7,8,9,10};
constexpr int defaultChoices[N]={0,0,0,1,1,1,0,1,1,1,1,0};
juce::StringArray choices(int i){switch(i){case 3:return {"1 Calm","2 Sullen","3 Moody","4 Angry"};case 5:return {"47 nF","100 nF","220 nF"};case 6:return {"Follow Voice","Off","100 nF","1 uF","22 uF"};case 7:return {"6.8 kOhm","10 kOhm","15 kOhm"};case 8:case 9:return {"10 nF","22 nF","47 nF"};case 10:return {"680 Ohm","1 kOhm","1.5 kOhm"};default:return {};}}
juce::AudioProcessorValueTreeState::ParameterLayout makeLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ids[0],1},names[0],0.f,1.f,.5f));
 l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ids[1],1},names[1],-24.f,12.f,-6.f));
 l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ids[2],1},names[2],0.f,1.f,.5f));
 for(int i=3;i<N;++i){if(i==4||i==11)l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ids[i],1},names[i],i==4));else l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{ids[i],1},names[i],choices(i),defaultChoices[i]));}
 return l;
}
}
class Editor;
class Processor final:public juce::AudioProcessor {
public:
 juce::AudioProcessorValueTreeState state;
 std::array<std::atomic<float>*,N> raw{};
 std::array<juce::SmoothedValue<float>,3> smoothing;
 std::array<kw::Core,2> cores;
 std::unique_ptr<juce::dsp::Oversampling<float>> os;
 std::atomic<float> inPeak{0},outPeak{0};
 Processor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"KWBuilderV3State",makeLayout()){for(int i=0;i<N;++i)raw[(size_t)i]=state.getRawParameterValue(ids[i]);}
 const juce::String getName()const override{return "KW Builder V3 Knobs";}
 bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}double getTailLengthSeconds()const override{return 0;}
 int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}
 const juce::String getProgramName(int)override{return {}; }void changeProgramName(int,const juce::String&)override{}
 bool isBusesLayoutSupported(const BusesLayout& b)const override{auto a=b.getMainInputChannelSet(),o=b.getMainOutputChannelSet();return a==o&&(o==juce::AudioChannelSet::mono()||o==juce::AudioChannelSet::stereo());}
 void prepareToPlay(double sr,int)override{
  os=std::make_unique<juce::dsp::Oversampling<float>>((size_t)getTotalNumInputChannels(),2,juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,true);os->initProcessing(512);os->reset();setLatencySamples((int)std::lround(os->getLatencyInSamples()));
  for(auto& c:cores)c.prepare(sr*4);for(int i=0;i<3;++i){smoothing[(size_t)i].reset(sr*4,.03);smoothing[(size_t)i].setCurrentAndTargetValue(raw[(size_t)i]->load());}inPeak=0;outPeak=0;
 }
 void releaseResources()override{if(os)os->reset();}
 void processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)override{
  juce::ScopedNoDenormals nd;if(!os){b.clear();return;}
  for(int c=getTotalNumInputChannels();c<b.getNumChannels();++c)b.clear(c,0,b.getNumSamples());float ip=0,op=0;
  for(int c=0;c<getTotalNumInputChannels();++c)for(int n=0;n<b.getNumSamples();++n){float& x=b.getWritePointer(c)[n];if(!std::isfinite(x))x=0;ip=std::max(ip,std::abs(x));}
  for(int i=0;i<3;++i)smoothing[(size_t)i].setTargetValue(raw[(size_t)i]->load());
  kw::Settings s;s.voice=(int)std::lround(raw[3]->load());s.path=raw[4]->load()>.5f?1:0;s.inputCap=(int)std::lround(raw[5]->load());s.bypass=(int)std::lround(raw[6]->load());s.drain=(int)std::lround(raw[7]->load());s.coupling=(int)std::lround(raw[8]->load());s.shunt=(int)std::lround(raw[9]->load());s.source=(int)std::lround(raw[10]->load());s.load=raw[11]->load()>.5f?1:0;
  juce::dsp::AudioBlock<float> block(b);
  for(size_t start=0;start<block.getNumSamples();start+=512){auto sub=block.getSubBlock(start,std::min((size_t)512,block.getNumSamples()-start));auto up=os->processSamplesUp(sub);
   for(size_t n=0;n<up.getNumSamples();++n){s.gain=smoothing[0].getNextValue();s.level=smoothing[1].getNextValue();s.texture=smoothing[2].getNextValue();if(n%32==0)for(auto& c:cores)c.configure(s);for(size_t c=0;c<up.getNumChannels();++c)up.getChannelPointer(c)[n]=(float)cores[c].tick(up.getChannelPointer(c)[n]);}os->processSamplesDown(sub);
  }
  for(int c=0;c<getTotalNumOutputChannels();++c)for(int n=0;n<b.getNumSamples();++n){float& y=b.getWritePointer(c)[n];y=std::isfinite(y)?juce::jlimit(-1.f,1.f,y):0;op=std::max(op,std::abs(y));}inPeak.store(std::max(ip,inPeak.load()*.92f));outPeak.store(std::max(op,outPeak.load()*.92f));
 }
 bool hasEditor()const override{return true;}juce::AudioProcessorEditor* createEditor()override;
 void getStateInformation(juce::MemoryBlock& d)override{auto x=state.copyState().createXml();copyXmlToBinary(*x,d);}
 void setStateInformation(const void* d,int n)override{auto x=getXmlFromBinary(d,n);if(x&&x->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*x));}
 void setParam(int i,float x){if(auto* p=state.getParameter(ids[i])){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(x));p->endChangeGesture();}}
 void scream(int v){setParam(3,(float)v);setParam(4,1);setParam(6,0);}
 void resetStock(){for(int i=3;i<N;++i)setParam(i,(float)defaultChoices[i]);setParam(0,.5f);setParam(1,-6);setParam(2,.5f);}
};

// Locally drawn controls: no ComboBox or popup menu is used anywhere.
class KnobLook final:public juce::LookAndFeel_V4 {
public:
 void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float position,float start,float end,juce::Slider&)override{
  auto bounds=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(7);float radius=std::min(bounds.getWidth(),bounds.getHeight())*.5f;auto centre=bounds.getCentre();
  auto circle=juce::Rectangle<float>(radius*2,radius*2).withCentre(centre);
  g.setColour(juce::Colour(0xff314e60));g.fillEllipse(circle.expanded(3));g.setColour(juce::Colour(0xff0d1821));g.fillEllipse(circle);g.setColour(juce::Colour(0xff87a5b7));g.drawEllipse(circle,1.5f);
  float a=start+position*(end-start);juce::Path pointer;pointer.addRoundedRectangle(-2,-radius+8,4,radius*.42f,1.5f);g.setColour(juce::Colour(0xffffcd80));g.fillPath(pointer,juce::AffineTransform::rotation(a).translated(centre.x,centre.y));
 }
};
class Editor final:public juce::AudioProcessorEditor,private juce::Timer {
 Processor& proc;KnobLook look;
 std::array<juce::Slider,10> knobs;
 std::array<juce::Label,10> titles,values;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,10> attachments;
 juce::ToggleButton pathSwitch{"SCREAM (off = WHISPER)"},loadSwitch{"2 MOhm (off = 1 MOhm)"};
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pathAttachment,loadAttachment;
 juce::Label pathTitle,loadTitle,status,meters;
 juce::TextButton reset{"RESET STOCK"},v2{"SCREAM / 2"},v3{"SCREAM / 3"},v4{"SCREAM / 4"};
 juce::String display(int id,float v)const{
  if(id==0||id==2)return juce::String(v*10,1)+" / 10";
  if(id==1)return juce::String(v,1)+" dB";
  auto list=choices(id);int i=juce::jlimit(0,list.size()-1,(int)std::lround(v));return list[i];
 }
 void timerCallback()override{
  for(int i=0;i<10;++i){int id=rotaryIDs[i];values[(size_t)i].setText(display(id,proc.raw[(size_t)id]->load()),juce::dontSendNotification);}
  int voice=(int)std::lround(proc.raw[3]->load()),bp=(int)std::lround(proc.raw[6]->load());bool scream=proc.raw[4]->load()>.5f;
  juce::TextButton* buttons[]={&v2,&v3,&v4};for(int i=0;i<3;++i)buttons[i]->setToggleState(scream&&voice==i+1&&bp==0,juce::dontSendNotification);
  auto voiceNames=choices(3),bypassNames=choices(6);
  status.setText("ACTIVE: "+voiceNames[juce::jlimit(0,3,voice)]+" | "+(scream?juce::String("SCREAM"):juce::String("WHISPER"))+" | BYPASS: "+bypassNames[juce::jlimit(0,4,bp)],juce::dontSendNotification);
  auto db=[](float v){return juce::String(juce::Decibels::gainToDecibels(v,-100.f),1);};meters.setText("IN "+db(proc.inPeak.load())+" dBFS   OUT "+db(proc.outPeak.load())+" dBFS",juce::dontSendNotification);
 }
public:
 explicit Editor(Processor& p):AudioProcessorEditor(p),proc(p){
  setSize(1120,800);
  for(int i=0;i<10;++i){auto k=(size_t)i;int id=rotaryIDs[i];titles[k].setText(names[id],juce::dontSendNotification);titles[k].setJustificationType(juce::Justification::centred);titles[k].setColour(juce::Label::textColourId,juce::Colours::white);addAndMakeVisible(titles[k]);
   knobs[k].setLookAndFeel(&look);knobs[k].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knobs[k].setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);knobs[k].setScrollWheelEnabled(true);knobs[k].setTooltip(id<3?juce::String("Drag to adjust ")+names[id]:juce::String("Stepped component selector: ")+names[id]);addAndMakeVisible(knobs[k]);
   // SliderAttachment applies the parameter range, including integer steps for choices.
   attachments[k]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(proc.state,ids[id],knobs[k]);
   values[k].setJustificationType(juce::Justification::centred);values[k].setColour(juce::Label::textColourId,juce::Colour(0xffffcd80));addAndMakeVisible(values[k]);
  }
  pathTitle.setText("OUTPUT PATH",juce::dontSendNotification);loadTitle.setText("INPUT LOAD",juce::dontSendNotification);
  for(auto* l:{&pathTitle,&loadTitle,&status,&meters}){l->setColour(juce::Label::textColourId,juce::Colours::white);addAndMakeVisible(*l);}
  pathTitle.setJustificationType(juce::Justification::centred);loadTitle.setJustificationType(juce::Justification::centred);status.setJustificationType(juce::Justification::centred);
  pathSwitch.setColour(juce::ToggleButton::textColourId,juce::Colours::white);loadSwitch.setColour(juce::ToggleButton::textColourId,juce::Colours::white);addAndMakeVisible(pathSwitch);addAndMakeVisible(loadSwitch);
  pathAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,ids[4],pathSwitch);loadAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.state,ids[11],loadSwitch);
  for(auto* b:{&reset,&v2,&v3,&v4}){addAndMakeVisible(*b);b->setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff72562f));}
  reset.onClick=[this]{proc.resetStock();};v2.onClick=[this]{proc.scream(1);};v3.onClick=[this]{proc.scream(2);};v4.onClick=[this]{proc.scream(3);};
  startTimerHz(20);timerCallback();
 }
 ~Editor()override{stopTimer();for(auto& k:knobs)k.setLookAndFeel(nullptr);}
 void paint(juce::Graphics& g)override{
  g.fillAll(juce::Colour(0xff101a23));g.setColour(juce::Colour(0xffffcd80));g.setFont(26.f);g.drawText("KW BUILDER V3 - KNOBS + SWITCHES",24,12,1070,38,juce::Justification::centredLeft);
  g.setColour(juce::Colours::lightgrey);g.setFont(12.f);g.drawText("10 visible rotary controls + 2 switches | drag up/down on each knob | fixed steps for circuit parts",24,53,1070,24,juce::Justification::centredLeft);
  g.setColour(juce::Colour(0xff203340));g.fillRoundedRectangle(18.f,278.f,1084.f,366.f,12.f);
  g.setColour(juce::Colours::lightgrey);g.drawText("BUILDER PARTS - EACH TITLE HAS A ROTARY CONTROL AND CURRENT VALUE",29,281,1040,25,juce::Justification::centredLeft);
  g.drawText("Scream buttons change voice + output + Follow Voice. Main knobs and other component edits are preserved.",24,735,1070,23,juce::Justification::centredLeft);
  g.drawText("Behavioral prototype, not a matched capture. Reference GAIN = 5/10. No amp/cab. Start monitoring quietly.",24,762,1070,22,juce::Justification::centredLeft);
 }
 void resized()override{
  for(int i=0;i<3;++i){int x=83+i*353;titles[(size_t)i].setBounds(x,88,247,24);knobs[(size_t)i].setBounds(x+46,115,155,128);values[(size_t)i].setBounds(x,244,247,25);}
  for(int j=0;j<7;++j){int i=j+3,col=j%4,row=j/4,x=29+col*271,y=313+row*161;titles[(size_t)i].setBounds(x,y,250,24);knobs[(size_t)i].setBounds(x+70,y+25,110,105);values[(size_t)i].setBounds(x,y+131,250,24);}
  pathTitle.setBounds(848,480,237,25);pathSwitch.setBounds(848,508,237,35);loadTitle.setBounds(848,552,237,25);loadSwitch.setBounds(848,580,237,35);
  reset.setBounds(24,660,170,36);v2.setBounds(210,660,170,36);v3.setBounds(396,660,170,36);v4.setBounds(582,660,170,36);meters.setBounds(766,660,330,36);status.setBounds(24,702,1070,26);
 }
};
juce::AudioProcessorEditor* Processor::createEditor(){return new Editor(*this);}juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Processor();}
