var vows = require('vows'),
    assert = require('assert');

var future = require('../fork').future;

vows.describe('future').addBatch({
    'simple resturn 42': {
        topic: future (function () {
            return 42;
        }),

        'we get 42': function (topic) {
            assert.equal (topic.get (), 42);
        }
    }
}).export(module);
