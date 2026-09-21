// Mechanical contact sheet of unchanged native sprite parts for image_gen edits.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let repo=URL(fileURLWithPath:CommandLine.arguments[1]),source=URL(fileURLWithPath:CommandLine.arguments[2])
let files=["PeaShooter_Head.png","PeaShooter_mouth.png","ThreePeater_head.png","ThreePeater_mouth.png","GatlingPea_head.png"]
let w=1600,h=960,cell=320
let ctx=CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:CGColorSpace(name:CGColorSpace.sRGB)!,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!
ctx.interpolationQuality = .none
for row in 0..<3 {for (col,file) in files.enumerated(){
 let url=source.appendingPathComponent("reanim/"+file),s=CGImageSourceCreateWithURL(url as CFURL,nil)!,image=CGImageSourceCreateImageAtIndex(s,0,nil)!
 let dw=image.width*4,dh=image.height*4
 ctx.draw(image,in:CGRect(x:col*cell+(cell-dw)/2,y:h-row*cell-(cell+dh)/2,width:dw,height:dh))
}}
let out=repo.appendingPathComponent("art/expansion/native-redraw-reference.png")
let d=CGImageDestinationCreateWithURL(out as CFURL,UTType.png.identifier as CFString,1,nil)!
CGImageDestinationAddImage(d,ctx.makeImage()!,nil);precondition(CGImageDestinationFinalize(d));print(out.path)
