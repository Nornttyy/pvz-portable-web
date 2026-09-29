// Export generated raster components. Cropping/scaling only; no painted pixels.
import Foundation
import CoreGraphics
import ImageIO
import UniformTypeIdentifiers
let root=URL(fileURLWithPath:CommandLine.arguments[1]).appendingPathComponent("art/fusion")
let space=CGColorSpace(name:CGColorSpace.sRGB)!
func context(_ w:Int,_ h:Int)->CGContext { CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:space,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)! }
func load(_ name:String)->CGImage { CGImageSourceCreateImageAtIndex(CGImageSourceCreateWithURL(root.appendingPathComponent("source/"+name+"-v1.png") as CFURL,nil)!,0,nil)! }
func bounds(_ im:CGImage)->CGRect {
 let c=context(im.width,im.height);c.draw(im,in:CGRect(x:0,y:0,width:im.width,height:im.height))
 let b=c.data!.assumingMemoryBound(to:UInt8.self)
 var x0=im.width,y0=im.height,x1=0,y1=0,clear=0
 for y in 0..<im.height { for x in 0..<im.width {
  let a=b[(y*im.width+x)*4+3];if a<8 {clear+=1};if a>16{x0=min(x0,x);y0=min(y0,y);x1=max(x1,x);y1=max(y1,y)}
 }}
 precondition(x1>x0 && y1>y0 && clear>im.width*im.height/30,"Nonempty real-alpha sprite required")
 return CGRect(x:x0,y:y0,width:x1-x0+1,height:y1-y0+1)
}
struct Part:Codable { let file:String;let width:Int;let height:Int;let ink:[Int];let source:String;let cell:[Int] }
var parts:[Part]=[]
func export(_ family:String,_ names:[String],_ rows:[Double],_ sizes:[Int]) {
 let atlas=load(family)
 for (i,name) in names.enumerated() {
  let columns:[Double]=family=="dandelion" && i<3 ? [0,0.35,2.0/3.0,1]:[0,1.0/3.0,2.0/3.0,1]
  let row=i/3,x=Int(columns[i%3]*Double(atlas.width)),cw=Int(columns[i%3+1]*Double(atlas.width))-x,y=Int(rows[row]*Double(atlas.height)),bottom=Int(rows[row+1]*Double(atlas.height))
  let rect=CGRect(x:x,y:y,width:cw,height:bottom-y)
  let cell=atlas.cropping(to:rect)!,ink=bounds(cell)
  print(family,name,rect,ink)
  precondition(ink.minX>0 && ink.minY>0 && ink.maxX<CGFloat(cw) && ink.maxY<CGFloat(bottom-y),"Sprite touches crop edge: "+family+"-"+name+" "+String(describing:ink))
  let piece=cell.cropping(to:ink)!,side=sizes[i],scale=Double(side-4)/Double(max(piece.width,piece.height))
  let w=max(5,Int(Double(piece.width)*scale)+4),h=max(5,Int(Double(piece.height)*scale)+4)
  let c=context(w,h);c.interpolationQuality = .high;c.draw(piece,in:CGRect(x:2,y:2,width:w-4,height:h-4))
  let im=c.makeImage()!,file=family+"-"+name+".png"
  let dst=CGImageDestinationCreateWithURL(root.appendingPathComponent("parts/"+file) as CFURL,UTType.png.identifier as CFString,1,nil)!
  CGImageDestinationAddImage(dst,im,nil);precondition(CGImageDestinationFinalize(dst))
  parts.append(Part(file:file,width:w,height:h,ink:[2,2,w-4,h-4],source:family+"-v1.png",cell:[x,y,cw,bottom-y]))
 }
}
// Walnut redraw is an archived draft. The live fusion now attaches the original
// PeaShooter_mouth directly to the original Wallnut reanimation; no exports needed.
export("dandelion",["head","blink","empty","stem","left","right","seed","hit","tuft"],[0,0.40,0.745,1],[192,192,192,112,112,112,64,80,40])
let encoder=JSONEncoder();encoder.outputFormatting=[.prettyPrinted,.sortedKeys]
try encoder.encode(parts).write(to:root.appendingPathComponent("parts.json"))
print("Exported \(parts.count) new RGBA components; all sprites clear of cell boundaries.")
