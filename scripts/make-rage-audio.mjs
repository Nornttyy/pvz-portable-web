// Derive a three-second pitched cartoon vocal from the documented CC0 recording.
import {readFile,writeFile} from 'node:fs/promises';
const b=await readFile(process.argv[2]);let data,rate,channels,bits;
for(let o=12;o+8<=b.length;){const id=b.toString('ascii',o,o+4),n=b.readUInt32LE(o+4);if(id==='fmt '){channels=b.readUInt16LE(o+10);rate=b.readUInt32LE(o+12);bits=b.readUInt16LE(o+22);}if(id==='data')data=b.subarray(o+8,o+8+n);o+=8+n+(n%2);}
if(bits!==16||!data||!rate)throw Error('16-bit PCM WAV required');
const count=data.length/(2*channels),input=Float32Array.from({length:count},(_,i)=>{let v=0;for(let c=0;c<channels;c++)v+=data.readInt16LE((i*channels+c)*2)/32768;return v/channels;});
let first=input.findIndex(v=>Math.abs(v)>.06),last=count-1;
while(last>first&&Math.abs(input[last])<=.06)--last;
first=Math.max(0,first-Math.floor(rate*.008));last=Math.min(count-1,last+Math.floor(rate*.008));
if(first<0||last-first<rate)throw Error('No sustained vocal found');
const sr=24000,n=sr*3,out=new Float32Array(n);let peak=0;
for(let i=0;i<n;i++){const pos=first+i*(last-first)/(n-1),lo=Math.floor(pos),v=input[lo]*(1-(pos-lo))+input[Math.min(count-1,lo+1)]*(pos-lo),t=i/sr;
 // Fast, shallow syllabic tremolo, not clipped bursts; short end fades.
 const amp=(.83+.17*Math.cos(t*Math.PI*2*6))*Math.min(1,t/.025,(3-t)/.09);out[i]=v*amp;peak=Math.max(peak,Math.abs(out[i]));}
const wav=Buffer.alloc(44+n*2);wav.write('RIFF');wav.writeUInt32LE(36+n*2,4);wav.write('WAVEfmt ',8);wav.writeUInt32LE(16,16);wav.writeUInt16LE(1,20);wav.writeUInt16LE(1,22);wav.writeUInt32LE(sr,24);wav.writeUInt32LE(sr*2,28);wav.writeUInt16LE(2,32);wav.writeUInt16LE(16,34);wav.write('data',36);wav.writeUInt32LE(n*2,40);
for(let i=0;i<n;i++)wav.writeInt16LE(Math.round(out[i]*.8/Math.max(peak,.01)*32767),44+i*2);
await writeFile(new URL('../addons/audio/rage-scream.wav',import.meta.url),wav);
console.log({sourceSeconds:count/rate,trimmedSeconds:(last-first)/rate,duration:3,peak:.8,bytes:wav.length});
