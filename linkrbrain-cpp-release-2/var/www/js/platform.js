var datasets, defaultSettings;
requester.get('/datasets', function ok(response) {
    datasets = response.data;
    var dataset = search_dataset('functions2');
    defaultSettings.correlations.dataset = {
        id: dataset.id,
        label: dataset.label,
    };
}, function ko() {
    alert('Could not load dataset types');
});
var search_dataset = function(dataset_label) {
    for (var i=0, n=datasets.length; i<n; ++i) {
        if (dataset_label == datasets[i].label) {
            return datasets[i];
        }
    }
    throw ('Could not find dataset identifier for label ' + dataset_label);
};

defaultSettings = {
    "correlations": {
        "dataset": {
            "id": 1,
            "label": "functions"
        },
        "limit": 100
	},
    "graph": {
        "threshold": 0.05,
        "limit": 20
	},
    "view": {
        "type": "3D",
        "tags": {},
        "opacity": 0.9,
        "points": {
            "radius": 5,
            "opacity": 0.5,
            "byOpacity": true,
            "byRadius": false
		},
        "zones": {
            "centers": true,
            "surfaces": false,
            "volumes": false
		},
        "position": {
            "x": 0,
            "y": 0,
            "z": 0
		},
        "box": {
            "x": {
                "min": -90,
                "max": 90
			},
            "y": {
                "min": -126,
                "max":	90
			},
            "z": {
                "min": -72,
                "max": 108
			}
		}
	}
};
var subgroupTypes = [
    {
        title : 'Datasets',
        name : 'datasets',
        description : 'Known mappings for cognitive functions and genes',
        // description : 'Known mappings for cognitive functions, genes and areas',
        html : '<ul class="tabs"><li class="tab"></li><li class="tab"></li><li class="tab"></li></ul><ul class="tabs-select"><li data-label="functions2">Functions</li><li data-label="phenotypes">Phenotype</li><li data-label="genes">Transcriptome</li></ul>',
        initalize : function() {
            var contents = $(this);
            var tabs = contents.find('ul.tabs li');
            var tabsSelect = contents.find('ul.tabs-select li');
            //    Tabs management
            tabsSelect.each(function(i){
                var tabSelect = $(this);
                var tab = tabs.eq(i);
                var div = $('<div>').addClass('list-search').appendTo(tab);
                var input = $('<input type="text">').prependTo(div);
                var detailsDiv = $('<div>').addClass('list-details').appendTo(tab);
                var pagesUl = $('<ul>').appendTo(detailsDiv);
                var span = $('<span>').appendTo(detailsDiv);
                var table = $('<table></table>').addClass('list').appendTo(tab).uniqueId();
                var tbody = $('<tbody></tbody>').appendTo(table);
                //    Callback function to display results
                var displayCallback = function(response){
                    table.data({query: input.val()});
                    //    Display results
                    tbody.empty();
                    $.each(response.data, function(index, group){
                        var tr = $('<tr>').appendTo(tbody).addClass(index%2 ? 'odd' : 'even');
                        $('<input type="checkbox">').attr({value:group.id}).appendTo(
                            $('<td>').appendTo(tr)
                        );
                        $('<span>').text(group.label).appendTo(
                            $('<td>').appendTo(tr)
                        );
                        $('<strong>').text(group.label).appendTo(
                            $('<td>').appendTo(tr)
                        );
                    });
                    tbody.find('tr').click(function(e){
                        if (e.target.tagName != 'INPUT'){
                            $(this).find('input:checkbox').each(function(){
                                this.checked = !this.checked;
                            });
                        }
                    });
                    //    Pagination
                    var text = '';
                    if (response.pagination.count > 0){
                        text += 'Results ';
                        text += response.pagination.offset + 1;
                        text += ' to ';
                        if (response.pagination.count > response.pagination.limit) {
                            text += response.pagination.offset + response.pagination.limit + 1;
                            text += ' out of ';
                        }
                        text += response.pagination.count
                    } else{
                        text += 'No results';
                    }
                    span.text(text);
                    //
                    pagesUl.empty();
                    var maxPage = Math.ceil(response.pagination.count / response.pagination.limit);
                    if (maxPage > 1){
                        var previousShown = false;
                        for (var page=0; page<maxPage; page++){
                            var currentPage = Math.ceil(response.pagination.offset / response.pagination.limit);
                            if (page==0 || (page<=currentPage+2 && page>=currentPage-2) || page==maxPage-1){
                                var li = $('<li>').appendTo(pagesUl);
                                var button = $('<button>').text(page + 1).appendTo(li);
                                if (page == currentPage){
                                    button.addClass('active');
                                } else{
                                    button.click(function(){
                                        var page = parseInt($(this).text()) - 1;
                                        refreshResults(page);
                                    });
                                }
                                previousShown = true;
                            } else if (previousShown){
                                var li = $('<li>').appendTo(pagesUl).text('...');
                                previousShown = false;
                            } else{
                                previousShown = false;
                            }
                        }
                    }
                    //    Display
                    tab.removeClass('loading');
                    table.width(input.width());
                    input.focus();
                };
                //    Perform keywords search on dataset and display results
                var refreshResults = function(page) {
                    tab.addClass('loading');
                    var dataset_label = tabSelect.data('label');
                    var dataset_id;
                    var parameters = {
                        contains: input.val(),
                        limit: 20,
                        offset: 20 * page
                    };
                    dataset_id = search_dataset(dataset_label).id;
                    requester.get('/datasets/' + dataset_id + '/groups', parameters, function(response) {
                        displayCallback(response);
                    }, function() {
                        alert('Could not load dataset items');
                    });
                }
                //    Add & configure search field
                var timer;
                input.keyup(function(){
                    clearTimeout(timer);
                    timer = setTimeout(function(){
                        var page = 0;
                        if (input.val() == table.data('query')){
                            return;
                        }
                        refreshResults(0);
                    }, 500);
                });
                //    Show the clicked tab
                tabSelect.click(function(){
                    var tabIndex = tabsSelect.removeClass('selected').index(this);
                    tabs.removeClass('selected').eq(tabIndex).addClass('selected')
                        .find('input').keyup().focus();
                    tabsSelect.eq(tabIndex).addClass('selected');
                });
            }).first().click();
        },
        getData : function(contents, callback) {
            // fetch identifiers from checked items
            var identifiers = [];
            var tab = contents.find('li.tab.selected');
            var tabSelect = contents.find('ul.tabs-select li.selected');
            tab.find('input:checked').each(function(i, input){
                identifiers.push(parseInt($(input).val()));
            });
            // dataset info
            var dataset_label = tabSelect.data('label').toLowerCase();
            var dataset_id = search_dataset(dataset_label).id;
            // retrieve points from identifiers
            requester.get('/datasets/' + dataset_id + '/groups', {
                with_points: 'true',
                identifiers: identifiers
            }, function(result){
                // reorganize received group data
                var new_pointsets = [];
                for (var i=0, n=result.data.length; i<n; ++i) {
                    var group = result.data[i];
                    new_pointsets.push({
                        source: 'datasets',
                        label: dataset_label + ': ' + group.label,
                        points: group.points,
                        metadata: {
                            dataset: {
                                id: dataset_id,
                                label: dataset_label,
                            },
                            group: {
                                id: group.id,
                                label: group.label,
                            },
                        },
                    });
                }
                // add data to query through callback
                callback(new_pointsets);
            }, function(){
                throw 'Could not retrieve points from dataset group identifiers';
            });
        }
    }, {
        title : 'Input coordinates',
        name : 'input',
        description : 'User-specified 3D coordinates',
        html : '<textarea placeholder="Example of coordinates:  10 -8 4"></textarea>',
        getData : function(contents, callback) {
            // retrieve points from field
            var re = /\s*([-+]?\d+\.?\d*)\s+([-+]?\d+\.?\d*)\s+([-+]?\d+\.?\d*)(?!\s+([-+]?\d+\.?\d*))\s*/g;
            var raw = contents.find('textarea').val();
            var lines = raw.split(/\s*\n\s*/);
            var points = [];
            for (var i = 0; i < lines.length; i++) {
                var match = lines[i].trim().split(/\s+/);
                if (match.length < 3) {
                    continue;
                }
                var point = [
                    parseFloat(match[0]),
                    parseFloat(match[1]),
                    parseFloat(match[2]),
                    (match.length > 3) ? parseFloat(match[3]) : 1.0,
                ];
                points.push(point);
            }
            // callback with formatted data
            callback([{
                source: 'text',
                label: 'Coordinates (' + points.length + 'points)',
                metadata: {
                    raw: raw
                },
                points: points
            }]);
        }
    }, {
        title : 'Text files',
        name : 'text',
        description : 'Text file consisting of a list of 3D coordinates',
        html : '<iframe src="/upload/"></iframe><ul class="uploads"></ul>',
        getData : function(contents, callback) {
            // retrieve original file paths
            var labels = [];
            contents.find('li span').each(function() {
                labels.push($(this).text());
            });
            // retrieve uploaded file paths, and extract points from them
            var uploads = contents.find('iframe').data('files');
            var pointsets = [];
            for (var i = 0; i < uploads.length; i++) {
                // compose data sent to retrieve points
                var data = {
                    source: 'upload',
                    format: 'text',
                    files: [uploads[i]],
                };
                // send data, retrieve points, add pointset
                let index = i;
                requester.get('/points', data, function(response) {
                    // add a new pointset
                    pointsets.push({
                        source: 'text_file',
                        label: labels[index],
                        metadata: {
                            original_path: labels[index],
                            uploaded_path: uploads[index],
                        },
                        points: response.points
                    });
                    // when all points data has been retrieved
                    if (pointsets.length == uploads.length) {
                        callback(pointsets);
                    }
                });
            }
        },
    }, {
        title : 'NIfTI files',
        name : 'nifti',
        description : 'Neuroimaging Informatics Technology Initiative file',
        html : '<iframe src="/upload/"></iframe><ul class="uploads"></ul>',
        getData : function(contents, callback) {
            // retrieve original file paths
            var labels = [];
            contents.find('li span').each(function() {
                labels.push($(this).text());
            });
            // retrieve uploaded file paths, and extract points from them
            var uploads = contents.find('iframe').data('files');
            var pointsets = [];
            for (var i = 0; i < uploads.length; i++) {
                // compose data sent to retrieve points
                var data = {
                    source: 'upload',
                    format: 'nifti',
                    files: [uploads[i]],
                };
                // send data, retrieve points, add pointset
                let index = i;
                requester.get('/points', data, function(response) {
                    // add a new pointset
                    pointsets.push({
                        source: 'nifti_file',
                        label: labels[index],
                        metadata: {
                            original_path: labels[index],
                            uploaded_path: uploads[index],
                        },
                        points: response.points
                    });
                    // when all points data has been retrieved
                    if (pointsets.length == uploads.length) {
                        callback(pointsets);
                    }
                });
            }
        }
    }, {
        title : 'Previous queries',
        name : 'previous',
        description : 'Points used in a previous query',
        html : '<span>(soon to come)</span>',
        getData : function(contents, callback){
            callback([]);
        }
    }
];
exportPdfParameters = {
    groups: true,
    correlations: true,
    view2D: true,
    view2D_figures: [],
    view3D: true,
    view3D_figures: [],
    graph: true,
    graph_figure: ''
};


var distanceFromCamera = 300;
var standardViews = [
    {
        title: 'Lateral (left)',
        position: new THREE.Vector3(-distanceFromCamera, 0, 0),
        up: new THREE.Vector3(0, 0, 1)
    }, {
        title: 'Lateral (right)',
        position: new THREE.Vector3(+distanceFromCamera, 0, 0),
        up: new THREE.Vector3(0, 0, 1)
    }, {
        title: 'Rostral',
        position: new THREE.Vector3(0, +distanceFromCamera, 0),
        up: new THREE.Vector3(0, 0, 1)
    }, {
        title: 'Caudal',
        position: new THREE.Vector3(0, -distanceFromCamera, 0),
        up: new THREE.Vector3(0, 0, 1)
    }, {
        title: 'Dorsal',
        position: new THREE.Vector3(0, 0, +distanceFromCamera),
        up: new THREE.Vector3(0, 1, 0)
    }, {
        title: 'Ventral',
        position:    new THREE.Vector3(0, 0, -distanceFromCamera),
        up: new THREE.Vector3(0, 1, 0)
    }
];



var query = new (function(selector){

    var query = this;
    var queryContainer = $(selector).first().empty().addClass('query');
    var queryTitle;
    var queryFrameset;

    queryData = undefined;

    //
    //    Client/server interaction
    //

    var querySend = function(action, data, dialogTitle, callback, skip_dialog_and_reload, recompute){
        // what we do depends on the action
        var method, url;
        switch (action) {
            case 'new':
                method = 'post';
                url = '/queries';
                if (!data) data = {};
                if (!data.name) data.name = 'Query';
                if (!data.groups) data.groups = [];
                if (!data.settings) data.settings = defaultSettings;
                if (!data.permissions) data.permissions = {read:[user.id], write:[user.id], own:[user.id]};
                break;
            case 'load':
                method = 'get';
                url = '/queries/' + data.id;
                data = undefined;
                break;
            case 'update':
                method = 'patch';
                url = '/queries/' + queryData.id;
                if (data.id !== undefined) {
                    delete data.id;
                }
                break;
            default:
                throw ('Unknown query action: ' + action);
                return;
        }
        // Entertain the user
        if (!skip_dialog_and_reload) {
            queryTitle.hide();
            queryFrameset.addClass('loading');
            var w = $.window({
                title: (dialogTitle ? dialogTitle : 'Updating query...'),
                height: '25%',
                width: '50%',
                blocking: true
            });
        }
        // perform & interpret request
        requester[method](url, data, function(response) {
            if (!skip_dialog_and_reload) {
                if (!response.id){
                    queryNew();
                    return;
                }
                queryData = response;
                // clear window
                w.parent().remove();
                queryFrameset.removeClass('loading');
                // show title
                queryTitle.show();
                titleUpdate();
                // load frames
                $.each(frames, function(f, frame){
                    frame.html.addClass('loading');
                });
                if (!frames.view.initialized) {
                    frames.view['init' + queryData.settings.view.type]();
                }
                $.each(frames, function(f, frame){
                    frame.update();
                    frame.html.removeClass('loading');
                });
                $(window).resize(); // propagate to frames
                // URL
                location.hash = '#' + queryData.id
            }
            // recompute correlations & graph
            if (recompute || (data !== undefined && data.groups !== undefined)) {
                queryData.is_computed = true;
                queryUpdate('is_computed');
            }
            // user-defined callback
            if ($.isFunction(callback)) {
                callback(response);
            }
        }, function(error) {
            console.log('Server disagreed to this: ' + action + ' query.');
        });
    };
    var queryLoad = function(id){
        querySend('load', {id: id}, 'Loading query...');
    };
    var queryNew = function(){
        querySend('new', {}, 'Creating new query...');
    };
    var queryUpdate = function(key, callback, skip_dialog_and_reload, recompute){
        var data = {};
        data[key] = queryData[key];
        var title = (key == 'is_computed') ? ('Recomputing query correlations and graph...') : ('Updating query ' + key + '...');
        if (key == 'groups') {
            for (var g = 0; g < queryData.groups.length; g++) {
                var group = queryData.groups[g];
                group.points = [];
                for (var s = 0; s < group.subgroups.length; s++) {
                    var subgroup = group.subgroups[s];
                    for (var p = 0; p < subgroup.points.length; p++) {
                        var point = subgroup.points[p];
                        group.points.push(point);
                    }
                }
            }
        }
        querySend('update', data, title, callback, skip_dialog_and_reload, recompute);
    };
    var querySettings = function(callback, force_dialog_and_reload, recompute){
        queryUpdate('settings', callback, (force_dialog_and_reload!==true), recompute);
    };
    var queryDelete = function(queryId){
        //    Delete query
        $.window({
            title: 'Deleting query...',
            height: '25%',
            width: '50%',
            blocking: true
        });
        var url = '/queries/' + (queryId ? queryId : queryData.id);
        requester.delete(url, function(){
            queryNew();
        }, function(){
            alert('Could not delete query');
        });
    };

    //
    //    Build the title
    //

    var titleInit = function(){
        queryTitle = $('<h1>').appendTo(queryContainer).click(function(){
            var newTitle;
            if (newTitle = prompt('Enter a new title for this query:', queryData.name)){
                queryData.name = newTitle;
                queryUpdate('name');
            }
        });
        $('<span>').appendTo(queryTitle);
        if (document.body.mozRequestFullScreen){
            $.fullscreen = function(){
                document.body.mozRequestFullScreen();
            };
        } else if (document.body.webkitRequestFullScreen){
            $.fullscreen = function(){
                document.body.webkitRequestFullScreen();
            };
        } else if (document.body.msRequestFullScreen){
            $.fullscreen = function(){
                document.body.msRequestFullScreen();
            };
        } else if (document.body.requestFullScreen){
            $.fullscreen = function(){
                document.body.requestFullScreen();
            };
        } else if (typeof(window.ActiveXObject) != 'undefined'){
            $.fullscreen = function(){
                var wScript = new ActiveXObject("WScript.Shell");
                if (wScript != null) {
                    wScript.SendKeys("{F11}");
                }
            };
        }
        if ($.isFunction($.fullscreen)){
            var div = $('<div>').title('Fullscreen').prependTo(queryTitle).append(
                $('<img>').attr({src:'/icons.png'})
            ).click(function(){
                $.fullscreen();
                return false;
            });
        }
    };
    var titleUpdate = function(){
        var span = queryTitle.find('span');
        if (!queryData.name){
            return;
        }
        var text = queryData.name.trim();
        var textDefault = 'Untitled query';
        text = text ? text : textDefault;
        span.text(text);
        if (text == textDefault){
            span.addClass('untitled');
        } else{
            span.removeClass('untitled');
        }
        $('title').text(text);
    };

    //
    //    Build the frames
    //

    var frames =
    {    "groups":
        {    "init":        function(){
                var frame = frames.groups;
                var frameTitle = frame.htmlTitle;
                $('<button>').text('-').appendTo(frameTitle);
                var input = $('<input>').appendTo(frameTitle);
                $('<button>').text('+').appendTo(frameTitle);
                $('<span>').text('groups').appendTo(frameTitle);
                //    Events
                var changeGroupNumber = function(number){
                    if (number < 1  ||  number > 9  ||  number == queryData.groups.length){
                        input.val(queryData.groups.length);
                        return;
                    }
                    if (number < queryData.groups.length) {
                        if (confirm('Are you sure you want to remove groups from this query?')) {
                            while (number < queryData.groups.length) {
                                queryData.groups.pop();
                            }
                        } else {
                            input.val(queryData.groups.length);
                            return;
                        }
                    } else {
                        queryData.groups.push({
                            label: 'Group ' + (queryData.groups.length + 1).toString(),
                            description: '',
                            subgroups: [],
                            points: [],
                            hue: 6 * Math.random()
                        });
                    }
                    queryUpdate('groups');
                };
                input.change(function(){
                    var number = $(this).val().replace(/[^\d]/g, '');
                    number = (number == '') ? 0 : parseInt(number);
                    changeGroupNumber(number);
                }).blur(function(){
                    $(this).change();
                });
                frameTitle.children('button').click(function(){
                    changeGroupNumber(queryData.groups.length + parseInt($(this).text() + '1'));
                });
            }
        ,    "resize":    function(){
            }
        ,    "update":    function(){
                var container = frames.groups.htmlContainer.empty();
                var groups = queryData.groups;
                if (!groups){
                    return;
                }
                //    Title
                frames.groups.htmlTitle.children('input').val(queryData.groups.length);
                //    Events
                var deleteSubgroupUI = function(deleted_subgroup){
                    if (confirm('Are you sure you want to remove these points?')){
                        $.each(queryData.groups, function(g, group){
                            $.each(group.subgroups, function(s, subgroup){
                                if (subgroup == deleted_subgroup) {
                                    group.subgroups.splice(s, 1);
                                }
                            });
                        });
                        queryUpdate('groups');
                    }
                };
                var insertSubgroup = function(subgroup, ddAdd){
                    var dd = $('<dd>').insertBefore(ddAdd).text(subgroup.label || '\xa0');
                    $('<button>').text('x').appendTo(dd).attr({title:'Click here to remove these points from the group...'}).click(function(){
                        deleteSubgroupUI(subgroup, dd);
                    });
                };
                var insertSubgroupUI = function(group, ddAdd){
                    var color = ddAdd.css('color');
                    var wContainer = $('<div>').addClass('container');
                    //    Menu list
                    var menu = $('<ul>').addClass('menu').appendTo(wContainer);
                    $.each(subgroupTypes, function(){
                        $('<li>').text(this.title).attr('title', this.description).appendTo(menu);
                    });
                    //    Contents
                    var contents = $('<div>').addClass('contents loading').appendTo(wContainer);
                    //    Float blocker
                    $('<div>').addClass('clear').appendTo(wContainer);
                    //    Window rendering
                    var w = $.window(
                    {    title    :    'Add points to "' + group.label + '"'
                    ,    width    :    '75%'
                    ,    height    :    '75%'
                    ,    html    :    wContainer
                    });
                    wContainer.height(
                        w.height() - w.find('h1').outerHeight(true)
                    ).css({overflow:'visible'});
                    w.find('h1,li,button').css({color:color, outlineColor:color, borderColor:color});
                    contents.height(0.8 * (w.height() - w.find('h1').outerHeight() - parseInt(contents.css('marginTop'))));
                    //    Menu list event
                    menu.find('li').mousedown(function(){
                        var li = $(this);
                        if (!li.hasClass('selected')){
                            menu.find('li').removeClass('selected').css({backgroundColor:'#FFF', color:color});
                            li.addClass('selected').css({backgroundColor:color, color:'#FFF'});
                            $.each(subgroupTypes, function(p, subgroupType){
                                if (li.text() == subgroupType.title){
                                    contents.html(subgroupType.html).append(
                                        $('<button>').addClass('add').text('Add these points to the group').click(function() {
                                            subgroupType.getData(contents, function(data) {
                                                group.subgroups = group.subgroups.concat(data);
                                                queryUpdate('groups');
                                            });
                                        })
                                    ).removeClass().addClass('contents').addClass(subgroupType.name);
                                    if ($.isFunction(subgroupType.initalize)){
                                        ($.proxy(subgroupType.initalize, contents.get(0)))();
                                    }
                                }
                            });
                        }
                    }).first().mousedown();
                    //
                    contents.removeClass('loading');
                };
                var updateGroup = function(group, dl){
                    var color = Raphael.hsb(group.hue, 0.8, 0.8);
                    dl = $(dl).css({outlineColor:color});
                    dl.find('dt').css({backgroundColor: color, borderColor: color})
                      .find('span').text(group.label)
                    dl.find('dd.add').css({color: color});
                };
                var updateGroupUI = function(group, dl){
                    var form = $('<form>').css({boxShadow:''});
                    var label = $('<label>').appendTo(form).text('Title');
                    $('<input type="text">').prependTo(label).val(group.label).attr({placeholder:'Label of this group', name: 'label'});
                    label = $('<label>').appendTo(form).text('Description');
                    $('<input type="text">').prependTo(label).val(group.description).attr({placeholder:'Short description for this group', name: 'description'});
                    label = $('<label>').appendTo(form).text('Color');
                    hueInput = $('<input type="text">').prependTo(label).val(group.hue).attr({name: 'hue'});
                    label = $('<label>').appendTo(form);
                    $('<input type="submit">').prependTo(label).addClass('button').val('Save changes').css({marginTop:'1em'}).click(function(){
                        form.find('input,select').each(function(){
                            var field = $(this);
                            var name = field.attr('name');
                            var value = field.val();
                            if (name == 'hue') {
                                value = parseFloat(value);
                            }
                            group[name] = value;
                        });
                        queryUpdate('groups');
                        return false;
                    });
                    var w = $.window
                    ({    title    :    'Edit group settings'
                    ,    width    :    600
                    ,    html    :    form
                    });
                    hueInput.hue();
                };
                //    Building each group
                $.each(groups, function(g, group){
                    //    Group's frame
                    var dl = $('<dl>').appendTo(container);
                    if (g >= groups.length){
                        dl.hide();
                    }
                    //    Title bar
                    var dt = $('<dt>').appendTo(dl);
                    $('<button>').text('edit').attr({title: 'Click here to edit the settings for this group...'}).appendTo(dt).click(function(){
                        updateGroupUI(group, dl);
                    });
                    $('<span>').appendTo(dt);
                    //    Below contents
                    var dd = $('<dd>').addClass('add').text('Add data to this group...').appendTo(dl);
                    dd.click(function(){
                        insertSubgroupUI(group, dd);
                    });
                    $.each(group.subgroups, function(p, subgroup){
                        insertSubgroup(subgroup, dd);
                    });
                    updateGroup(group, dl);
                });
            }
        }
    ,    "correlations":
        {    "init":        function(){
                var frameTitle = frames.correlations.htmlTitle;
                $('<span>').text('Correlation with ').appendTo(frameTitle);
                //    Create buttons
                var dataset_buttons_config = {
                    'functions': 'functions2',
                    'phenotype': 'phenotypes',
                    'transcriptome': 'genes',
                };
                $.each(dataset_buttons_config, function(text, label){
                    $('<button>').text(text).data({label:label}).appendTo(frameTitle);
                });
                //    Add events to buttons
                var buttons = frameTitle.find('button');
                buttons.click(function(){
                    if (queryData && queryData.settings){
                        buttons.removeClass('selected');
                        var dataset_label = $(this).addClass('selected').data('label');
                        queryData.settings.correlations.dataset = {
                            id: search_dataset(dataset_label).id,
                            label: dataset_label
                        };
                        querySettings(undefined, true, true);
                    }
                });
            }
        ,    "resize":    function(){
            }
        ,    "update":    function(){
                //    Show the right type of correlation is selected
                frames.correlations.htmlTitle.find('button').each(function(){
                    var button = $(this);
                    if (button.text() == queryData.settings.correlations.dataset.label){
                        button.addClass('selected');
                    } else{
                        button.removeClass('selected');
                    }
                });
                //    Initialize stuff
                var groupColors = ['#888'];
                var groups = queryData.groups;
                if (!queryData){
                    return;
                }
                var correlations = queryData.correlations;
                var frameContainer = frames.correlations.htmlContainer.empty();
                if (!groups || !correlations){
                    frameContainer.empty();
                    return;
                }
                var table = $('<table>').appendTo(frameContainer);
                var thead = $('<thead>').appendTo(table);
                var tbody = $('<tbody>').appendTo(table);
                //    Fill header
                var tr = $('<tr>').appendTo(thead);
                $('<th>').attr({colspan:2}).appendTo(tr);
                $('<th>').text('Overall').appendTo(tr);
                for (var g=0; g<groups.length; g++){
                    var group = groups[g];
                    var groupColor = Raphael.hsb(group.hue, 0.8, 0.8);
                    groupColors.push(groupColor);
                    $('<th>').text(group.label).css({color:groupColor}).appendTo(tr);
                }
                //    Fill body
                $.each(correlations, function(c, correlation){
                    var tr = $('<tr>').appendTo(tbody).addClass(c%2 ? 'even' : 'odd');
                    $('<td>').text(c + 1).appendTo(tr);
                    $('<td>').text(correlation.label).appendTo(tr);
                    $.each(correlation.scores, function(s, score){
                        $('<td>').text(Math.round(100 * score))
                         .css({textAlign:'center', color:groupColors[s], fontWeight:'bold'}).appendTo(tr);
                    });
                });
                //    The end!
                table.tablesorter({onSorted: function(){
                    $(this).find('tbody tr').removeClass('even odd').each(function(i){
                        $(this).addClass(i%2 ? 'even' : 'odd').find('td:first').text(i+1);
                    });
                }});
            }
        }
    ,    "view":
        {    "init":        function(){
                var frame = frames.view;
                var frameTitle = frame.htmlTitle;
                frame.initialized = '';
                $('<span>').text('Brain mapping in ').appendTo(frameTitle);
                $.each(['2D','3D'], function(){
                    $('<button>').text(this).appendTo(frameTitle);
                });
                frameTitle.find('button').click(function(){
                    var type = $(this).text();
                    frame.select(type);
                });
            }
        ,    "select": function(type, callback) {
                 if (queryData && queryData.settings){
                     var frame = frames.view;
                     if (frame.initialized == type) {
                         if (callback) {
                             callback();
                         }
                         return;
                     }
                     queryData.settings.view.type = type;
                     frame.update(function() {
                         frame.initialized = type;
                         if (callback) {
                             callback();
                         }
                     });
                     if (type == '3D'){
                         frame.resize();
                     }
                     querySettings();
                 }
             }
        ,    "init2D":    function(callback){
                var frame = frames.view;
                var container = frame.htmlContainer;
                var settings = queryData.settings.view;
                var box = settings.box;
                if (frame.initializing == '2D' || frame.initialized == '2D') {
                    return;
                }
                frame.initializing = '2D';
                frame.html.css({backgroundColor:'#000'});
                delete frame['2D'];
                var frame2D = frame['2D'] = {};
                //    DOM initialization
                container.empty();
                var view = $('<div>').addClass('view view2D').appendTo(container);
                var controlsMenu = $('<ul>').addClass('controls controls2D').appendTo(container);
                container.find('.view3D,.controls3D').hide();
                var groups = queryData.groups;
                //    Prepare the slices
                $.each(box, function(coordinate, values){
                    this.total = this.max - this.min;
                });
                var scale = Math.min
                (    container.width() / (box.x.total + box.y.total)
                ,    (container.height() - controlsMenu.height()) / (box.y.total + box.z.total)
                );
                view.width(scale * (box.x.total + box.y.total))
                    .height(scale * (box.y.total + box.z.total))
                    .css({marginTop:controlsMenu.height()+'px'});
                //    Display slices
                var slices = {};
                $.each(['xyz', 'yzx', 'xzy'], function(s, sliceName){
                    var c0 = sliceName.charAt(0);
                    var c1 = sliceName.charAt(1);
                    var width = scale * box[c0].total;
                    var height = scale * box[c1].total;
                    var wrapper = $('<div>').addClass('view-slice').addClass('view-slice-' + sliceName)
                        .appendTo(view).width(width).height(height).data({sliceName:sliceName});
                    var image = $('<img>').attr({src: '/brain/'+sliceName+'.png'}).load(function(){
                        $(this).data({loaded: true})
                    }).appendTo(wrapper);
                    var paperContainer = $('<div>').addClass('paper').appendTo(wrapper);
                    var paper = Raphael(paperContainer.get(0), width, height);
                    paper.setViewBox
                    (    box[c0].min
                    ,    box[c1].min
                    ,    1 + box[c0].total
                    ,    1 + box[c1].total
                    );
                    var overlay = $('<div>').addClass('overlay').appendTo(wrapper);
                    //    Add to slices list
                    slices[sliceName] =
                    {    wrapper    :    wrapper
                    ,    image    :    image
                    ,    paper    :    paper
                    ,    overlay    :    overlay
                    };
                });
                slices.xyz.wrapper.css({right:0, top:0});
                slices.yzx.wrapper.css({left: 0, bottom:0});
                slices.xzy.wrapper.css({right:0, bottom:0});
                //    Store important stuff
                frame2D.slices = slices;

                //    Adjust position of pictures...
                var position = settings.position;
                var movePictures = function(){
                    var slices = frame2D.slices
                    $.each(slices, function(sliceName, slice){
                        var c2 = sliceName.charAt(2);
                        var top = (slice.image.height() - slice.wrapper.height()) * (position[c2] - box[c2].min) / box[c2].total;
                        slice.image.css({top: -top + 'px'});
                    });
                };
                //    ...but before that, wait until they're loaded!
                var movePicturesWhenLoaded = function(){
                    var unloadedPictures = 0;
                    $.each(frame2D.slices, function(sliceName, slice){
                        if (!slice.image.data('loaded')){
                            unloadedPictures++;
                        }
                    });
                    if (unloadedPictures > 0){
                        setTimeout(arguments.callee, 100);
                    } else{
                        movePictures();
                    }
                };
                movePicturesWhenLoaded();

                //    Draw points
                var drawPoints = function(){
                    console.log('Redraw points for 2D');
                    var position = settings.position;
                    var opacity = settings.points.opacity;
                    var radius = settings.points.radius;
                    var radius2 = radius * radius;
                    var box = settings.box;
                    $.each(frame2D.slices, function(sliceName, slice){
                        //    Initialize view
                        var c0 = sliceName.charAt(0);
                        var c1 = sliceName.charAt(1);
                        var c2 = sliceName.charAt(2);
                        var paper = slice.paper;
                        paper.clear();
                        //    Horizontal line
                        paper.line
                        (    box[c0].min
                        ,    box[c1].min + box[c1].max - position[c1]
                        ,    box[c0].max
                        ,    box[c1].min + box[c1].max - position[c1]
                        ).attr({stroke:'#888'});
                        //    Vertical line
                        paper.line
                        (    position[c0]
                        ,    box[c1].min
                        ,    position[c0]
                        ,    box[c1].max
                        ).attr({stroke:'#888'});
                        //    Draw points
                        var cx = 'xyz'.indexOf(c0);
                        var cy = 'xyz'.indexOf(c1);
                        var cz = 'xyz'.indexOf(c2);
                        for (var g=0; g<groups.length; g++){
                            var group = groups[g];
                            var color = 'hsba(' + group.hue + ', 1, 0.8, ' + opacity + ')';
                            $.each(group.points, function(p, point){
                                var h = Math.abs(position[c2] - point[cz]);
                                if (h <= radius){
                                    paper.circle(
                                        point[cx],
                                        box[c1].min + box[c1].max - point[cy],
                                        Math.sqrt(radius2 - h*h)
                                    ).attr({
                                        fill: color,
                                        stroke: 0
                                    });
                                }
                            });
                        }
                    });
                };
                drawPoints();
                //    Position coordinates
                var coordinatesSlice = $('<div>').addClass('view-slice').css({left:0,top:0}).appendTo(view);
                var updatePosition;
                $.each(['x','y','z'], function(i, c){
                    var label = $('<label>').text(c + ' = ').appendTo(coordinatesSlice);
                    $('<input>').attr({name:c}).val(position[c]).appendTo(label).change(function(){
                        var newPosition = {};
                        newPosition[this.name] = this.value;
                        updatePosition(newPosition);
                    });
                });
                updatePosition = function(newPosition){
                    var box = settings.box;
                    $.each(newPosition, function(c, value){
                        value = parseFloat(value);
                        value = isNaN(value) ? 0 : (2*Math.round(value/2));
                        if (value < box[c].min){
                            value = box[c].min;
                        }
                        if (value > box[c].max){
                            value = box[c].max;
                        }
                        view.find('input[name=' + c + ']').val(settings.position[c] = value);
                    });
                    movePictures();
                    drawPoints();
                };

                //    Mouse events
                $.each(slices, function(sliceName, slice){
                    var overlay = slice.overlay;
                    var size0 = overlay.width();
                    var size1 = overlay.height();
                    var mouse = function(e, overlay){
                        var c0 = sliceName.charAt(0);
                        var c1 = sliceName.charAt(1);
                        var newPosition = {};
                        newPosition[c0] = e.offsetX / size0 * box[c0].total + box[c0].min;
                        newPosition[c1] = (1 - e.offsetY / size1) * box[c1].total + box[c1].min;
                        updatePosition(newPosition);
                    };
                    overlay.mousedown(function(e){
                        overlay.data({viewDragging:true});
                        mouse(e, overlay);
                        return false;
                    }).mousemove(function(e){
                        if (overlay.data('viewDragging')){
                            mouse(e, overlay);
                        }
                        return false;
                    }).mouseout(function(){
                        overlay.data({viewDragging:false});
                    }).mouseup(function(){
                        overlay.data({viewDragging:false});
                    });
                });

                //
                //    Settings menu
                //
                controlsMenu.menu
                ({    'Representative points':
                    [    {    title:        'Radius'
                        ,    type:        'slider'
                        ,    context:    settings.points
                        ,    key:        'radius'
                        ,    min:        0.1
                        ,    max:        9.9
                        ,    action:        function(){
                                drawPoints();
                            }
                        }
                    ,    {    title:        'Opacity'
                        ,    type:        'slider'
                        ,    context:    settings.points
                        ,    min:        0
                        ,    max:        1
                        ,    key:        'opacity'
                        ,    action:        function(){
                                drawPoints();
                            }
                        }
                    ,
                    ]
                });
                // the end!
                frame.initializing = undefined;
                frame.initialized = '2D';
                if (callback) {
                    callback();
                }
            }
        ,    "init3D":    function(callback){
                var frame = frames.view;
                frame.initialized = '3D';
                frame.htmlContainer.empty();
                //
                if (frame['3D'] === undefined) {
                    frame['3D'] = {};
                }
                var frame3D = frame['3D'];
                var settings = queryData.settings.view;
                frame.html.css({background: ''});
                frame.view3D = $('<div>').addClass('view view3D').appendTo(frame.htmlContainer);
                frame3D.controlsMenu = $('<ul>').addClass('controls controls3D').appendTo(frame.htmlContainer);
                //    3D variables
                var camera, scene, renderer;
                var controls;
                var mouseX, mouseY;
                var windowX, windowY;
                var windowHalfX, windowHalfY;
                var brainCache = {};
                var brainLoader = new THREE.OBJLoader();
                var brainColor = new THREE.Color(0xFFFFFF);
                var brainPosition = new THREE.Vector3(0, 0, 0);
                var mapSpheres = function(brainSubObject){
                    //    Groups & spheres stuff
                    var radius = settings.points.radius;
                    var radius2 = radius * radius;
                    var groups = queryData.groups;
                    var groupColors = [];
                    for (var g=0; g<groups.length; g++){
                        var groupRGBColor = Raphael.hsb2rgb(groups[g].hue, 1, 0.8);
                        var groupColor = new THREE.Color();
                        groupColor.r = groupRGBColor.r / 255;
                        groupColor.g = groupRGBColor.g / 255;
                        groupColor.b = groupRGBColor.b / 255;
                        groupColors.push(groupColor);
                    }
                    //    Initialize values
                    var mesh = brainSubObject.children[0];
                    var geometry = mesh.geometry;
                    var faces = geometry.faces;
                    var vertices = geometry.vertices;
                    //    Determine each vertex' color
                    var vertexIndex = vertices.length;
                    var vertexColors = new Array(vertexIndex);
                    while (vertexIndex--){
                        var mappingColor = brainColor;
                        // change surface color to group's if requested
                        if (settings.zones.surfaces) {
                            var mapping_count = 0;
                            var vertex = vertices[vertexIndex];
                            var x = vertex.x;
                            var y = vertex.y;
                            var z = vertex.z;
                            //    Check if inside one of the spheres
                            var groupIndex = groups.length;
                            while (groupIndex--){
                                let group = groups[groupIndex];
                                let points = group.points;
                                let pointIndex = points.length;
                                let groupColor = groupColors[groupIndex];
                                while (pointIndex--) {
                                    let point = points[pointIndex];
                                    let dx = Math.abs(point[0] - x);
                                    if (dx > radius) continue;
                                    let dy = Math.abs(point[1] - y);
                                    if (dy > radius) continue;
                                    let dz = Math.abs(point[2] - z);
                                    if (dz > radius) continue;
                                    if (dx*dx + dy*dy + dz*dz > radius2) continue;
                                    if (mapping_count == 0) {
                                        mappingColor = groupColor;
                                    } else {
                                        mappingColor = new THREE.Color();
                                        mappingColor.r = (mapping_count * mappingColor.r + groupColor.r) / (mapping_count + 1);
                                        mappingColor.g = (mapping_count * mappingColor.g + groupColor.g) / (mapping_count + 1);
                                        mappingColor.b = (mapping_count * mappingColor.b + groupColor.b) / (mapping_count + 1);
                                    }
                                    mapping_count++;
                                    break;
                                }
                            }
                        }
                        // the end
                        vertexColors[vertexIndex] = mappingColor;
                    }
                    //    Color faces
                    var faceIndex = faces.length;
                    while (faceIndex--){
                        var face = faces[faceIndex];
                        face.vertexColors =
                        [    vertexColors[face.a]
                        ,    vertexColors[face.b]
                        ,    vertexColors[face.c]
                        ];
                    }
                    //    Repaint
                    geometry.verticesNeedUpdate = true;
                    geometry.elementsNeedUpdate = true;
                    geometry.morphTargetsNeedUpdate = true;
                    geometry.uvsNeedUpdate = true;
                    geometry.normalsNeedUpdate = true;
                    geometry.colorsNeedUpdate = true;
                    geometry.tangentsNeedUpdate = true;
                };
                var unMapSpheres = function(brainSubObject){
                    //    Initialize values
                    var mesh = brainSubObject.children[0];
                    var geometry = mesh.geometry;
                    var faces = geometry.faces;
                    //    Color faces
                    var faceIndex = faces.length;
                    var vertexColors = [brainColor, brainColor, brainColor];
                    while (faceIndex--){
                        faces[faceIndex].vertexColors = vertexColors;
                    }
                    //    Repaint
                    geometry.verticesNeedUpdate = true;
                    geometry.elementsNeedUpdate = true;
                    geometry.morphTargetsNeedUpdate = true;
                    geometry.uvsNeedUpdate = true;
                    geometry.normalsNeedUpdate = true;
                    geometry.colorsNeedUpdate = true;
                    geometry.tangentsNeedUpdate = true;
                };
                var drawBrain = function(){
                    var brainMaterials = [
                        new THREE.MeshLambertMaterial({color: 0x888888, emissive:0x888888, shading: THREE.SmoothShading, vertexColors: THREE.VertexColors, side:THREE.DoubleSide, depthTest:true, depthWrite:true}),
                        new THREE.MeshBasicMaterial({color: 0xFFFFFF, shading: THREE.FlatShading, wireframe: false, transparent: true, opacity:0})
                    ];
                    // browse organ elements
                    var loaded_elements_count = 0;
                    var total_elements_count = organ.elements.length;
                    $.each(organ.elements, function(e, element) {
                        // check element visibility
                        var element_visibility = true;
                        $.each(settings.tags, function(tag, visibility) {
                            if (!visibility && element.tags.indexOf(tag) != -1) {
                                element_visibility = false;
                            }
                        });
                        // render element if required
                        var url = '/organs/' + organ.path + '/' + element.url;
                        if (url in brainCache) {
                            var object = brainCache[url];
                            if (element_visibility) {
                                scene.add(object);
                                mapSpheres(object);
                            } else {
                                scene.remove(object);
                            }
                            // callback
                            loaded_elements_count++;
                            if (loaded_elements_count == total_elements_count) {
                                console.log('Loaded 3D organ parts (' + loaded_elements_count + '/' + total_elements_count + ')');
                                if (callback) {
                                    callback();
                                }
                            }
                        } else if (element_visibility) {
                            brainLoader.load(url, function(sourceObject){
                                var object = THREE.SceneUtils.createMultiMaterialObject(sourceObject.children[0].geometry, brainMaterials);
                                scene.add(object);
                                object.name = url;
                                object.children[0].traverse(function(node){
                                    if (node.material){
                                        node.material.opacity = settings.opacity;
                                        node.material.transparent = true;
                                    }
                                });
                                brainCache[url] = object;
                                mapSpheres(object);
                                // callback
                                loaded_elements_count++;
                                if (loaded_elements_count == total_elements_count) {
                                    console.log('Loaded 3D organ parts (' + loaded_elements_count + '/' + total_elements_count + ')');
                                    if (callback) {
                                        callback();
                                    }
                                }
                            });
                        } else {
                            loaded_elements_count++;
                            if (loaded_elements_count == total_elements_count) {
                                console.log('Loaded 3D organ parts (' + loaded_elements_count + '/' + total_elements_count + ')');
                                if (callback) {
                                    callback();
                                }
                            }
                        }
                    });
                };
                //    Draw spheres
                var spheres = [];
                var drawZones = function(){
                    //    Delete previous spheres
                    if (spheres){
                        $.each(spheres, function(s, sphere){
                            scene.remove(sphere);
                            sphere.geometry.dispose();
                            sphere.material.dispose();
                            // sphere.texture.dispose();
                        });
                    }
                    spheres = [];
                    //    Represent points as spheres
                    if (settings.zones.centers){
                        var radius = settings.points.radius;
                        var opacity = settings.points.opacity;
                        var groups = queryData.groups;
                        for (var g=0; g<groups.length; g++){
                            var group = groups[g];
                            var rgb = Raphael.hsb2rgb(group.hue, 0.8, 0.5);
                            var color = new THREE.Color();
                            color.r = rgb.r / 255;
                            color.g = rgb.g / 255;
                            color.b = rgb.b / 255;
                            var sphereMaterial = new THREE.MeshLambertMaterial({color:color, transparent:true, emissive:color, opacity:0.75, overdraw: true});
                            $.each(group.points, function(p, point){
                                var sphereRadius = settings.points.byRadius ? (radius * point[3]) : radius;
                                var sphereGeometry = new THREE.SphereGeometry(sphereRadius, 8, 8);
                                var sphere = new THREE.Mesh(sphereGeometry, sphereMaterial);
                                sphere.position.x = point[0];
                                sphere.position.y = point[1];
                                sphere.position.z = point[2];
                                sphere.traverse(function(node){
                                    if (node.material){
                                        node.material.opacity = opacity * (settings.points.byOpacity ? point[3] : 1);
                                        node.material.transparent = true;
                                    }
                                });
                                scene.add(sphere);
                                spheres.push(sphere);
                            });
                        }
                    }
                };
                //    Initialize
                var init = function(){
                    //    Dimensions stuff
                    mouseX = mouseY = 0;
                    windowX = frame.view3D.width();
                    windowY = frame.view3D.height();
                    windowHalfX = windowX / 2;
                    windowHalfY = windowY / 2;
                    var container = frame.view3D.empty().get(0);
                    //    Camera
                    if (frame3D.camera === undefined) {
                        frame3D.camera = camera = new THREE.PerspectiveCamera(45, 1, 1, 1000);
                        camera.position.x = distanceFromCamera;
                        camera.position.y = 0;
                        camera.position.z = 0;
                        camera.aspect = windowX / windowY;
                        camera.updateProjectionMatrix();
                        camera.up = new THREE.Vector3(0, 0, 1);
                    } else {
                        camera = frame3D.camera;
                    }
                    //    Scene
                    if (frame3D.scene === undefined) {
                        frame3D.scene = scene = new THREE.Scene();
                    } else {
                        scene = frame3D.scene;
                    }
                    //    Lights
                    if (frame3D.directionalLight1 === undefined) {
                        frame3D.directionalLight1 = directionalLight1 = new THREE.DirectionalLight(0xFFFFFF);
                        scene.add(directionalLight1);
                        frame3D.directionalLight2 = directionalLight2 = new THREE.DirectionalLight(0xFFFFFF);
                        scene.add(directionalLight2);
                    } else {
                        directionalLight1 = frame3D.directionalLight1;
                        directionalLight2 = frame3D.directionalLight2;
                    }
                    //    Renderer
                    if (frame3D.renderer === undefined) {
                        console.log('make renderer');
                        try {
                            renderer = new THREE.WebGLRenderer({preserveDrawingBuffer: true});
                        } catch (e) {
                            try {
                                renderer = new THREE.CanvasRenderer({preserveDrawingBuffer: true});
                            } catch (e) {
                                frame.view3D.html
                                (    '<div style="padding:16px"><h3>Error Detected!</h3><br/>For Mac Safari users, WebGL is disabled by Default (the Windows version of Safari does not yet support WebGL). To enable it you should follow these instructions:<br/><br/>'
                                +    '(1) Open Safari and in the Safari menu select Preferences<br/>'
                                +    '(2) Click Advanced tab in the Preferences window<br/>'
                                +    '(3) At the bottom of the window check the "Show Develop menu in menu bar" checkbox<br/>'
                                +    '(4) Open the Develop menu in the menu bar and select Enable WebGL<br/><br/>'
                                +    'Please let us know if you have any questions.</div>'
                                );
                                return container;
                            }
                        }
                        renderer.setSize(windowX, windowY);
                        frame3D.renderer = renderer;
                    } else {
                        renderer = frame3D.renderer;
                    }
                    container.appendChild(renderer.domElement);
                    //    Draw objects
                    frame3D.draw = function(){
                        drawBrain();
                        drawZones();
                    };
                    //    Axis
                    var showAxis = function(axisLength){
                        function v(x,y,z){
                            return new THREE.Vector3(x,y,z);
                        }
                        function drawLine(p1, p2, color){
                            var line, lineGeometry = new THREE.Geometry(),
                            lineMat = new THREE.LineBasicMaterial({color: color, lineWidth: 1, opacity:0.5});
                            lineGeometry.vertices.push(p1, p2);
                            line = new THREE.Line(lineGeometry, lineMat);
                            scene.add(line);
                        }
                        drawLine(v(-axisLength, 0, 0), v(axisLength, 0, 0), 0x000000);
                        drawLine(v(0, -axisLength, 0), v(0, axisLength, 0), 0x000000);
                        drawLine(v(0, 0, -axisLength), v(0, 0, axisLength), 0x000000);
                    };
                    showAxis(2048);
                    //    Controls
                    controls = new THREE.TrackballControls(camera, container.parentNode);
                    frame3D.controls = controls;
                };
                var render = function(){
                    controls.update();
                    directionalLight1.position.set(camera.position.x, camera.position.y, camera.position.z).normalize();
                    directionalLight2.position.set(-camera.position.x, -camera.position.y, -camera.position.z).normalize();
                    renderer.render(scene, camera);
                };
                var animate = function(){
                    requestAnimationFrame(animate);
                    render();
                };
                //
                init();
                animate();
                //
                //    Make a menu
                //
                var menu =
                {    'Surface':
                    [    {    title:        'Opacity'
                        ,    type:        'slider'
                        ,    context:    settings
                        ,    min:        0
                        ,    max:        1
                        ,    key:        'opacity'
                        ,    action:        function(){
                                drawBrain();
                                querySettings();
                            }
                        }
                    ,    {    type:        'separator'
                        }
                    ]
                ,    'Representative points':
                    [    {    title:        'Spheres'
                        ,    type:        'bool'
                        ,    context:    settings.zones
                        ,    key:        'centers'
                        ,    action:        function(){
                                drawZones();
                                querySettings();
                            }
                        }
                    ,    {    title:        'Spheres projections on cortex'
                        ,    type:        'bool'
                        ,    context:    settings.zones
                        ,    key:        'surfaces'
                        ,    action:        function(){
                                drawBrain();
                                querySettings();
                            }
                        }
                    ,    {    title:        'Original volumes'
                        ,    type:        'bool'
                        ,    context:    settings.zones
                        ,    key:        'volumes'
                        ,    action:        function(){
                                drawZones();
                                querySettings();
                            }
                        }
                    ,    {    type:        'separator'
                        }
                    ,    {    title:        'Intensity represented by radius'
                        ,    type:        'bool'
                        ,    context:    settings.points
                        ,    key:        'byRadius'
                        ,    action:        function(){
                                drawZones();
                                querySettings();
                            }
                        }
                    ,    {    title:        'Intensity represented by opacity'
                        ,    type:        'bool'
                        ,    context:    settings.points
                        ,    key:        'byOpacity'
                        ,    action:        function(){
                                drawZones();
                                querySettings();
                            }
                        }
                    ]
                ,    'Standard views':
                    []
                ,    'Spheres representation':
                    [    {    title:        'Radius'
                        ,    type:        'slider'
                        ,    context:    settings.points
                        ,    key:        'radius'
                        ,    min:        0.1
                        ,    max:        9.9
                        ,    action:        function(){
                                drawZones();
                                drawBrain();
                                querySettings();
                            }
                        }
                    ,    {    title:        'Opacity'
                        ,    type:        'slider'
                        ,    context:    settings.points
                        ,    min:        0
                        ,    max:        1
                        ,    key:        'opacity'
                        ,    action:        function(){
                                drawZones();
                                querySettings();
                            }
                        }
                    ,
                    ]
                };
                // integrate surface tags into the menu
                $.each(organ.menu, function(e, element) {
                    if (element.type == 'item') {
                        menu['Surface'].push({
                            title: element.label,
                            type: 'bool',
                            context: settings.tags,
                            key: element.tag,
                            action: function(){
                                drawBrain();
                                drawZones();
                                querySettings();
                            }
                        });
                    } else if (element.type == 'separator') {
                        menu['Surface'].push({type: 'separator'});
                    }
                });
                // integrate standard views into the menu
                $.each(standardViews, function(){
                    var view = this;
                    menu['Standard views'].push
                    ({    title:        view.title
                    ,    type:        'click'
                    ,    action:        function(){
                            controls.target = new THREE.Vector3(0, 0, 0);
                            $.each(['x','y','z'], function(i, c){
                                camera.position[c] = view.position[c];
                                camera.up[c] = view.up[c];
                            });
                            camera.updateProjectionMatrix();
                        }
                    });
                });
                frame3D.controlsMenu.menu(menu);
            }
        ,    "resize2D":    function(){
                frames.view.init2D();
            }
        ,    "resize3D":    function(windowX, windowY){
                var frame = frames.view;
                var frame3D = frame['3D'];
                var camera = frame3D.camera;
                var renderer = frame3D.renderer;
                if (!windowX){
                    windowX = frame.htmlContainer.width();
                }
                if (!windowY){
                    windowY = frame.htmlContainer.height();
                }
                frame.view3D.width(windowX).height(windowY);
                camera.aspect = windowX / windowY;
                camera.updateProjectionMatrix();
                renderer.setSize(windowX, windowY);
                frame3D.controlsMenu.css({width:frame.htmlTitle.outerWidth()})
            }
        ,    "resize":    function(){
                if (!queryData){
                    return;
                }
                var viewType = queryData.settings.view.type;
                var frame = frames.view;
                if (frame.initialized != viewType){
                    frame['init' + viewType]();
                    frame.initialized = viewType;
                }
                frames.view['resize' + viewType]();
            }
        ,    "update2D":    function(){
                frames.view.init2D();
            }
        ,    "update3D":    function(){
                var frame3D = frames.view['3D'];
                frame3D.draw();
            }
        ,    "update":    function(callback){
                var viewType = queryData.settings.view.type;
                var frame = frames.view;
                //    Select the right type of visualization
                frame.htmlTitle.find('button').each(function(){
                    var button = $(this);
                    if (button.text().replace(/s$/, '') == viewType){
                        button.addClass('selected');
                    } else{
                        button.removeClass('selected');
                    }
                });
                //    Initialize when necessary
                if (frame.initialized != viewType){
                    frame['init' + viewType](callback);
                    frame.initialized = viewType;
                }
                //    Update stuff
                frames.view['update' + viewType]();
            }
        }
    ,    "graph":
        {    "init":        function(){
                $('<span>').text('Graph').appendTo(frames.graph.htmlTitle);
            }
        ,    "resize":    function(width, height){
                frames.graph.update(width, height);
            }
        ,    "update":    function(width, height){
                if (!queryData){
                    return;
                }
                var settings = queryData.settings.graph;
                var container = frames.graph.htmlContainer;
                if (!queryData.graph){
                    container.empty();
                    return;
                }
                var threshold = settings.threshold;
                var nodes = queryData.graph.nodes;
                var edges = queryData.graph.edges;
                if (!nodes || !edges || !threshold){
                    return;
                }
                container.removeClass('loading').empty();
                if (!width){
                    width = container.width();
                }
                if (!height){
                    height = container.height();
                }
                var paper = Raphael(container[0], width, height);
                var extrema = {xMin:+99,xMax:-99,yMin:+99,yMax:-99};
                frames.graph.paper = paper;
                //    Calculates on-screen coordinates
                $.each(nodes, function(){
                    if (extrema.xMin > this.x){
                        extrema.xMin = this.x;
                    } else if (extrema.xMax < this.x){
                        extrema.xMax = this.x;
                    }
                    if (extrema.yMin > this.y){
                        extrema.yMin = this.y;
                    } else if (extrema.yMax < this.y){
                        extrema.yMax = this.y;
                    }
                });
                var r = Math.min(width, height) / 20;
                var scale = Math.min
                (    (width  - 3 * r) / (extrema.xMax - extrema.xMin)
                ,    (height - 3 * r) / (extrema.yMax - extrema.yMin)
                );
                var x0 = 0.5 * (width  - scale * (extrema.xMax + extrema.xMin));
                var y0 = 0.5 * (height - scale * (extrema.yMax + extrema.yMin));
                var onscreenCoordinates = function(point){
                    var x = Math.round(x0 + scale * point.x);
                    var y = Math.round(y0 + scale * point.y);
                    return {x:x, y:y};
                };
                //    Draw edges
                $.each(edges, function(i, row){
                    var centerI = onscreenCoordinates(nodes[i]);
                    var isGroup = (nodes[i].source == 'query');
                    $.each(row, function(j, score){
                        if (score > 0) {
                            var color;
                            if (nodes[i].source == 'query') {
                                if (nodes[j].source == 'query') {
                                    color = Raphael.hsb(0.5*(nodes[i].hue + nodes[j].hue), 0.8, 0.8);
                                } else {
                                    color = Raphael.hsb(nodes[i].hue, 0.8, 0.8);
                                }
                            } else {
                                color = '#888';
                            }
                            var centerJ = onscreenCoordinates(nodes[j]);
                            var width =  Math.max(0.0, 3 + Math.log10(score));
                            var opacity = width / 3.0;
                            paper.line(centerI.x, centerI.y, centerJ.x, centerJ.y).attr({
                                opacity: opacity,
                                stroke: color,
                                'stroke-width': width});
                        }
                    });
                });
                //    Draw nodes
                $.each(nodes, function(i){
                    var center = onscreenCoordinates(this);
                    var backgroundColor = '#EEE';
                    var foregroundColor = '#000';
                    var label = this.label;
                    if (this.source == 'query'){
                        backgroundColor = Raphael.hsb(this.hue, 0.8, 0.8);
                        foregroundColor = '#FFF';
                        label = this.label;
                    }
                    this.disc = paper.circle(center.x, center.y, r).attr
                    ({    fill    :    backgroundColor
                    ,    stroke    :    '#000'
                    ,    'stroke-opacity'    :    0.5
                    ,    opacity    :    0.85
                    ,    cursor    :    'pointer'
                    });
                    // write centered text, line by line
                    let lines = label.split(/\s+/);
                    let line_height = r / 2.0;
                    for (var i = 0; i < lines.length; i++) {
                        let y = center.y + (i - lines.length / 2.0 + 0.5) * line_height;
                        this.text = paper.text(center.x, y, lines[i]).attr({
                            fill: foregroundColor,
                            title: label,
                            fontSize: r/2.5,
                            'line-height': 0.0,
                            'font-family': 'Ubuntu,sans-serif',
                            cursor: 'pointer'
                        });
                    }
                });
                //    Adjust font
                container.find('*').css
                ({    fontSize:    (r/2) + 'px'
                ,    lineHeight:    (r/2) + 'px'
                });
            }
        }
    };

    var addRow = function(frameset, ratio){
        var row = $('<div>').addClass('row').appendTo(frameset);
        if (ratio){
            row.data('frameset-ratio', ratio);
        }
        return row;
    };
    var addFrame = function(row, name, ratio){
        var frame = frames[name];
        frame.html = $('<div>').addClass('frame').addClass(name).appendTo(row);
        frame.htmlTitle = $('<div>').addClass('title').appendTo(frame.html);
        frame.htmlContainer = $('<div>').addClass('container').appendTo(frame.html);
        frame.html.framesetResizeCallback(function(){
            frame.htmlContainer.height(
                frame.html.height() - frame.htmlTitle.outerHeight(true) - frame.htmlContainer.outerHeight(true) + frame.htmlContainer.height()
            );
            frame.resize();
        });
        if (ratio){
            frame.html.data('frameset-ratio', ratio);
        }
        frame.init();
    };

    var framesetInit = function(){
        //    Build the frameset
        queryFrameset = $('<div>').addClass('frameset').appendTo(queryContainer);
        var row;
        row = addRow(queryFrameset, 0.3);
        addFrame(row, 'groups', 0.5)
        addFrame(row, 'correlations', 0.5);
        row = addRow(queryFrameset, 0.7);
        addFrame(row, 'view', 0.5);
        addFrame(row, 'graph', 0.5);
        queryFrameset.frameset();
        var slimScrollOptions = {height:'auto', width:'auto', size:'1em', railVisible:true, railOpacity:0.5, railColor:'#888', color:'#000'};
        frames.groups.htmlContainer.slimScroll(slimScrollOptions);
        frames.correlations.htmlContainer.slimScroll(slimScrollOptions);
    };

    //
    //    Exportation function
    //

    var exportPDF = function(){
        //    Show a window
        var div = $('<div>').addClass('container list');
        var w = $.window
        ({    title    :    'Exporting to PDF'
        ,    html    :    div
        ,    width    :    '50%'
        ,    height    :    '75%'
        });
        div.height(
            w.height() - w.find('h1').outerHeight(true)
        );
        //    Choices for user
        var pdfSections = [
            {
                title: '2D view',
                key: 'view2D',
                description: 'If checked, the PDF will include the 2D representation of the brain, following the 3 standard planes.'
            },
            {
                title: '3D view',
                key: 'view3D',
                description: 'If checked, the PDF will include the 3D representation of the brain, following the 6 standard views.'
            },
            {
                title: 'Graph',
                key: 'graph',
                description: 'If checked, the PDF will include a nodes graph showing the proximity between groups and a datasets.'
            },
            {
                title: 'Correlations',
                key: 'correlations',
                description: 'If checked, the PDF will include a table displaying the correlations between the groups and a dataset.'
            },
            {
                title: 'Groups',
                key: 'groups',
                description: 'If checked, the PDF will include a description of every group, along with all the point it contains.'
            }
        ];
        $('<p>').text('Check which sections you want to be displayed in the generated PDF:').appendTo(div);
        $.each(pdfSections, function(s, section){
            var label = $('<label>')
             .text(section.title)
             .addClass(s%2 ? 'odd' : 'even')
             .attr({title: section.description})
             .appendTo(div);
            var input = $('<input type="checkbox">')
             .attr({name: section.key})
             .prependTo(label);
            if (exportPdfParameters[section.key]){
                input.attr({checked: 'checked'});
            }
        });
        $('<button>').text('Generate PDF').appendTo(div).focus().click(function(){
            // compute export parameters from form
            div.find(':checkbox').each(function(){
                exportPdfParameters[this.name] = $(this).prop('checked');
            });
            // initialize DOM
            div.empty();
            var ul = $('<ul>').addClass('list').appendTo(div);
            var li;
            var liNew = function(text){
                if (!text){
                    text = '';
                }
                var l = ul.children('li').length;
                li = $('<li>').addClass(l%2 ? 'odd' : 'even').appendTo(ul);
                $('<b>').text(text).appendTo(li);
                $('<span>').text(' - ').css({color:'#888'}).appendTo(li);
            };
            var liUpdate = function(text){
                if (!text){
                    text = '';
                }
                li.children('span').append(' ' + text);
            };
            // save initial view state
            var initial_view_type = frames.view.initialized;
            // multi-step process
            var snapshotView3DCreate, snapshotView3DCallback, snapshotView3DUploadCallback, snapshotView2D, snapshotGraph, snapshotGraphCallback, pdfCreate, pdfCreateCallback;
            snapshotView2D = function() {
                // if 2D snapshot shouldn't be taken, or everything is ready, proceed to what's next
                if (!exportPdfParameters.view2D) {
                    snapshotGraph();
                    frames.view.select(initial_view_type);
                    return;
                }
                // show things to user
                liNew('2D view');
                liUpdate('initializing 2D...');
                // otherwise, let's take snapshots and send them to upload
                frames.view.select('2D', function() {
                    // fetch SVG raw contents
                    liUpdate('fetching contents...');
                    let files = [];
                    let names = [];
                    frames.view.htmlContainer.find('.view2D:last .view-slice').each(function(){
                        let svg = $(this).find('svg').get(0);
                        if (svg) {
                            let name = $(this).data('sliceName');
                            names.push(name);
                            files.push({
                                filename: 'pdf-2D-' + files.length + '-' + name + '.svg',
                                format: 'raw',
                                data: svg.outerHTML
                            });
                        }
                    });
                    // send it to server
                    liUpdate('sending data...');
                    requester.post('/uploads', {files: files}, function(response) {
                        for (var i = 0; i < response.files.length; i++) {
                            exportPdfParameters.view2D_figures.push({
                                name: names[i],
                                path: response.files[i].path
                            });
                        }
                        liUpdate('done.');
                        snapshotGraph();
                        frames.view.select(initial_view_type);
                    });
                });
            };
            snapshotView3DCreate = function() {
                // if 3D snapshot shouldn't be taken, or everything is ready, proceed to what's next
                if (!exportPdfParameters.view3D || exportPdfParameters.view3D_figures.length == standardViews.length) {
                    frames.view.resize3D();
                    if (exportPdfParameters.graph) {
                        snapshotView2D();
                    } else {
                        pdfCreate();
                    }
                    return;
                }
                // otherwise, upload another image
                var frame = frames.view;
                liNew('3D view');
                liUpdate('initializing 3D...');
                frame.select('3D', function() {
                    var snapshots = exportPdfParameters.view3D_figures;
                    var s = snapshots.length;
                    if (s == 0) {
                        frame.resize3D(768, 768);
                    }
                    var view = standardViews[s];
                    liUpdate('preparing ' + view.title + ' view...');
                    snapshots.push({
                        name: view.title,
                        path: ''
                    });
                    var frame3D = frame['3D'];
                    frame3D.controls.target = new THREE.Vector3(0, 0, 0);
                    frame3D.camera.position = view.position;
                    frame3D.camera.up = view.up;
                    frame3D.camera.updateProjectionMatrix();
                    setTimeout(snapshotView3DCallback, s ? 50 : 100);
                });
            };
            snapshotView3DCallback = function(){
                var snapshots = exportPdfParameters.view3D_figures;
                var index = snapshots.length - 1;
                var view = standardViews[index];
                var snapshot = snapshots[index];
                liUpdate('taking snapshot...');
                var png = frames.view['3D'].renderer.domElement.toDataURL('image/png');
                liUpdate('sending data...');
                requester.post('/uploads', {
                    files: [{
                        filename: 'pdf-3D-' + queryData.id.toString() + '-' + index + '.png',
                        format: 'base64',
                        data: png.split(',')[1],
                    }]
                }, snapshotView3DUploadCallback);
            };
            snapshotView3DUploadCallback = function(response){
                var snapshots = exportPdfParameters.view3D_figures;
                var s = snapshots.length - 1;
                var view = standardViews[s];
                var snapshot = snapshots[s];
                snapshot.path = response.files[0].path;
                liUpdate('done.');
                snapshotView3DCreate();
            };
            snapshotGraph = function(){
                liNew('Graph');
                liUpdate('taking snapshot...');
                frames.graph.resize(1024, 1024);
                var svg = frames.graph.paper.canvas.outerHTML;
                svg = svg.replace(/ xlink:title="/g, ' title="');
                frames.graph.resize();
                liUpdate('sending data...');
                requester.post('/uploads', {
                    files: [{
                        filename: 'pdf-graph-' + queryData.id.toString() + '.svg',
                        format: 'raw',
                        data: svg,
                    }]
                }, snapshotGraphCallback);
            };
            snapshotGraphCallback = function(response){
                liUpdate('done.');
                exportPdfParameters.graph_figure = response.files[0].path;
                pdfCreate();
            };
            pdfCreate = function(){
                liNew('PDF');
                liUpdate('generating file...');
                requester.post('/queries/' + queryData.id + '/pdf', exportPdfParameters, pdfCreateCallback);
            };
            pdfCreateCallback = function(response){
                liUpdate('done.');
                $('<a>').addClass('button').text('Click here to view the generated PDF').attr({
                    href: response.url,
                    target: '_blank'
                }).appendTo(div);
            };
            // start!
            exportPdfParameters.view2D_figures = [];
            exportPdfParameters.view3D_figures = [];
            exportPdfParameters.graph_figure = '';
            snapshotView3DCreate();
        });
    };

    //
    //    Build the sidebar
    //
    var sidebar =
    [    {    "title"            :    "New"
        ,    "description"    :    "Create an empty query"
        ,    "icon"            :    0
        ,    "action"        :    queryNew
        }
    ,    {    "title"            :    "Delete"
        ,    "description"    :    "Delete this query"
        ,    "icon"            :    1
        ,    "action"        :    function(){
                if (confirm('Are you sure you want to delete this query?')){
                    queryDelete();
                }
            }
        }
    ,    {    "title"            :    "Open"
        ,    "description"    :    "Load an existing query from your collection"
        ,    "icon"            :    3
        ,    "action"        :    function(){
                var div = $('<div>').addClass('container');
                var ul = $('<ul>').appendTo(div);
                var w = $.window(
                {    title    :    'Load a query'
                ,    width    :    '75%'
                ,    height    :    '75%'
                ,    html    :    div
                });

                div.height(
                    w.height() - w.find('h1').outerHeight(true)
                ).addClass('loading');

                requester.get('/queries', function(response){
                    div.empty();
                    if (!response.length){
                        $('<p>').text('You have no queries recorded for this session.').appendTo(div).css({
                            color: '#444',
                            fontStyle: 'italic'
                        });
                    }
                    var table = $('<table>').appendTo(div);
                    var tbody = $('<tbody>').appendTo(table);
                    $.each(response.data, function(q, query){
                        var tr = $('<tr>').appendTo(tbody).addClass(q%2 ? 'even' : 'odd').click(function(){
                            queryLoad(query.id);
                            return false;
                        });
                        //    Title
                        var td = $('<td>').appendTo(tr);
                        $('<a>').attr('href', '#'+query.id).text(query.name).appendTo(td);
                        //    Date
                        $('<td>').text(query.updated_at.substring(0, 10)).appendTo(tr);
                        //    Groups
                        td = $('<td>').appendTo(tr);
                        $.each(query.groups, function(g, group){
                            $('<span>').text(group.title).addClass('group')
                             .css({background: Raphael.hsb(group.hue, 0.8, 0.8)}).appendTo(td);
                        });
                    });
                    div.removeClass('loading');
                });
            }
        }
    ,    {    "title"            :    "PDF"
        ,    "description"    :    "Save this query on your computer in the PDF format"
        ,    "icon"            :    2
        ,    "action"        :    exportPDF
        }
    ,    {    "title"            :    "Feedback"
        ,    "description"    :    "How can we make this application better for you?"
        ,    "icon"            :    5
        ,    "action"        :    function(){
                var container = $('<div>');
                var label;
                var w;

                $('<p>').text('What kind of feedback do you want to give us?').appendTo(container);

                label = $('<label>').text(' Bug report').appendTo(container);
                $('<input type="checkbox">').attr('name', 'bug').prependTo(label);
                $('<br>').appendTo(container);

                label = $('<label>').text(' Feature request').appendTo(container);
                $('<input type="checkbox">').attr('name', 'feature').prependTo(label);
                $('<br>').appendTo(container);

                label = $('<label>').text(' Other: ').appendTo(container);
                $('<input type="checkbox">').attr('name', 'other').prependTo(label);
                $('<input type="text">').attr('name', 'subject').insertAfter(label);
                $('<br>').appendTo(container);

                $('<br>').appendTo(container);
                $('<textarea>').attr({name:'message', placeholder: 'Type your message here'}).appendTo(container).css({width:'100%', border:'solid 1px #CCC', height:'8em', resize:'none'});

                $('<br>').appendTo(container);
                $('<br>').appendTo(container);
                label = $('<label>').text('How would you rate this application?').appendTo(container);
                var select = $('<select>').attr('name', 'rating').appendTo(label);
                $('<option>').val(0).appendTo(select);
                $.each(['Awful', 'Bad', 'OK', 'Good', 'Excellent'], function(i, name){
                    $('<option>').val(i + 1).text((i+1) + ' - ' + name).appendTo(select);
                });
                $('<br>').appendTo(container);
                $('<br>').appendTo(container);

                $('<button>').text('Send').appendTo(container).click(function(){
                    var data =
                    {    types    :    []
                    ,    subject    :    container.find('[name=subject]').val()
                    ,    message    :    container.find('[name=message]').val()
                    ,    rating    :    container.find('[name=rating]').val()
                    };
                    container.find('input:checkbox:checked').each(function(){
                        data.types.push(this.name);
                    });
                    w.parent().remove();
                    $.mySend
                    ({    name    :    'feedback'
                    ,    data    :    data
                    });
                });

                w = $.window
                ({    title    :    'Feedback'
                ,    width    :    '50%'
                ,    html    :    container
                });
            }
        }
    ,    {    "title"            :    user ? "Account" : "Login"
        ,    "description"    :    user ? "Manage my account" : "Login using your email address and password"
        ,    "icon"            :    4
        ,    "action"        :    function(li){
                session.UI(function(){
                    var p = li.find('p');
                    if (session.data.u){
                        p.text('Account');
                        li.attr({title: 'Manage my account'});
                    } else{
                        p.text('Login');
                        li.attr({title: 'Login using your email address and password'});
                        queryNew();
                    }
                });
            }
        }
    ];
    var sidebarInit = function(){
        var ul = $('<ul>').addClass('sidebar').appendTo(queryContainer);
        $.each(sidebar, function(i, item){
            var li = $('<li>').attr({title: item.description}).appendTo(ul);
            li.click(function(){
                item.action(li);
            });
            var div = $('<div>').appendTo(li);
            $('<img>').attr({src: '/icons.png', style: 'top:-'+(3*item.icon)+'em'}).appendTo(div);
            $('<p>').text(item.title).appendTo(li);
        });
        sidebarLoginIcon = ul.find('li p:contains(Log)').parent();
    };

    //
    //    Query events
    //

    var queryInit = function(){
        var $window = $(window);
        titleInit();
        framesetInit();
        sidebarInit();
        var windowResize = function(){
                // General stuff
            var width = $window.width();
            var height = $window.height();
                // Font size
            var fontSize = (Math.sqrt(width * height) / 80);
            $('body').css({fontSize: fontSize + 'px'});
                // Query container
            queryContainer.width(
                width + queryContainer.width() - queryContainer.outerWidth(true)
            ).height(
                height + queryContainer.height() - queryContainer.outerHeight(true)
            );
                // Frameset
            queryFrameset.width(
                queryContainer.width() + queryFrameset.width() - queryFrameset.outerWidth(true)
            ).height(
                queryContainer.height() + queryFrameset.height() - queryFrameset.outerHeight(true) - queryTitle.outerHeight(true)
            ).framesetResize();
            return false;
        };
        windowResize();
        $window.resize(windowResize);
    };

    //
    //    Get it all done!
    //

    var user;
    var organ;

    requester.get('/users/me', function(result){
        user = result;
        queryInit();
        var organ_path = location.pathname.split('/')[2];
        $.getJSON('/organs/' + organ_path + '.json', function(result) {
            // prepare organ
            organ = result;
            defaultSettings.view.tags = {};
            $.each(organ.menu, function(i, item) {
                if (item.type == 'item') {
                    defaultSettings.view.tags[item.tag] = item.default;
                }
            });
            // load or create query
            if (location.hash){
                var query_id = location.hash.replace(/[^\d]+/g, '');
                queryLoad(query_id);
            } else {
                queryNew();
            }
        });
    });

})('#contents');
