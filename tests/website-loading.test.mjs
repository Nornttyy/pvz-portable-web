import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import vm from 'node:vm';
import {syncFileSystem, fitCanvas} from '../web/loading-utils.mjs';

test('storage deadline distinguishes success, failure and timeout; late callbacks settle only once', async () => {
  await syncFileSystem({syncfs(_p, done) {done(null);}}, true, 10);
  await assert.rejects(syncFileSystem({syncfs() {throw Error('denied');}}, true, 10), /denied/);
  let late;
  const pending = syncFileSystem({syncfs(_p, done) {late = done;}}, true, 5);
  await assert.rejects(pending, e => e.name === 'TimeoutError' && /已有存档不会删除/.test(e.message));
  late(null);
  await assert.rejects(pending, {name:'TimeoutError'});
});
test('phone, tablet and desktop canvas sizing fits without stretching', () => {
  for (const [w,h] of [[390,844],[844,390],[1366,768],[320,480]]) {
    const size = fitCanvas(800,600,w,h);
    assert.ok(size.width <= w && size.height <= h);
    assert.ok(Math.abs(size.width/size.height - 4/3) < 0.005);
  }
  assert.equal(fitCanvas(800,600,0,0),null);
});

async function bootHarness(clipboardWorks = true) {
  const elements = new Map(), events = new Map(), timers = [], copied = [];
  const element = id => {
    if (!elements.has(id)) {const handlers = new Map(); elements.set(id,{hidden:true, textContent:'', value:'', focus(){},select(){}, addEventListener(k,fn){handlers.set(k,fn);}, dispatch(k,event){return handlers.get(k)?.(event);}});}
    return elements.get(id);
  };
  const window = {location:{href:'https://nornttyy.github.io/pvz-portable-web/index.html?choose-resources=1',reload(){}}, addEventListener(k,fn){events.set(k,fn);},dispatchEvent(event){events.get(event.type)?.(event);}};
  const navigator = {onLine:true,clipboard:{async writeText(value){if (!clipboardWorks) throw Error('blocked');copied.push(value);}}};
  const document = {getElementById:element,documentElement:{dataset:{version:'abc123'}}};
  const ctx = vm.createContext({window,document,navigator,URL,console,CustomEvent:class{constructor(type,options){this.type=type;Object.assign(this,options);}},setTimeout:fn=>{timers.push(fn);return timers.length;},clearTimeout(){}});
  vm.runInContext(await readFile(new URL('../web/bootstrap.js',import.meta.url),'utf8'),ctx);
  return {ctx,window,navigator,element,events,timers,copied};
}
test('bootstrap shows early script failures even when the main module never loaded', async () => {
  const app = await bootHarness();
  app.events.get('error')({target:{tagName:'SCRIPT'}});
  assert.match(app.element('status').textContent,/下载失败/);
  assert.equal(app.element('reload').hidden,false);
  assert.equal(app.element('start').hidden,true);
  app.timers[0]();
  assert.match(app.window.pvzEarlyFailure,/超时/);
});
test('sharing strips resource-selection parameters and has a clipboard-denied fallback', async () => {
  for (const works of [true,false]) {
    const app = await bootHarness(works);
    await app.element('share-site').dispatch('click');
    const actual = works ? app.copied[0] : app.element('share-link').value;
    assert.equal(actual,'https://nornttyy.github.io/pvz-portable-web/');
    assert.match(app.element('share-status').textContent, works ? /打开即可加载游玩/ : /不会分享你的存档/);
  }
});
test('bootstrap reports offline state and loads the same-version WASM under the project path', async () => {
  const app=await bootHarness();
  app.navigator.onLine=false;app.events.get('offline')();
  assert.equal(app.element('connection-status').hidden,false);
  app.navigator.onLine=true;app.events.get('online')();
  assert.equal(app.element('connection-status').hidden,true);
  assert.equal(app.ctx.Module.locateFile('pvz-portable.wasm'),'https://nornttyy.github.io/pvz-portable-web/sandbox-engine/pvz-portable.wasm?v=abc123');
});
test('built entry and module dependencies carry one version, preventing mixed cached releases', async () => {
  const read = name => readFile(new URL('../site/'+name,import.meta.url),'utf8');
  const html = await read('index.html'), version=html.match(/data-version="([a-f0-9]{12})"/)[1];
  for(const [,src] of html.matchAll(/(?:src|href)="([^"?#]+\.(?:js|mjs|css)[^"]*)"/g)) assert.ok(src.endsWith('?v='+version),src);
  for(const file of ['runtime.mjs','resource-loader.mjs','sandbox.mjs']){
    const code=await read(file);
    for(const [,src] of code.matchAll(/from ['"]([^'"]+)['"]/g))assert.ok(src.endsWith('?v='+version),src);
  }
});
