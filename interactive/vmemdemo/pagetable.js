function setupVM(vmdesc) {

    var graph = new joint.dia.Graph;
	var dims =  { width: $(window).width()-20, height: ($(window).height() < 800) ? 800 : $(window).height()-20};
    var paper = new joint.dia.Paper({ el: $('#paper-html-elements'), 
					width: dims.width, height: dims.height, gridSize: 1, model: graph });

	frames=[]
	frames.allocated = []
	for (var i=0, s = 0 ; s < dims.height - 50; i++, s += 30) {
		frames.push(new joint.shapes.basic.Rect({position:{x: $(window).width() - 60, y:50+i*20}, size: { width:30, height: 20}}));
		frames.allocated.push(false);
	}

	frames.allocate = function() {
		for (var i in frames.allocated) {
			if (!frames.allocated[i]) {
				frames.allocated[i] = true;
				frames[i].attr("rect/fill","#ffe0e0");
				return frames[i];
			}
		}
		return undefined;
	}


	conn = new connections();

	var ylevels = Array(10).fill(0);

    // Create a custom element.
    // ------------------------

    joint.shapes.pagetable = {};
    joint.shapes.pagetable.Element = joint.shapes.basic.Rect.extend({
        defaults: joint.util.deepSupplement({
            type: 'pagetable.Element',
            attrs: {
                rect: { stroke: 'none', 'fill-opacity': 0 }
            }
        }, joint.shapes.basic.Rect.prototype.defaults),
    });

    // Create a custom view for that element that displays an HTML div above it.
    // -------------------------------------------------------------------------

    joint.shapes.pagetable.ElementView = joint.dia.ElementView.extend({

        template: [
			'<table class="html-pagetable">',
			'<thead><tr><th>index</th><th>PTE</th></tr></thead>',
			'<tbody></tbody>',
			'</table>'
        ].join(''),

        initialize: function() {
            _.bindAll(this, 'updateBox');
            joint.dia.ElementView.prototype.initialize.apply(this, arguments);

            this.$box = $(_.template(this.template)());
            // Prevent paper from handling pointerdown.
            this.$box.find('input,select').on('mousedown click', function(evt) {
                evt.stopPropagation();
            });
            // This is an example of reacting on the input change and storing the input data in the cell model.
            this.$box.find('input').on('change', _.bind(function(evt) {
                this.model.set('input', $(evt.target).val());
            }, this));
            this.$box.find('select').on('change', _.bind(function(evt) {
                this.model.set('select', $(evt.target).val());
            }, this));
            this.$box.find('select').val(this.model.get('select'));
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
            // Set the position and dimension of the box so that it covers the JointJS element.
            var bbox = this.model.getBBox();
            // Example of updating the HTML with a data stored in the cell model.
            this.$box.css({
                width: bbox.width,
                height: bbox.height,
                left: bbox.x,
                top: bbox.y,
                transform: 'rotate(' + (this.model.get('angle') || 0) + 'deg)'
            });
			var tableentries = this.$box.find('tbody');
			var prev = undefined;
			tableentries.html("")
			if (this.model.entries == undefined || this.model.entries[0] == undefined) {
					tableentries.append('<tr class="invalidentry"><td></td><td></td></tr>');
			}
			prev = -1;
			for (var addr in this.model.entries) {
				if (prev == undefined || addr-prev > 1) {
					tableentries.append('<tr class="invalidentry"><td></td><td></td></tr>');
				}
				tableentries.append('<tr class="validentry"><td id="idx_' + this.model.pageid + '_' +addr +'">'  + 
									addr + "(" + parseInt(addr).toString(16) + ")" + 
									'</td><td id="out_' + this.model.pageid + '_' + addr + '"></td></tr>');
				prev = addr;
			}
			for (var addr in this.model.ports) {
				var t = tableentries.find("#out_"+this.model.pageid + '_'+addr);
				var p = $('#paper-html-elements').offset();
				this.model.ports[addr].position( t.offset().left - p.left + t.width()/2,  
							t.offset().top + t.height()/2 - p.top );
			}
        },
        removeBox: function(evt) {
            this.$box.remove();
        }
    });

	function connections() {
		this.edges = {};
		this.frompage = function (srcpage, idx) {
			var source = $("#out_" + srcpage.pageid + '_' + idx );
			var t = source.offset();
        	var ptr1 = new joint.shapes.standard.Circle({ position: { x: t.left + source.width()-25, y: t.top-2 },
                             size: { width: 5, height: 5}});
        	ptr1.attr('body/fill','black');
			srcpage.ports[idx] = ptr1;
        	srcpage.embed(ptr1);

		/*
			source = $("#idx_" + srcpage.pageid + '_' + idx );
			t = source.offset();
        	var ptr2 = new joint.shapes.standard.Circle({ position: { x: t.left, y: t.top },
                             size: { width: 2, height: 2}});
			srcpage.inports[idx] = ptr2;
        	srcpage.embed(ptr2);
		*/

			var dest = srcpage.entries[idx];
        	var pl = new joint.dia.Link({
            	source: { id: ptr1.id },
            	target: { id: srcpage.entries[idx].id, anchor: { name: 
					(srcpage.entries[idx].attributes.type == 'basic.Rect') ? 'left' : 'topLeft', 
						   args: { dx:-10 } }},
            	router: { name: 'manhattan',
                      	args: {
                      	padding: 25,
                      	perpendicular: true,
                      	startDirections: ['right'],
                      	endDirections:['left']}},
            	attrs: { '.connection': { 'stroke-width': 2, stroke: '#34495E' },
                	'.marker-target': { fill: '#34495E', d: 'M 10 0 L 0 5 L 10 10 z' },}
        	});
        	pl.toFront();
        	//graph.addCells([ptr1,ptr2,pl]);
        	graph.addCells([ptr1,pl]);
        	//this.edges[srcpage.pageid + '_' + addr ] = { el: s2, ptr : ptr1, lel: pl};
		}
	};

	class PageTable {
		constructor (id, x, y) {
			this.el = new joint.shapes.pagetable.Element({
        		position: { x: x, y: y },
        		size: { width: 100, height: 100 },
    		});
			this.el.pageid = id;
			graph.addCell(this.el);
			this.el.entries = {};
			this.el.ports = {};
			//this.el.inports = {};
			this.el.pg = this;
		}
		set(idx, obj) {
			this.el.entries[idx] = obj;
			this.el.findView(paper).updateBox();
			conn.frompage(this.el, idx);
		}
	};

	class VirtualAddress {
		constructor (levels, x = 10, y = 20) {
			const k = 15;
			this.el = [];
			this.txt = [];
			this.levels = []
			this.links = []
			this.n = levels.length;
			var sum = 0;
			this.group = new joint.shapes.standard.Rectangle();
			for (var l of levels) {
				this.levels.push(l);
				var c = new joint.shapes.standard.Rectangle();
				var t = new joint.shapes.standard.Rectangle();
        		c.position(sum*k+x, y);
        		t.position(sum*k+x, y-23);
				t.attr("label/text",l);
				t.attr("body/stroke-width",0);
        		c.resize(k*l, 30);
        		t.resize(k*l, 30);
				t.addTo(graph);
				c.addTo(graph);
				this.group.embed(t);
				this.group.embed(c);
				sum += l;
				this.el.push(c);
				this.txt.push(t);
			}
			this.sum = sum;
			this.group.attr("body/stroke-width",0);
			this.group.attr("body/fill","transparent");
			this.group.position(x,y-25);
        	this.group.resize(sum*k, 55);
			this.group.addTo(graph);
        };


		access(address) {
			function mask(bits) {
				return ~((~0)<<bits);
			}
			function traverseandlink(as, comps, rootel, lev) {
				var addr = comps[0];
				var rest = comps.slice(1);
				var target;
				if (rootel.entries[addr] == undefined) {
					if (rest.length <= 1) {
						target = frames.allocate();
					} else {
					 	target = ( new PageTable((lev +"") + addr, lev * 250, ylevels[lev]*120+100 )).el;
						ylevels[lev]++;
					}
					rootel.pg.set(addr, target);
				} else {
					target = rootel.entries[addr];
				}
				$("#idx_" + rootel.pageid + '_' + addr ).css("background-color","#c0ffc0");
				
				if (rest.length > 1) {
				/*
					if (as.links[lev-1] == undefined) {
        				as.links[lev-1] = new joint.dia.Link({
            					source: { id: as.el[lev-1].id },
            					target: { id: rootel.inports[addr].id, anchor: { name: 'left', args: { dx:-10 } }},
            					router: { name: 'manhattan',
                      					args: {
                      					padding: 25,
                      					perpendicular: true,
                      					startDirections: ['down'],
                      					endDirections:['left']}},
            					attrs: { '.connection': { 'stroke-width': 2, stroke: '#34495E' },
                				'.marker-target': { fill: '#34495E', d: 'M 10 0 L 0 5 L 10 10 z' }}
        						})));
        				as.links[lev-1].toFront();
        				as.links[lev-1].addTo(graph);
					} else {
						as.links[lev-1].target(target); 
					}
				*/
					traverseandlink(as, rest, target, lev+1);
				}
			}
			var tot = this.sum;
			var comps = [];
			for (var i in this.levels) {
				let component = (BigInt(address) >> BigInt(tot - this.levels[i])) & BigInt(mask(this.levels[i]));
				comps.push(parseInt(component.toString()));
				tot -= this.levels[i];
				this.el[i].attr("label/text", comps[i] + "(" + comps[i].toString(16) +")" );
			}
			$(".validentry td:first-child").css("background-color","#ffffff");
			traverseandlink(this,comps, topdir.el, 1);
		}

	};




    // Create JointJS elements and add them to the graph as usual.
    // -----------------------------------------------------------

	va = new VirtualAddress(vmdesc.split(";").map(s => { return parseInt(s)}));

	var topdir = new PageTable(1, 20, 80);

    //graph.addCells([el1, el2, l]);
	graph.addCells(frames);
}
