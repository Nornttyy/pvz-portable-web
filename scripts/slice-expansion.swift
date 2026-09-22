// Mechanical production extraction: no drawing or recoloring of generated art.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let repo=URL(fileURLWithPath:CommandLine.arguments[1])
let native=URL(fileURLWithPath:CommandLine.arguments[2])
let output=repo.appendingPathComponent("art/expansion/parts")
try FileManager.default.createDirectory(at:output,withIntermediateDirectories:true)
let space=CGColorSpace(name:CGColorSpace.sRGB)!
func load(_ path:URL)->CGImage {guard let source=CGImageSourceCreateWithURL(path as CFURL,nil),let image=CGImageSourceCreateImageAtIndex(source,0,nil)else{fatalError("Missing \(path.path)")};return image}
func context(_ w:Int,_ h:Int)->CGContext{CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:space,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!}
func pixels(_ image:CGImage)->[UInt8]{let c=context(image.width,image.height);c.draw(image,in:CGRect(x:0,y:0,width:image.width,height:image.height));return Array(UnsafeBufferPointer(start:c.data!.assumingMemoryBound(to:UInt8.self),count:image.width*image.height*4))}
func bounds(_ image:CGImage,main:Bool=false)->CGRect{
 let w=image.width,h=image.height,bytes=pixels(image)
 guard stride(from:3,to:bytes.count,by:4).filter({bytes[$0]<8}).count>w*h/20 else{fatalError("Expected actual transparent alpha")}
 if !main{var x1=w,y1=h,x2=0,y2=0;for y in 0..<h{for x in 0..<w where bytes[(y*w+x)*4+3]>32{x1=min(x1,x);y1=min(y1,y);x2=max(x2,x);y2=max(y2,y)}};return CGRect(x:x1,y:y1,width:x2-x1+1,height:y2-y1+1)}
 var seen=[Bool](repeating:false,count:w*h),best=0,result=CGRect.zero
 for start in 0..<w*h where !seen[start] && bytes[start*4+3]>64{
  var queue=[start],cursor=0,x1=start%w,x2=x1,y1=start/w,y2=y1;seen[start]=true
  while cursor<queue.count{let p=queue[cursor];cursor+=1;let x=p%w,y=p/w;x1=min(x1,x);x2=max(x2,x);y1=min(y1,y);y2=max(y2,y)
   for (nx,ny) in [(x-1,y),(x+1,y),(x,y-1),(x,y+1)] where nx>=0 && nx<w && ny>=0 && ny<h{let n=ny*w+nx;if !seen[n] && bytes[n*4+3]>64{seen[n]=true;queue.append(n)}}
  }
  if queue.count>best{best=queue.count;result=CGRect(x:max(0,x1-1),y:max(0,y1-1),width:min(w-1,x2+1)-max(0,x1-1)+1,height:min(h-1,y2+1)-max(0,y1-1)+1)}
 }
 guard best>100 else{fatalError("Empty generated part")};return result
}
func cell(_ atlas:CGImage,_ index:Int)->CGImage{let w=atlas.width/2,h=atlas.height/2;let q=atlas.cropping(to:CGRect(x:(index%2)*w,y:(index/2)*h,width:w,height:h))!;return q.cropping(to:bounds(q,main:true))!}
func save(_ image:CGImage,_ url:URL){let d=CGImageDestinationCreateWithURL(url as CFURL,UTType.png.identifier as CFString,1,nil)!;CGImageDestinationAddImage(d,image,nil);guard CGImageDestinationFinalize(d)else{fatalError("PNG save failed")}}
struct Record:Codable{var file:String;var native:String;var width:Int;var height:Int;var ink:[Int]}
var records=[Record]()
func fit(_ part:CGImage,_ filename:String,_ reference:String){
 let ref=load(native.appendingPathComponent(reference));let ink=bounds(ref),w=ref.width,h=ref.height
 let c=context(w,h);c.interpolationQuality = .high
 let s=min(ink.width/CGFloat(part.width),ink.height/CGFloat(part.height))
 // Bone-bound pieces reach the native joint envelope on BOTH axes. Aspect-fit
 // caused narrow mouths and gaps between head/jaw/body. Loose props keep their aspect.
 let boneBound = !["-prop","-hat","-battery","-gum"].contains(where:filename.hasSuffix)
 let dw=boneBound ? ink.width : CGFloat(part.width)*s,dh=boneBound ? ink.height : CGFloat(part.height)*s
 let x=ink.minX+(ink.width-dw)/2,y=CGFloat(h)-ink.maxY+(ink.height-dh)/2
 c.draw(part,in:CGRect(x:x,y:y,width:dw,height:dh));let image=c.makeImage()!
 save(image,output.appendingPathComponent(filename+".png"));let b=bounds(image)
 records.append(Record(file:filename+".png",native:reference,width:w,height:h,ink:[Int(b.minX),Int(b.minY),Int(b.width),Int(b.height)]))
}
let peas=["echo-lily","electric-pea","tiny-pea","heavy-pea","scatter-pea","seeker-pea","acid-pea"]
for name in peas{let a=load(repo.appendingPathComponent("art/expansion/generated/"+name+".png"));fit(cell(a,0),name+"-head","PeaShooter_Head.png");fit(cell(a,1),name+"-blink","PeaShooter_Head.png");fit(cell(a,2),name+"-mouth","PeaShooter_mouth.png")}
for (name,refs,names) in [
 ("spring-nut",["Wallnut_body.png","Wallnut_body.png","Wallnut_cracked1.png","Wallnut_cracked2.png"],["head","blink","cracked1","cracked2"]),
 ("rhythm-flower",["SunFlower_head.png","SunFlower_head.png"],["head","blink"]),
 ("relay-mushroom",["PuffShroom_body.png","PuffShroom_body.png","PuffShroom_head.png","PuffShroom_stem.png"],["head","blink","cap","stem"])
]{let a=load(repo.appendingPathComponent("art/expansion/generated/"+name+".png"));for i in 0..<refs.count{fit(cell(a,i),name+"-"+names[i],refs[i])}}
let zombies=["parcel-zombie","bell-zombie","gum-zombie","ice-bucket-zombie","battery-zombie","light-zombie","armored-cone-zombie","repair-zombie","smoke-zombie","twin-zombie"]
for name in zombies{
 let a=load(repo.appendingPathComponent("art/expansion/generated/"+name+".png"))
 fit(cell(a,0),name+"-head","Zombie_head.png");fit(cell(a,1),name+"-body","Zombie_body.png");fit(cell(a,2),name+"-jaw","Zombie_jaw.png")
 let ref=name=="bell-zombie" ? "Zombie_flag1.png" : ["ice-bucket-zombie","repair-zombie"].contains(name) ? "Zombie_bucket1.png" : "Zombie_cone1.png"
 fit(cell(a,3),name+"-prop",ref)
 if name=="gum-zombie"{fit(cell(a,3),name+"-gum","Zombie_jaw.png")}
 if name=="battery-zombie"{fit(cell(a,3),name+"-battery","Zombie_tie.png")}
 if ["light-zombie","smoke-zombie","twin-zombie"].contains(name){fit(cell(a,3),name+"-hat","Zombie_hair.png")}
}
for family in ["fire","ice"]{
 let path=repo.appendingPathComponent("art/expansion/generated/"+family+"-hardware.png")
 if FileManager.default.fileExists(atPath:path.path){let a=load(path);for (i,pair) in [("gatling-mouth","GatlingPea_mouth.png"),("gatling-barrel","GatlingPea_barrel.png"),("gatling-overlay","GatlingPea_mouth_overlay.png"),("small-mouth","ThreePeater_mouth.png")].enumerated(){fit(cell(a,i),family+"-"+pair.0,pair.1)}}
}
let encoder=JSONEncoder();encoder.outputFormatting=[.prettyPrinted,.sortedKeys];try encoder.encode(records).write(to:repo.appendingPathComponent("art/expansion/parts.json"))
// QA contact sheet: every atlas at the same scale, not a replacement game asset.
let sheet=context(1200,1500);sheet.setFillColor(CGColor(gray:0.55,alpha:1));sheet.fill(CGRect(x:0,y:0,width:1200,height:1500));sheet.interpolationQuality = .high
let all=["echo-lily","spring-nut","rhythm-flower","relay-mushroom"]+Array(peas.dropFirst())+zombies
for (i,name) in all.enumerated(){let a=load(repo.appendingPathComponent("art/expansion/generated/"+name+".png"));sheet.draw(a,in:CGRect(x:(i%4)*300,y:1500-(i/4+1)*300,width:300,height:300))}
save(sheet.makeImage()!,repo.appendingPathComponent("art/expansion/atlas-proof.png"))
print("Prepared \(records.count) native-sized transparent parts")
// Keep the approved native-derived replacements authoritative on rebuilds.
if FileManager.default.fileExists(atPath:repo.appendingPathComponent("art/expansion/generated/native-blink.png").path){
 let registration=Process();registration.executableURL=URL(fileURLWithPath:"/usr/bin/swift")
 registration.arguments=[repo.appendingPathComponent("scripts/register-native-redraw.swift").path,repo.path,native.deletingLastPathComponent().path]
 try registration.run();registration.waitUntilExit();precondition(registration.terminationStatus==0)
}
if FileManager.default.fileExists(atPath:repo.appendingPathComponent("art/expansion/seam-generated/acid-pea.png").path){
 let registration=Process();registration.executableURL=URL(fileURLWithPath:"/usr/bin/swift")
 registration.arguments=[repo.appendingPathComponent("scripts/register-mouth-seams.swift").path,repo.path,native.deletingLastPathComponent().path]
 try registration.run();registration.waitUntilExit();precondition(registration.terminationStatus==0)
}
if FileManager.default.fileExists(atPath:repo.appendingPathComponent("art/expansion/role-generated/storm-mushroom-0-fixed.png").path){
 let registration=Process();registration.executableURL=URL(fileURLWithPath:"/usr/bin/swift")
 registration.arguments=[repo.appendingPathComponent("scripts/register-role-art.swift").path,repo.path,native.deletingLastPathComponent().path]
 try registration.run();registration.waitUntilExit();precondition(registration.terminationStatus==0)
}
if FileManager.default.fileExists(atPath:repo.appendingPathComponent("art/expansion/zombie-native-generated/paper-thrower.png").path){
 let registration=Process();registration.executableURL=URL(fileURLWithPath:"/usr/bin/swift")
 registration.arguments=[repo.appendingPathComponent("scripts/register-zombie-redraw.swift").path,repo.path,native.deletingLastPathComponent().path]
 try registration.run();registration.waitUntilExit();precondition(registration.terminationStatus==0)
}
