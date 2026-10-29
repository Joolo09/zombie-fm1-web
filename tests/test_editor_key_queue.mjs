import assert from 'node:assert/strict';
import vm from 'node:vm';
import {readFileSync} from 'node:fs';
const editor=process.argv[2]||'groovebox-140',src=readFileSync('webapp/'+editor+'/app.js','utf8'),code=src.slice(src.indexOf('function keyDown('),src.indexOf('async function changeTransport('));
const queued=[],commands=[],events=[],state={selected:0},scope={state,held:new Map(),computerHeld:new Map(),audio:{send:(...a)=>events.push(a)},link:{connected:true,command:async(...a)=>commands.push(a)},perform:fn=>queued.push(fn),renderDrumKit:()=>{}};
vm.runInNewContext(code,scope);scope.keyDown('a',60,null);state.selected=1;scope.keyUp('a');for(const fn of queued)await fn();assert.deepEqual(commands.map(x=>x[1][0]),[0,0]);assert.deepEqual(events.map(x=>x[1].track),[0,0]);assert.equal(scope.held.size,0);
console.log('PASS deferred browser Note On keeps its captured track through a selection change');

commands.length=events.length=queued.length=0;state.selected=0;scope.keyDown('pointer',60,null);scope.keyDown('keyboard',60,null);scope.keyUp('pointer');assert.equal(scope.held.size,1);state.selected=3;scope.keyUp('keyboard');for(const fn of queued)await fn();assert.deepEqual(commands.map(x=>x[1][0]),[0,0]);assert.equal(commands.length,2);console.log('PASS duplicate input owners share one Note On and release once on their original track');
