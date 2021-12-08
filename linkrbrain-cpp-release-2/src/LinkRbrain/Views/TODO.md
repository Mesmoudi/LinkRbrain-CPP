# New API calls

## Datasets

```
GET /organs
GET /organs/ID
GET /organs/ID/datasets
GET /datasets
GET /datasets/ID
GET /datasets/ID/groups
```

## Queries

```
GET /queries
POST /queries

GET /queries/ID
PATCH /queries/ID
DELETE /queries/ID
```

# API calls found in JS code

## Presets

### Retrieve paginated list of presets

Can be filtered by `query`, a text value (space-separated list of keywords).

```js
$.mySend({
    name: 'presets',
    data: {
        time: (new Date()).getTime(),
        type: tabSelect.text().toLowerCase(),
        query: input.val(),
        page: page,
        htmlId: table.attr('id')
    },
    callback: displayCallback
});
```

```js
$.mySend({
    name: 'presets',
    data: {
        time: (new Date()).getTime(),
        type: tabSelect.text().toLowerCase(),
        query: input.val(),
        page: 0,
        htmlId: table.attr('id')
    },
    callback: displayCallback
});
```

## Queries

### Retrieve list of user queries

```js
$.mySend({
    name: 'queries',
    callback: function(response) {
        // ...
    }
});
```

### Wrapper

```js
var querySend = function(action, data, dialogTitle) {
    //    Entertain the user
    queryTitle.hide();
    queryFrameset.addClass('loading');
    var w = $.window
    ({    title    :    (dialogTitle ? dialogTitle : 'Updating query data...')
    ,    height    :    '25%'
    ,    width    :    '50%'
    ,    blocking:    true
    });
    //    Complete the data fields
    if (!data){
        data = {};
    }
    if (!data.queryId){
        data.queryId = queryData ? queryData.id : 0;
    }
    data.action = action;
    //    Send data to server
    $.mySend({
        name: 'query',
        data: data,
        callback: function(response){
            if (!response.id){
                queryNew();
            }
            queryData = response;
            location.hash = queryData.id;
            w.parent().remove();
            queryFrameset.removeClass('loading');
            queryTitle.show();
            queryUpdate();
        }
    });
}
```

### Load existing query

```js
var queryLoad = function(id){
    querySend('load', {queryId: id}, 'Loading query...');
};
```

### Create new query

```js
var queryNew = function(){
    querySend('new', {}, 'Creating new query...');
};
```

### Update query settings

```js
var querySettings = function(synchronous){
    $.mySend({
        name: 'query',
        synchronous: (synchronous === true),
        data: {
            queryId: queryData.id,
            action: 'settings',
            values: queryData.settings
        }
    });
};
```

### Delete query

```js
var queryDelete = function(queryId){
    $.mySend({
        name: 'query',
        data: {
            action: 'delete',
            queryId: queryId ? queryId : queryData.id
        },
        callback: function(response){
            queryNew();
        }
    });
};
```

## PDF generation

### Create PDF

```js
$.mySend({
    name: 'pdf',
    callback: pdfCreateCallback,
    data: {
        queryId: queryData.id,
        action: 'pdf',
        settings: exportPdfSettings
    }
});
```

### Send PNG data to server

```js
$.mySend({
    name: 'pdf',
    callback: snapshotViewUploadCallback,
    data: {
        queryId: queryData.id,
        action: 'snapshot',
        png: png,
        index: s
    }
});
```

### Send graph data to server

```js
$.mySend({
    name: 'pdf',
    callback: snapshotGraphCallback,
    data: {
        queryId: queryData.id,
        action: 'graph',
        svg: svg
    }
});
```

## User feedback

```js
$.mySend({
    name: 'feedback',
    data: data
});
```
