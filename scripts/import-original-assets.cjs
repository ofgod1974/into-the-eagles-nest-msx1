const fs=require('fs'),path=require('path'),{png}=require('./png.cjs');
const root=path.resolve(__dirname,'..'),src=path.join(root,'assets/source');
const {tiles,maps}=JSON.parse(fs.readFileSync(path.join(src,'c64-decoded.json')));
const sprites=JSON.parse(fs.readFileSync(path.join(src,'amiga-sprites-decoded.json')));
const rgb=[[0,0,0],[0,0,0],[62,184,73],[116,208,125],[84,85,237],[125,118,252],[212,82,77],[66,235,245],[252,85,84],[255,121,120],[212,193,84],[230,206,128],[33,176,59],[201,91,186],[204,204,204],[255,255,255]];
const cmap=[1,11,15,10,7,1,14,12,14,6,3,7,13,8,9,15,15,4,5,4,5,4,4,14,6];
const patterns=Array.from({length:256},()=>Array(8).fill(0));
const colors=Array.from({length:256},()=>Array(8).fill(0x11));
function dist(a,b){return rgb[a].reduce((s,v,i)=>s+(v-rgb[b][i])**2,0)}
for(let t=0;t<tiles.length;t++)for(let y=0;y<8;y++){
 const bayer=[[0,8,2,10],[12,4,14,6],[3,11,1,9],[15,7,13,5]];
 let row=tiles[t].slice(y*8,y*8+8).map((c,x)=>c===6?(bayer[y&3][x&3]<7?14:1):cmap[c]);
 let choices=[...new Set(row)],best=Infinity,bg=1,fg=1;
 for(let a of choices)for(let b of choices){let error=row.reduce((s,c)=>s+Math.min(dist(c,a),dist(c,b)),0);if(error<best){best=error;bg=a;fg=b}}
 let bits=0;for(let x=0;x<8;x++)if(dist(row[x],fg)<dist(row[x],bg))bits|=128>>x;
 patterns[t][y]=bits;colors[t][y]=fg*16+bg;
}
colors[152].fill(0xbb);colors[153].fill(0x11);colors[154].fill(0x77);colors[155].fill(0x11);colors[156].fill(0x88);
const font={
A:'01110/10001/10001/11111/10001/10001/10001',B:'11110/10001/10001/11110/10001/10001/11110',C:'01111/10000/10000/10000/10000/10000/01111',D:'11110/10001/10001/10001/10001/10001/11110',E:'11111/10000/10000/11110/10000/10000/11111',F:'11111/10000/10000/11110/10000/10000/10000',G:'01111/10000/10000/10111/10001/10001/01110',H:'10001/10001/10001/11111/10001/10001/10001',I:'11111/00100/00100/00100/00100/00100/11111',J:'00111/00010/00010/00010/00010/10010/01100',K:'10001/10010/10100/11000/10100/10010/10001',L:'10000/10000/10000/10000/10000/10000/11111',M:'10001/11011/10101/10101/10001/10001/10001',N:'10001/11001/10101/10011/10001/10001/10001',O:'01110/10001/10001/10001/10001/10001/01110',P:'11110/10001/10001/11110/10000/10000/10000',Q:'01110/10001/10001/10001/10101/10010/01101',R:'11110/10001/10001/11110/10100/10010/10001',S:'01111/10000/10000/01110/00001/00001/11110',T:'11111/00100/00100/00100/00100/00100/00100',U:'10001/10001/10001/10001/10001/10001/01110',V:'10001/10001/10001/10001/10001/01010/00100',W:'10001/10001/10001/10101/10101/11011/10001',X:'10001/10001/01010/00100/01010/10001/10001',Y:'10001/10001/01010/00100/00100/00100/00100',Z:'11111/00001/00010/00100/01000/10000/11111',
0:'01110/10011/10101/10101/11001/10001/01110',1:'00100/01100/00100/00100/00100/00100/01110',2:'01110/10001/00001/00010/00100/01000/11111',3:'11110/00001/00001/01110/00001/00001/11110',4:'00010/00110/01010/10010/11111/00010/00010',5:'11111/10000/10000/11110/00001/00001/11110',6:'01110/10000/10000/11110/10001/10001/01110',7:'11111/00001/00010/00100/01000/01000/01000',8:'01110/10001/10001/01110/10001/10001/01110',9:'01110/10001/10001/01111/00001/00001/01110',
' ':'00000/00000/00000/00000/00000/00000/00000','-':'00000/00000/00000/11111/00000/00000/00000'};
for(let [ch,rows] of Object.entries(font)){let id=ch.charCodeAt(0)+128;patterns[id]=rows.split('/').map(r=>parseInt(r,2)<<2).concat(0);colors[id].fill(0x1b)}
const attributes=Array(152).fill(0);
function attr(ids,n){for(let id of ids)attributes[id]=n}
attr([0,47,48,55,56,67,82,83,84,85,88,95],1);
attr([22,34,66,68],2);
attr([4,5,49,64,65,86,101,102,103,104,107,108,110],3);
attr([3,8,9,96,97,99,100,112,123,124,125],4);
attr([14,15],5);
attr([28,29,42,43,44,45,50,51,52,53,60,63],6);
attr([20,21,25,26,27,37,38,39,69,70,71,72,79,80,81],7);
attr([17,18,89,90],8);
attr([73,74,105,106,119,120,121,122,138,139],9);
// The atlas contains the stationary player at the initial position. Remove
// those four photographed sprite fragments from the background map.
for(let m of maps)for(let i=0;i<m.length;i++)if(m[i]>=113&&m[i]<=116)m[i]=6;
// PackBits: literal runs and repeated runs; floor data stays exact after decode.
function pack(a){let out=[];for(let i=0;i<a.length;){let run=1;while(i+run<a.length&&a[i+run]===a[i]&&run<128)run++;if(run>=3){out.push(127+run,a[i]);i+=run}else{let start=i;i+=run;while(i<a.length&&i-start<128){let n=1;while(i+n<a.length&&a[i+n]===a[i]&&n<3)n++;if(n>=3)break;i+=Math.min(n,128-(i-start))}out.push(i-start-1,...a.slice(start,i))}}return out}
const packed=maps.map(pack);
const overviewRows=maps.map(m=>Array.from({length:256},(_,i)=>Array.from({length:4},(_,y)=>{
 let bits=0;for(let x=0;x<4;x++){let a=attributes[m[((i>>4)*4+y)*64+(i%16)*4+x]];if(a>=1&&a<=4)bits|=0xc0>>(x*2)}return bits;
})));
// Native source frames are two characters wide, three high (16x24 after
// removing the doubled horizontal Amiga pixels). Three transparent layers.
let frames=[];for(let set=0;set<2;set++)for(let f=0;f<8;f++){
 let out=[];for(let layer=0;layer<4;layer++)for(let part=0;part<2;part++){
  let bytes=Array(32).fill(0);for(let y=0;y<16;y++)for(let x=0;x<16;x++){
   let sy=part*16+y;if(sy>=24)continue;let c=sprites[set][f][sy*16+x],l=-1;
   // Amiga characters already contain floor pixels (palette 0,7,9).
   // They are a background matte, not part of the soldier silhouette.
   if(c&&c!==7&&c!==9){if([10,11,13,15].includes(c))l=1;else if(set?([2,8].includes(c)):c===6)l=2;else if(c===14||(!set&&c===1))l=3;else l=0}
   if(l===layer)bytes[(x>>3)*16+y]|=128>>(x&7);
  }out.push(bytes);
 }frames.push(out);
}
function carray(name,a,type='unsigned char'){return 'static const '+type+' '+name+'='+JSON.stringify(a).replaceAll('[','{').replaceAll(']','}')+';\n'}
let c='/* Extracted C64 background/maps and Amiga directional frames. See ASSET-SOURCES.md. */\n';
c+=carray('tile_pattern[256][8]',patterns)+carray('tile_color[256][8]',colors)+carray('tile_attr[152]',attributes);
for(let f=0;f<4;f++)c+=carray('floor_'+f+'['+packed[f].length+']',packed[f]);
c+='static const unsigned char * const floor_data[4]={floor_0,floor_1,floor_2,floor_3};\n';
c+=carray('actor_frames[16][8][32]',frames);
// The runtime builds the small overview cache on entering each floor.
// This leaves cartridge space for the original theme and SCC driver.
fs.writeFileSync(path.join(root,'src/original_assets.h'),c);
// Authoring previews use the exact per-scanline two-colour ROM data.
function pixel(id,x,y){return rgb[(patterns[id][y]&(128>>x))?(colors[id][y]>>4):(colors[id][y]&15)]}
let w=16*40,h=Math.ceil(tiles.length/16)*40,img=Buffer.alloc(w*h*3,25);
for(let t=0;t<tiles.length;t++)for(let y=0;y<8;y++)for(let x=0;x<8;x++)for(let sy=0;sy<4;sy++)for(let sx=0;sx<4;sx++)img.set(pixel(t,x,y),(((t>>4)*40+y*4+sy)*w+(t%16)*40+x*4+sx)*3);
png(path.join(root,'assets/previews/msx1-tiles.png'),w,h,img);
w=8*72;h=2*112;img=Buffer.alloc(w*h*3,25);
for(let f=0;f<16;f++)for(let y=0;y<24;y++)for(let x=0;x<16;x++){
 let c=1;for(let layer of [0,3,1,2]){let b=frames[f][layer*2+(y>>4)][(x>>3)*16+(y&15)];if(b&(128>>(x&7)))c=f<8?[12,10,3,1][layer]:[14,10,15,1][layer]}
 for(let sy=0;sy<4;sy++)for(let sx=0;sx<4;sx++)img.set(rgb[c],((((f>>3)*112)+y*4+sy)*w+(f&7)*72+x*4+sx)*3);
}
png(path.join(root,'assets/previews/msx1-sprites.png'),w,h,img);
w=1024;h=1024;img=Buffer.alloc(w*h*3);for(let f=0;f<4;f++)for(let ty=0;ty<64;ty++)for(let tx=0;tx<64;tx++)for(let y=0;y<8;y++)for(let x=0;x<8;x++)img.set(pixel(maps[f][ty*64+tx],x,y),((((f>>1)*512+ty*8+y)*w+(f&1)*512+tx*8+x)*3));
png(path.join(root,'assets/previews/four-floors-msx1.png'),w,h,img);
// Verify every encoded map byte before emitting it into the ROM.
for(let f=0;f<4;f++){let result=[];for(let i=0;i<packed[f].length;){let n=packed[f][i++];if(n&128){let value=packed[f][i++];result.push(...Array(n-127).fill(value))}else{result.push(...packed[f].slice(i,i+n+1));i+=n+1}}if(JSON.stringify(result)!==JSON.stringify(maps[f]))throw Error('Map roundtrip failed '+f)}
console.log(JSON.stringify({sourceTiles:tiles.length,mapDimensions:'64x64',floors:4,compressedMapBytes:packed.map(p=>p.length),actorFrames:frames.length,hardware:'TMS9918 two colours per 8-pixel scanline'}));
