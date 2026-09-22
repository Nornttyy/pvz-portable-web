// Mechanical registration of image_gen head/mouth pairs, without painting new pixels.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let repo=URL(fileURLWithPath:CommandLine.arguments[1]),source=URL(fileURLWithPath:CommandLine.arguments[2])
let out=repo.appendingPathComponent("art/expansion/parts"),space=CGColorSpace(name:CGColorSpace.sRGB)!
func load(_ u:URL)->CGImage{let s=CGImageSourceCreateWithURL(u as CFURL,nil)!;return CGImageSourceCreateImageAtIndex(s,0,nil)!}
func context(_ w:Int,_ h:Int)->CGContext{CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:space,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!}
func bounds(_ image:CGImage)->CGRect{
 let w=image.width,h=image.height,c=context(w,h);c.draw(image,in:CGRect(x:0,y:0,width:w,height:h));let p=c.data!.assumingMemoryBound(to:UInt8.self)
 var x1=w,y1=h,x2=0,y2=0,transparent=0
 for y in 0..<h{for x in 0..<w{let a=p[(y*w+x)*4+3];if a>64{x1=min(x1,x);x2=max(x2,x);y1=min(y1,y);y2=max(y2,y)};if a<8{transparent+=1}}}
 precondition(x2>=x1&&y2>=y1&&transparent>w*h/50,"Expected a separate transparent part")
 return CGRect(x:x1,y:y1,width:x2-x1+1,height:y2-y1+1)
}
struct Record:Codable{var file:String;var native:String;var width:Int;var height:Int;var ink:[Int]}
struct Part:Decodable{let part:String;let native:String;let export:Bool}
struct Spec:Decodable{let key:String;let cols:Int;let parts:[Part]}
struct Plan:Decodable{let specs:[Spec]}
let manifest=repo.appendingPathComponent("art/expansion/parts.json")
var records=try JSONDecoder().decode([Record].self,from:Data(contentsOf:manifest))
let specs=try JSONDecoder().decode(Plan.self,from:Data(contentsOf:repo.appendingPathComponent("art/expansion/prompts-mouth-seams.json"))).specs
for s in specs{
 let atlas=load(repo.appendingPathComponent("art/expansion/seam-generated/"+s.key+".png"))
 let cw=CGFloat(atlas.width)/CGFloat(s.cols),ch=CGFloat(atlas.height)/2
 for (i,p) in s.parts.enumerated() where p.export{
  let ref=load(source.appendingPathComponent("reanim/"+p.native)),ink=bounds(ref)
  let cell=atlas.cropping(to:CGRect(x:CGFloat(i%s.cols)*cw,y:CGFloat(i/s.cols)*ch,width:cw,height:ch).integral)!
  let piece=cell.cropping(to:bounds(cell))!,c=context(ref.width,ref.height);c.interpolationQuality = .high
  c.draw(piece,in:CGRect(x:ink.minX,y:CGFloat(ref.height)-ink.maxY,width:ink.width,height:ink.height))
  let image=c.makeImage()!,name=s.key+"-"+p.part+".png",b=bounds(image)
  let d=CGImageDestinationCreateWithURL(out.appendingPathComponent(name) as CFURL,UTType.png.identifier as CFString,1,nil)!
  CGImageDestinationAddImage(d,image,nil);precondition(CGImageDestinationFinalize(d))
  records.removeAll{$0.file==name};records.append(Record(file:name,native:p.native,width:image.width,height:image.height,ink:[Int(b.minX),Int(b.minY),Int(b.width),Int(b.height)]))
 }
}
records.sort{$0.file<$1.file};let encoder=JSONEncoder();encoder.outputFormatting=[.prettyPrinted,.sortedKeys]
try encoder.encode(records).write(to:manifest);print("Registered \(specs.count) matching head/blink/mouth sets, keeping original bone canvases")
