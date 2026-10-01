import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
const read=p=>readFile(new URL('../'+p,import.meta.url),'utf8');
test('clever variants share the requested probabilities and registered action-only jaw art',async()=>{
 const h=await read('src/SandboxZombies.h'),custom=await read('src/SandboxZombies.cpp'),rig=await read('src/AbstractRigVisuals.cpp');
 assert.match(h,/DodgePercent=70,LaneChangePercent=60,ForwardFlightPercent=30/);
 assert.match(h,/ForwardFlightDistance=160\.0f/);
 assert.match(custom,/FlipTravel\(t\)-FlipTravel\(previous\)/);
 assert.doesNotMatch(custom,/Zombie_head_sunglasses/);
 assert.match(rig,/CleverHead\(p->state\[1\]!=0\)/);
 const meta=JSON.parse(await read('site/sandbox-engine/build.json')).cleverZombies;
 assert.equal(meta.dodgePercentWhenReady,70);assert.equal(meta.laneChangePercent,60);assert.equal(meta.forwardFlightPercentPerFlip,30);assert.equal(meta.forwardFlightTiles,2);
 assert.equal(meta.flipTicks,160);assert.equal(meta.flipJawTicks,180);assert.equal(meta.recoveryTicks,120);
});
test('slow motion drives every local movement channel without slowing the board',async()=>{
 const custom=await read('src/SandboxZombies.cpp');
 assert.match(custom,/progress=slow\?SlowFlipProgress\(t\):t/);
 assert.match(custom,/GetPosYBasedOnRow\(from\)\*\(1-progress\)/);
 assert.match(custom,/progress-SlowFlipProgress\(previous\)/);
 assert.match(custom,/55\*std::sin\(3\.14159265f\*progress\)/);
 assert.match(custom,/UsesSlowFlip\(z\)\?SlowFlipProgress\(t\)/);
 assert.doesNotMatch(custom,/mUpdateMultiplier\s*=|mTimeStopCounter\s*=|mMainCounter\s*=|mPaused\s*=/);
});
test('clever dodge intercepts a real projectile hit before effects, not general damage',async()=>{
 const shot=await read('src/Lawn/Projectile.cpp'),zombie=await read('src/Lawn/Zombie.cpp'),custom=await read('src/SandboxZombies.cpp');
 const impact=shot.slice(shot.indexOf('void Projectile::DoImpact('),shot.indexOf('void Projectile::Draw('));
 assert.ok(impact.indexOf('DodgeProjectile(this,theZombie)')<impact.indexOf('PlayImpactSound(theZombie)'));
 assert.match(impact,/DodgeProjectile\(this,theZombie\)\) return;/);
 assert.match(custom,/int Damage\(Zombie\*,int damage,unsigned\)\{return damage;\}/);
 assert.match(custom,/void ForgetShot\(Projectile\* shot\)\{dodged.erase\(shot\);\}/);
 const eat=zombie.slice(zombie.indexOf('void Zombie::EatPlant('),zombie.indexOf('void Zombie::EatZombie('));
 assert.ok(eat.indexOf('StealPlant(this,thePlant)')>eat.indexOf('STATE_LILYPAD_INVULNERABLE'));
 assert.ok(eat.indexOf('StealPlant(this,thePlant)')>eat.indexOf('SEED_POTATOMINE'));
 assert.doesNotMatch(custom,/mTimeStopCounter\s*=|mTimeStopCounter\+\+/);
});
test('flip, recovery, lane and stolen plant use native portable-save fields without healing on restore',async()=>{
 const save=await read('src/Lawn/System/SaveGame.cpp'),custom=await read('src/SandboxZombies.cpp');
 for(const field of ['mZombiePhase','mPhaseCounter','mBossStompCounter','mBossHeadCounter','mBossBungeeCounter','mBossMode','mSummonCounter','mTargetRow','mPosY','mAltitude','mInPool'])assert.ok(save.includes('theZombie.'+field),field);
 assert.match(save,/SyncInt32\(theObject.mRow\)/); // Inherited GameObject TLV.
 assert.match(save,/SyncGameObjectPortable\(aContext, theObject\)/);
 const restore=custom.slice(custom.indexOf('bool Restore('),custom.indexOf('void Assign('));
 assert.doesNotMatch(restore,/mBodyHealth\s*=|mHelmHealth\s*=|mBossHeadCounter\s*=|mBossStompCounter\s*=/);
 assert.match(custom,/p->Die\(\);z->StopEating\(\);/);
});
