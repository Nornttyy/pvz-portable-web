// Offline alpha-overlap regression against original animation frames, not a browser test.
import Foundation
import CoreGraphics
import ImageIO
let repo=URL(fileURLWithPath:CommandLine.arguments[1]),source=URL(fileURLWithPath:CommandLine.arguments[2])
struct Pose{var x=0.0,y=0.0,kx=0.0,ky=0.0,sx=1.0,sy=1.0,f = -1.0,a=1.0}
struct Track{var name="",frames=[Pose]()}
class Reader:NSObject,XMLParserDelegate{
 var tracks=[Track](),track=Track(),pose=Pose(),text=""
 func parser(_ p:XMLParser,didStartElement n:String,namespaceURI:String?,qualifiedName:String?,attributes:[String:String]){text="";if n=="track"{track=Track();pose=Pose()}}
 func parser(_ p:XMLParser,foundCharacters s:String){text+=s}
 func parser(_ p:XMLParser,didEndElement n:String,namespaceURI:String?,qualifiedName:String?){
  let v=Double(text) ?? 0
  switch n{case "name":track.name=text;case "x":pose.x=v;case "y":pose.y=v;case "kx":pose.kx=v;case "ky":pose.ky=v;case "sx":pose.sx=v;case "sy":pose.sy=v;case "f":pose.f=v;case "a":pose.a=v;case "t":track.frames.append(pose);case "track":tracks.append(track);default:break}
 }
}
var rigs=[String:[Track]](),images=[String:CGImage]()
func image(_ path:String)->CGImage{if let i=images[path]{return i};let s=CGImageSourceCreateWithURL(URL(fileURLWithPath:path) as CFURL,nil)!,i=CGImageSourceCreateImageAtIndex(s,0,nil)!;images[path]=i;return i}
func rig(_ name:String)->[Track]{if let t=rigs[name]{return t};let s=try! String(contentsOf:source.appendingPathComponent("reanim/"+name+".reanim"),encoding:.utf8);let p=XMLParser(data:Data(("<root>"+s+"</root>").utf8)),r=Reader();p.delegate=r;precondition(p.parse());rigs[name]=r.tracks;return r.tracks}
let w=384,h=320,scale=3.0,space=CGColorSpace(name:CGColorSpace.sRGB)!
func alpha(_ image:CGImage,_ p:Pose)->[UInt8]{
 let c=CGContext(data:nil,width:w,height:h,bitsPerComponent:8,bytesPerRow:w*4,space:space,bitmapInfo:CGImageAlphaInfo.premultipliedLast.rawValue)!
 c.interpolationQuality = .high;c.translateBy(x:24,y:Double(h)-36);c.scaleBy(x:scale,y:-scale)
 let kx=p.kx*Double.pi/180,ky=p.ky*Double.pi/180
 c.concatenate(CGAffineTransform(a:cos(kx)*p.sx,b:sin(kx)*p.sx,c:-sin(ky)*p.sy,d:cos(ky)*p.sy,tx:p.x,ty:p.y))
 c.translateBy(x:0,y:Double(image.height));c.scaleBy(x:1,y:-1);c.draw(image,in:CGRect(x:0,y:0,width:image.width,height:image.height))
 let bytes=c.data!.assumingMemoryBound(to:UInt8.self);return (0..<w*h).map{bytes[$0*4+3]}
}
struct Result:Encodable{let family:String;let rig:String;let pair:Int;let blink:Bool;let testedFrames:Int;let detachedFrames:Int;let minimumOverlapRatio:Double;let worstFrame:Int}
var results=[Result]()
let single=["fire","ice","electric-pea","echo-lily","tiny-pea","heavy-pea","scatter-pea","seeker-pea","acid-pea"]
let roster=single.map{($0,"PeaShooterSingle")}+[("fire","PeaShooter"),("ice","PeaShooter"),("fire","ThreePeater"),("ice","ThreePeater")]
for (family,name) in roster{
 let tracks=rig(name),triple=name=="ThreePeater"
 for pair in 1...(triple ? 3:1){
  let face=tracks.first{$0.name==(triple ? "anim_face\(pair)":"anim_face")}!,mouth=tracks.first{$0.name==(triple ? "ThreePeater_mouth\(pair)":"idle_mouth")}!
  let nativeHead=image(source.appendingPathComponent("reanim/"+(triple ? "ThreePeater_head.png":"PeaShooter_Head.png")).path)
  let nativeMouth=image(source.appendingPathComponent("reanim/"+(triple ? "ThreePeater_mouth.png":"PeaShooter_mouth.png")).path)
  let customMouth=image(repo.appendingPathComponent("art/expansion/parts/"+family+(triple ? "-small-mouth.png":"-mouth.png")).path)
  for blink in [false,true]{
   let customHead=image(repo.appendingPathComponent("art/expansion/parts/"+family+(triple ? "-small":"")+(blink ? "-blink.png":"-head.png")).path)
   precondition(customHead.width==nativeHead.width&&customHead.height==nativeHead.height&&customMouth.width==nativeMouth.width&&customMouth.height==nativeMouth.height)
   var count=0,detached=0,minimum=Double.infinity,worst=0
   for frame in 0..<min(face.frames.count,mouth.frames.count){
    let a=face.frames[frame],b=mouth.frames[frame];if a.f<0||b.f<0||a.a<=0||b.a<=0{continue}
    let nh=alpha(nativeHead,a),nm=alpha(nativeMouth,b),ch=alpha(customHead,a),cm=alpha(customMouth,b)
    var original=0,edited=0
    for i in 0..<w*h{if nh[i]>96&&nm[i]>96{original+=1};if ch[i]>96&&cm[i]>96{edited+=1}}
    if original==0{continue};count+=1;let ratio=Double(edited)/Double(original)
    if edited==0{detached+=1};if ratio<minimum{minimum=ratio;worst=frame}
   }
   precondition(count>0);results.append(Result(family:family,rig:name,pair:pair,blink:blink,testedFrames:count,detachedFrames:detached,minimumOverlapRatio:minimum,worstFrame:worst))
  }
 }
}
let encoder=JSONEncoder();encoder.outputFormatting=[.prettyPrinted,.sortedKeys]
let filename=CommandLine.arguments.contains("--before") ? "mouth-joints-before.json":"mouth-joints-audit.json"
try encoder.encode(results).write(to:repo.appendingPathComponent("art/expansion/"+filename))
print("\(results.reduce(0){$0+$1.testedFrames}) head/mouth frames checked; \(results.reduce(0){$0+$1.detachedFrames}) detached; worst native-overlap ratio \(results.map{$0.minimumOverlapRatio}.min()!)")
if !CommandLine.arguments.contains("--before"){precondition(results.allSatisfy{$0.detachedFrames==0&&$0.minimumOverlapRatio>=0.45},"A seam lost more than half its native overlap; inspect before shipping")}
