package {
	import flash.desktop.NativeApplication;
	import flash.display.Bitmap;
	import flash.display.BitmapData;
	import flash.display.BitmapDataChannel;
	import flash.display.Loader;
	import flash.display.LoaderInfo;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.InvokeEvent;
	import flash.filesystem.File;
	import flash.geom.Point;
	import flash.net.URLRequest;
	import flash.text.TextField;
	import flash.utils.ByteArray;
	import mx.events.Request;
	import utils.FileUtil;
	
	/**
	 * ...
	 * @author Yinqiang
	 */
	public class Main extends Sprite {
		
		var args:Array;
		var tasks:Array;
		var curTask:Array;
		var txt:TextField;
		var png1:Loader, png2:Loader;
		
		public function Main():void {
			args = [];
			
			txt = new TextField();
			txt.defaultTextFormat.size = 32;
			txt.defaultTextFormat.font = "Helvetica,Arial,Verdana,sans-serif";
			txt.width = stage.stageWidth;
			addChild(txt);
			
			NativeApplication.nativeApplication.addEventListener(InvokeEvent.INVOKE, init);
		}
		
		private function println(str:String):void {
			txt.appendText(str + "\n");
		}
		
		private function printUsage():void {
			println("Usage: margetool.exe -i dir_path_to_pngs -o dir_path_for_output");
		}
		
		private function init(e:InvokeEvent):void {
			NativeApplication.nativeApplication.removeEventListener(InvokeEvent.INVOKE, init);
			
			for (var i:int = 0; i < e.arguments.length; i++) {
				switch (e.arguments[i]) {
					case "-i":
						args["input"] = e.arguments[++i];
						break;
					case "-o":
						args["output"] = e.arguments[++i];
						break;
					default:
						printUsage();
						return;
				}
			}
			if (args["input"] == undefined
			    || args["output"] == undefined)
			{
				printUsage();
				return;
			}
			
			var inputDir:File = new File(args["input"]);
			if (!inputDir.isDirectory) {
				printUsage();
				return;
			}
			var outputDir:File = new File(args["output"]);
			if (!outputDir.exists) {
				outputDir.createDirectory();
			}
			
			var pngs:Array = [];
			var alphas:Array = [];
			for each (var file:File in inputDir.getDirectoryListing()) {
				if (file.extension == "png") {
					if (file.name.search("_alpha") == -1) {
						pngs[file.name] = file.nativePath;
					} else {
						alphas[file.name.replace("_alpha", "")] = file.nativePath;
					}
				}
			}
			
			tasks = [];
			for (var name:String in pngs) {
				if (alphas[name] != null) {
					tasks.push([
						name,
						pngs[name],
						alphas[name]
					]);
				}
			}
			
			load();
		}
		
		private function load():void {
			if (tasks.length > 0) {
				png1 = new Loader();
				png2 = new Loader();
				png1.contentLoaderInfo.addEventListener(Event.COMPLETE, onLoadComplete);
				png2.contentLoaderInfo.addEventListener(Event.COMPLETE, onLoadComplete);
				curTask = tasks.pop();
				var file:File = new File(curTask[1]);
				png1.load(new URLRequest(file.nativePath));
				file = new File(curTask[2]);
				png2.load(new URLRequest(file.nativePath));
			} else {
				println("all done");
			}
		}
		
		private function onLoadComplete(e:Event):void {
			const info:LoaderInfo = e.currentTarget as LoaderInfo;
			info.removeEventListener(Event.COMPLETE, onLoadComplete);
			if ((png1.content as Bitmap).bitmapData != null
				&& (png2.content as Bitmap).bitmapData != null)
			{
				merge();
				load();
			}
		}
		
		private function merge():void {
			const bmpdata1:BitmapData = (png1.content as Bitmap).bitmapData;
			const bmpdata2:BitmapData = (png2.content as Bitmap).bitmapData;
			const bmpdata:BitmapData = new BitmapData(bmpdata1.width, bmpdata1.height, true, 0);
			const bytes1:ByteArray = bmpdata1.getPixels(bmpdata1.rect);
			const bytes2:ByteArray = bmpdata2.getPixels(bmpdata2.rect);
			const bytes:ByteArray = new ByteArray();
			bytes1.position = 0;
			bytes2.position = 0;
			bytes.position = 0;
			while (bytes1.position < bytes1.length) {
				bytes.writeUnsignedInt((bytes1.readUnsignedInt() & 0xFFFFFF) | (bytes2.readUnsignedInt() & 0xFF0000) << 8);
			}
			bytes.position = 0;
			bmpdata.setPixels(bmpdata.rect, bytes);
			FileUtil.savePNGFile(bmpdata, new File(args["output"] + File.separator + curTask[0]));
			bmpdata1.dispose();
			bmpdata2.dispose();
			bmpdata.dispose();
			png1.unload();
			png2.unload();
		}
		
	}
	
}