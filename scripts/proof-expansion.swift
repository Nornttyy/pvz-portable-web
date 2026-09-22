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
let poseMode=CommandLine.arguments.contains("--poses")
let roleMode=CommandLine.arguments.contains("--roles")
let zombieMode=CommandLine.arguments.contains("--zombies")
var damaged=false
var armorStage=0
var closed=false
let space=CGColorSpace(name:CGColorSpace.sRGB)!,w=1400,h=zombieMode ? 940:roleMode ? 900:poseMode ? 1260:1120
let ctx=CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:space,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!
ctx.setFillColor(CGColor(red:0.54,green:0.66,blue:0.3,alpha:1));ctx.fill(CGRect(x:0,y:0,width:w,height:h));ctx.interpolationQuality = .high
func override(_ family:String,_ part:String)->CGImage?{
 let actualFamily=family=="ice-fire" ? (part.contains("mouth") ? "fire":"ice"):family
 let actualPart=closed&&part.contains("head") ? part.replacingOccurrences(of:"head",with:"blink"):part
 let generated=repo.appendingPathComponent("art/expansion/parts/"+actualFamily+"-"+actualPart+".png")
 return image(FileManager.default.fileExists(atPath:generated.path) ? generated : source.appendingPathComponent("images/sandbox/"+actualFamily+"-"+actualPart+".png"))
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
    if FileManager.default.fileExists(atPath:repo.appendingPathComponent("art/expansion/zombie-native-generated/"+family+".png").path){
     if n=="Zombie_tie"&&family != "battery-zombie"{continue}
     if damaged&&(n=="Zombie_outerarm_lower"||n=="Zombie_outerarm_hand"){continue}
     if n=="anim_innerarm1"{selected=override(family,"inner-upper")!}
     if n=="anim_innerarm2"{selected=override(family,"inner-lower")!}
     if n=="Zombie_outerarm_upper"{selected=override(family,damaged ? "outer-upper-damaged":"outer-upper")!}
     if n=="Zombie_outerarm_lower"{selected=override(family,"outer-lower")!}
    }
    if n=="anim_head1"{selected=override(family,"head")!}
    if n=="anim_head2"{selected=override(family,"jaw")!}
    if n=="Zombie_body"{selected=override(family,"body")!}
    if n=="anim_cone"||n=="anim_bucket"{selected=override(family,armorStage==0 ? "prop":"prop-damage\(armorStage)")!}
    if n=="anim_hair" && ["light-zombie","smoke-zombie","twin-zombie","paper-thrower","ink-painter"].contains(family){selected=override(family,"hat")!}
    if n=="Zombie_tie" && family=="battery-zombie"{selected=override(family,"battery")!}
   }
  }else if name=="Zombie_flagpole"{if n=="Zombie_flag"{selected=override(family,"prop")!}}
  else if !family.isEmpty{
   if n.lowercased().contains("blink") || (name=="PuffShroom" && n=="PuffShroom_eyes"){continue}
   if name=="SunFlower"{if n=="anim_idle"{selected=override(family,"head")!}}
   else if name=="PuffShroom"{if n=="anim_face"{selected=override(family,"head")!};if n=="PuffShroom_head"{selected=override(family,"cap")!};if n=="PuffShroom_stem"{selected=override(family,"stem")!}}
   else if n.hasPrefix("anim_face"){selected=override(family,name=="GatlingPea" ? "gatling-head":name=="ThreePeater" ? "small-head":"head")!}
   if name != "GatlingPea" && (n=="idle_mouth"||n.hasPrefix("ThreePeater_mouth")){selected=override(family,name=="ThreePeater" ? "small-mouth":"mouth")!}
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
let plants:[(String,String,Double)]=[("PeaShooter","",1),("PeaShooter","fire",1),("PeaShooter","ice",1),("ThreePeater","fire",1),("ThreePeater","ice",1),("GatlingPea","fire",1),("GatlingPea","ice",1),("PeaShooter","echo-lily",1),("SunFlower","rhythm-flower",1),("PuffShroom","storm-mushroom-2",1.36),("PeaShooter","electric-pea",1),("PeaShooter","tiny-pea",0.72),("PeaShooter","heavy-pea",1.04),("PeaShooter","scatter-pea",1),("PeaShooter","seeker-pea",1),("PeaShooter","acid-pea",1)]
for (i,item) in (poseMode||roleMode||zombieMode ? []:plants).enumerated(){let (name,family,size)=item;let x=Double(i%7*200+35)+(1-size)*40,y=Double(i/7*185+50)+(1-size)*65
 draw(name,family,"anim_idle",x,y,size*1.25)
 if name=="ThreePeater"{for layer in ["anim_head_idle1","anim_head_idle3","anim_head_idle2"]{draw(name,family,layer,x,y,size*1.25)}}
 else if name=="PeaShooter"||name=="GatlingPea"{draw(name,family,"anim_head_idle",x,y,size*1.25)}
}
let zombies=[("",0),("parcel-zombie",2),("bell-zombie",1),("gum-zombie",0),("ice-bucket-zombie",4),("battery-zombie",0),("light-zombie",0),("armored-cone-zombie",2),("repair-zombie",4),("smoke-zombie",0),("twin-zombie",0)]
for (i,p) in (poseMode||roleMode||zombieMode ? []:zombies).enumerated(){draw("Zombie",p.0,"anim_walk",Double(i%7*200+5),Double(550+i/7*260),p.0=="light-zombie" ? 1.0:1.2,p.1,4)}
if poseMode{
 let roster:[(String,String,Double)]=[("PeaShooterSingle","fire",1),("PeaShooter","ice",1),("PeaShooter","fire",1),("ThreePeater","ice",1),("ThreePeater","fire",1),("GatlingPea","ice",1),("GatlingPea","fire",1),("PeaShooter","ice-fire",1),("PeaShooterSingle","echo-lily",1),("SunFlower","rhythm-flower",1),("PuffShroom","storm-mushroom-2",1.36),("PeaShooterSingle","electric-pea",1),("PeaShooterSingle","tiny-pea",0.72),("PeaShooterSingle","heavy-pea",1.04),("PeaShooterSingle","scatter-pea",1),("PeaShooterSingle","seeker-pea",1),("PeaShooterSingle","acid-pea",1)]
 for (i,item) in roster.enumerated(){let (name,family,size)=item
  for state in 0..<3{
   closed=state==2;let frame=state==0 ? 0:6
   let x=Double(i%6*232+60)+(1-size)*40,y=Double(i/6*410+40+state*118)+(1-size)*65
   draw(name,family,"anim_idle",x,y,size*1.25,0,frame)
   let tracks=rig(name),start=tracks.first(where:{$0.name=="anim_idle"})!.frames.firstIndex(where:{$0.f>=0})!
   let links=name=="ThreePeater" ? [("anim_head1","anim_head_idle1","anim_shooting1"),("anim_head3","anim_head_idle3","anim_shooting3"),("anim_head2","anim_head_idle2","anim_shooting2")]:name.contains("PeaShooter")||name=="GatlingPea" ? [(tracks.contains(where:{$0.name=="anim_stem"}) ? "anim_stem":"anim_idle","anim_head_idle","anim_shooting")]:[]
   for (attach,idle,shoot) in links{if let t=tracks.first(where:{$0.name==attach}){
    let transform=matrix(t.frames[start]).inverted().concatenating(matrix(t.frames[min(start+frame,t.frames.count-1)]))
    draw(name,family,state==1 ? shoot:idle,x,y,size*1.25,0,frame,transform)
   }}
  }
 }
}
if roleMode{
 for state in 0..<3{
  closed=state==2;damaged=state==2
  for stage in 0..<3{let size=1.0+Double(stage)*0.18;draw("PuffShroom","storm-mushroom-\(stage)","anim_idle",Double(stage*250+35)+(1-size)*40,Double(state*290+65)+(1-size)*65,size*1.5,0,state*6)}
  closed=false
  for (i,family) in ["paper-thrower","ink-painter"].enumerated(){draw("Zombie",family,state==1 ? "anim_eat":"anim_walk",Double(790+i*275),Double(state*290+30),1.5,0,state*6)}
 }
}
if zombieMode{
 let roster=Array(zombies.dropFirst())+[("paper-thrower",0),("ink-painter",0)]
 for (i,p) in roster.enumerated(){
  for state in 0..<3{
   damaged=state==2;closed=false;armorStage=state
   let size=p.0=="light-zombie" ? 0.72:1.0
   draw("Zombie",p.0,state==1 ? "anim_eat":"anim_walk",Double(i%4*350+state*108+4),Double(i/4*300+55)+(1-size)*145,size,p.1,state*7)
  }
 }
}
let target=repo.appendingPathComponent(zombieMode ? "art/expansion/zombie-poses-proof.png":roleMode ? "art/expansion/role-poses-proof.png":poseMode ? "art/expansion/plant-poses-proof.png":"art/expansion/rig-proof.png"),dest=CGImageDestinationCreateWithURL(target as CFURL,UTType.png.identifier as CFString,1,nil)!;CGImageDestinationAddImage(dest,ctx.makeImage()!,nil);precondition(CGImageDestinationFinalize(dest));print(target.path)
