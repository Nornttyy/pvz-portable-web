// Mechanical alpha cropping / native-pivot registration of generated bitmaps. No painted pixels.
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
 precondition(x1>x0&&y1>y0&&clear>im.width*im.height/50,"Requires genuine alpha and a nonempty part")
 return CGRect(x:x0,y:y0,width:x1-x0+1,height:y1-y0+1)
}
struct Part:Decodable{let part:String;let native:String;let export:Bool}
struct Spec:Decodable{let key:String;let cols:Int;let parts:[Part];let family:String?}
struct Plan:Decodable{let specs:[Spec]}
struct Record:Codable{let file:String;let native:String;let width:Int;let height:Int;let ink:[Int]}
let manifest=root.appendingPathComponent("parts.json"),vfxManifest=root.appendingPathComponent("vfx-parts.json")
var records=try JSONDecoder().decode([Record].self,from:Data(contentsOf:manifest))
var vfx=try JSONDecoder().decode([Record].self,from:Data(contentsOf:vfxManifest))
func emit(_ im:CGImage,_ name:String,_ native:String,_ effect:Bool=false){
 let b=bounds(im),url=root.appendingPathComponent("parts/"+name+".png"),d=CGImageDestinationCreateWithURL(url as CFURL,UTType.png.identifier as CFString,1,nil)!
 CGImageDestinationAddImage(d,im,nil);precondition(CGImageDestinationFinalize(d))
 let r=Record(file:name+".png",native:native,width:im.width,height:im.height,ink:[Int(b.minX),Int(b.minY),Int(b.width),Int(b.height)])
 if effect{vfx.removeAll{$0.file==r.file};vfx.append(r)}else{records.removeAll{$0.file==r.file};records.append(r)}
}
for planName in ["role-redraw.json","prompts-triple-seams.json"]{
 let specs=try JSONDecoder().decode(Plan.self,from:Data(contentsOf:root.appendingPathComponent(planName))).specs
 for s in specs{
  let suffix=s.key.hasPrefix("storm-") ? "-fixed":""
  let atlas=load(root.appendingPathComponent("role-generated/"+s.key+suffix+".png")),cw=atlas.width/s.cols,ch=atlas.height/2
  for (i,p) in s.parts.enumerated() where p.export{
   let ref=load(source.appendingPathComponent("reanim/"+p.native)),ink=bounds(ref)
   let cell=atlas.cropping(to:CGRect(x:(i%s.cols)*cw,y:(i/s.cols)*ch,width:cw,height:ch))!,piece=cell.cropping(to:bounds(cell))!
   // Extra room grows upward while the native bottom / neck joint stays fixed.
   let extra=(s.key=="paper-thrower"||s.key=="ink-painter")&&p.part=="head" ? 16:s.key.hasPrefix("storm-")&&p.part=="cap" ? 14:0
   let c=context(ref.width,ref.height+extra*2);c.interpolationQuality = .high
   c.draw(piece,in:CGRect(x:ink.minX,y:CGFloat(ref.height)-ink.maxY+CGFloat(extra),width:ink.width,height:ink.height+CGFloat(extra)))
   emit(c.makeImage()!,(s.family ?? s.key)+"-"+p.part,p.native)
  }
 }
}
let damage=load(root.appendingPathComponent("role-generated/ranged-damage.png")),dcw=damage.width/2,dch=damage.height/2
for (i,family) in ["paper-thrower","ink-painter"].enumerated(){
 let ref=load(source.appendingPathComponent("reanim/Zombie_outerarm_upper2.png")),ink=bounds(ref)
 let cell=damage.cropping(to:CGRect(x:dcw,y:i*dch,width:dcw,height:dch))!,piece=cell.cropping(to:bounds(cell))!,c=context(ref.width,ref.height);c.interpolationQuality = .high
 c.draw(piece,in:CGRect(x:ink.minX,y:CGFloat(ref.height)-ink.maxY,width:ink.width,height:ink.height));emit(c.makeImage()!,family+"-outer-upper-damaged","Zombie_outerarm_upper2.png")
}
let atlas=load(root.appendingPathComponent("role-generated/vfx-ranged.png")),cw=atlas.width/4,ch=atlas.height/2
let names=["paper-shot","paper-hit-0","paper-hit-1","paper-ready","ink-shot","ink-hit-0","ink-hit-1","ink-ready"]
let sizes=[(44,44),(72,64),(72,64),(44,44),(54,40),(72,64),(72,64),(40,40)]
for i in 0..<8{
 let cell=atlas.cropping(to:CGRect(x:i%4*cw,y:i/4*ch,width:cw,height:ch))!,piece=cell.cropping(to:bounds(cell))!
 let (w,h)=sizes[i],c=context(w,h);c.interpolationQuality = .high;c.draw(piece,in:CGRect(x:1,y:1,width:w-2,height:h-2))
 emit(c.makeImage()!,"vfx-"+names[i],"generated-vfx",true)
}
let encoder=JSONEncoder();encoder.outputFormatting=[.prettyPrinted,.sortedKeys]
try encoder.encode(records.sorted{$0.file<$1.file}).write(to:manifest);try encoder.encode(vfx.sorted{$0.file<$1.file}).write(to:vfxManifest)
print("Registered \(records.count) skeletal parts and \(vfx.count) VFX sprites")
