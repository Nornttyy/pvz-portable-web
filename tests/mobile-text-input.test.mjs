import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {runInNewContext} from 'node:vm';
const source=await readFile(new URL('../web/text-input.js',import.meta.url),'utf8');
function setup(touch=true){
 const listeners=new Map(),input={value:'',selectionStart:0,selectionEnd:0,dataset:{},style:{},focuses:0,
  addEventListener(t,f){const a=listeners.get(t)||[];a.push(f);listeners.set(t,a);},
  setSelectionRange(a,b){this.selectionStart=a;this.selectionEnd=b;},
  focus(){this.focuses++;document.activeElement=this;},blur(){document.activeElement=null;}};
 const canvas={getBoundingClientRect:()=>({left:40,top:20,width:400,height:300})},container={style:{}};
 const document={activeElement:null,getElementById:id=>id==='canvas-container'?container:input,addEventListener(){}};
 const Module={canvas,pvzResizeCanvas(){}};
 const window={addEventListener(){},matchMedia:()=>({matches:touch}),visualViewport:{height:390,offsetTop:0,addEventListener(){}}};
 runInNewContext(source,{Module,document,window,navigator:{maxTouchPoints:touch?5:0}});
 const field={owner:123,text:'原名',start:0,end:2,x:200,y:270,width:280,height:28,gameWidth:800,gameHeight:600};
 const emit=(type,data={})=>{const e={key:'',stopPropagation(){this.stopped=true;},preventDefault(){this.prevented=true;},...data};for(const f of listeners.get(type)||[])f(e);return e;};
 return {input,bridge:Module.pvzTextInput,field,emit,document,container};
}
test('mobile name field is tappable, uses native bounds and focuses only in the user gesture',()=>{
 const {input,bridge,field,emit,container}=setup();bridge.sync(field);bridge.start();
 assert.equal(input.focuses,0);assert.equal(input.value,'原名');assert.equal(input.dataset.nativeEdit,'true');
 assert.equal(input.style.left,'140px');assert.equal(input.style.top,'155px');assert.equal(input.style.width,'140px');assert.equal(input.style.height,'24px');assert.equal(input.style.fontSize,'16px');
 emit('touchend');assert.equal(input.focuses,1);
 bridge.stop();assert.equal(input.dataset.nativeEdit,undefined);assert.equal(container.style.height,'');assert.equal(bridge.hasEvents(),false);
});
test('Chinese composition is committed once, never submitted by the IME confirmation key',()=>{
 const {input,bridge,field,emit}=setup();bridge.sync(field);bridge.start();emit('compositionstart');input.value='xiao';emit('input',{isComposing:true});
 bridge.sync({...field,text:'原名'});assert.equal(input.value,'xiao');assert.equal(bridge.pending,null);
 emit('keydown',{key:'Enter',isComposing:true,keyCode:229});assert.equal(bridge.popKey(),0);
 input.value='小猫';input.setSelectionRange(2,2);emit('compositionend');emit('input');assert.equal(bridge.pending.text,'小猫');
 emit('keydown',{key:'Enter'});assert.equal(bridge.popKey(),13);assert.equal(bridge.popKey(),0);
 const done=emit('beforeinput',{inputType:'insertLineBreak'});assert.equal(done.prevented,true);assert.equal(bridge.popKey(),13);
});
test('replacement, deletion, paste and native limits sync without stale edits or duplicate SDL keys',()=>{
 const {input,bridge,field,emit}=setup();bridge.sync(field);bridge.start();
 input.value='abcdef';input.setSelectionRange(3,3);emit('input');assert.equal(bridge.pending.text,'abcdef');assert.equal(bridge.pending.start,3);
 input.value='abef';input.setSelectionRange(2,2);emit('input');assert.equal(bridge.pending.text,'abef');
 bridge.sync(field);assert.equal(input.value,'abef');
 assert.equal(emit('keydown',{key:'Backspace'}).stopped,true);assert.equal(bridge.popKey(),0);
 bridge.pending=null;bridge.applying=true;bridge.sync(field);assert.equal(input.value,'abef');
 bridge.applying=false;bridge.sync({...field,text:'abe',start:2,end:2});assert.equal(input.value,'abe');
 input.value='';input.setSelectionRange(0,0);emit('input');assert.equal(bridge.pending.text,'');
 bridge.stop();assert.equal(bridge.pending,null);bridge.sync({...field,owner:456,text:'另一个',start:3,end:3});bridge.start();assert.equal(input.value,'另一个');
});
test('desktop keeps the existing native keyboard path',()=>{
 const {input,bridge,field,emit}=setup(false);bridge.sync(field);bridge.start();assert.equal(bridge.active,false);assert.equal(input.dataset.nativeEdit,undefined);assert.equal(emit('keydown',{key:'a'}).stopped,undefined);
});
test('mobile input stays inside the fullscreen canvas container and native validation precedes submit',async()=>{
 const html=await readFile(new URL('../web/index.html',import.meta.url),'utf8');
 assert.match(html,/<canvas[^>]*id="canvas"[^>]*><\/canvas>\s*<textarea id="pvz-soft-keyboard"/);
 assert.match(html,/src="text-input.js"/);
 const native=await readFile(new URL('../src/SexyAppFramework/platform/default/Input.cpp',import.meta.url),'utf8');
 assert.ok(native.indexOf('const int size = WasmPendingTextSize()')<native.indexOf('int aPendingKey = WasmPopSoftKeyboardKey()'));
 assert.match(native,/reinterpret_cast<uintptr_t>\(edit\)/);assert.match(native,/edit->KeyText\(value\)/);
});
