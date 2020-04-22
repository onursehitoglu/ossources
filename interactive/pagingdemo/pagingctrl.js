/* Title: pagingctrl.js
   Actions controlling the simulation
*/
var frames = [];		// list of physical pages
var nfaults = 0;
var nwrites = 0;
var policy = undefined;
var timer = 0;
var acccnt = 0;
var vacclist;
var interval = -1000;

/* Structure: logger
   object for event output
*/
logger = { 
/* Function: log(txt)
   logs the txt as a line
*/
log: function (txt) {
		console.log("(" + acccnt + ")" + txt);
		$("#logger").append("(" + acccnt + ")" + txt + "\n");
	   }
	};

/* Class: Translation
   Map keeping address translation from virtual page to frame

   Constructor: Translation(framelist)

   Parameters:
   framelist - list of frames used in cleaning the table
*/
Translation = function (framelist) {
        return {
                table : {},
/* Function: get(vaddr)
   Get the frame for the vaddr in the mapping

   Parameters: 
   vaddr - Virtual page number
*/
                get: function (vaddr) {
                        return this.table[vaddr];
                },
/* Function: set(vaddr,fid)
   Set the mapping from virtual address to frame

   Parameters: 
   vaddr - Virtual page number
   fid - Frame number
*/
                set: function (vaddr, fid) {
                        this.table[vaddr] = fid;
                },
/* Function: clean(fid)
   Clear the mapping to frame if there is any

   Parameters: 
   fid - Frame number
*/
                clean: function (fid) {
                        if (framelist[fid] != undefined && 
					framelist[fid].vpage != undefined) {
                                delete this.table[framelist[fid].vpage];
                        }
                },
/* Function: cleanup()
   Clear all mappings in address translation
*/
		cleanup: function () {
			this.table = {};
		}
        }
}

/* Section: Functions
*/

/* Function: mem_load(v, size)
   Load simulation, setup menu and action handlers

   Parameters:
   v - access list constructor
   size - number of frames
*/
function mem_load(v, size) {
	translation = Translation(frames);
	var plist = [ 
		{name: "Optimum Policy",
		summary: "optimum replacement policy minimizing number of page faults. Needs all future references of pages in advance. Calculates the forecoming reference of each frame. Picks the latest to be accessed next page. Requires calculation of next access for each frame. Example shows times as relative value.",
		get: function () { return OPTIMUM(translation);}
		},
		{name: "FIFO Policy",
		 summary: "first in first out replacement policy. It maintains frames as a queue. when a page is taken in, it is inserted at the end. Stolen pages are picked from head of the queue. The earliest a page is taken into a frame, earliest it is stolen.",
		get: function () { return FIFO(size, translation);}
		},
		{name: "Second Chance (queue)",
		 summary: "Second chance algorithm, gives referenced frames another change. Queue implementation works as FIFO but if a page to be stolen has referenced bit 1, it is cleared and inserted at the end of the queue. Each frame has a second change between it is referenced and to be stolen",
		get: function () { return SECOND_QUEUE(size, translation);}
		},
		{name: "Second Chance (clock)",
		 summary: "Second chance implemented as a clock. Instead of maintaining a queue, frames are visited one by one in a circular manner. If the frame under the clock arm is referenced, it clears reference bit and skips to next frame. If frame is not referenced, it is stolen. Does not need extra space for queue and operations for queue manipulation.",
		get: function () { return SECOND_CLOCK(size, translation);}
		},
		{name: "Enhanced Second Chance (clock)",
		 summary: "Enhanced second chance implemented as a clock. Modification bit is also taken into account. R,M = 0,0 is directly evicted, otherwise a 0,1 is tried, then 1,0, then 1,1. This specific implementation uses a clock. Clock arm evicts 0,0, remembers 0,1 and cleans reference bit. If a full round is made, it evicts remembered 0,1. Otherwise continue to next round where referenced bits cleared.",
		get: function () { return ENHANCED_CLOCK(size, translation);}
		},
		{name: "Least Recently Used",
		 summary: "assumes most recently referenced frame is to be refernced soon. Each frame is assigned a timestamp of last reference. Stolen page is chosen as the frame with the minimum (oldest) timestamp",
		 get: function () { return LRU(translation);}
		},
		{name: "Least Recently Used (4 bits bitmap)",
		 summary: "instead of a absolute timestamp uses an n-bit history of page accesses. History information is kept as n bits. With some periods (once on two page accesses in the example) history information is shifted right and most significant bit is set as referenced bit of the frame. Looking at bits of the history, we can see which of the last n accesses the frame is referenced. The smallest number gives us oldest page (since referenced bit is pushed to most significant bit). Therefore we can use the history info as the timestamp in LRU",
		 get: function () { return LRUBITS(translation, 4, 3);}
		}];
	/* create all menu items */
	for (var i in plist) { 
                $("#policylist").append("<li><a href=\"#\">" + 
                        plist[i].name + "</a></li>"); 
        } 
	/* create names and actions on menu */
        $("#menu").menu().on("menuselect", function (ev, ui) {
                mem_cleanup();
                var pol = plist[ui.item.index()].get;
		$("#policytitle").text(plist[ui.item.index()].name);
		$("#policysummary").text(plist[ui.item.index()].summary);
                mem_init(v, size, pol);
        });
       /* button actions */
       $("#slower").click(function () {
                interval *= 1.2;
                if (Math.abs(interval) > 5000)
                        $("#slower").button("disable");
                $("#faster").button("enable");

        });
        $("#faster").click(function () {
                interval /= 1.2;
                if (Math.abs(interval) < 100)
                        $("#faster").button("disable");
                $("#slower").button("enable");
        });

}

/* Function: mem_cleanup()
   Clear all variables for restart and initialization
*/
function mem_cleanup() {
	acccnt = 0;
	nfaults = 0;
	nwrites = 0;
	$("#time").text(acccnt);
	$("#pagefaults").text(nfaults);
	$("#pagewrites").text(nwrites);
	$("#logger").empty();
	$("#playpause").off("click");
	$("#singlestep").off("click");
	$("#restart").off("click");
	interval = -Math.abs(interval);
}


/* Function: mem_init(v, size, pol)
   Initialize simulation for given policy

   Parameters:
   v - access list constructor
   size - number of frames
   pol - policy constructor
*/
function mem_init(v, size, pol) 
{
	policy = pol();
	$("#frames").empty();
	translation.cleanup();
	for (var i = 1 ; i <= size ; i++) {
		frames[i] = Page(i);
	}
	timer = 0;
	acccnt = 0;
	vacclist = v();
	$("#accesslist").empty();
	for (var i in vacclist) {
		var op = (vacclist[i][1]) ? " W" : " R";
		$("#accesslist").append("<div class=\"access\">" +
			vacclist[i][0] + op + "</div>");
	}

	$("#accesslist .access:first").addClass("current");
        $("#playpause").button("option","icons",
                                {primary: "ui-icon-play"})
			.button("enable")
                       .click(function () {
                interval *= -1;
                if (interval > 0) {
                        $("#playpause").button("option","icons",
                                {primary: "ui-icon-pause"});
                        $("#singlestep").button("disable");
                        mem_next();
                } else {
                        $("#playpause").button("option","icons",
                                {primary: "ui-icon-play"});
                        $("#singlestep").button("enable");
                }
        });
        $("#singlestep").click(function () {
                mem_next();
        });
        $("#restart").click(function () {
                mem_cleanup();
                mem_init(v,size,pol);
        });
	history_init();
	$("button").button("enable");
}

function mem_next() {
	var t = vacclist[acccnt];
	var pf = undefined;
	$("#time").text(acccnt);
	if (t == undefined)  {
		console.log("simulation over");
		if (interval > 0)
			interval = interval - 1;
		$("button").button("disable");
		$("#restart").button("enable");
		return;
	}
	var curacc = $("#accesslist .current");
	var result = accesspage(t[0], t[1]);
	if (!result) {
		curacc.addClass("pagefault");
		pf = t[0];
	} 
	if (result != "RETRY" && result != "STEP") {
		var n = curacc.next();
		curacc.removeClass("current");
		n.addClass("current");
		history_add(t[0],pf);
		acccnt++;
		pf = undefined;
	}
	$("#pagefaults").text(nfaults);
	$("#pagewrites").text(nwrites);
        if (interval > 0)
                timer = setTimeout(function () { 
                                mem_next();}, interval);
	//console.log("next is " + vacclist[acccnt][0]);
}

function accesspage(vaddr,write)
{
	var fid = translation.get(vaddr);
	if (fid) {  // translation success, no page fault
		policy.accesspage(frames[fid],write);
		return true;
	} else {	// page fault
		frame = policy.getfree(frames);
		if (frame == "RETRY" || frame == "STEP")
			return frame;
		else  {
			logger.log("vpage " + vaddr + " not in memory, page fault");
			nfaults++;
		}
		policy.load(frames, frame, vaddr);
		translation.set(vaddr,frame.id);
		logger.log("vpage " + vaddr + " is loaded at frame " + 
			+ frame.id );
		policy.accesspage(frame,write);
		return false;
	}
}

function history_init()
{
	$("#history tbody").empty();
	$("#history thead").empty().append("<th>t</th>");
	for (var i in frames) {
		$("#history tbody").append("<tr><td style=\"background-color: #e0e0e0\"><b>" + i + "</b></td></tr");
	}
	$("#history tbody").append("<tr><td style=\"background-color: #e0e0e0;border:0px\"></td></tr>");
	
}
	
function history_add(ap,pf)
{
	var txt;
	var his = $("#history");
	if (pf != undefined)
		pf = translation.get(pf);
	his.find("thead").append("<th>" + acccnt + "</th>");
	for (var i in frames) {
		if (frames[i].vpage != undefined) 
			txt = frames[i].vpage;
		else 
			txt = "F";
		his.find("tbody tr:eq(" + (i-1) + ")").append("<td>" +
							 txt + "</td>");
		if (i == pf) 
			his.find("tbody tr:eq(" + (i-1) + ") td:last")
				.addClass("pagefault");
			
	}
	his.find("tbody tr:last").append("<td style=\"border:0px\">" + ap + "</td>");
}
