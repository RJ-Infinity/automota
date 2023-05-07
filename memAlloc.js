/* (c) RJ_Infinity
i cant be borthered to find a proper license so just be sensible when using it and give me credit thanks
*/

class memAlloc{
	static DEBUG = false
	memAllocs={};
	freeMem={};
	setMemory(memory){this.memory = memory;}
	alloc(size){
		// size must be 4 byte alligned
		size += 4-(size%4)

		//TODO: make the allocator detect if the free memory is too small but at the
		// end of the memory buffer and use this but extend the memory so it fits
		for (const loc of Object.keys(this.freeMem)) {
			if (this.freeMem[loc]>=size){
				this.memAllocs[loc]=size;
				if (this.freeMem[loc]!=size){
					this.freeMem[Number(loc)+size]=this.freeMem[loc]-size;
				}
				memAlloc.DEBUG && console.log("allocating "+size+"b at "+loc+(
					this.freeMem[loc]!=size?
					" leaving "+(this.freeMem[loc]-size)+"b free at "+(Number(loc)+size):
					""
				));
				delete this.freeMem[loc];
				return Number(loc);
			}
		}
		// there is no free memory of the correct size so we need to create some
		var loc = this.memory.buffer.byteLength;
		this.memAllocs[loc]=size;
		//the ceil and /64*1000 is to find it in pages not bytes
		var allocSize = Math.ceil(size/(64*1000));
		this.memory.grow(allocSize)
		allocSize *= 64*1000; // turn the alloc size into bytes (total allocated size)
		if (allocSize - size > 0){
			// no need to merge blocks as there is always the memory we are
			// using before the current location and this memory always is the
			// end of the total memory
			
			// this could be made more effective by allocating at the end of
			// the grown block but in practice the memory is usualy freed
			// before the next memory is allocated so it shouldnt matter
			this.freeMem[loc + size]=allocSize - size;
		}
		memAlloc.DEBUG && console.log("allocating "+size+"b at "+loc+(
			allocSize - size > 0?
			" leaving "+(allocSize - size)+"b free at "+(loc+size):
			""
		)+" by growing the memory");
		return loc;
	};
	free(mem){
		var size = this.memAllocs[mem];
		//this means this is not memory we alloced so just ignore it
		if (size === undefined){
			console.warn("tried to free memory that was allocated with a different allocator");
			return;
		}
		delete this.memAllocs[mem];
		memAlloc.DEBUG && console.log("freeing memory of size "+size+"b at "+mem);
		for (const loc of Object.keys(this.freeMem)) {
			if (Number(loc)===mem+size){
				memAlloc.DEBUG && console.log("merging memory of size "+this.freeMem[loc]+"b at "+loc+" with the memory of size "+size+"b at "+mem+" which has just been freed");
				size += this.freeMem[loc];
				delete this.freeMem[loc];
				break; // unless somthing has gone horribly wrong once we have found
				// one instance of memory after the freed section there should be no
				// other memory in that location so we can break
			}
		}
		for (const loc of Object.keys(this.freeMem)) {
			if (Number(loc)+this.freeMem[loc] == mem){
				memAlloc.DEBUG && console.log("merging memory of size "+this.freeMem[loc]+"b at "+loc+" with the memory of size "+size+"b at "+mem+" which has just been freed");
				this.freeMem[loc]+=size
				return; // similar to the break above except as this memory location has
				// been modified in place we dont need to add a new entry in the free table
				// so we can do an early return
			}
		}
		memAlloc.DEBUG && console.log("creating new free memory entry for memory of size "+size+"b at "+mem)
		this.freeMem[mem]=size;
	};
}