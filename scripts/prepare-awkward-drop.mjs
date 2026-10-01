// Asset preparation only: alpha-trim and downsample the generated water drop.
// Preserve its colors/silhouette/alpha; no procedural drawing or recoloring.
import {readFile,writeFile} from 'node:fs/promises';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const input=process.argv[2];if(!input)throw Error('Expected generated transparent drop PNG');
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true});
try{
 const page=await browser.newPage();
 const result=await page.evaluate(async url=>{
  const image=new Image();image.src=url;await image.decode();
  const c=document.createElement('canvas');c.width=image.width;c.height=image.height;const g=c.getContext('2d');g.drawImage(image,0,0);
  const {data}=g.getImageData(0,0,c.width,c.height);let l=c.width,r=-1,t=c.height,b=-1,transparent=0;
  for(let y=0;y<c.height;++y)for(let x=0;x<c.width;++x){const a=data[(y*c.width+x)*4+3];if(a>8){l=Math.min(l,x);r=Math.max(r,x);t=Math.min(t,y);b=Math.max(b,y);}else ++transparent;}
  if(r<l||transparent<c.width*c.height*.05)throw Error('Generated image lacks a cutout with real transparency');
  const out=document.createElement('canvas');out.width=48;out.height=72;
  const w=r-l+1,h=b-t+1,fit=Math.min(46/w,70/h),ctx=out.getContext('2d');ctx.imageSmoothingQuality='high';
  ctx.drawImage(c,l,t,w,h,(48-w*fit)/2,(72-h*fit)/2,w*fit,h*fit);
  return {png:out.toDataURL('image/png').split(',')[1],source:[c.width,c.height],bounds:[l,t,r,b]};
 },'data:image/png;base64,'+(await readFile(input)).toString('base64'));
 await writeFile('addons/art/awkward-blue-drop.png',Buffer.from(result.png,'base64'));
 console.log(result.source,result.bounds,'=> addons/art/awkward-blue-drop.png (48 x 72)');
}finally{await browser.close();}
