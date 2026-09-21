// Only crops/resizes generated raster artwork; never draws substitute game assets.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let repo=URL(fileURLWithPath:CommandLine.arguments[1]),root=repo.appendingPathComponent("art/expansion")
let space=CGColorSpace(name:CGColorSpace.sRGB)!
func context(_ w:Int,_ h:Int)->CGContext{CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:space,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!}
func load(_ name:String)->CGImage{let s=CGImageSourceCreateWithURL(root.appendingPathComponent("generated/"+name+".png") as CFURL,nil)!;return CGImageSourceCreateImageAtIndex(s,0,nil)!}
func save(_ im:CGImage,_ url:URL){let d=CGImageDestinationCreateWithURL(url as CFURL,UTType.png.identifier as CFString,1,nil)!;CGImageDestinationAddImage(d,im,nil);precondition(CGImageDestinationFinalize(d))}
func bounds(_ image:CGImage)->CGRect{
 let c=context(image.width,image.height);c.draw(image,in:CGRect(x:0,y:0,width:image.width,height:image.height));let b=c.data!.assumingMemoryBound(to:UInt8.self)
 var x0=image.width,y0=image.height,x1=0,y1=0,clear=0
 for y in 0..<image.height{for x in 0..<image.width{let a=b[(y*image.width+x)*4+3];if a<8{clear+=1};if a>80{x0=min(x0,x);y0=min(y0,y);x1=max(x1,x);y1=max(y1,y)}}}
 precondition(clear>image.width*image.height/20,"No real alpha: regenerate, never chroma-key")
 precondition(x1>x0 && y1>y0,"Empty sprite")
 return CGRect(x:max(0,x0-2),y:max(0,y0-2),width:min(image.width-1,x1+2)-max(0,x0-2)+1,height:min(image.height-1,y1+2)-max(0,y0-2)+1)
}
func cells(_ name:String,_ rows:Int)->[CGImage]{let a=load(name),w=a.width/4,h=a.height/rows;return (0..<(4*rows)).map{a.cropping(to:CGRect(x:($0%4)*w,y:($0/4)*h,width:w,height:h))!}}
struct Record:Codable{var file:String;var native:String;var width:Int;var height:Int;var ink:[Int]}
var records=[Record](),sprites=[CGImage]()
func emit(_ source:CGImage,_ crop:CGRect,_ name:String,_ w:Int,_ h:Int){
 let im=source.cropping(to:crop)!,c=context(w,h);c.interpolationQuality = .high
 c.draw(im,in:CGRect(x:1,y:1,width:w-2,height:h-2));let out=c.makeImage()!;save(out,root.appendingPathComponent("parts/vfx-"+name+".png"));sprites.append(out)
 let r=bounds(out);records.append(Record(file:"vfx-"+name+".png",native:"generated-vfx",width:w,height:h,ink:[Int(r.minX),Int(r.minY),Int(r.width),Int(r.height)]))
}
let names=["fire","ice","electric","tiny","heavy","scatter","seeker","acid"]
let shots=cells("vfx-projectiles",2),sizes=[(60,44),(46,40),(50,46),(20,20),(48,48),(26,26),(50,36),(46,46)]
for i in 0..<8{emit(shots[i],bounds(shots[i]),names[i],sizes[i].0,sizes[i].1)}
let impacts=cells("vfx-impacts",4)
for i in 0..<8{let r=bounds(impacts[i*2]).union(bounds(impacts[i*2+1]));for f in 0..<2{emit(impacts[i*2+f],r,names[i]+"-hit-\(f)",80,80)}}
let skills=cells("vfx-skills",4)
let spec:[(String,Int,Int)]=[("link-0",80,20),("link-1",80,20),("sonic-0",40,84),("sonic-1",40,84),("muzzle-warm",40,30),("muzzle-ice",40,30),("spring-0",64,64),("spring-1",64,64),("smoke-0",96,96),("smoke-1",96,96),("smoke-2",96,96),("notes",36,44),("poison",36,48),("repair",40,40),("gum",48,48),("charge",60,60)]
for i in 0..<16{var r=bounds(skills[i]);if i<4{let start=i/2*2;r=bounds(skills[start]).union(bounds(skills[start+1]))};if i==6||i==7{r=bounds(skills[6]).union(bounds(skills[7]))};if i>=8&&i<=10{r=bounds(skills[8]).union(bounds(skills[9])).union(bounds(skills[10]))};emit(skills[i],r,spec[i].0,spec[i].1,spec[i].2)}
let encoder=JSONEncoder();encoder.outputFormatting=[.prettyPrinted,.sortedKeys];try encoder.encode(records).write(to:root.appendingPathComponent("vfx-parts.json"))
let proof=context(800,500);proof.setFillColor(CGColor(red:0.55,green:0.66,blue:0.34,alpha:1));proof.fill(CGRect(x:0,y:0,width:800,height:500))
for(i,s)in sprites.enumerated(){let scale=min(70.0/Double(s.width),70.0/Double(s.height));let w=Double(s.width)*scale,h=Double(s.height)*scale;proof.draw(s,in:CGRect(x:Double(i%8*100)+(100-w)/2,y:500-Double(i/8*100)-(100+h)/2,width:w,height:h))}
save(proof.makeImage()!,root.appendingPathComponent("vfx-proof.png"));print("Prepared \(records.count) alpha VFX sprites with shared frame registration")
