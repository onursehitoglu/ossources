/* File: sched.js
   Class and objects defining process behaviour and scheduling policies
*/

/* Function: animatemove(element, newpos)
   Animated move of element from one div to other
*/
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


/* Class: Process
   Class to implement each process.

   Constructor: Process(id, seq, pri, arr)

   Parameters:
   id - a string denoting id of the process 
   seq - semicoloumn seperated list of execute and sleep times ex;sleep;ex;...
   pri - priority used in some policies
   arr - arrival time of process in ticks from start of the simulation
*/
Process = function (id, seq, pri, arr) {
	var viewtext = "<table id=\"proc_" + id + "\" class=\"process\"><tr><td>"+
		id + "</td></tr><tr><td class=\"time\">" +
		seq[0] + "</td></tr></table>";
	if (arr == undefined)
		arr = 0;
	return {  pid: id,
		  viewtext: viewtext,
		  state: "NEW",
		  pri: parseInt(pri),
		  startwait: time,
		  sequence: seq,
		  arrives: parseInt(arr),
		  tick: function () {
			if (this.state == "RUNNING") {
				this.running += 1;
				this.sequence[0] -= 1;
				$("#proc_" + id + " .time").text(this.burst());
				if (this.sequence[0] == 0) {
					this.sequence.shift();
					delete this.running;
					if (this.sequence.length == 0) {
						this.state = "TERM";
						return this;
					} else {
						this.state = "SLEEP";
						this.duration = this.sequence.shift();
						return this;
					}
				} else {
					return this;
				}
			} else 
				return this;
		  },
		  wakeup: function() {
			if (this.state == "SLEEP") {
				delete this.duration;
				this.state = "READY";
			} 
			return this;
		  },
		  burst: function() {
			if (this.state == "READY" ||
				this.state == "RUNNING") {
				return parseInt(this.sequence[0]);
			}
		  },
		 settxt: function(txt) {
			$("#proc_" + id + " .time").text(txt);
		 },
		 start: function() {
				this.state = "RUNNING";
				this.running = 0;
			 }
		};
};

	

/* Class: sleepqueue
   Object to implement sleepqueue. Processes are inserted in sorted time
   diferrences. i.e. process:sleep time pairs A:5, B:2, C:7 are stored
   as:   B:2, A:3, C:2.
*/
sleepqueue =		// a time difference queue for waiting threads
	 { procs: [],
	       next: function () {
			if (this.procs.length == 0) {
				return undefined;
			}
			return this.procs[0];
		},
		pop: function () {
			this.procs.shift();
		},
/* Function: insert(proc, time)
   insert process <proc> to sleep for <time> ticks
   inserted in sorted. sleep time is accumulated sum of all
   values from first element to the element.
   It also updates HTML model of #sleepqueue element
  
   Parameters:
   proc - Process object to sleep
   time - Sleep amount
*/
		insert: function (p, t) {
			var procp = $("#proc_" + p.pid);
			animatemove(procp, $("#sleepqueue"));
			if (this.procs.length == 0) { // empty queue case
				$("#sleepqueue").append(procp);
				p.settxt("+"+t);
				this.procs.push({proc: p, time: parseInt(t)})
				return;
			}
			for (var i in this.procs) {
				if (this.procs[i].time > t) {
					/* position found, insert here */
					/* position in the model */
					var np = $("#sleepqueue .process:eq(" + 
							i + ")");
					/* set relative sleep time */
					this.procs[i].time -= t;
					/* update model */
					this.procs[i].proc.settxt("+"+
						this.procs[i].time);
					np.before(procp);
					p.settxt("+"+t);
					/* insert in the procs list */
					this.procs.splice(i,0,{time:t, proc:p});
					return;
				} else {
					/* subtract timer and move to next element*/
					t -= this.procs[i].time;
				}
			}
			/* if did not return yet. insert as the last element */
			$("#sleepqueue").append(procp);
			p.settxt("+"+t);
			this.procs.push({time:t, proc:p});
		},
/* Function: tick()
   implements a time tick on sleep queue. All processes should
   spend a tick from their sleep times. Sorted time differences
   implementation makes it easier, decrementing first
   element is sufficient. 

   Return:
   A list of <Process> objects that completed their sleep times.
   More than one process might wake-up.
*/
		tick: function () {
		 	if (this.procs.length == 0)
				return undefined;
			/* decrement timer of first element */
			this.procs[0].time -= 1;
			this.procs[0].proc.settxt("+"+this.procs[0].time);
			/* if time gets zero there is at least one proc to wake up */
			if (this.procs[0].time == 0) {
				var wulist = [];
				/* wake up all elements with (relative) timer 0 */
				while (this.procs.length > 0 && this.procs[0].time == 0) {
					wulist.push(this.procs.shift());
				}
				return wulist;
			}
			return undefined;
		},
/* Function: empty()
   Test if sleep queue is empty
*/
		empty: function () {
			return this.procs.length == 0;
		}
	     };
				
/* Class: FIFO
   First Come First Served scheduling class implementation */
FIFO = function() {
	return { policy: "First Come First Served",
	 waiting: 0,
/* Variable: ready
   The queue of ready processes */
	 ready: [],
/* Function: reset()
   reset policy, delete al processes */
	 reset: function () {
		this.ready = [];
		this.waiting=0;
		$("#policy").empty();
	 },
/* Function: procready(proc)
   insert ready process to policy
   FIFO insertion is done at the end of the list

   Parameters:
   proc - <Process> object to insert */
	 procready: function (proc) {
		this.waiting ++;
		$("#policy").append($("#proc_" + proc.pid));
		this.ready.push(proc);
	 },
/* Function: getready()
   get next ready process from policy
   FIFO gives the first element in the list (front of the queue)
*/
	 getready: function () {
		if (this.ready.length > 0) {
			this.waiting --;
			return this.ready.shift();
		} else 
			return undefined;
	 },
/* Function: preempt(proc)
   Test if process needs to be preemptted
   FIFO always return false
*/
	 preempt: function (proc) {
		return false;	// non-preemptive policy
	 },
/* Function: empty()
   Test if policy has ready process to schedule
*/
	 empty: function () {
		return this.ready.length == 0;
	 }
	}
	}

FCFS = function () {
	var policy = FIFO();
	policy.defready = policy.getready;
	policy.last = undefined;
	policy.preempt = function (proc) {
		policy.last = proc;
		return false;  // non-preemptive just get last process
	}
	policy.getready = function () {
		if (policy.last) 
			if (policy.last.state == 'SLEEP')
				return undefined
			else if (policy.last.state == 'READY')
				return policy.last;

		policy.last = undefined;
		return policy.defready();
	}
	return policy;
}
	
	

/* Class: RR(quantum)
   Round Robin policy implementation. Inherits <FIFO>

   Parameters:
   quantum - number of ticks for each quantum
*/
RR = function(q) {
	var policy = FIFO();
	policy.policy = "Round Robin (" + q + ")";
	policy.quantum = q,
/* Function: preempt(proc)
   Test if process needs to be preemptted
   RR preempts if process is running for quantum ticks
   Otherwise same as <FIFO>
*/
	policy.preempt = function (proc) {
		if (proc.running >= q) {
			if (this.ready.length == 0)
				return false
			else
				return true;
		}
		return false;	// non-preemptive policy
	 };
	return policy;
	};

/* Class: SJF
   Shortest Job First policy implementation
*/
SJF = function() {
	return { policy: "Shortest Job First",
	 waiting: 0,
/* Variable: ready
   list of ready processes sorted in increasing burst times */
	 ready: [],
	 reset: function () {
		this.ready = [];
		this.waiting=0;
		$("#policy").empty();
	 },
/* Function: procready(proc)
   insert ready process to policy
   SJF inserts into sorted list on burst times

   Parameters:
   proc - <Process> object to insert */
	 procready: function (proc) {
		this.waiting ++;
		if (this.ready.length == 0) {
			/* insert into empty list */
			$("#policy").append($("#proc_" + proc.pid));
			this.ready.push(proc);
			return;
		}
		var burst = proc.burst();
		for (var i in this.ready) {
			if (this.ready[i].burst() > burst) {
				/* insert position found */
				$("#policy .process:eq(" + i + ")")
				  .before($("#proc_" + proc.pid));
				this.ready.splice(i,0,proc);
				return;
			}
		}
		$("#policy").append($("#proc_" + proc.pid));
		this.ready.push(proc); // insert into last position
	 },
/* Function: getready()
   get next ready process from policy
   returns first process in the <ready> list which has minimum burst
*/
	 getready: function () {
		if (this.ready.length > 0) {
			this.waiting --;
			return this.ready.shift();
		} else 
			return undefined;
	 },
/* Function: preempt(proc)
   Test if process needs to be preemptted
   always returns false
*/
	 preempt: function (proc) {
		return false;	// non-preemptive policy
	 },
/* Function: empty()
   Test if policy has ready process to schedule
*/
	 empty: function () {
		return this.ready.length == 0;
	 }
	}
	};

/* Class: SRTF
   Shortest Remaining Task First policy implementation. inherits <SJF>
*/
SRTF = function() {
	 // same as SJF but preemptive
	 var policy = SJF();
	 policy.policy = "Shortest Remaining Time";
/* Function: preempt(proc)
   Test if process needs to be preemptted
   SRTF preempts if process has larger burst than current minimum in the policy
   Otherwise same as <SJF>
*/
	 policy.preempt = function (proc) {
		if (this.ready.length == 0) {
			return false;	
		} else {
			return this.ready[0].burst() < proc.burst();
		}
	 };
	 return policy;
	};

/* Class: Priority
   implementation of multi-level fixed priority based scheduler. No
   quantum. Preempts only a higher priority process becomes available

   Constructor: Priority(numpri)
   Constructor
 
   numpri - number of priority levels from 1 to numpri
*/
Priority = function(numpri) {
	function initready() {
		var ready = [];
		for (var i=1; i <= numpri ; i++)  {
			ready[i] = [];
		}
		return ready;
	}
	return { policy: "Multilevel Priority [1-" + numpri +"]",
	 waiting: 0,
/* Variable: ready
   list of queues, a queue in each priority level */
	 ready: initready(),
	 reset: function () {
		this.waiting=0;
		this.ready = initready();
		$("#policy").empty();
		$("#policy").append("<table></table>");
		for (var i=1; i <= numpri ; i++)  {
			$("#policy table").append("<tr><td>" + i + 
				"</td><td id=\"policy_" + i + "\"></td></tr>");
		}
		$("#policy").css("height",numpri * 70 + "px");
		$("#policy tr").css("height","70px");
	 },
/* Function: procready(proc)
   insert ready process to policy
   Insertion is made at the end of the priority queue of the process
   based on its priority.

   Parameters:
   proc - <Process> object to insert */
	 procready: function (proc) {
		var pri = (proc.pri) ? proc.pri : numpri; // default: last
		$("#policy_" + pri).append($("#proc_" + proc.pid));
		this.waiting ++;
		this.ready[pri].push(proc);
	 },
/* Function: getready()
   get next ready process from policy
   returns first process in the highest non-empty priority queue
*/
	 getready: function () {
		for (var pri = 1; pri <= numpri; pri++) {
			if (this.ready[pri].length > 0) {
				this.waiting --;
				return this.ready[pri].shift();
			}
		}
		return undefined;
	 },
/* Function: preempt(proc)
   Test if process needs to be preemptted. Preempts if there is a
   higher priority process.
*/
	 preempt: function (proc) {
		var ppri = (proc.pri) ? proc.pri : numpri; // default: last
		/* test all process from 1 to priority level of the process */
		for (var pri = 1; pri < ppri; pri++) {
			if (this.ready[pri].length > 0)
				return true;
		}
		return false;	
	 },
/* Function: empty()
   Test if policy has ready process to schedule
*/
	 empty: function () {
		for (var pri = 1; pri <= numpri; pri++) {
			if (this.ready[pri].length > 0)
				return false;
		}
		return true;
	 }
	}};

/* Class: PriorityRR
   implementation of multi-level fixed priority based scheduler with
   quantum. Inherits <Priority>. Premption is different. Also prempts if  
   there is a same level process and quantum expired.

   Constructor: PriorityRR(numpri, q)
   Constructor
 
   numpri - number of priority levels from 1 to numpri
   q - quantum
*/
PriorityRR = function (numpri, q) {
	var policy = Priority(numpri);
	policy.policy = "Multilevel RR [1-" + numpri +
			"] with quantum=" + q;
/* Function: preempt(proc)
   Test if process needs to be preemptted
   Preempts if there is a higher priority process exists or
   if same priority process exists and quantum is expired.
*/
	policy.preempt = function (proc) {
		var ppri = (proc.pri) ? proc.pri : numpri; // default: last
		for (var pri = 1; pri < ppri; pri++) {
			/* a higher level process exists */
			if (this.ready[pri].length > 0)
				return true;
		}
		/* same level not empty and quantum expired */
		if (proc.running >= q && this.ready[pri].length > 0) 
			return true;
		else
			return false;	

	}
	return policy;
};

