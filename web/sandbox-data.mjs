const plantNames = ['豌豆射手','向日葵','樱桃炸弹','坚果墙','土豆地雷','寒冰射手','大嘴花','双发射手','小喷菇','阳光菇','大喷菇','墓碑吞噬者','魅惑菇','胆小菇','寒冰菇','毁灭菇','睡莲','窝瓜','三线射手','缠绕海草','火爆辣椒','地刺','火炬树桩','高坚果','海蘑菇','路灯花','仙人掌','三叶草','裂荚射手','杨桃','南瓜头','磁力菇','卷心菜投手','花盆','玉米投手','咖啡豆','大蒜','叶子保护伞','金盏花','西瓜投手','机枪射手','双子向日葵','忧郁菇','香蒲','冰西瓜','吸金磁','地刺王','玉米加农炮'];
const groups = {
  common:[0,1,3,5,7,18,22,23,39,44], attack:[0,5,6,7,8,10,13,18,21,26,28,29,32,34,39],
  defense:[3,23,30,36,37], instant:[2,4,12,14,15,17,20,27],
  support:[1,9,11,22,25,31,33,35,38], water:[16,19,24,43], upgrade:[40,41,42,43,44,45,46,47],
};
const notes = {11:'需要墓碑',16:'水上底座',19:'仅限水路',24:'仅限水路',30:'可套在植物外',35:'需要睡眠蘑菇',43:'放在泳池',47:'占两格 · 操作后发射'};
export const ORIGINAL_PLANTS = [
  [100,0,'火焰豌豆','火球攻击 · 小范围伤害'],[101,7,'双发寒冰','连续两发 · 寒冰减速'],
  [102,7,'双发火焰','连续两发 · 火球攻击'],[103,18,'三线寒冰','三路攻击 · 寒冰减速'],
  [104,18,'三线火焰','三路攻击 · 火球攻击'],[105,40,'寒冰机枪','连续四发 · 寒冰减速'],
  [106,40,'火焰机枪','连续四发 · 火球攻击'],[107,7,'冰火双发','冰火交替 · 火焰会解除减速'],
].map(([id,base,name,note])=>({id,base,name,note}));
export const PLANTS = [...plantNames.map((name,id) => ({id,name,note:notes[id] ?? '免费 · 无冷却'})),...ORIGINAL_PLANTS];
const validPlant = id => (id>=0&&id<48)||ORIGINAL_PLANTS.some(p=>p.id===id);
export const ZOMBIES = [[0,'普通僵尸'],[1,'旗帜僵尸'],[2,'路障僵尸'],[3,'撑杆僵尸'],[4,'铁桶僵尸'],[5,'读报僵尸'],[6,'铁门僵尸'],[7,'橄榄球僵尸'],[8,'舞王僵尸'],[10,'鸭子救生圈'],[11,'潜水僵尸'],[12,'冰车僵尸'],[14,'海豚骑士'],[15,'玩偶匣僵尸'],[16,'气球僵尸'],[17,'矿工僵尸'],[18,'跳跳僵尸'],[19,'雪人僵尸'],[21,'梯子僵尸'],[22,'投石车僵尸'],[23,'巨人僵尸'],[24,'小鬼僵尸'],[32,'红眼巨人']].map(([id,name])=>({id,name,note:[10,11,14].includes(id)?'仅限水路':'手动放置'}));
export function plantsFor(category) { return category === 'original' ? ORIGINAL_PLANTS : category === 'all' ? PLANTS : (groups[category] ?? groups.common).map(id=>PLANTS[id]); }
export function boardCell(x,y,pool=false) {
  if (!Number.isFinite(x) || !Number.isFinite(y)) return null;
  const height = pool ? 85 : 100;
  const col = Math.floor((x-40)/80), row = Math.floor((y-80)/height);
  return col>=0 && col<9 && row>=0 && row<(pool?6:5) ? {col,row} : null;
}
export function cellRect(col,row,pool=false) { return {x:40+col*80,y:80+row*(pool?85:100),width:80,height:pool?85:100}; }
export const LAYOUT_KEY = 'pvz.sandbox.formation.v1';
export function validateLayout(value) {
  if (value?.schema !== 1 || ![0,1].includes(value.map) || !Array.isArray(value.plants) || value.plants.length > 180) throw Error('不是支持的沙盒阵型文件');
  const seen = new Set();
  const plants = value.plants.map(p=>{
    if (!p || ![p.type,p.col,p.row].every(Number.isInteger) || !validPlant(p.type) || p.col<0 || p.col>=9 || p.row<0 || p.row >= (value.map===1?6:5)) throw Error('阵型中有无效的植物或位置');
    const key = `${p.type}:${p.col}:${p.row}`;
    if (seen.has(key)) throw Error('阵型中有重复植物');
    seen.add(key);
    return {type:p.type,col:p.col,row:p.row};
  });
  const layer = p => [16,33].includes(p.type)?0:[30,35].includes(p.type)?2:1;
  const occupied=new Map();
  for(const p of plants) {
    const key=`${p.col}:${p.row}`,kind=[16,33].includes(p.type)?'base':p.type===30?'shell':p.type===35?'coffee':'normal';
    const cell=occupied.get(key)??{};
    if(cell[kind]!==undefined) throw Error('同一个格子的植物位置冲突');
    cell[kind]=p.type;occupied.set(key,cell);
  }
  for(const p of plants) {
    const cell=occupied.get(`${p.col}:${p.row}`), water=value.map===1&&[2,3].includes(p.row);
    if([16,19,24,43].includes(p.type)&&!water) throw Error('水生植物不在水路');
    if(water&&[4,21,33,46].includes(p.type)) throw Error('陆生植物不能放在水路');
    if(water&&![16,19,24,35,43].includes(p.type)&&cell.base!==16&&!(p.type===30&&cell.normal===43)) throw Error('水路植物缺少睡莲');
    if([19,24,43].includes(cell.normal)&&cell.base!==undefined) throw Error('水生植物与底座冲突');
    if(p.type===11) throw Error('阵型不包含墓碑，不能保存正在吞噬墓碑的植物');
    if(p.type===35&&![8,9,10,12,13,14,15,24,31,42].includes(cell.normal)) throw Error('咖啡豆缺少蘑菇');
    if(p.type===47) {
      const right=occupied.get(`${p.col+1}:${p.row}`)??{};
      if(p.col>=8||right.normal!==undefined||right.shell!==undefined||cell.shell!==undefined) throw Error('玉米加农炮需要连续两个空位');
      if(water&&right.base!==16) throw Error('玉米加农炮的第二格缺少睡莲');
    }
  }
  return {schema:1,map:value.map,plants:plants.sort((a,b)=>layer(a)-layer(b))};
}
