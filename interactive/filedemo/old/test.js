'use strict';
joint.setTheme('material');
var graph = new joint.dia.Graph;

var paper = new joint.dia.Paper({
    el: document.getElementById('paper'),
    model: graph,
    width: '100%',
    height: '100%',
    gridSize: 10,
    background: { color: '#f6f6f6' },
    magnetThreshold: 'onleave',
    moveThreshold: 5,
    clickThreshold: 5,
    linkPinning: false,
    interactive: {
        linkMove: false,
        elementMove: false
    },
    markAvailable: true,
    snapLinks: { radius: 40 },
    defaultRouter: {
        name: 'mapping',
        args: { padding: 30 }
    },
    defaultConnectionPoint: { name: 'anchor' },
    defaultAnchor: { name: 'mapping' },
    defaultConnector: {
        name: 'jumpover',
        args: { jump: 'cubic' }
    },
    highlighting: {
        magnetAvailability: {
            name: 'addClass',
            options: {
                className: 'record-item-available'
            }
        },
        connecting: {
            name: 'stroke',
            options: {
                padding: 8,
                attrs: {
                    'stroke': 'none',
                    'fill': '#7c68fc',
                    'fill-opacity': 0.2
                }
            }
        }
    },
    defaultLink: function() {
        return new joint.shapes.mapping.Link();
    },
    validateConnection: function(sv, sm, tv, tm, end) {
        if (sv === tv) return false;
        if (sv.model.isLink() || tv.model.isLink()) return false;
        if (end === 'target') return tv.model.getItemSide(tv.findAttribute('item-id', tm)) !== 'right';
        return sv.model.getItemSide(sv.findAttribute('item-id', sm)) !== 'left';
    }
});

(function(joint, graph) {
    var order = new joint.shapes.mapping.Record({
        items: [{
            id: 'file',
            label: 'File: (default)',
            icon: 'images/file.svg',
            highlighted: true}]});
    order.setName('Order');
    order.position(780,200);
    order.addTo(graph);
 })(joint, window.graph);


