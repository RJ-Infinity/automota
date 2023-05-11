DEBUG = true;

function readCanvasFromMemory(memory_buffer, canvas_ptr)
{
	const canvas_memory = new Uint32Array(memory_buffer, canvas_ptr, 4);
	return {
		pixels: canvas_memory[0],
		width: canvas_memory[1],
		height: canvas_memory[2],
		stride: canvas_memory[3],
	};
}
function InteropKeyboardEvent(buffer, ptr, e){
	var evnt = new Uint32Array(buffer, ptr, 3);
	evnt[0]=GetStringPtr(e.code);
	evnt[1]=e.keyCode;

	var evnt = new Uint8Array(buffer, ptr+2*4, 3);
	evnt[0]=e.altKey;
	evnt[1]=e.ctrlKey;
	evnt[2]=e.shiftKey;
	return ptr;
}
InteropKeyboardEvent.size = 2*4+3;

function InteropMouseEvent(buffer, ptr, e){
	var evnt = new Uint32Array(buffer, ptr, 3);
	evnt[0]=e.offsetX;
	evnt[1]=e.offsetY;
	evnt[2]=e.button;

	var evnt = new Uint8Array(buffer, ptr+3*4, 3);
	evnt[0]=e.altKey;
	evnt[1]=e.ctrlKey;
	evnt[2]=e.shiftKey;
	return ptr;
}
InteropMouseEvent.size = 3*4+3;


function GetStringPtr(str){
	var bytes = new TextEncoder().encode(str+"\x00");
	var strPtr = memAllocer.alloc(bytes.length);
	new Uint8Array(
		automotaCLib.instance.exports.memory.buffer,
		strPtr,
		bytes.byteLength
	).set(bytes);
	return strPtr;
}
function GetStrLen(ptr){
	var iPtr = ptr;
	var byte;
	var length = 0;
	while (byte!==0){
		byte = new Uint8Array(automotaCLib.instance.exports.memory.buffer, iPtr, 1)[0];
		length++;
		iPtr++;
	}
	return length-1;
}
function GetString(ptr){
	return new TextDecoder().decode(new Uint8Array(
		automotaCLib.instance.exports.memory.buffer,
		ptr,
		GetStrLen(ptr)
	));
}


function addScript(src) {
	// https://stackoverflow.com/a/69616316
	return new Promise((resolve, reject) => {
		const s = document.createElement('script');

		s.setAttribute('src', src);
		s.addEventListener('load', resolve);
		s.addEventListener('error', reject);

		document.body.appendChild(s);
	});
}

async function main(){
	const canvasEl = document.getElementById("canvas")
	const memAllocer = new memAlloc();

	var debugLoggerBuffer = "";

	var fetchRes = fetch("/build/automota.wasm")
	if (DEBUG){
		// first import the d2sm script
		try{
			await addScript("/d2sm.js");
		}catch{
			console.error("you dont have d2sm. try turning debug mode off")
			return;
		}
		fetchRes = new Response(
			patchWithSourceMap(await (await fetchRes).arrayBuffer()), {
				headers: {
					'Content-Type': 'application/wasm'
				}
			}
		)
	}

	const automotaCLib = await WebAssembly.instantiateStreaming(
		fetchRes, {env:new Proxy(
		{
			"render": function(pointer){
				const canvas = readCanvasFromMemory(
					automotaCLib.instance.exports.memory.buffer,pointer
				);
				if (canvas.width != canvas.stride) {
					console.error(`Canvas width (${canvas.width}) is not equal to its stride (${canvas.stride}). Unfortunately we can't easily support that in a browser because ImageData simply does not accept stride. Welcome to 2022.`);
					return;
				}
				const ctx = canvasEl.getContext("2d");
				const image = new ImageData(new Uint8ClampedArray(automotaCLib.instance.exports.memory.buffer, canvas.pixels, canvas.width*canvas.height*4), canvas.width);
			
				canvasEl.width = canvas.width;
				canvasEl.height = canvas.height;
			
				ctx.putImageData(image, 0, 0);
			},
			"consoleLog": (ptr)=>console.log(GetString(ptr)),
			"consoleLogi": console.log,
			"consoleLogb": (bool)=>console.log(bool!=0),
			"malloc": memAllocer.alloc.bind(memAllocer),
			"free": memAllocer.free.bind(memAllocer),
			"itoa": GetStringPtr,
			"strcmp": function(a, b){
				i=0;
				while (true){
					Abyte = new Uint8Array(automotaCLib.instance.exports.memory.buffer, a+i, 1)[0];
					Bbyte = new Uint8Array(automotaCLib.instance.exports.memory.buffer, b+i, 1)[0];
					if (Abyte > Bbyte){return 1;}
					if (Abyte < Bbyte){return -1;}
					if (/*Abyte == Bbyte && */Abyte==0){return 0;}
					i++
				}
			},
			"strlen": GetStrLen,
			"debug": function(line){console.log(line);debugger;},
			"pushToBuffer": function(x){debugLoggerBuffer+=GetString(x);},
			"pushToBufferi": function(i){debugLoggerBuffer+=i;},
			"pushToBufferb": function(bool){debugLoggerBuffer+=bool!=0;},
			"logBuffer": function(){console.log(debugLoggerBuffer)},
			"clearBuffer": function(){debugLoggerBuffer="";},
			"getBuffer": ()=>GetStringPtr(debugLoggerBuffer),
			"runAfterTime": (callbackPtr, time)=>setTimeout(
				automotaCLib.instance.exports.__indirect_function_table.get(callbackPtr),
				time
			),
			"getUnixTimeStamp":()=>Number(new Date),
		},
		{
			get(target, prop, receiver) {
				if (target.hasOwnProperty(prop)) {
					return target[prop];
				}
				return (...args) => {console.error("NOT IMPLEMENTED: "+prop, args)}
			}
		}
	)});
	memAllocer.setMemory(automotaCLib.instance.exports.memory);
	window.automotaCLib = automotaCLib;//FIXME: this is debug only
	window.memAllocer = memAllocer;
	automotaCLib.instance.exports.main();
	function render(){
		automotaCLib.instance.exports.resize(
			window.innerWidth,
			window.innerHeight
		);
		automotaCLib.instance.exports.draw();
	}
	render();
	canvasEl.addEventListener("mousedown",(e)=>{
		var evnt = InteropMouseEvent(
			automotaCLib.instance.exports.memory.buffer,
			memAllocer.alloc(InteropMouseEvent.size),
			e
		);
		automotaCLib.instance.exports.mouseDown(evnt);
		memAllocer.free(evnt);
		if (e.button == 1){
			e.preventDefault();
			return false;
		}
	});
	canvasEl.addEventListener("mouseup",(e)=>{
		var evnt = InteropMouseEvent(
			automotaCLib.instance.exports.memory.buffer,
			memAllocer.alloc(InteropMouseEvent.size),
			e
		);
		automotaCLib.instance.exports.mouseUp(evnt);
		memAllocer.free(evnt);
	});
	canvasEl.addEventListener("mousemove",(e)=>{
		var evnt = InteropMouseEvent(
			automotaCLib.instance.exports.memory.buffer,
			memAllocer.alloc(InteropMouseEvent.size),
			e
		);
		automotaCLib.instance.exports.mouseMove(evnt);
		memAllocer.free(evnt);
	});
	canvasEl.addEventListener("keydown",(e)=>{
		var evnt = InteropKeyboardEvent(
			automotaCLib.instance.exports.memory.buffer,
			memAllocer.alloc(InteropMouseEvent.size),
			e
		);
		automotaCLib.instance.exports.keyDown(evnt);
		memAllocer.free(new Uint32Array(automotaCLib.instance.exports.memory.buffer, evnt, 3)[0]);
		memAllocer.free(evnt);
	});
	canvasEl.addEventListener("contextmenu",(e)=>{
		if (automotaCLib.instance.exports.contextMenu()){e.preventDefault();}
	});
	window.addEventListener("resize",render)
}
main();
