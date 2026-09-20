// Optional fallback when git's HTTPS transport is unavailable. Uses gh's existing login.
// Publishes only HEAD, only over its direct remote parent, and requires identical Git hashes.
import {spawn, execFileSync} from 'node:child_process';
import assert from 'node:assert/strict';
const repo = 'Nornttyy/pvz-portable-web';
const git = (...args) => execFileSync('git', args, {encoding:'utf8'}).trim();
assert.equal(git('remote','get-url','origin'), `https://github.com/${repo}.git`);
assert.equal(git('branch','--show-current'), 'main');
const head=git('rev-parse','HEAD'), parent=git('rev-parse','HEAD^'), tree=git('rev-parse','HEAD^{tree}');
async function api(endpoint, body) {
  return new Promise((resolve,reject) => {
    const args=['api',`repos/${repo}/${endpoint}`, ...(body ? ['--method',endpoint.startsWith('git/refs/')?'PATCH':'POST','--input','-'] : [])];
    const child=spawn('gh',args,{stdio:['pipe','pipe','pipe']});
    let output='', error='';
    const timeout=setTimeout(() => {child.kill('SIGTERM');reject(Error('GitHub request timed out: '+endpoint));},90000);
    child.stdout.on('data',d=>output+=d); child.stderr.on('data',d=>error+=d);
    child.on('error',e=>{clearTimeout(timeout);reject(e);});
    child.on('close',code=>{clearTimeout(timeout);if(code!==0)reject(Error(endpoint+': '+error));else {try {resolve(JSON.parse(output));}catch(e){reject(e);}}});
    child.stdin.end(body ? JSON.stringify(body) : undefined);
  });
}
const ref=await api('git/ref/heads/main');
assert.equal(ref.object.sha,parent,'Remote changed; no branch will be overwritten.');
const base=await api('git/commits/'+parent);
const entries=git('diff','--name-only',parent,head).split('\n').map(path=>{
  const match=git('ls-tree',head,'--',path).match(/^(\d+) blob ([a-f0-9]+)\t(.+)$/);
  assert.ok(match,'This publisher only supports added/modified files: '+path);
  return {path,mode:match[1],type:'blob',sha:match[2]};
});
let next=0;
await Promise.all(Array.from({length:4},async()=>{
  while(next<entries.length) {
    const entry=entries[next++];
    const bytes=execFileSync('git',['show',head+':'+entry.path],{maxBuffer:32*1024*1024});
    const blob=await api('git/blobs',{content:bytes.toString('base64'),encoding:'base64'});
    assert.equal(blob.sha,entry.sha,entry.path);
  }
}));
const remoteTree=await api('git/trees',{base_tree:base.tree.sha,tree:entries});
assert.equal(remoteTree.sha,tree,'Uploaded source differs from local HEAD.');
const raw=execFileSync('git',['cat-file','-p',head],{encoding:'utf8'});
function identity(key) {
  const m=raw.match(new RegExp('^'+key+' (.*) <([^>]*)> (\\d+) ([+-])(\\d{2})(\\d{2})$','m'));
  assert.ok(m);
  const minutes=(+m[5]*60 + +m[6])*(m[4]==='-'?-1:1);
  const date=new Date((+m[3]+minutes*60)*1000).toISOString().replace('.000Z',m[4]+m[5]+':'+m[6]);
  return {name:m[1],email:m[2],date};
}
const commit=await api('git/commits',{message:raw.slice(raw.indexOf('\n\n')+2),tree,parents:[parent],author:identity('author'),committer:identity('committer')});
assert.equal(commit.sha,head,'Commit metadata differs; remote branch was not changed.');
await api('git/refs/heads/main',{sha:head,force:false});
execFileSync('git',['update-ref','refs/remotes/origin/main',head,parent]);
console.log('Published verified commit '+head);
