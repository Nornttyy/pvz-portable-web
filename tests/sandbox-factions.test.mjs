import test from 'node:test';
import assert from 'node:assert/strict';
import {validateLayout} from '../web/sandbox-data.mjs';
test('mixed allegiance formations retain each plant team and legacy saves remain ordinary',()=>{
 const legacy={schema:1,map:0,plants:[{type:501,col:1,row:2}]};
 assert.deepEqual(validateLayout(legacy),legacy);
 const mixed={schema:1,map:0,plants:[{type:501,col:1,row:2},{type:5,col:5,row:2,charmed:true}]};
 assert.deepEqual(validateLayout(mixed),mixed);
 for(const charmed of ['true',1,null,{},[]])assert.throws(()=>validateLayout({...legacy,plants:[{...legacy.plants[0],charmed}]}));
 const stacked=validateLayout({schema:1,map:1,adapted:true,stacked:true,plants:[{type:5,col:1,row:2,charmed:true},{type:501,col:1,row:2},{type:16,col:1,row:2,charmed:true}]});
 assert.equal(stacked.plants[0].type,16);assert.equal(stacked.plants[0].charmed,true);
 assert.equal(stacked.plants.find(p=>p.type===5).charmed,true);assert.equal(stacked.plants.find(p=>p.type===501).charmed,undefined);
});
