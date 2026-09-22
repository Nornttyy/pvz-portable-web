// Mechanical registration of image_gen edits. Native faces/hands are reused byte-for-byte.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let repo=URL(fileURLWithPath:CommandLine.arguments[1]),source=URL(fileURLWithPath:CommandLine.arguments[2]),root=URL(fileURLWithPath:CommandLine.arguments[1]).appendingPathComponent("art/expansion")
let space=CGColorSpace(name:CGColorSpace.sRGB)!
func context(_ w:Int,_ h:Int)->CGContext{CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:space,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!}
func load(_ u:URL)->CGImage{let s=CGImageSourceCreateWithURL(u as CFURL,nil)!;return CGImageSourceCreateImageAtIndex(s,0,nil)!}
func bounds(_ im:CGImage)->CGRect{
 let c=context(im.width,im.height);c.draw(im,in:CGRect(x:0,y:0,width:im.width,height:im.height));let b=c.data!.assumingMemoryBound(to:UInt8.self)
 var x0=im.width,y0=im.height,x1=0,y1=0,clear=0
 for y in 0..<im.height{for x in 0..<im.width{let a=b[(y*im.width+x)*4+3];if a<8{clear+=1};if a>64{x0=min(x0,x);y0=min(y0,y);x1=max(x1,x);y1=max(y1,y)}}}
 precondition(x1>x0&&y1>y0&&clear>im.width*im.height/50,"Requires real alpha and a nonempty piece")
 return CGRect(x:x0,y:y0,width:x1-x0+1,height:y1-y0+1)
}
struct Part:Decodable{let part:String;let native:String;let export:Bool}
struct Spec:Decodable{let parts:[Part];let key:String?}
struct Variant:Decodable{let key:String;let hat:Bool}
struct Plan:Decodable{let specs:[Spec];let variants:[Variant]}
struct ArmorPlan:Decodable{let specs:[Spec]}
struct Record:Codable{let file:String;let native:String;let width:Int;let height:Int;let ink:[Int]}
let manifest=root.appendingPathComponent("parts.json")
var records=try JSONDecoder().decode([Record].self,from:Data(contentsOf:manifest))
let plan=try JSONDecoder().decode(Plan.self,from:Data(contentsOf:root.appendingPathComponent("zombie-native-redraw.json")))
func record(_ im:CGImage,_ name:String,_ native:String){
 let b=bounds(im);records.removeAll{$0.file==name+".png"};records.append(Record(file:name+".png",native:native,width:im.width,height:im.height,ink:[Int(b.minX),Int(b.minY),Int(b.width),Int(b.height)]))
}
func emit(_ im:CGImage,_ name:String,_ native:String){
 let d=CGImageDestinationCreateWithURL(root.appendingPathComponent("parts/"+name+".png") as CFURL,UTType.png.identifier as CFString,1,nil)!
 CGImageDestinationAddImage(d,im,nil);precondition(CGImageDestinationFinalize(d));record(im,name,native)
}
for v in plan.variants{
 let url=root.appendingPathComponent("zombie-native-generated/"+v.key+".png")
 guard FileManager.default.fileExists(atPath:url.path)else{continue}
 let atlas=load(url),cw=atlas.width/3,ch=atlas.height/3
 for (i,p) in plan.specs[0].parts.enumerated() where p.export && (p.part != "hat" || v.hat){
  let ref=load(source.appendingPathComponent("reanim/"+p.native)),ink=bounds(ref)
  let cell=atlas.cropping(to:CGRect(x:i%3*cw,y:i/3*ch,width:cw,height:ch))!,piece=cell.cropping(to:bounds(cell))!
  let c=context(ref.width,ref.height);c.interpolationQuality = .high
  // Head is never rescaled for its hat. Hat lives on the native hair bone above it.
  let target=p.part=="hat" ? CGRect(x:15,y:2,width:38,height:24):CGRect(x:ink.minX,y:CGFloat(ref.height)-ink.maxY,width:ink.width,height:ink.height)
  c.draw(piece,in:target);emit(c.makeImage()!,v.key+"-"+p.part,p.native)
 }
 for (part,native) in [("head","Zombie_head.png"),("jaw","Zombie_jaw.png"),("hand","Zombie_outerarm_hand.png")]{
  let url=source.appendingPathComponent("reanim/"+native)
  try Data(contentsOf:url).write(to:root.appendingPathComponent("parts/"+v.key+"-"+part+".png"));record(load(url),v.key+"-"+part,native)
 }
}
let armorPlan=try JSONDecoder().decode(ArmorPlan.self,from:Data(contentsOf:root.appendingPathComponent("zombie-armor-redraw.json")))
for s in armorPlan.specs{
 let family=s.key!,url=root.appendingPathComponent("zombie-armor-generated/"+family+".png")
 guard FileManager.default.fileExists(atPath:url.path)else{continue}
 let atlas=load(url),cw=atlas.width/3,ch=atlas.height
 for (i,p) in s.parts.enumerated() where p.export{
  let ref=load(source.appendingPathComponent("reanim/"+p.native)),ink=bounds(ref)
  let cell=atlas.cropping(to:CGRect(x:i%3*cw,y:i/3*ch,width:cw,height:ch))!,piece=cell.cropping(to:bounds(cell))!
  let c=context(ref.width,ref.height);c.interpolationQuality = .high
  c.draw(piece,in:CGRect(x:ink.minX,y:CGFloat(ref.height)-ink.maxY,width:ink.width,height:ink.height));emit(c.makeImage()!,family+"-"+p.part,p.native)
 }
}
records.sort{$0.file<$1.file};let encoder=JSONEncoder();encoder.outputFormatting=[.prettyPrinted,.sortedKeys]
try encoder.encode(records).write(to:manifest)
print("Registered native-derived zombie costumes, unchanged native faces and independent hats: \(records.count) parts")
