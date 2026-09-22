// Storage/fullscreen bridge only. All sandbox UI and input live in the native game.
import { validateLayout, requiresStacking, LAYOUT_KEY } from './sandbox-data.mjs';
const api=(command,type=0,col=0,row=0)=>Module._pvz_sandbox_command?.(command,type,col,row)??-1;
const input=document.getElementById('layout-file');
let importRevision=-1, stopped=false;
function snapshot() {
  const flags=api(0);
  if(flags<0)throw Error('Sandbox is inactive');
  const plants=[];
  for(let i=0;i<180;i++){
    const type=Module._pvz_sandbox_plant_data(i,0);if(type<0)break;
    plants.push({type,col:Module._pvz_sandbox_plant_data(i,1),row:Module._pvz_sandbox_plant_data(i,2)});
  }
  return validateLayout({schema:1,map:flags&4?1:0,stacked:Boolean(flags&16)||requiresStacking(plants),plants});
}
function restore(value,revision) {
  const layout=validateLayout(value);
  if(api(18)!==revision||api(0)<0)return;
  const awake=Boolean(api(0)&8);
  if(api(8,layout.map)<0)throw Error('Cannot reset sandbox');
  api(19,layout.stacked?1:0);
  api(12,0);
  let rejected=0;
  for(const p of layout.plants)if(api(1,p.type,p.col,p.row)<0)rejected++;
  api(12,awake?1:0);
  api(16,rejected?6:2);
}
window.addEventListener('pvz-sandbox-action',event=>{
  if(stopped||api(0)<0)return;
  try {
    switch(event.detail){
    case 1:localStorage.setItem(LAYOUT_KEY,JSON.stringify(snapshot()));api(16,1);break;
    case 2:{
      const data=localStorage.getItem(LAYOUT_KEY);
      if(!data){api(16,5);break;}
      restore(JSON.parse(data),api(18));break;
    }
    case 3:{
      const url=URL.createObjectURL(new Blob([JSON.stringify(snapshot(),null,2)],{type:'application/json'}));
      const link=document.createElement('a');link.href=url;link.download='沙盒阵型.json';link.click();
      setTimeout(()=>URL.revokeObjectURL(url),10000);api(16,3);break;
    }
    case 4:importRevision=api(18);input.value='';input.click();break;
    case 5:document.getElementById('fullscreen').click();break;
    }
  }catch{api(16,4);}
});
input.addEventListener('change',async event=>{
  const revision=importRevision,file=event.target.files?.[0];event.target.value='';
  if(!file||stopped||api(18)!==revision)return;
  try{
    if(file.size>65536)throw Error('Formation too large');
    restore(JSON.parse(await file.text()),revision);
  }catch{if(api(18)===revision)api(16,4);}
});
function refresh(){
  if(stopped)return;
  // Only hide the browser's backup tools. Never resize or replace the canvas.
  document.body.classList.toggle('native-sandbox',api(0)>=0);
}
const poll=setInterval(refresh,250);
window.addEventListener('pvz-fatal',()=>{stopped=true;clearInterval(poll);document.body.classList.remove('native-sandbox');});
window.addEventListener('pagehide',()=>{stopped=true;clearInterval(poll);},{once:true});
window.addEventListener('beforeunload',event=>{if(api(17)>0){event.preventDefault();event.returnValue='';}});
document.addEventListener('visibilitychange',()=>{if(document.hidden&&api(0)>=0)api(4,1);});
