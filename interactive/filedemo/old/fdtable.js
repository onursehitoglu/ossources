var graph = new joint.dia.Graph;
var paper = new joint.dia.Paper({ el: $('.paper'), width: 2048, height: 2048, gridSize: 1, model: graph });

// Create a custom element.
// ------------------------

joint.shapes.html = {};
joint.shapes.html.Element = joint.shapes.basic.Rect.extend({
    defaults: joint.util.deepSupplement({
        type: 'html.Element',
        attrs: {
            rect: { stroke: 'none', 'fill-opacity': 0 }
        }
    }, joint.shapes.basic.Rect.prototype.defaults),
    getField: function(k) {
	return this.get('fields')[k];
    },
    setField: function(k,v) {
	this.get('fields')[k] = v;
    },
    delField: function(k,v) {
	delete this.get('fields')[k];  
    }
});

// Create a custom view for that element that displays an HTML div above it.
// -------------------------------------------------------------------------
function selectfile(id,vals) {
	if (! vals ) {
		vals = [0,1,2,3,4,5,6,7];
	}
	var txt='<select id="' + id + '">';
	for (var i in vals)
		txt += '<option>' + vals[i] + '</option>';
	txt += '</select>';
	return txt;
}

joint.shapes.html.ElementView = joint.dia.ElementView.extend({


    template: [
        '<div class="html-element">',
	'<div class="pmenu">',
	'<button>fork</button>',
	'<button>pipe</button>',
	'<button>exit</button><br>',
	'<button>open</button>(<input type="text" id="fname" size="5"/>)<br>',
	'<button>close</button>(' + selectfile("c1")  + ')<br>',
	'<button>dup2</button>(' + selectfile("d1") + ',' + selectfile("d2") + ')<br>',
	'<button>read</button>(' + selectfile("r1") + ',' + selectfile("r2",[1,2,3,4,5,10,15,20]) + ')<br>',
	'</div>',
        '<label></label>',
	'<table></table><br/>',
        '</div>'
    ].join(''),

    initialize: function() {
        _.bindAll(this, 'updateBox');
        joint.dia.ElementView.prototype.initialize.apply(this, arguments);

        this.$box = $(_.template(this.template)());

        // Prevent paper from handling pointerdown.
        this.$box.find('input,select').on('mousedown click', function(evt) { 
		evt.stopPropagation(); });
        // This is an example of reacting on the input change and storing the input data in the cell model.
	this.$box.find('button').on('click', _.bind(function(evt) {
		evt.stopPropagation();
		var but=$(evt.target);
		if (but.text() == "fork") {
			console.log(this.model.get('process')().fork());
		} else if (but.text() == "exit") {
			console.log(this.model.get('process')().exit());
		} else if (but.text() == "pipe") {
			console.log(this.model.get('process')().pipe());
		} else if (but.text() == "close") {
			var fd = this.$box.find("#c1").val();
			this.model.get('process')().close(fd);
		} else if (but.text() == "dup2") {
			var fdo = this.$box.find("#d1").val();
			var fdn = this.$box.find("#d2").val();
			this.model.get('process')().dup2(fdo,fdn);
		} else if (but.text() == "read") {
			var fd = this.$box.find("#r1").val();
			var num = this.$box.find("#r2").val();
			console.log(fd + ":" + num);
			this.model.get('process')().read(fd,num);
		} else if (but.text() == "open") {
			var fname = this.$box.find("#fname").val();
			this.model.get('process')().open(fname);
		}
	},this));
	/*
        this.$box.find('input').on('change', _.bind(function(evt) {
            this.model.set('input', $(evt.target).val());
        }, this));
         this.$box.find('select').on('change', _.bind(function(evt) {
           this.model.set('select', $(evt.target).val());
         }, this));
         this.$box.find('select').val(this.model.get('select'));
	*/
        this.$box.find('.delete').on('click', _.bind(this.model.remove, this.model));
        // Update the box position whenever the underlying model changes.
        this.model.on('change', this.updateBox, this);
        // Remove the box when the model gets removed from the graph.
        this.model.on('remove', this.removeBox, this);

        this.updateBox();
    },
    render: function() {
        joint.dia.ElementView.prototype.render.apply(this, arguments);
        this.paper.$el.prepend(this.$box);
        this.updateBox();
        return this;
    },
    updateBox: function() {
	function updateoptions(pr,sel, foropen) {
		var pr = pr();
		if (foropen == undefined) {
			foropen = true;
		}
		sel.find("option").each(function (idx) {
			var e = $(this);

			if (! (pr[e.val()]) ==  !(foropen) ) {
				e.removeAttr('disabled');
			} else {
				e.attr('disabled',true);
			}
		});
	}
        // Example of updating the HTML with a data stored in the cell model.
	var fields = this.model.get('fields');
	var process = this.model.get('process');
	var strid = this.model.get('strid');

	if (process) {
		this.$box.find('.pmenu').show();
		updateoptions(process, this.$box.find("#c1"));
		updateoptions(process, this.$box.find("#d1"));
		updateoptions(process, this.$box.find("#r1"));
	} else {
		this.$box.find('.pmenu').hide();
	}

        this.$box.find('label').text(this.model.get('label'))
			       .attr('id','label_' + strid);
	var ftable = this.$box.find('table');

	ftable.empty();
	for (var f in fields) {
		var fval = fields[f];
		ftable.append('<tr><td>' + f + '</td><td id="str_'+ strid +
				'_' + f + '">' +
				fval + '</td></tr>');
	}
        var bbox = this.model.getBBox();
        this.$box.css({ width: bbox.width, height: bbox.height, left: bbox.x, top: bbox.y, transform: 'rotate(' + (this.model.get('angle') || 0) + 'deg)' });
    },
    removeBox: function(evt) {
        this.$box.remove();
    }
});

function struct(id, name, pos, width, height, fields, process) {
	var obj = {};
	obj.el = new joint.shapes.html.Element({ position: pos,
		size: { width: width, height: height}, label: name,
			fields:fields,
			process: (process) ? function () { return obj;} : false,
			strid: id});
	obj.id = id;
	obj.set = function (fname, val) {
		this.el.setField(fname, val);
		}
	obj.get = function (fname, val) {
		return this.el.getField(fname, val);
	}
	obj.refresh = function () {
		for (var i in this.inlinks) {
			this.inlinks[i].remove();
		}
		//obj.el.trigger('change');
		this.el.findView(paper).updateBox();
		graph.addCells(this.inlinks);
	}
	obj.inlinks = [];
	return obj;
}


function connections() {
	this.edges = {};
	this.connect = function(s1, f, s2) {
		var grel =  s2.el  || s2;

		if (s2.type && s2.type == "pipe") {
			if (s2.inlinks[0]) {
				if (s2.inlinks[1])
					return false;
				else {
					grel = s2.outP;
				}
					
			} else {
				grel = s2.inP;
			}
		} else if (s2.inp) {
			grel = s2.inp;
		} else {
			var loff = $('#label_' + s2.id).offset();
			var grel = new joint.shapes.basic.Circle({
				position: { x: loff.left-5, y: loff.top-5},
				size: {width: 1, height: 1}});
			s2.el.embed(grel);
			graph.addCell(grel);
			s2.inp = grel;
		}

		var t = $('#str_' + s1.id + '_' + f).offset();
		var ptr1 = new joint.shapes.basic.Circle({ position: { x: t.left, y: t.top },
						     size: { width: 10, height: 10}})
		s1.el.embed(ptr1);
		graph.addCell(ptr1);
			
		var pl = new joint.dia.Link({
		    source: { id: ptr1.id },
		    target: { id: grel.id },
		    /*router: { name: 'manhattan',
					  args: {
					  startDirections: ['right'],
					  endDirections: ['left']}},*/
			/*router: { name: 'oneSide',
					  args: { side: 'left'}},*/
		    attrs: { '.connection': { 'stroke-width': 2, stroke: '#34495E' }, 
			    '.marker-target': { fill: '#34495E', d: 'M 10 0 L 0 5 L 10 10 z' },}
		});
		//pl.set('router', { name : 'manhattan'});
		pl.toFront();
		graph.addCell(pl);
		this.edges[s1.id + '_' + f ] = { el: s2, ptr : ptr1, lel: pl};
		if (s2.el || s2.type == 'pipe') { s2.inlinks.push(pl); }
	}
	this.disconnect = function (s1, f) {
		var edgeid = s1.id + '_' + f;
		var edge = this.edges[edgeid];
		if (! edge)
			return;
		if (edge.el.inlinks.indexOf(edge.lel) >= 0) {
			edge.el.inlinks.splice(edge.el.inlinks.indexOf(edge.lel),1);
		}
		edge.lel.remove();
		s1.el.unembed(edge.ptr);
		edge.ptr.remove();
		delete this.edges[edgeid];
	}
}

var conn = new connections();
var files = {}

var fdslots = [];
var fileslots = [];
var inodeslots = [];

function file_str(id,mod) {
	var sl = 0;
	if (mod == undefined)
		mod = "R";
	for (; fileslots[sl]; sl++) {
	}
	fileslots[sl] = true;
	var t =  struct(id, "file", {x: 300, y: 10+sl*130}, 150, 120, { mode: mod, nrefs: 1, offset: 0, inode: ""});
	t.slot = sl;
	t.type = "file";
	return t;
}

function inode_str(id, name) {
	var sl = 0;
	for (; inodeslots[sl]; sl++) {
	}
	inodeslots[sl] = true;
	var t =  struct(id, "inode",{x: 500, y: 10+sl*150} , 200, 125, { path: name });
	t.slot = sl;
	t.type = "inode";
	return t;
}

function pipe_str(id) {
	var obj = {};
	var sl = 0;
	for (; inodeslots[sl]; sl++) {
	}
	inodeslots[sl] = true;

	var x = 500;
	var y = 10+sl*150;
	obj.el = new joint.shapes.basic.Rect({ position: {x: x, y: y},
		size: { width: 150, height: 30},
		attrs: { rect: {fill: '#f0fff0'}, text:{ text: '<<PIPE<<' }}});

	obj.id = id;
	obj.refresh = function () {} //function() { this.el.remove(); graph.addCell(this.el);}
	obj.inP = new joint.shapes.basic.Circle({ position: { x: x+5,y: y+10 }, size: { width: 10, height: 10}});
	obj.outP = new joint.shapes.basic.Circle({ position: { x: x+135,y:y+10 }, size: { width: 10, height: 10}});
	obj.el.embed(obj.outP);
	obj.el.embed(obj.inP);
	obj.type = "pipe";
	obj.inlinks = [];
	return obj;
}

function fdtable(id) {
	var fields = {};
	for (var i = 0; i < 8; i++)
		fields[i] = "NULL";
	var sl = 0;
	for (; fdslots[sl]; sl++) {
		// find first nonempty slot
	}
	fdslots[sl] = true;
	var pos = {x:10, y: 10+sl*400};
	var obj =  struct(id, "fd Table",pos, 220, 400, fields,true);
	obj.slot = sl;
	obj.nchildren = 0;
	obj.open = function(fname,mod) {
			if (files[fname]) {
				var istr = files[fname];
			} else {
				var istrid = (id + "_" + fname).replace(/\./g,"_");
				var istr = new inode_str(istrid, fname);
				files[fname] = istr
			}
			var fd;
			for (fd = 0 ; fd < 10; fd++) {
				var empty = obj.get(fd);
				if (empty == undefined || empty == "NULL")
					break;
					
			}
			if (fd == 10)
				return false;

			var fstr = file_str(id + "_" + fd + "_file", mod);
			obj.set(fd, "");
			obj[fd] = fstr;
			graph.addCell(istr.el);
			graph.addCell(fstr.el);
			conn.connect(fstr, 'inode', istr);
			fstr.inode = istr;
			conn.connect(obj, fd, fstr);
			//obj.refresh();
			return true;
		}
	obj.close = function(fd) {
		var fstr = obj[fd];
		if ( fstr == undefined) 
			return false;
		obj.set(fd, "NULL");
		var nref = fstr.get('nrefs');
		nref = nref - 1;
		fstr.set('nrefs',nref);
		if (nref == 0) {
			fileslots[fstr.slot] = undefined;
			conn.disconnect(fstr, 'inode');
			var inode = fstr.inode;
			if (inode.type == 'inode') {
				if (inode.inlinks.length == 0) {
					inodeslots[inode.slot] = undefined;
					inode.el.remove();
					delete files[inode.get('path')];
				}
			} else if (inode.type == 'pipe') {
				if (inode.inlinks.length == 0) {
					inodeslots[inode.slot] = undefined;
					inode.el.remove();
				}
			}
			fstr.el.remove();
		} else {
			fstr.refresh();
		}
		conn.disconnect(obj, fd);
		obj[fd] = undefined;
		obj.refresh();
		return true;
	}
	obj.pipe = function() {
			var fd1,fd2;
			for (fd1 = 0 ; fd1 < 10; fd1++) {
				var empty = obj.get(fd1);
				if (empty == undefined || empty == "NULL")
					break;
					
			}
			if (fd1 == 10)
				return false;

			for (fd2 = fd1+1 ; fd2 < 10; fd2++) {
				var empty = obj.get(fd2);
				if (empty == undefined || empty == "NULL")
					break;
					
			}
			if (fd2 == 10)
				return false;

			var fstr1 = file_str(id + "_" + fd1 + "_file", "R");
			var fstr2 = file_str(id + "_" + fd2 + "_file", "W");
			obj.set(fd1, "");
			obj[fd1] = fstr1;
			obj.set(fd2, "");
			obj[fd2] = fstr2;

			var pipnode = pipe_str(id + "_" + fd1 + "_pipe");

			graph.addCells([pipnode.el,pipnode.inP, pipnode.outP]);
			graph.addCell(fstr1.el);
			graph.addCell(fstr2.el);
			conn.connect(obj, fd1, fstr1);
			conn.connect(obj, fd2, fstr2);
			conn.connect(fstr1, 'inode', pipnode);
			conn.connect(fstr2, 'inode', pipnode);
			fstr1.inode = pipnode;
			fstr2.inode = pipnode;
			//obj.refresh();
			return true;
		};
	obj.dup2 = function(fd, tfd) {
		var fstr = obj[fd];
		var tfstr = obj[tfd];

		if ( fstr == undefined) 
			return false;

		if (tfstr != undefined) {
			obj.close(tfd);
		}

		obj.set(tfd, "");
		obj[tfd] = fstr;

		var nref = fstr.get('nrefs');
		nref = nref + 1;
		fstr.set('nrefs',nref);
		conn.connect(obj, tfd, fstr);
		obj.refresh();
		fstr.refresh();
		return true;
	};
	obj.fork =  function() {
		var child =  fdtable("child_" + id + "_" + this.nchildren);
		this.nchildren++;
		for (var i=0; i < 20; i++) {
			if (obj[i]) {
				var nref = obj[i].get('nrefs');
				nref = nref + 1;
				obj[i].set('nrefs',nref);
				obj[i].refresh();
				conn.connect(child, i, obj[i]);
				child.set(i,"");
			}
			child[i] = obj[i];
		}
		child.refresh();
		return child;
	};
	obj.read = function(fd, nbytes) {
		if (this[fd]) {
			var off = Number(this[fd].get('offset'));
			off += Number(nbytes);
			this[fd].set('offset',off);
			this[fd].refresh();
		} else
			return 0;
	};
	obj.exit = function() {
		for (var i=0; i < 20; i++) {
			if (this[i]) {
				this.close(i);
			}
		}
		this.el.remove();
		fdslots[this.slot] = undefined;
		for (k in this) {
			delete this[k];
		}
	}
	graph.addCell(obj.el);
	return obj;
}



// Create JointJS elements and add them to the graph as usual.
// -----------------------------------------------------------

p1 = new fdtable("process1");
p1.open('tty0','R');
p1.open('tty0','W');
p1.dup2(1,2);
