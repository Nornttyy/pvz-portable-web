import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
const read=p=>readFile(new URL('../'+p,import.meta.url),'utf8');
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
 for(const field of ['mZombiePhase','mPhaseCounter','mBossStompCounter','mBossHeadCounter','mBossBungeeCounter','mTargetRow','mPosY','mAltitude','mInPool'])assert.ok(save.includes('theZombie.'+field),field);
 assert.match(save,/SyncInt32\(theObject.mRow\)/); // Inherited GameObject TLV.
 assert.match(save,/SyncGameObjectPortable\(aContext, theObject\)/);
 const restore=custom.slice(custom.indexOf('bool Restore('),custom.indexOf('void Assign('));
 assert.doesNotMatch(restore,/mBodyHealth\s*=|mHelmHealth\s*=|mBossHeadCounter\s*=|mBossStompCounter\s*=/);
 assert.match(custom,/p->Die\(\);z->StopEating\(\);/);
});
