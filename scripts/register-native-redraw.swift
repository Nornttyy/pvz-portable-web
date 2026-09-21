// Mechanical extraction/registration of image_gen edits onto the original bone canvases.
// No recoloring or procedural art: hardware and green mouths are copied byte-for-byte.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let repo=URL(fileURLWithPath:CommandLine.arguments[1]),source=URL(fileURLWithPath:CommandLine.arguments[2])
let out=repo.appendingPathComponent("art/expansion/parts")
let space=CGColorSpace(name:CGColorSpace.sRGB)!
func load(_ u:URL)->CGImage{let s=CGImageSourceCreateWithURL(u as CFURL,nil)!;return CGImageSourceCreateImageAtIndex(s,0,nil)!}
func context(_ w:Int,_ h:Int)->CGContext{CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:space,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!}
func bounds(_ image:CGImage)->CGRect{
 let w=image.width,h=image.height,c=context(w,h);c.draw(image,in:CGRect(x:0,y:0,width:w,height:h));let p=c.data!.assumingMemoryBound(to:UInt8.self)
 var x1=w,y1=h,x2=0,y2=0,transparent=0
 for y in 0..<h{for x in 0..<w{let a=p[(y*w+x)*4+3];if a>64{x1=min(x1,x);x2=max(x2,x);y1=min(y1,y);y2=max(y2,y)};if a<8{transparent+=1}}}
 precondition(x2>=x1&&y2>=y1&&transparent>w*h/50,"Expected transparent sprite")
 return CGRect(x:x1,y:y1,width:x2-x1+1,height:y2-y1+1)
}
func save(_ i:CGImage,_ name:String){let d=CGImageDestinationCreateWithURL(out.appendingPathComponent(name) as CFURL,UTType.png.identifier as CFString,1,nil)!;CGImageDestinationAddImage(d,i,nil);precondition(CGImageDestinationFinalize(d))}
struct Record:Codable{var file:String;var native:String;var width:Int;var height:Int;var ink:[Int]}
let manifest=repo.appendingPathComponent("art/expansion/parts.json")
var records=try JSONDecoder().decode([Record].self,from:Data(contentsOf:manifest))
func record(_ name:String,_ ref:String,_ image:CGImage){
 let b=bounds(image);records.removeAll{$0.file==name}
 records.append(Record(file:name,native:ref,width:image.width,height:image.height,ink:[Int(b.minX),Int(b.minY),Int(b.width),Int(b.height)]))
}
let refs=["PeaShooter_Head.png","PeaShooter_mouth.png","ThreePeater_head.png","ThreePeater_mouth.png","GatlingPea_head.png"]
let parts=["head","mouth","small-head","small-mouth","gatling-head"]
let families=["fire","ice","electric-pea"]
for (closed,filename) in [(false,"native-redraw.png"),(true,"native-blink.png")]{
 let atlas=load(repo.appendingPathComponent("art/expansion/generated/"+filename))
 for (row,family) in families.enumerated(){for col in 0..<5 where (row<2||col<2)&&(!closed||[0,2,4].contains(col)){
  let ref=load(source.appendingPathComponent("reanim/"+refs[col])),ink=bounds(ref)
  let cw=CGFloat(atlas.width)/5,ch=CGFloat(atlas.height)/3
  let cell=atlas.cropping(to:CGRect(x:CGFloat(col)*cw,y:CGFloat(row)*ch,width:cw,height:ch).integral)!
  let piece=cell.cropping(to:bounds(cell))!
  let c=context(ref.width,ref.height);c.interpolationQuality = .high
  c.draw(piece,in:CGRect(x:ink.minX,y:CGFloat(ref.height)-ink.maxY,width:ink.width,height:ink.height))
  let suffix=closed ? parts[col].replacingOccurrences(of:"head",with:"blink"):parts[col]
  let name=family+"-"+suffix+".png",image=c.makeImage()!;save(image,name);record(name,refs[col],image)
 }}
}
// Original hardware retains exact alpha holes, occluders, perspective and mating edges.
for family in ["fire","ice"]{for (part,ref) in [("gatling-mouth","GatlingPea_mouth.png"),("gatling-barrel","GatlingPea_barrel.png"),("gatling-overlay","GatlingPea_mouth_overlay.png")]{
 let name=family+"-"+part+".png",url=source.appendingPathComponent("reanim/"+ref)
 try Data(contentsOf:url).write(to:out.appendingPathComponent(name));record(name,ref,load(url))
}}
for family in ["tiny-pea","heavy-pea","scatter-pea","seeker-pea","acid-pea"]{
 let name=family+"-mouth.png",ref="PeaShooter_mouth.png",url=source.appendingPathComponent("reanim/"+ref)
 try Data(contentsOf:url).write(to:out.appendingPathComponent(name));record(name,ref,load(url))
}
records.sort{$0.file<$1.file};let encoder=JSONEncoder();encoder.outputFormatting=[.prettyPrinted,.sortedKeys]
try encoder.encode(records).write(to:manifest);print("Registered \(records.count) rig parts; native hardware copied unchanged")
