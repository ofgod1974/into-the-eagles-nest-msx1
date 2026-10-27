const fs=require('fs'),path=require('path');
const root=path.resolve(__dirname,'..');
const source=fs.readFileSync(path.join(root,'assets/source/IntoEaglesNest_Theme.abc.txt'),'utf8');
const voices=['','',''];let active=-1;
for(let line of source.split(/\r?\n/)){
 if(/^V:[123]\s*$/.test(line)){active=Number(line[2])-1;continue}
 if(/^[A-Z]:/.test(line))continue;if(active>=0)voices[active]+=' '+line;
}
function parse(text){let events=[],barTicks=0,bars=0,nextFactor=1,consumed='';
 const regex=/([A-Ga-gz])([,']*)(\d*)(\/\d*)?|([<>])|(\|:?)/g;let last=0;
 for(let m of text.matchAll(regex)){
  let between=text.slice(last,m.index).replace(/[\s:]/g,'');if(between)throw Error('Unsupported ABC '+between);last=m.index+m[0].length;
  if(m[5]){let factor=m[5]==='>'?1.5:0.5;let prev=events.at(-1),delta=prev.duration*(factor-1);prev.duration*=factor;barTicks+=delta;nextFactor=m[5]==='>'?0.5:1.5;continue}
  if(m[6]){if(barTicks!==36)throw Error('Unexpected bar length '+barTicks);barTicks=0;bars++;continue}
  const duration=12*(m[3]?Number(m[3]):1)/(m[4]?(m[4].length===1?2:Number(m[4].slice(1))):1)*nextFactor;nextFactor=1;
  if(duration!==Math.round(duration)||duration<1||duration>255)throw Error('Invalid duration');
  let midi=null,period=0;if(m[1]!=='z'){
   midi={C:60,D:62,E:64,F:65,G:67,A:69,B:71}[m[1].toUpperCase()]+(m[1]===m[1].toLowerCase()?12:0);
   for(let modifier of m[2])midi+=modifier===','?-12:12;
   period=Math.round(1789772.5/(16*440*Math.pow(2,(midi-69)/12)));
   if(period<1||period>4095)throw Error('Out of range note '+midi);
  }
  events.push({period,duration,midi});barTicks+=duration;
 }
 if(text.slice(last).replace(/[\s:]/g,''))throw Error('Unparsed ABC tail');
 return {events,bars,ticks:events.reduce((s,e)=>s+e.duration,0)};
}
const tracks=voices.map(parse);if(!tracks.every(t=>t.ticks===tracks[0].ticks&&t.bars===tracks[0].bars))throw Error('Voices are not synchronized');
let header='/* Original C64 theme, ABC transcription by Stephan Pabst / Stephan64. */\n';
header+='typedef struct { unsigned int period; unsigned char duration; } MusicNote;\n';
for(let i=0;i<3;i++)header+='static const MusicNote original_voice_'+i+'['+tracks[i].events.length+']={'+tracks[i].events.map(e=>'{'+e.period+','+e.duration+'}').join(',')+'};\n';
header+='static const MusicNote * const original_voices[3]={original_voice_0,original_voice_1,original_voice_2};\n';
header+='static const unsigned int original_lengths[3]={'+tracks.map(t=>t.events.length).join(',')+'};\n';
header+='#define ORIGINAL_THEME_TICKS '+tracks[0].ticks+'\n';
fs.writeFileSync(path.join(root,'src/original_music.h'),header);
const metadata={title:"Into Eagle's Nest Theme (Commodore 64)",credit:'Music: Keith Harvey; ABC transcription: Stephan Pabst (Stephan64)',source:'https://www.c64-wiki.de/images/0/0c/IntoEaglesNest_Theme.abc.txt',tempoQuarter:250,meter:'3/4',ticksPerSecond:50,bars:tracks[0].bars,durationSeconds:tracks[0].ticks/50,events:tracks.map(t=>t.events.length),tracks};
fs.writeFileSync(path.join(root,'assets/source/original-music-score.json'),JSON.stringify(metadata,null,2));console.log(JSON.stringify({...metadata,tracks:undefined}));
