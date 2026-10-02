// Production asset preparation: preserve generated pixels/alpha, trim and resize.
// Decode the original logo's shipped luminance mask; never redraw its lettering.
import {readFile,writeFile,mkdir,copyFile} from 'node:fs/promises';
const {chromium}=await import(process.env.PVZ_PLAYWRIGHT||'playwright-core');
const [input,resources]=process.argv.slice(2);
if(!input||!resources)throw Error('Usage: prepare-nb-branding.mjs generated.png original-resource-directory');
const browser=await chromium.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true});
try{
 const page=await browser.newPage();
 const urls=await Promise.all([input,resources+'/images/PvZ_Logo.jpg',resources+'/images/PvZ_Logo_.png'].map(async(p,i)=>'data:image/'+(i===1?'jpeg':'png')+';base64,'+(await readFile(p)).toString('base64')));
 const result=await page.evaluate(async urls=>{
  const images=await Promise.all(urls.map(async url=>{const im=new Image();im.src=url;await im.decode();return im;}));
  const canvas=(w,h)=>Object.assign(document.createElement('canvas'),{width:w,height:h});
  const c=canvas(images[0].width,images[0].height),g=c.getContext('2d');g.drawImage(images[0],0,0);
  const {data}=g.getImageData(0,0,c.width,c.height);let l=c.width,r=-1,t=c.height,b=-1,transparent=0;
  for(let y=0;y<c.height;y++)for(let x=0;x<c.width;x++){if(data[(y*c.width+x)*4+3]>8){l=Math.min(l,x);r=Math.max(r,x);t=Math.min(t,y);b=Math.max(b,y);}else transparent++;}
  if(r<l||transparent<c.width*c.height*.05)throw Error('Generated badge must have real transparent alpha');
  const badge=canvas(256,100),bg=badge.getContext('2d'),w=r-l+1,h=b-t+1,fit=Math.min(254/w,98/h);bg.imageSmoothingQuality='high';
  bg.drawImage(c,l,t,w,h,(256-w*fit)/2,(100-h*fit)/2,w*fit,h*fit);
  const logo=canvas(images[1].width,images[1].height),lg=logo.getContext('2d');lg.drawImage(images[1],0,0);
  const mask=canvas(logo.width,logo.height),mg=mask.getContext('2d');mg.drawImage(images[2],0,0);
  const pixels=lg.getImageData(0,0,logo.width,logo.height),alpha=mg.getImageData(0,0,logo.width,logo.height);
  for(let i=3;i<pixels.data.length;i+=4)pixels.data[i]=alpha.data[i-3];
  lg.putImageData(pixels,0,0);
  return {badge:badge.toDataURL().split(',')[1],logo:logo.toDataURL().split(',')[1],source:[c.width,c.height],bounds:[l,t,r,b],transparent};
 },urls);
 for(const dir of ['art/branding','site/branding','addons/images'])await mkdir(dir,{recursive:true});
 await copyFile(input,'art/branding/nb-edition-source.png');
 const badge=Buffer.from(result.badge,'base64');
 await writeFile('addons/images/nb-edition.png',badge);
 await writeFile('site/branding/nb-edition.png',badge);
 await writeFile('site/branding/original-logo.png',Buffer.from(result.logo,'base64'));
 console.log({source:result.source,bounds:result.bounds,transparent:result.transparent,badge:'256x100'});
}finally{await browser.close();}
