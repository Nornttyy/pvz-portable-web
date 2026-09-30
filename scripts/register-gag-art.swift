// Mechanical alpha crop and native-pivot registration, never repaints generated art.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let root=URL(fileURLWithPath:CommandLine.arguments[1])
func ctx(_ w:Int,_ h:Int)->CGContext{CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:CGColorSpace(name:CGColorSpace.sRGB)!,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!}
for (name,w,h) in [("squash-headband",86,96),("bucket-glove",24,26)]{
 let version=name=="squash-headband" ? "v2" : "v1"
 let url=root.appendingPathComponent("art/abstract/\(name)-source-\(version).png")
 let image=CGImageSourceCreateImageAtIndex(CGImageSourceCreateWithURL(url as CFURL,nil)!,0,nil)!
 let raw=ctx(image.width,image.height);raw.draw(image,in:CGRect(x:0,y:0,width:image.width,height:image.height));let b=raw.data!.assumingMemoryBound(to:UInt8.self)
 var x0=image.width,y0=image.height,x1=0,y1=0,clear=0
 for y in 0..<image.height{for x in 0..<image.width{let a=b[(y*image.width+x)*4+3];if a<8{clear+=1};if a>32{x0=min(x0,x);x1=max(x1,x);y0=min(y0,y);y1=max(y1,y)}}}
 precondition(clear>image.width*image.height/10 && x1>x0,"Real alpha required")
 let crop=image.cropping(to:CGRect(x:x0,y:y0,width:x1-x0+1,height:y1-y0+1))!,c=ctx(w,h);c.interpolationQuality = .high
 // Register just the accessory above the ORIGINAL eyes, leaving the face clear.
 let rect=name=="squash-headband" ? CGRect(x:0,y:96-8-26,width:73,height:26) : CGRect(x:1,y:1,width:w-2,height:h-2)
 c.draw(crop,in:rect)
 let destination=root.appendingPathComponent("addons/art/\(name).png"),enc=CGImageDestinationCreateWithURL(destination as CFURL,UTType.png.identifier as CFString,1,nil)!
 CGImageDestinationAddImage(enc,c.makeImage()!,nil);precondition(CGImageDestinationFinalize(enc));print(destination.path)
}
