/* File: schedctrl.js
   Actions of scheduler
*/

var CPU = undefined;
var CPUlast = undefined;
var timer = undefined;
var time = 0;
var interval = -1000;
var cswitch = 0;
var waitcount = 0;
var maxwait = 0;


/* Structure: logger
   An object to log in console and web element with id logger

*/
logger = {
/* Function: log(txt)
   logs the txt as a line
*/
	   log: function (txt) {
		console.log(txt);
		$("#logger").append(txt + "\n");
	   }
	};

/* Section: Functions */

/* Function: sched_tick(policy, plist)
   simulates each clock tick in scheduler

   Parameters:
   policy - policy object to give policy decisions
   plist - list of processes
*/
function sched_tick(policy,plist) {
	var csw = false;
	/* update variables */
	time ++;
	waitcount += policy.waiting;
	
	/* update counters/values on page */
	$("#time").text(time);
	$("#totalwait").text(waitcount);

	var p = undefined;

	/* process newly arriving process in <plist> and add to policy */
	sched_arrivals(policy, plist);

	/* when CPU, sleep queue and policy is empty, simulationis over */
	if (CPU == undefined && sleepqueue.empty() && policy.empty()) {
		logger.log("(" + time + ") " + "no process left. stopping scheduler");
		interval = -Math.abs(interval);
		$("#slower").button("disable");
		$("#faster").button("disable");
		$("#playpause").button("disable");
		$("#singlestep").button("disable");
		$("#restart").button("enable");
		return;
	}

	/* get a processes slept enough from sleep queue */
	var wakingup = sleepqueue.tick();
	for (var p in wakingup) {
		logger.log("(" + time + ") " + "process " + wakingup[p].proc.pid + " woke up");
		var pr = wakingup[p].proc;
		pr.state = "READY";
		pr.startwait = time;
		pr.settxt(pr.burst());
		animatemove($("#proc_" + pr.pid), $("#policy"));
		/* add process to policy as ready */
		policy.procready(pr);
	}

	/* execute the process in CPU for a tick, define new state */
	if (CPU) { 
		p = CPU.tick();
		if (p.state == "SLEEP") {
			logger.log("(" + time + ") " + "process " + p.pid + " is going to sleep for " +
				p.duration + " ticks");
			/* insert into sleep queue */
			sleepqueue.insert(p, p.duration);
		} else if (p.state == "TERM") {
			$("#proc_" + p.pid).remove();
			logger.log("(" + time + ") " + "process " + p.pid + " terminated");
			/* terminated */
		}
		if (p.state != "RUNNING") {
			/* if still not running define CPU as free */
			CPU = undefined;
		}
		
	} 

	/* if CPU is busy and policy decides on preemption */
	if (CPU && policy.preempt(CPU)) {
		CPU.state = "READY";
		logger.log("(" + time + ") " + "process " + CPU.pid + 
			"[" + CPU.burst() + "] is preemptted");
		animatemove($("#proc_" + CPU.pid), $("#policy"));
		CPU.startwait = time;
		/* get process from CPU and add to policy, mark CPU as free,
		   policy will asign the new process to CPU */
		policy.procready(CPU);
		CPU = undefined;
	}

	/* if CPU is free test policy for new process */
	if (CPU == undefined) {
		CPU = policy.getready();
		if (CPU) {
			var element = $("#proc_" + CPU.pid );
			animatemove(element, $("#cpu"));
			$("#cpu").append(element);
			if (maxwait < (time - CPU.startwait)) {
				maxwait = time - CPU.startwait;
				$("#maxwait").text(maxwait);
			}
			/* assign next process from policy and start it */
			if (CPUlast == undefined || CPU != CPUlast) {
				cswitch++;
				$("#cswitch").text(cswitch);
				csw = true;
			}
			CPU.start();
			CPUlast = CPU;
			logger.log("(" + time + ") " + "process " + CPU.pid + " is now running");
		} else {
			logger.log("(" + time + ") " + "CPU is idle");
		}
	}

	/* add a new column to history table for states of processes */
	history_add(plist, csw);

	/* if simulation is in play mode adjust timer for next tick */
	if (interval > 0)
		timer = setTimeout(function () { 
				sched_tick(policy,plist);}, interval);
}

/* Function: sched_arrivals(policy, plist)
   add process to arrival later to policy

   Parameters:
   policy - policy object to give policy decisions
   plist - list of processes
*/
function sched_arrivals(policy, plist) {
	for (var i in plist) {
		/* insert depending on the arrives member of process input */
		if (plist[i].state == "NEW" && plist[i].arrives <= time) {
			plist[i].state = "READY";
			plist[i].startwait = time;
			policy.procready(plist[i]);
			logger.log("(" + time + ") " + "process " + plist[i].pid +
				" has arrived");
		}
			
	}
}

/* Function: sched_load()
   First time initialization of controller
*/
function sched_load() {
	/* click handlers of simulation +/- buttons */
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
	/* Policy selector initialization */
	var plist = ["FCFS", "FCFS (switch on sleep)", "Round Robin q=3", "Round Robin q=5",
		"Shortest Job First", "Shortest Remaining Time First",
		"Priority FIFO", "Priority RR q=3"];
	/* on click of menu items, proper policy will be created and returned */
	var pevlist = [function () { return FCFS(); },
		       function () { return FIFO(); },
		       function () { return RR(3); },
		       function () { return RR(5); },
		       function () { return SJF(); },
		       function () { return SRTF(); },
		       function () { return Priority(3); },
		       function () { return PriorityRR(3,3); }];
	for (var i in plist) {
		$("#policylist").append("<li><a href=\"#\">" +
			plist[i] + "</a></li>");
	}
	/* menu select event */
	$("#menu").menu().on("menuselect", function (ev, ui) {
		sched_cleanup();
		var policy = pevlist[ui.item.index()]();
		sched_init(policy);
	});
	/* hide process input editor */
	$("#processlist thead").on("click", function () { 
				$(this).parent().find("tbody").fadeToggle();
			});
}


/* Function: sched_cleanup()
   Reset all variables and clear all HTML elements
*/
function sched_cleanup() {
		CPU = undefined;
		CPUlast = undefined;
		time = 0;
		waitcount = 0;
		maxwait = 0;
		cswitch = 0;
		$("#time").text(time);
		$("#totalwait").text(waitcount);
		$("#cswitch").text(cswitch);
		$("#maxwait").text(maxwait);
		$("#logger").empty();
		$("#policy").empty();
		$("#policy").css("height","70px");
		$(".process").detach();
		$("#playpause").off("click");
		$("#singlestep").off("click");
		$("#restart").off("click");
		/* empty sleep queue */
		while (sleepqueue.pop()) {
		}	
}

/* Function: sched_init(policy)
   Initialize the given policy with a new process list and start simulation

   Parameters:
   policy - policy object
*/
function sched_init(policy) {
	var processlist = loadprocessview();
	/* clean policy */
	policy.reset();
	/* start in single step mode (negative interval)*/
	interval = -Math.abs(interval);

	/* policy text on view */
	$("#policytitle").text(policy.policy);

	/* free CPU */
	CPU = undefined;

	/* construct process list, list of Process objects */
	var plist = processlist();

	/* simulation button events handler */
	$("#playpause").button("option","icons",
				{primary: "ui-icon-play"})
		       .click(function () {
		interval *= -1;
		if (interval > 0) {
			$("#playpause").button("option","icons",
				{primary: "ui-icon-pause"});
			$("#singlestep").button("disable");
			sched_tick(policy,plist);
		} else {
			$("#playpause").button("option","icons",
				{primary: "ui-icon-play"});
			$("#singlestep").button("enable");
		}
	});
	$("#singlestep").click(function () {
		sched_tick(policy, plist);
	});
	$("#restart").click(function () {
		sched_cleanup();
		sched_init(policy);
	});

	/* create views for each process on a hidden div. */
	for (var p in plist) {
		$("#newprocs").append(plist[p].viewtext);
		plist[p].settxt(plist[p].burst());
	}

	/* enable all buttons */
	$("button").button("enable");

	/* initialize history view */
	history_init(plist);

	/* start schedule simulation */
	sched_start(policy,plist);
}

/* Function: sched_start(policy, plist)
   First time start of a schedule simulation, do tasks until the first tick 

   Parameters:
   policy - policy object
   plist - list of process objects
*/
function sched_start(policy,plist) 
{
	sched_arrivals(policy, plist);
	/* get first process and activate on CPU */
	if (CPU == undefined) {
		CPU = policy.getready();
		if (CPU) {
			var element = $("#proc_" + CPU.pid );
			animatemove(element, $("#cpu"));
			$("#cpu").append(element);
			CPU.start();
			CPUlast = CPU;
			logger.log("(" + time + ") " + "process " + CPU.pid + " is now running");
		}
	}

	/* add first column to history view */
	history_add(plist,true);

	/* start timer if in play mode */
	if (interval > 0)
		timer = setTimeout(function () { sched_tick(policy,plist);}, 
				interval);
}

/* Function: history_init(plist)
   Initialize history view

   Parameters:
   plist - list of process objects in simulation
*/
function history_init(plist)
{
        $("#history tbody").empty();
        $("#history thead").empty().append("<th>t</th>");
        for (var i in plist) {
                $("#history tbody").append("<tr id=\"hrow_" +
			plist[i].pid + 
			"\"><td style=\"background-color: #e0e0e0\"><b>" + 
			plist[i].pid + "</b></td></tr");
        }

}

		
/* Function: history_add(plist, csw)
   Add a new column to history view

   Parameters:
   plist - list of process objects in simulation
   csw - boolean indicating if a context switch occured in this clock tick
*/
function history_add(plist, csw)
{
        var txt;
        var his = $("#history");
	var tmap = { READY: "W", RUNNING: "R", TERM: " ", SLEEP: "S", NEW: "."};
	var smap = { RUNNING: "running", SLEEP: "sleep"};
	var ctxt;

        his.find("thead").append("<th>" + time + "</th>");
	if (csw)
		ctxt = " style=\"border-left-style: double; border-left-width: 5px\"";
	else
		ctxt = "";
        for (var i in plist) {
		var p = plist[i];
		var stxt;
		txt = tmap[p.state];
		if (smap[p.state]) 
			stxt = " class=\"" + smap[p.state] + "\"";
		else
			stxt = "";
		if (txt == undefined)
			txt = "?";
                his.find("#hrow_" + p.pid).append("<td" + stxt + ctxt + ">" + txt + 
					"</td>");
        }
        //his.find("tbody tr:last").append("<td style=\"border:0px\">" + ap + "</td>");
}

/* Function: setprocessview(processes)
   Setup process list editor in the view. Default values are put into
   view and user events are adjusted.

   Parameters:
   processes - Default list of proces data given in HTML file during page load
*/
function setprocessview(processes) {
	/* Function: spsequence(sq)
	   convert a list of times into styled sleep and execude code in HTML */
	function spsequence(sq) {
		var seq = [];
		var ex = true;
		var cls;
		for (var i in sq) {
			cls = (ex) ?  "ex"  : "sl";
			seq.push("<span class=\"" + cls + "\">" +
					sq[i] + "</span>");
			ex = ! ex;
		}
		return seq.join("");
	}
	function sqvalidate(sqinp) {
		var vals = sqinp.split(";");
		var res = [];
		for (var i in vals) {
			if (parseInt(vals[i]) > 0)
				res.push(parseInt(vals[i]));
		}
		if (res.length % 2 == 0)  // last op is sleep, add 1 at the end
			res.push(1);
		return res;
	}
	/* Function: prrow(sq)
	   return a table row for each process entry */
	function prrow(name, ops, pri, arr) {
		var up = "<tr class=\"prentry\">";
		up = up + "<td class=\"prnam\">" + name + "</td>";
		up = up + "<td class=\"prseq\"><div id=\"pr_" + name + "\">" + spsequence(ops) + 
			"</div><input name=\"pr_" + name +
			"_seq\" value=\""+ ops.join(";") + "\" style=\"position: absolute\"></td>";
		up = up + "<td class=\"prpri\"><input id=\"pr_" +
			 + name + "_pri\" value=\"" + pri +"\"></td>";
		up = up + "<td class=\"prarr\"><input id=\"pr_" +
			 + name + "_arr\" value=\"" + arr +"\"></td>";
		up = up + "<td><button class=\"prdel\"/></td>";
		return up;
	} 
	/* Function: setupevents(plel)
	   update widgets and event handlers for user events in process input view */
	function setupevents(plel) {
		plel.find(".prpri input").spinner({min: 1, max: 9});
		plel.find(".prarr input").spinner({min: 0, max: 30});
		plel.find("input[name$=_seq]").hide();
		plel.find(".prdel").on("click", function (ev) {
			$(this).parent().parent().remove();
		});
		plel.find(".prseq div").on("click",function (ev) { 
			var tdel = $(this);
			var inel = "input[name=" + this.id + "_seq]" ;
			var oldoff = $(inel).offset();
			$(inel).show()
			       .offset(tdel.offset())
			       .width(tdel.width())
			       .height(tdel.height())
			       .focus()
			       .on("keypress", function (ev) {
					if (ev.which == 13)
						$(this).trigger("enterpress");
				})
			       .on("blur enterpress", function(ev) {
				tdel.html(spsequence(sqvalidate(this.value)));
				$(inel).hide()
				       .off("change");
			       });
		}); 
		plel.find("button.prdel").width(20);
		plel.find("button.prdel").button({icons:{primary: "ui-icon-minusthick"},
						  text:false})
					 .width(20);
	}

	$("#processlist tbody").empty();
	var plel = $("#processlist");
	for (var i in processes) {
		var name = processes[i][0];
		var ops = processes[i][1];
		var pri = processes[i][2];
		var arr = processes[i][3];
		plel.append(prrow(name, ops, pri, arr));
	}
	plel.append('<tr id="pr_new_row"><td><input name="pr_new_name"/></td><td><input name="pr_new_sq"/></td><td colspan="3" align="right"><button id="pr_new_add"></td></tr>');
	plel.find("input[name=pr_new_name]").width(10);
	plel.find("input[name=pr_new_sq]").width(60)
				       .on("keypress", function (ev) {
						if (ev.which == 13)
							$("#pr_new_add").trigger("click");
					})
	plel.find("#pr_new_add").width(20)
				.button({icons: {primary: "ui-icon-plusthick"},
					 text: false})
				.width(20)
				.on("click", function (ev) {
					plel.find("#pr_new_row").before(prrow(
					plel.find("input[name=pr_new_name]").val(),
						sqvalidate(plel.find("input[name=pr_new_sq]").val()),
					1,0));
					plel.find("input[name=pr_new_name]").val("");
					plel.find("input[name=pr_new_sq]").val("");
					setupevents(plel);
					plel.find("input[name=pr_new_name]").focus();
				});
	setupevents(plel);
//	plel.append('<tr id="pr_submit_row"><td colspan="5" align="right"><button id="pr_submit"/></td></tr>');
}

/* Function: loadprocessview()
   Load process list editor view and return a process list constructor. Current
   editted status of editor form is parsed and returned as a function returning
   the <Process> list.

   Returns:
   A contructor function that will return list of <Process> objects that is
   going to be use an input to the simulation, <sched_init> specifically
*/
function loadprocessview() {

	var prlist = [];
	$("#processlist .prentry").each(function (index) {
		var t = [];
		t.push($(this).find(".prnam").text());
		t.push($(this).find(".prseq input").val().split(";"));
		t.push($(this).find(".prpri input").val());
		t.push($(this).find(".prarr input").val());
		prlist.push(t);
	});
	processes = prlist;
	return function() {
		var t = [];
		for (var i in processes) {
			var name = processes[i][0];
			var ops = processes[i][1];
			var pri = processes[i][2];
			var arr = processes[i][3];
			/* construct processes and push */
			t.push(Process(name, ops, pri, arr));
		}
		return t;
	}
}
