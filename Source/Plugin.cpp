#include <JuceHeader.h>
#include "BuilderDSP.h"
#include <array>

namespace {
constexpr int count=12;
const char* ids[count]={"gain","level","voice","path","inputCap","bypass","drain","coupling","shunt","source","load","texture"};
const char* names[count]={"GAIN","OUTPUT LEVEL","SOURCE VOICE","OUTPUT PATH","INPUT CAP C1","Q1 BYPASS CAP","Q1 DRAIN R4","COUPLING C4","HF SHUNT C3","Q2 SOURCE R8","INPUT LOAD","DEVICE TEXTURE"};
juce::AudioProcessorValueTreeState::ParameterLayout layout() {
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 using ID=juce::ParameterID;
 l.add(std::make_unique<juce::AudioParameterFloat>(ID{ids[0],1},names[0],0.0f,1.0f,.45f));
 l.add(std::make_unique<juce::AudioParameterFloat>(ID{ids[1],1},names[1],-24.0f,12.0f,-6.0f));
 auto choice=[&](int n,juce::StringArray items,int def){l.add(std::make_unique<juce::AudioParameterChoice>(ID{ids[n],1},names[n],items,def));};
 choice(2,{"1 Calm","2 Sullen","3 Moody","4 Angry"},1);
 choice(3,{"Whisper","Scream"},1);
 choice(4,{"47 nF","100 nF (stock)","220 nF"},1);
 choice(5,{"Follow voice (stock)","Off","100 nF","1 uF","22 uF"},0);
 choice(6,{"6.8 kOhm","10 kOhm (stock)","15 kOhm"},1);
 choice(7,{"10 nF","22 nF (stock)","47 nF"},1);
 choice(8,{"10 nF","22 nF (stock)","47 nF"},1);
 choice(9,{"680 Ohm","1 kOhm (stock)","1.5 kOhm"},1);
 choice(10,{"1 MOhm (stock)","2 MOhm experiment"},0);
 l.add(std::make_unique<juce::AudioParameterFloat>(ID{ids[11],1},names[11],.5f,1.5f,1.0f));
 return l;
}
}
class BuilderEditor;
class BuilderProcessor final : public juce::AudioProcessor {
public:
 juce::AudioProcessorValueTreeState state;
 std::array<std::atomic<float>*,count> raw{};
 std::array<juce::SmoothedValue<float>,count> smooth;
 std::array<kw::Core,2> cores;
 std::unique_ptr<juce::dsp::Oversampling<float>> os;
 static constexpr int chunkSize=512;
 BuilderProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"KWBuilderState",layout()) {
  for(int i=0;i<count;++i) raw[(size_t)i]=state.getRawParameterValue(ids[i]);
 }
 const juce::String getName() const override{return "KW Builder Prototype";}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;}
 double getTailLengthSeconds() const override{return 0;}
 int getNumPrograms() override{return 1;} int getCurrentProgram() override{return 0;}
 void setCurrentProgram(int) override{} const juce::String getProgramName(int) override{return {};}
 void changeProgramName(int,const juce::String&) override{}
 bool isBusesLayoutSupported(const BusesLayout& b) const override {
  auto in=b.getMainInputChannelSet(),out=b.getMainOutputChannelSet();
  return in==out && (out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo());
 }
 void prepareToPlay(double sr,int) override {
  os=std::make_unique<juce::dsp::Oversampling<float>>((size_t)getTotalNumInputChannels(),2,juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,true);
  os->initProcessing(chunkSize);os->reset();setLatencySamples((int)std::lround(os->getLatencyInSamples()));
  for(auto& core:cores)core.prepare(sr*4);
  for(int i=0;i<count;++i){smooth[(size_t)i].reset(sr*4,.035);smooth[(size_t)i].setCurrentAndTargetValue(raw[(size_t)i]->load());}
 }
 void releaseResources() override{if(os)os->reset();}
 void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&) override {
  juce::ScopedNoDenormals noDenormals;
  if(!os){buffer.clear();return;}
  for(int c=getTotalNumInputChannels();c<buffer.getNumChannels();++c)buffer.clear(c,0,buffer.getNumSamples());
  for(int i=0;i<count;++i)smooth[(size_t)i].setTargetValue(raw[(size_t)i]->load());
  juce::dsp::AudioBlock<float> block(buffer);
  for(size_t start=0;start<block.getNumSamples();start+=chunkSize){
   auto sub=block.getSubBlock(start,juce::jmin((size_t)chunkSize,block.getNumSamples()-start));
   auto up=os->processSamplesUp(sub);
   for(size_t n=0;n<up.getNumSamples();++n){
    std::array<float,count> v{};for(int i=0;i<count;++i)v[(size_t)i]=smooth[(size_t)i].getNextValue();
    if((n%32)==0){kw::Settings s; s.gain=v[0];s.level=v[1];s.voice=v[2];s.path=v[3];s.inputCap=v[4];s.bypass=v[5];s.drain=v[6];s.coupling=v[7];s.shunt=v[8];s.source=v[9];s.load=v[10];s.texture=v[11];for(auto& c:cores)c.configure(s);}
    for(size_t c=0;c<up.getNumChannels();++c)up.getChannelPointer(c)[n]=(float)cores[c].tick(up.getChannelPointer(c)[n]);
   }
   os->processSamplesDown(sub);
  }
 }
 bool hasEditor() const override{return true;} juce::AudioProcessorEditor* createEditor() override;
 void getStateInformation(juce::MemoryBlock& dest) override {auto xml=state.copyState().createXml();copyXmlToBinary(*xml,dest);}
 void setStateInformation(const void* data,int size) override {auto xml=getXmlFromBinary(data,size);if(xml&&xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
 void stock(int voiceIndex=1){
  const float defaults[count]={.45f,-6.f,(float)voiceIndex,1,1,0,1,1,1,1,0,1};
  for(int i=0;i<count;++i)if(auto* p=state.getParameter(ids[i])){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(defaults[i]));p->endChangeGesture();}
 }
};
class BuilderEditor final : public juce::AudioProcessorEditor {
 BuilderProcessor& proc;
 std::array<juce::Label,count> labels;
 std::array<juce::Slider,count> knobs;
 std::array<juce::ComboBox,count> boxes;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,count> sliders;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>,count> combos;
 juce::TextButton stockButton{"RESET STOCK"},v2{"SCREAM / 2"},v3{"SCREAM / 3"},v4{"SCREAM / 4"};
public:
 explicit BuilderEditor(BuilderProcessor& p):AudioProcessorEditor(p),proc(p){
  setSize(1040,670);
  for(int i=0;i<count;++i){auto k=(size_t)i;labels[k].setText(names[i],juce::dontSendNotification);labels[k].setJustificationType(juce::Justification::centred);addAndMakeVisible(labels[k]);
   if(auto* param=dynamic_cast<juce::AudioParameterChoice*>(p.state.getParameter(ids[i]))){boxes[k].addItemList(param->choices,1);addAndMakeVisible(boxes[k]);combos[k]=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,ids[i],boxes[k]);}
   else {knobs[k].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knobs[k].setTextBoxStyle(juce::Slider::TextBoxBelow,false,100,22);knobs[k].setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffffca7a));addAndMakeVisible(knobs[k]);sliders[k]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,ids[i],knobs[k]);}
  }
  for(auto* b:{&stockButton,&v2,&v3,&v4})addAndMakeVisible(*b);
  stockButton.onClick=[this]{proc.stock();};v2.onClick=[this]{proc.stock(1);};v3.onClick=[this]{proc.stock(2);};v4.onClick=[this]{proc.stock(3);};
 }
 void paint(juce::Graphics& g) override {
  g.fillAll(juce::Colour(0xff111b24));g.setColour(juce::Colour(0xffffca7a));g.setFont(26.f);g.drawText("K/W BUILDER",24,12,650,38,juce::Justification::centredLeft);
  g.setColour(juce::Colours::lightgrey);g.setFont(13.f);g.drawText("FIXED OPTIONS / EXPERIMENTAL BEHAVIORAL PROTOTYPE / NOT REFERENCE-MATCHED",24,53,990,25,juce::Justification::centredLeft);
  g.setFont(12.f);g.drawText("C3 stock = 22 nF. Voice presets reset all controls. Use low monitor volume; protection may clip extreme builds.",24,622,990,25,juce::Justification::centredLeft);
  g.drawText("Save builds with your DAW's plugin preset/state system. DEVICE TEXTURE is a proxy, not a transistor part selector.",24,645,990,20,juce::Justification::centredLeft);
 }
 void resized() override {
  for(int i=0;i<count;++i){int col=i%4,row=i/4,x=24+col*254,y=95+row*153;labels[(size_t)i].setBounds(x,y,238,24);if(combos[(size_t)i])boxes[(size_t)i].setBounds(x+4,y+59,230,32);else knobs[(size_t)i].setBounds(x+35,y+27,168,112);}
  stockButton.setBounds(24,567,190,36);v2.setBounds(230,567,190,36);v3.setBounds(436,567,190,36);v4.setBounds(642,567,190,36);
 }
};
juce::AudioProcessorEditor* BuilderProcessor::createEditor(){return new BuilderEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new BuilderProcessor();}
