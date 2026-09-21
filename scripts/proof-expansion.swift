// Offline rig-pose proof, not a browser screenshot. Uses the real .reanim transforms.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let repo=URL(fileURLWithPath:CommandLine.arguments[1]),source=URL(fileURLWithPath:CommandLine.arguments[2])
struct Pose{var x=0.0,y=0.0,kx=0.0,ky=0.0,sx=1.0,sy=1.0,f=0.0,a=1.0,image=""}
struct Track{var name="",frames=[Pose]()}
class Reader:NSObject,XMLParserDelegate{
 var tracks=[Track](),track=Track(),pose=Pose(),text=""
 func parser(_ p:XMLParser,didStartElement n:String,namespaceURI:String?,qualifiedName:String?,attributes:[String:String]){text="";if n=="track"{track=Track();pose=Pose()}}
 func parser(_ p:XMLParser,foundCharacters s:String){text+=s}
 func parser(_ p:XMLParser,didEndElement n:String,namespaceURI:String?,qualifiedName:String?){
  let v=Double(text) ?? 0
  switch n{case "name":track.name=text;case "x":pose.x=v;case "y":pose.y=v;case "kx":pose.kx=v;case "ky":pose.ky=v;case "sx":pose.sx=v;case "sy":pose.sy=v;case "f":pose.f=v;case "a":pose.a=v;case "i":pose.image=text;case "t":track.frames.append(pose);case "track":tracks.append(track);default:break}
 }
}
var rigs=[String:[Track]](),images=[String:CGImage](),nativePaths=[String:URL]()
func key(_ name:String)->String{name.replacingOccurrences(of:"IMAGE_REANIM_",with:"").replacingOccurrences(of:"_",with:"").lowercased()}
for file in try FileManager.default.contentsOfDirectory(at:source.appendingPathComponent("reanim"),includingPropertiesForKeys:nil) where file.pathExtension.lowercased()=="png"{nativePaths[key(file.deletingPathExtension().lastPathComponent)]=file}
func image(_ url:URL)->CGImage?{if let x=images[url.path]{return x};guard let s=CGImageSourceCreateWithURL(url as CFURL,nil),let i=CGImageSourceCreateImageAtIndex(s,0,nil)else{return nil};images[url.path]=i;return i}
func rig(_ name:String)->[Track]{if let t=rigs[name]{return t};let s=try! String(contentsOf:source.appendingPathComponent("reanim/"+name+".reanim"),encoding:.utf8);let p=XMLParser(data:Data(("<root>"+s+"</root>").utf8)),r=Reader();p.delegate=r;precondition(p.parse());rigs[name]=r.tracks;return r.tracks}
let space=CGColorSpace(name:CGColorSpace.sRGB)!,w=1400,h=1120
let ctx=CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:space,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!
ctx.setFillColor(CGColor(red:0.54,green:0.66,blue:0.3,alpha:1));ctx.fill(CGRect(x:0,y:0,width:w,height:h));ctx.interpolationQuality = .high
func override(_ family:String,_ part:String)->CGImage?{
 let generated=repo.appendingPathComponent("art/expansion/parts/"+family+"-"+part+".png")
 return image(FileManager.default.fileExists(atPath:generated.path) ? generated : source.appendingPathComponent("images/sandbox/"+family+"-"+part+".png"))
}
func matrix(_ p:Pose)->CGAffineTransform{let kx=p.kx*Double.pi/180,ky=p.ky*Double.pi/180;return CGAffineTransform(a:cos(kx)*p.sx,b:sin(kx)*p.sx,c:-sin(ky)*p.sy,d:cos(ky)*p.sy,tx:p.x,ty:p.y)}
func draw(_ name:String,_ family:String,_ layer:String,_ x:Double,_ y:Double,_ scale:Double=1,_ base:Int=0,_ poseIndex:Int=0,_ overlay:CGAffineTransform = .identity){
 let tracks=rig(name);guard let marker=tracks.first(where:{$0.name==layer}),let first=marker.frames.firstIndex(where:{$0.f>=0})else{return}
 let index=min(first+poseIndex,marker.frames.count-1)
 if name=="Zombie"&&base==1,let hand=tracks.first(where:{$0.name=="Zombie_flaghand"}){
  let attach=matrix(hand.frames[0]).inverted().concatenating(matrix(hand.frames[index]));draw("Zombie_flagpole",family,"Zombie_flag",x,y,scale,base,0,attach)
 }
 ctx.saveGState();ctx.translateBy(x:x,y:Double(h)-y);ctx.scaleBy(x:scale,y:-scale);ctx.concatenate(overlay)
 for t in tracks where index<t.frames.count{
  let p=t.frames[index];if p.f<0 || p.a<=0 || p.image.isEmpty{continue}
  guard let url=nativePaths[key(p.image)],let original=image(url)else{continue};var selected=original
  let n=t.name
  if name=="Zombie"{
   let forbidden=["anim_screendoor","Zombie_screendoor","anim_tongue","Zombie_mustache"]
   if forbidden.contains(n)||(n.contains("screendoor") && !(base==1&&n=="Zombie_innerarm_screendoor"))||n.contains("duckytube")||n.contains("whitewater")||(n=="Zombie_flaghand"&&base != 1)||(n=="anim_innerarm"&&base==1){continue}
   if n=="anim_cone" && base != 2{continue};if n=="anim_bucket" && base != 4{continue}
   if !family.isEmpty{
    if n=="anim_head1"{selected=override(family,"head")!}
    if n=="anim_head2"{selected=override(family,"jaw")!}
    if n=="Zombie_body"{selected=override(family,"body")!}
    if n=="anim_cone"||n=="anim_bucket"{selected=override(family,"prop")!}
    if n=="anim_hair" && ["light-zombie","smoke-zombie","twin-zombie"].contains(family){selected=override(family,"hat")!}
    if n=="Zombie_tie" && family=="battery-zombie"{selected=override(family,"battery")!}
   }
  }else if name=="Zombie_flagpole"{if n=="Zombie_flag"{selected=override(family,"prop")!}}
  else if !family.isEmpty{
   if n.lowercased().contains("blink") || (name=="PuffShroom" && n=="PuffShroom_eyes"){continue}
   if name=="SunFlower"{if n=="anim_idle"{selected=override(family,"head")!}}
   else if name=="PuffShroom"{if n=="anim_face"{selected=override(family,"head")!};if n=="PuffShroom_head"{selected=override(family,"cap")!};if n=="PuffShroom_stem"{selected=override(family,"stem")!}}
   else if n.hasPrefix("anim_face"){selected=override(family,name=="ThreePeater" ? "small-head":"head")!}
   if name=="GatlingPea"{if n=="GatlingPea_mouth"{selected=override(family,"gatling-mouth")!};if n.hasPrefix("GatlingPea_barrel"){selected=override(family,"gatling-barrel")!};if n=="GatlingPea_mouth_overlay"{selected=override(family,"gatling-overlay")!}}
   else if n.lowercased().contains("mouth"){selected=override(family,name=="ThreePeater" ? "small-mouth":"mouth")!}
  }
  let kx=p.kx*Double.pi/180,ky=p.ky*Double.pi/180
  ctx.saveGState();ctx.setAlpha(p.a);ctx.concatenate(CGAffineTransform(a:cos(kx)*p.sx,b:sin(kx)*p.sx,c:-sin(ky)*p.sy,d:cos(ky)*p.sy,tx:p.x,ty:p.y))
  ctx.translateBy(x:Double(original.width-selected.width)/2,y:Double(original.height-selected.height)/2+Double(selected.height));ctx.scaleBy(x:1,y:-1)
  ctx.draw(selected,in:CGRect(x:0,y:0,width:selected.width,height:selected.height));ctx.restoreGState()
  if family=="gum-zombie"&&n=="anim_head2",let bubble=override("vfx","gum"){
   ctx.saveGState();ctx.concatenate(matrix(p));ctx.translateBy(x:-7,y:17);ctx.scaleBy(x:1,y:-1);ctx.draw(bubble,in:CGRect(x:0,y:0,width:18,height:18));ctx.restoreGState()
  }
 }
 ctx.restoreGState()
}
let plants:[(String,String,Double)]=[("PeaShooter","",1),("PeaShooter","fire",1),("PeaShooter","ice",1),("ThreePeater","fire",1),("ThreePeater","ice",1),("GatlingPea","fire",1),("GatlingPea","ice",1),("PeaShooter","echo-lily",1),("Wallnut","spring-nut",1),("SunFlower","rhythm-flower",1),("PuffShroom","relay-mushroom",1),("PeaShooter","electric-pea",1),("PeaShooter","tiny-pea",0.72),("PeaShooter","heavy-pea",1.04),("PeaShooter","scatter-pea",1),("PeaShooter","seeker-pea",1),("PeaShooter","acid-pea",1)]
for (i,item) in plants.enumerated(){let (name,family,size)=item;let x=Double(i%7*200+35)+(1-size)*40,y=Double(i/7*185+50)+(1-size)*80
 draw(name,family,"anim_idle",x,y,size*1.25)
 if name=="ThreePeater"{for layer in ["anim_head_idle1","anim_head_idle3","anim_head_idle2"]{draw(name,family,layer,x,y,size*1.25)}}
 else if name=="PeaShooter"||name=="GatlingPea"{draw(name,family,"anim_head_idle",x,y,size*1.25)}
}
let zombies=[("",0),("parcel-zombie",2),("bell-zombie",1),("gum-zombie",0),("ice-bucket-zombie",4),("battery-zombie",0),("light-zombie",0),("armored-cone-zombie",2),("repair-zombie",4),("smoke-zombie",0),("twin-zombie",0)]
for (i,p) in zombies.enumerated(){draw("Zombie",p.0,"anim_walk",Double(i%7*200+5),Double(550+i/7*260),p.0=="light-zombie" ? 1.0:1.2,p.1,4)}
let target=repo.appendingPathComponent("art/expansion/rig-proof.png"),dest=CGImageDestinationCreateWithURL(target as CFURL,UTType.png.identifier as CFString,1,nil)!;CGImageDestinationAddImage(dest,ctx.makeImage()!,nil);precondition(CGImageDestinationFinalize(dest));print(target.path)
