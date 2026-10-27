const fs=require('fs'),path=require('path'),{read}=require('./png.cjs');
const src=path.resolve(__dirname,'../assets/source'),im=read(path.join(src,'c64-map-frelujean.png'));
let tiles=[],dict=new Map(),maps=[];
for(let f=0;f<4;f++){let map=[];for(let ty=0;ty<64;ty++)for(let tx=0;tx<64;tx++){
 let pixels=[];for(let y=0;y<8;y++)for(let x=0;x<8;x++)pixels.push(im.pix[(311+f*1143+ty*16+y*2)*im.w+10+tx*16+x*2]);
 let key=pixels.join(',');if(!dict.has(key)){dict.set(key,tiles.length);tiles.push(pixels)}map.push(dict.get(key));
}maps.push(map)}
if(tiles.length!==152)throw Error('Unexpected atlas layout');
fs.writeFileSync(path.join(src,'c64-decoded.json'),JSON.stringify({tiles,maps,pal:im.pal}));
const gfx=fs.readFileSync(path.join(src,'gfx')),asm=fs.readFileSync(path.join(src,'eagle-amiga.asm'),'utf8');
let amigaTiles=Array.from({length:256},(_,t)=>Array.from({length:8},(_,y)=>Array.from({length:16},(_,x)=>{
 let c=0;for(let plane=0;plane<4;plane++)c|=((gfx[t*64+y*8+plane*2+(x>>3)]>>(7-(x&7)))&1)<<plane;return c;
})));
function frames(label){let rows=asm.slice(asm.indexOf(label+':')).split(/\r?\n/),values=[];
 for(let line of rows){if(!line.includes('dc.b'))break;values.push(...[...line.matchAll(/\$([A-Fa-f0-9]+)/g)].map(m=>parseInt(m[1],16)))}
 return Array.from({length:values.length/6},(_,f)=>Array.from({length:384},(_,i)=>{
  let y=i>>4,x=i&15,t=values[f*6+(y>>3)*2+(x>>3)];return amigaTiles[t][y&7][(x&7)*2];
 }));
}
let actors=[frames('YCHRS'),frames('NMECHR')];
if(actors[0].length!==12||actors[1].length!==8)throw Error('Unexpected actor frame tables');
fs.writeFileSync(path.join(src,'amiga-sprites-decoded.json'),JSON.stringify(actors));
console.log('Extracted 152 C64 tiles, four 64x64 maps, 12 player frames and 8 guard frames.');
