// Arrange existing native bitmaps as edit targets. No sprite drawing or recoloring.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let repo=URL(fileURLWithPath:CommandLine.arguments[1]),source=URL(fileURLWithPath:CommandLine.arguments[2])
struct Part:Decodable{let native:String;let reference:String?}
struct Spec:Decodable{let key:String;let cols:Int;let parts:[Part]}
struct Plan:Decodable{let specs:[Spec]}
let plan=CommandLine.arguments.count>3 ? CommandLine.arguments[3]:"remaining-redraw.json"
let folder=CommandLine.arguments.count>4 ? CommandLine.arguments[4]:"remaining-reference"
let specs=try JSONDecoder().decode(Plan.self,from:Data(contentsOf:repo.appendingPathComponent("art/expansion/"+plan))).specs
let out=repo.appendingPathComponent("art/expansion/"+folder);try FileManager.default.createDirectory(at:out,withIntermediateDirectories:true)
for s in specs{
 let cell=384,w=s.cols*cell,h=cell*max(2,(s.parts.count+s.cols-1)/s.cols)
 let c=CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:CGColorSpace(name:CGColorSpace.sRGB)!,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!
 c.interpolationQuality = .none
 for (i,p) in s.parts.enumerated(){
  let url=p.reference.map{repo.appendingPathComponent($0)} ?? source.appendingPathComponent("reanim/"+p.native)
  let src=CGImageSourceCreateWithURL(url as CFURL,nil)!,im=CGImageSourceCreateImageAtIndex(src,0,nil)!
  let scale=min(5.0,min(292.0/Double(im.width),292.0/Double(im.height))),dw=Double(im.width)*scale,dh=Double(im.height)*scale
  c.draw(im,in:CGRect(x:Double(i%s.cols*cell)+(Double(cell)-dw)/2,y:Double(h-(i/s.cols+1)*cell)+(Double(cell)-dh)/2,width:dw,height:dh))
 }
 let u=out.appendingPathComponent(s.key+".png"),d=CGImageDestinationCreateWithURL(u as CFURL,UTType.png.identifier as CFString,1,nil)!
 CGImageDestinationAddImage(d,c.makeImage()!,nil);precondition(CGImageDestinationFinalize(d));print(u.path)
}
