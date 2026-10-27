// Restore the exact Windows SDCC distribution used to build this project.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'..'),lock=JSON.parse(fs.readFileSync(path.join(root,'toolchain.lock.json'),'utf8'));
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
if(process.platform!=='win32')throw Error('This bootstrap installs the Windows toolchain.');
let cursor=0,downloaded=0,verified=0;
async function worker(){while(cursor<lock.files.length){
 const item=lock.files[cursor++],dest=path.join(root,item.path);
 if(fs.existsSync(dest)&&hash(fs.readFileSync(dest))===item.sha256){verified++;continue;}
 const url='https://raw.githubusercontent.com/'+lock.repository+'/'+lock.commit+'/'+item.source;
 const response=await fetch(url,{signal:AbortSignal.timeout(60000)});
 if(!response.ok)throw Error('Download failed: '+item.source+' HTTP '+response.status);
 const data=Buffer.from(await response.arrayBuffer());if(hash(data)!==item.sha256)throw Error('Checksum mismatch: '+item.source);
 fs.mkdirSync(path.dirname(dest),{recursive:true});const temporary=dest+'.download';fs.writeFileSync(temporary,data);fs.renameSync(temporary,dest);downloaded++;
}}
(async()=>{await Promise.all(Array.from({length:4},worker));console.log('SDCC '+lock.version+': '+verified+' files verified, '+downloaded+' files installed.');})().catch(e=>{console.error(e.message);process.exitCode=1});
