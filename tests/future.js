var vows = require('vows'),
    assert = require('assert');

var future = require('../fork').future;
var sleep  = require('../fork').sleep;

vows.describe('future').addBatch({
    'simple return 42': {
        topic: future (function () {
            return 42;
        }),

        'we get 42': function (topic) {
            assert.equal (topic.get (), 42);
        }
    },

    'nested future return 42': {
        topic: future (function () {
            return future (function () {
                return 42;
            }).get ();
        }),

        'we get 42': function (topic) {
            assert.equal (topic.get (), 42);
        }

    },

    'disable multiple get': {
        topic: future (function () {
            return 42;
        }),

        'two gets should throw': function (topic) {
            assert.doesNotThrow (function () {
                topic.get ();
            });
            assert.throws (function () {
                topic.get ();
            },
            function (err) {
                return err == 'Future has already been got';
            });
        }
    },

    'result nothing': {
        topic: future (function () {

        }),

        'should get undefined': function (topic) {
            assert.equal (topic.get (), undefined);
        }
    },

    'throws in async function': {
        topic: future (function () {
            throw 42;
        }),

        'should get undefined': function (topic) {
            assert.equal (topic.get (), undefined);
        }
    }
}).export(module);
