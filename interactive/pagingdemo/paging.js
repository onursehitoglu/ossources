function animatemove(element, newpos)
{
        var oldoff = element.offset();
        var newoff = newpos.offset();
        var temp = element.clone().appendTo('body');
        temp    .css('position', 'absolute')
                .css('left', oldoff.left)
                .css('top', oldoff.top)
                .css('zIndex', 1000);
        element.hide();
        temp.animate( {'top': newoff.top, 'left':newoff.left}, 
                Math.abs(interval/3),
                function(){ element.show(); temp.remove(); });
}

Page = function (id) {
	$("#frames").append("<div class=\"page free\" id=\"frame_" + id + 
		"\"><div>" + id + "</div><div class=\"info\"></div>" +
		"<div id=\"point\" class=\"nocursor\"></div></div>");
	return {
	id: id,
	vpage: undefined,
	referenced: false,
	modified: false,
	out: function () {
		this.vpage  = undefined
	}
	};
}


function getfree(framelist) {
	for (var i in framelist) {
		if (framelist[i].vpage == undefined)
			return framelist[i];
	}
	return undefined;
}

Policy = function (trans) {
	return {
		translation : trans,
		accesspage: function (frame, write) {
			var rdwr = (write) ? "writing" : "reading";
			logger.log("frame " + frame.id + " with vaddr " +
				frame.vpage + " access for " + rdwr);
			frame.referenced = true;
			if (write)
				frame.modified = true;
			this.updateview(frame);
		},
		updateview: function (frame, infotxt, usecursor) {
			var fm =$("#frame_" + frame.id);
			var fim =$("#frame_" + frame.id + " .info");
			if (frame.vpage != undefined) {
				if (infotxt) {
					fim.html("P: " + frame.vpage +
					"<br/>" + infotxt);
				} else {
					fim.text("P: " + frame.vpage);
				}
				fm.removeClass("free");
				fm.addClass("nonfree");
				if (frame.modified) {
					fm.addClass("modified");
				} else {
					fm.removeClass("modified");
				}
			} else {
				fm.removeClass("nonfree");
				fm.addClass("free");
				fim.text("");
				fm.removeClass("modified");
			}
			if (usecursor) {
				$("#frames .cursor").removeClass("cursor").addClass("nocursor");
				$("#frame_" + this.clockpos + " #point").removeClass("nocursor").addClass("cursor");
			}
			
		},
		load: function (frames, frame, vaddr) {
			if (frame.vpage) {
				alert("loading on a used frame without freeing it");
			}
			frame.vpage = vaddr;
			frame.modified = false;
			frame.referenced = false;
			if (this.pagein) 
				this.pagein(frame);
		},
		getfree: function (frames) {
			var frame = getfree(frames);
			if (frame)
				return frame;
			frame = this.evictpage(frames);
			if (frame == "STEP")
				return frame;
			var dirty = (frame.modified) ? "dirty" : "not dirty";
			logger.log("frame " + frame.id + " with vaddr " +
				frame.vpage + " is evicted. page was " + dirty);
			if (frame.modified)
				nwrites++;
			this.translation.clean(frame.id);
			frame.vpage = undefined;
			this.updateview(frame);
			return "RETRY";
		}
	}
}

FIFO = function (size, trans) {
	var p = Policy(trans);
	p.queue = [];
	p.maxframes = size;
	p.pagein = function (frame) {
			this.queue.push(frame);
			animatemove($("#frame_"+frame.id), $("#frames :last"));
			$("#frames").append($("#frame_" + frame.id));
	}
	p.evictpage = function () {
			return this.queue.shift();
	}
	return p;
}

OPTIMUM = function (trans) {
	var p = Policy(trans);
	p.nextaccess = [];
	p.next = function (vid) {
			for (var i = acccnt+1; i < vacclist.length; i++) {
				if (vacclist[i][0] == vid)
					return i-acccnt;
			}
			return "NEVER"; // never
		};
	p.updatenext = function () {
			for (var i in frames) {
				if (frames[i].vpage != undefined) {
					frames[i].nextacc =this.next(frames[i].vpage); 
					this.nextaccess[i] = frames[i].nextacc;
				} else  {
					frames[i].nextacc = -1;
					this.nextaccess[i] = -1;
				}
				this.updateview(frames[i]);
			}
		};
	p._updateview = p.updateview;
	p.updateview = function (frame) {
		p._updateview(frame, "nxt:" + frame.nextacc);
		};
	p.evictpage = function () {
			var latest = -1;
			var frame;
			for (var i in frames) {
				if (frames[i].nextacc == "NEVER") {
					frame = frames[i];
					break;
				} else {
					if (frames[i].nextacc > latest) {
						latest = frames[i].nextacc;
						frame = frames[i];
					}
				}
			}
			return frame;
		};
	p._accesspage = p.accesspage;
	p.accesspage = function (frame,write) {
			p._accesspage(frame,write);
			this.updatenext();
		};
	return p;
};

SECOND_QUEUE= function (size,trans) {
	var p = Policy(trans);
	p.queue = [];
	p.maxframes = size;
	p._updateview = p.updateview;
	p.updateview = function (frame) {
			var reftxt = (frame.referenced) ? " R:1" : " R:0"
			p._updateview(frame, reftxt);
		};
	p.pagein = function (frame) {
			this.queue.push(frame);
			animatemove($("#frame_"+frame.id), $("#frames :last"));
			$("#frames").append($("#frame_" + frame.id));
		};
	p.evictpage = function (frames) {
			var frame = this.queue.shift();
			while (frame.referenced) {
				frame.referenced = false;
				this.queue.push(frame);
				this.updateview(frame);
				animatemove($("#frame_"+frame.id), $("#frames :last"));
				$("#frames").append($("#frame_" + frame.id));
				return "STEP";
				//frame = this.queue.shift();
			}
			return frame;
		};
	return p;
}
	
SECOND_CLOCK = function (size, trans) {
	var p = Policy(trans);
	p.clockpos = 1;
	p.maxframes = size;
	p._baseupdateview = p.updateview;
	p.updateview = function (frame) {
			var reftxt = (frame.referenced) ? " R:1" : " R:0"
			p._baseupdateview(frame, reftxt, true);
		};
	p.evictpage = function () {
			var frame = frames[this.clockpos];
			while (frame.vpage != undefined) {
				if (frame.referenced) {
					frame.referenced = false;
				} else {
					break;
				} 
				this.clockpos = (this.clockpos % size) + 1;
				this.updateview(frame);
				return "STEP";
			}

			this.clockpos = (this.clockpos % size) + 1;
			return frame;
		};
	return p;
};

ENHANCED_CLOCK = function (size, trans) {
	var p = Policy(trans);
	p.clockpos = 1;
	p.maxframes = size;
	p._baseupdateview = p.updateview;
	p.updateview = function (frame) {
			var reftxt = "R,M:";
			reftxt = reftxt + ((frame.referenced) ? "1," : "0,");
			reftxt = reftxt + ( (frame.modified) ? "1" : "0");
			if (this.candidate == frame.id) {
				reftxt = reftxt + "*";
			}
			
			p._baseupdateview(frame, reftxt, true);
		};
	p.evictpage = function () {
			var frame = frames[this.clockpos];
			if (this.start == undefined)
				this.start = this.clockpos;
			while (frame.vpage != undefined) {
				if (frame.referenced) {
					frame.referenced = false;
				} else if (frame.modified) {
					if (this.candidate == undefined)
						this.candidate = this.clockpos;
				} else {	// found 0,0
					break;
				} 
				this.clockpos = (this.clockpos % size) + 1;
				if (this.clockpos == this.start) {
					// we made a cycle and no 0,0
					if (this.candidate) {
						frame = frames[this.candidate];
						break;
					} else {	// start next round
						delete this.start;
					}
				}
					
				this.updateview(frame);
				return "STEP";
			}
			delete this.start;
			if (this.candidate) {
				var t = frames[this.candidate];
				delete this.candidate;
				this.updateview(t);
			}
			this.clockpos = (this.clockpos % size) + 1;
			return frame;
		};
	return p;
};

LRU = function (trans) {
	var p = Policy(trans);
	p.timestamps = [];
	p._accesspage = p.accesspage;
	p.accesspage = function (frame, write) {
			this.timestamps[frame.id] = acccnt;
			p._accesspage(frame, write);
		};
	p._updateview = p.updateview;
	p.updateview = function (frame) {
			p._updateview(frame, "ts:" + this.timestamps[frame.id]);
		};
	p.pagein = function (frame) {
			this.timestamps[frame.id] = acccnt;
		};
	p.evictpage = function (frames) {
			var frame = frames[1];
			var earliest = 1;

			for (var i in frames) {
				if (this.timestamps[i] < this.timestamps[earliest]) {
						earliest = i;
						frame = frames[i];
				}
			}
			return frame;
		};
	return p;
}

LRUBITS = function (trans,nbits,period) {
	var hbit = 1 << (nbits-1) ;
	function tobin(n) {
		var a = n.toString(2); // in binary
		var h = nbits;
		while (h > a.length) {
			a = "0" + a;
		}
		return a;
	}
	var p = Policy(trans);
	p.bits = [];
	p._accesspage = p.accesspage;
	p.accesspage  = function (frame, write) {
			p._accesspage(frame, write);
			if ((acccnt % period)  == period-1) {
			$("#accessedit").fadeIn().fadeOut();
				for (var i in frames) {
					if (frames[i].vpage != undefined) {
						if (frames[i].referenced) {
							this.bits[i] = (this.bits[i] >> 1) | hbit;
						} else {
							this.bits[i] = (this.bits[i] >> 1) ;
						}
						frames[i].referenced = false;
					}
					this.updateview(frames[i]);
				}
			}
		};
	p._updateview = p.updateview;
	p.updateview = function (frame) {
			if (this.bits[frame.id] != undefined) 
				p._updateview(frame,"r:" + ((frame.referenced)?1:0) + "<br/>b:" +
					 tobin(this.bits[frame.id]));
			else
				p._updateview(frame,"");
		};
	p.pagein = function (frame) {
			this.bits[frame.id] = 0;
		};
	p.evictpage = function	() {
			var eframe = frames[1];
			var earliest = 1;

			for (var i in frames) {
				if (( ! frames[i].referenced   && eframe.referenced) ||
						(frames[i].referenced == eframe.referenced && this.bits[i] < this.bits[earliest])) {
						earliest = i;
						eframe = frames[i];
				}
			}
			return eframe;
		};
	return p;
}

