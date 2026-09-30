// Derive an open, sharper three-second yell from the documented CC0 male vocal.
import {readFile,writeFile} from 'node:fs/promises';
const b=await readFile(process.argv[2]);let data,rate,channels,bits;
for(let o=12;o+8<=b.length;){const id=b.toString('ascii',o,o+4),n=b.readUInt32LE(o+4);if(id==='fmt '){channels=b.readUInt16LE(o+10);rate=b.readUInt32LE(o+12);bits=b.readUInt16LE(o+22);}if(id==='data')data=b.subarray(o+8,o+8+n);o+=8+n+(n%2);}
if(bits!==16||!data||!rate)throw Error('16-bit PCM WAV required');
const count=data.length/(2*channels),input=Float32Array.from({length:count},(_,i)=>{let v=0;for(let c=0;c<channels;c++)v+=data.readInt16LE((i*channels+c)*2)/32768;return v/channels;});
let first=input.findIndex(v=>Math.abs(v)>.06),last=count-1;
while(last>first&&Math.abs(input[last])<=.06)--last;
first=Math.max(0,first-Math.floor(rate*.008));last=Math.min(count-1,last+Math.floor(rate*.008));
if(first<0||last-first<rate)throw Error('No sustained vocal found');
const sr=24000,n=sr*3,out=new Float32Array(n),pitch=2**(6/12);let peak=0;
const dry=Float32Array.from({length:Math.ceil((last-first)*sr/rate)},(_,i)=>{const p=first+i*rate/sr,lo=Math.floor(p),f=p-lo;return input[lo]*(1-f)+input[Math.min(last,lo+1)]*f;});
// WSOLA separates duration from pitch. Match overlapping waveform phases before
// stretching, then resample up six semitones; the released cue stays three seconds.
const size=1024,hop=256,length=Math.ceil(n*pitch),stretch=length/dry.length;
const sum=new Float32Array(length+size),weight=new Float32Array(length+size);
for(let pos=0;pos<length;pos+=hop){
 const expected=Math.min(dry.length-size,Math.max(0,Math.round(pos/stretch)));let best=expected,score=-Infinity;
 if(pos)for(let candidate=Math.max(0,expected-96);candidate<=Math.min(dry.length-size,expected+96);candidate+=2){
  let dot=0,energy=0;for(let k=0;k<size-hop;k+=4){const previous=weight[pos+k]>0?sum[pos+k]/weight[pos+k]:0,v=dry[candidate+k];dot+=v*previous;energy+=v*v;}
  const fit=dot/Math.sqrt(energy+1e-9);if(fit>score){score=fit;best=candidate;}
 }
 for(let k=0;k<size;k++){const w=.5-.5*Math.cos(2*Math.PI*k/(size-1));sum[pos+k]+=dry[best+k]*w;weight[pos+k]+=w;}
}
for(let i=0;i<sum.length;i++)sum[i]/=Math.max(weight[i],1e-8);
for(let i=0;i<n;i++){const pos=i*pitch,lo=Math.floor(pos),v=sum[lo]*(1-(pos-lo))+sum[lo+1]*(pos-lo),t=i/sr;
 // Keep the performer's natural breaks. No synthetic vibrato/tremolo, no gate.
 const amp=Math.min(1,t/.025,(3-t)/.09);out[i]=v*amp;}
// Gentle brightening retains chest/body frequencies instead of a thin telephone EQ.
let low=0;const smooth=1-Math.exp(-2*Math.PI*2600/sr);
for(let i=0;i<n;i++){low+=smooth*(out[i]-low);out[i]+=0.16*(out[i]-low);peak=Math.max(peak,Math.abs(out[i]));}
const wav=Buffer.alloc(44+n*2);wav.write('RIFF');wav.writeUInt32LE(36+n*2,4);wav.write('WAVEfmt ',8);wav.writeUInt32LE(16,16);wav.writeUInt16LE(1,20);wav.writeUInt16LE(1,22);wav.writeUInt32LE(sr,24);wav.writeUInt32LE(sr*2,28);wav.writeUInt16LE(2,32);wav.writeUInt16LE(16,34);wav.write('data',36);wav.writeUInt32LE(n*2,40);
for(let i=0;i<n;i++)wav.writeInt16LE(Math.round(out[i]*.8/Math.max(peak,.01)*32767),44+i*2);
await writeFile(new URL('../addons/audio/rage-scream.wav',import.meta.url),wav);
console.log({sourceSeconds:count/rate,trimmedSeconds:(last-first)/rate,pitchSemitones:6,duration:3,peak:.8,bytes:wav.length});
