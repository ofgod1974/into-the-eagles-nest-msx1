#include "msx_fusion.h"
#include "audio.h"
#include "original_music.h"
typedef unsigned char u8;
typedef unsigned int u16;
void AudioInstallHook(void);
u8 AudioReadSlot(u8 slot,u16 address);
void AudioWriteSlot(u8 slot,u16 address,u8 value);
void AudioPSG(u8 reg,u8 value);
volatile u8 music_enabled,music_scc,music_loops;
volatile u16 music_clock;
static u8 scc_slot,refresh_rate,refresh_accumulator,psg_io;
static u16 next_note[3],note_period[3];
static u8 remaining[3],age[3];
static volatile u8 effect_request;
static u8 effect_kind,effect_age;
static const u8 levels[3]={13,8,11};
/* Signed SCC wavetable samples: pulse lead, triangle harmony, saw bass,
 * soft body layer. Channels 4/5 share the last waveform on the original SCC. */
static const signed char waves[4][32]={
 {100,110,116,120,120,116,110,100,90,75,50,20,-20,-50,-75,-90,-100,-110,-116,-120,-120,-116,-110,-100,-90,-75,-50,-20,20,50,75,90},
 {0,12,24,36,48,60,72,84,96,84,72,60,48,36,24,12,0,-12,-24,-36,-48,-60,-72,-84,-96,-84,-72,-60,-48,-36,-24,-12},
 {100,94,88,82,76,70,64,58,52,46,40,34,28,22,16,10,0,-10,-16,-22,-28,-34,-40,-46,-52,-58,-64,-70,-76,-82,-88,-94},
 {0,18,35,50,64,76,85,91,94,91,85,76,64,50,35,18,0,-18,-35,-50,-64,-76,-85,-91,-94,-91,-85,-76,-64,-50,-35,-18}
};
static void scc(u16 address,u8 value){AudioWriteSlot(scc_slot,address,value);}
static u8 probe(u8 slot){u8 old;
 /* Reject RAM before enabling the cartridge: ordinary writable RAM is not SCC. */
 old=AudioReadSlot(slot,0x9800);AudioWriteSlot(slot,0x9800,old^0x55);
 if(AudioReadSlot(slot,0x9800)==(u8)(old^0x55)){AudioWriteSlot(slot,0x9800,old);return 0;}
 AudioWriteSlot(slot,0x9000,0x3f);
 AudioWriteSlot(slot,0x9800,0x55);if(AudioReadSlot(slot,0x9800)!=0x55)return 0;
 AudioWriteSlot(slot,0x9800,0xaa);if(AudioReadSlot(slot,0x9800)!=0xaa)return 0;
 AudioWriteSlot(slot,0x9000,0);AudioWriteSlot(slot,0x9800,0x77);
 if(AudioReadSlot(slot,0x9800)==0x77)return 0;
 AudioWriteSlot(slot,0x9000,0x3f);return 1;
}
static void frequency(u8 channel,u16 period){u8 reg;
 if(!period)period=1;
 if(music_scc){period--;scc(0x9880+((u16)channel<<1),(u8)period);scc(0x9881+((u16)channel<<1),period>>8);}
 else {reg=channel<<1;AudioPSG(reg,(u8)period);AudioPSG(reg+1,period>>8);}
}
void AudioInit(void){u8 primary,secondary,count,slot,i,j;
 __asm di __endasm;
 music_enabled=1;music_scc=0;music_clock=0;music_loops=0;effect_request=0;effect_kind=0;refresh_accumulator=0;
 refresh_rate=(Peek(0x002b)&0x80)?50:60;
 OutPort(0xa0,7);psg_io=InPort(0xa2)&0xc0;
 for(i=0;i<3;i++){next_note[i]=0;note_period[i]=0;remaining[i]=0;age[i]=0;AudioPSG(8+i,0);}
 AudioPSG(7,psg_io|0x3f);
 for(primary=1;primary<4&&!music_scc;primary++){
 count=(Peek(0xfcc1+primary)&0x80)?4:1;
 for(secondary=0;secondary<count&&!music_scc;secondary++){
 slot=primary;if(count==4)slot|=0x80|(secondary<<2);
 if(probe(slot)){scc_slot=slot;music_scc=1;}
 }}
 if(music_scc){for(i=0;i<4;i++)for(j=0;j<32;j++)scc(0x9800+((u16)i<<5)+j,(u8)waves[i][j]);for(i=0;i<5;i++)scc(0x988a+i,0);scc(0x988f,31);}
 AudioInstallHook();
 __asm ei __endasm;
}
void AudioEffect(u8 effect){effect_request=effect;}
void AudioToggle(void){music_enabled^=1;}
/* Runs on the BIOS timer interrupt, independently from camera/map rendering. */
void AudioTick(void){u8 channel,volume,mixer,new_note;u16 period;
 refresh_accumulator+=50;if(refresh_accumulator<refresh_rate)return;refresh_accumulator-=refresh_rate;
 music_clock++;mixer=music_scc?0x3f:0x38;
 for(channel=0;channel<3;channel++){
 new_note=0;
 if(!remaining[channel]){
 if(next_note[channel]>=original_lengths[channel]){next_note[channel]=0;if(!channel)music_loops++;}
 note_period[channel]=original_voices[channel][next_note[channel]].period;
 remaining[channel]=original_voices[channel][next_note[channel]].duration;
 next_note[channel]++;age[channel]=0;new_note=1;
 }
 period=note_period[channel];volume=levels[channel];
 if(age[channel]>1)volume-=2;
 if(remaining[channel]==1||!period||!music_enabled)volume=0;
 if(new_note){frequency(channel,period);if(music_scc&&channel==0)frequency(3,period);if(music_scc&&channel==2)frequency(4,period);}
 if(music_scc){scc(0x988a+channel,volume);if(channel==0)scc(0x988d,volume?3:0);if(channel==2)scc(0x988e,volume?2:0);}
 else AudioPSG(8+channel,volume);
 remaining[channel]--;age[channel]++;
 }
 if(effect_request){effect_kind=effect_request;effect_request=0;effect_age=0;}
 if(effect_kind){
 volume=0;period=100;
 if(effect_kind==FX_SHOT||effect_kind==FX_DOOR){
 if(effect_age<(effect_kind==FX_SHOT?5:8)){volume=13-effect_age;AudioPSG(6,3+effect_age*3);mixer=(mixer|4)&~32;}
 }else if(effect_kind==FX_PICKUP){
 if(effect_age<12){volume=12-(effect_age>>1);period=effect_age<4?107:(effect_age<8?85:71);mixer&=~4;}
 }else if(effect_kind==FX_HIT||effect_kind==FX_KILL){
 if(effect_age<10){volume=13-effect_age;period=300+(u16)effect_age*90;AudioPSG(6,16+effect_age);mixer&=~36;}
 }
 if(volume){AudioPSG(4,(u8)period);AudioPSG(5,period>>8);AudioPSG(10,volume);effect_age++;}
 else {effect_kind=0;if(music_scc)AudioPSG(10,0);else frequency(2,note_period[2]);}
 }
 AudioPSG(7,psg_io|mixer);
}
